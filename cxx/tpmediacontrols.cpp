#include "tpmediacontrols.h"
#include "tpsettings.h"

#include <QPainter>

constexpr QLatin1StringView all_controls_source{":/images/media_controls.png"};
constexpr int16_t src_control_image_width{323};
constexpr int16_t src_control_image_height{323};

QImage TPMediaControls::img_all_controls{};
TPMediaControls::TPMediaControls(QQuickItem *parent)
	: QQuickPaintedItem{parent}, m_currentControl{nullptr}, m_qml_control_spacing{5}, m_qml_control_extra_height{10}
{
	if (img_all_controls.isNull())
		img_all_controls.load(all_controls_source);
	setAcceptTouchEvents(true);
	setAcceptedMouseButtons(Qt::LeftButton);
	connect(this, &TPMediaControls::widthChanged, [this] { createControls(); });
	connect(this, &TPMediaControls::heightChanged, [this] { createControls(); });
	m_pressedColor.fromString(appSettings()->primaryColor());
	switch (appSettings()->colorScheme()) {
		case TPSettings::Blue:
			m_pressedColor.setRgb(0, 0, m_pressedColor.blue());
		break;
		case TPSettings::Green:
			m_pressedColor.setRgb(0, m_pressedColor.green(), 0);
		break;
		case TPSettings::Red:
			m_pressedColor.setRgb(m_pressedColor.red(), 0, 0);
		break;
	}
	m_pressedColor.setAlpha(100);
}

void TPMediaControls::paint(QPainter *painter)
{
	if (painter->clipBoundingRect().width() == m_controlSize.width() && m_currentControl)
		painter->drawImage(painter->clipBoundingRect(), *(m_currentControl->current_image));
	else {
		for (const auto &ci : std::as_const(m_controls))
			painter->drawImage(ci->rect, *(ci->current_image));
	}
}

void TPMediaControls::setEnabled(TPMediaControls::ControlType type, const bool enabled)
{
	controlInfo *ci{controlFromType(type)};
	if (ci && ci->enabled != enabled) {
		ci->enabled = enabled;
		if (enabled)
			ci->default_image = std::move(getControlImageFromSource(type));
		else
			disableImage(ci->default_image);
		update(ci->rect);
	}
}

void TPMediaControls::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == acceptedMouseButtons()) {
		event->setAccepted(true);
		controlInfo* ci{controlFromMouseClick(event->position())};
		if (ci && ci->enabled) {
			emit controlPressed(ci->type);
			m_currentControl = ci;
			if (!ci->pressed) {
				if (ci->pressed_image.isNull()) {
					if (ci->type != CT_Play)
						ci->pressed_image = ci->default_image.copy();
					else {
						ci->pressed_image = std::move(getControlImageFromSource(CT_Pause));
						setEnabled(CT_Equalizer, false);
					}
					colorizeImage(ci->pressed_image);
				}
				ci->current_image = &ci->pressed_image;
				ci->pressed = true;
			}
			else {
				ci->current_image = &ci->default_image;
				ci->pressed = false;
				setEnabled(CT_Equalizer, true);
			}
			update(ci->rect);
		}
	}
}

void TPMediaControls::mouseReleaseEvent(QMouseEvent *event)
{
	if (!m_currentControl)
		return;

	controlInfo* ci{controlFromMouseClick(event->position())};
	if (ci == m_currentControl) {
		switch (ci->type) {
			case CT_Stop:
			case CT_Play:
			case CT_Pause:
			case CT_Equalizer:
			case CT_Mute:
				emit controlClicked(ci->type);
			break;
			case CT_Prev:
			case CT_Next:
				emit controlClicked(ci->type);
				ci->current_image = &ci->default_image;
				ci->pressed = false;
				update(ci->rect);
			break;
			case CT_Rewind:
			case CT_FastForward:
				emit controlReleased(ci->type);
				ci->current_image = &ci->default_image;
				ci->pressed = false;
				update(ci->rect);
			break;
		}
	}
}

QImage TPMediaControls::getControlImageFromSource(ControlType type)
{
	const int x{(type % 3) * src_control_image_width};
	const int y{(type <= 2 ? 0 : type <= 5 ? 1 : 2) * src_control_image_width};
	return std::move(img_all_controls.copy(QRect{x, y, src_control_image_width, src_control_image_height}));
}

void TPMediaControls::createControls()
{
	if (width() > 0 && height() > 0 && !m_types.isEmpty()) {
		int new_height{static_cast<int>(height()) - m_qml_control_extra_height};
		int new_width{qFloor((width() - 5 - (m_types.size() * m_qml_control_spacing)) / m_types.size())};
		if (new_height < new_width) {
			new_width = new_height;
			m_qml_control_spacing = qFloor((width() - 5 - (m_types.size() * new_width)) / m_types.size());
		}
		else if (new_height > new_width) {
			new_height = new_width;
			m_qml_control_extra_height = height() - new_height;
		}
		if (new_height != m_controlSize.height() || new_width != m_controlSize.width()) {
			m_controlSize.rwidth() = new_width;
			m_controlSize.rheight() = new_height;
			qDeleteAll(m_controls);
			m_controls.reserve(m_types.size());
			int control_x{qCeil((width() - m_types.size() * (m_qml_control_spacing + new_width))/2) + qCeil(m_qml_control_spacing / 2)};
			for (const auto type : std::as_const(m_types)) {
				controlInfo *ci{new controlInfo};
				ci->type = static_cast<ControlType>(type);
				ci->default_image = std::move(getControlImageFromSource(static_cast<ControlType>(type)));
				ci->default_image = std::move(ci->default_image.scaled(m_controlSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
				ci->current_image = &ci->default_image;
				ci->enabled = true;
				ci->rect = QRect{control_x, m_qml_control_extra_height / 2, m_controlSize.width(), m_controlSize.height()};
				control_x += m_controlSize.width() + m_qml_control_spacing;
				m_controls.append(ci);
			}
			update();
		}
	}
}

void TPMediaControls::colorizeImage(QImage &image)
{
	QPainter painter(&image);
	painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
	painter.fillRect(image.rect(), m_pressedColor);
	painter.end();
	image.save("/DATA/img.jpg", "jpg");
}

void TPMediaControls::disableImage(QImage &image)
{
	image = std::move(image.convertToFormat(image.hasAlphaChannel() ? QImage::Format_ARGB32 : QImage::Format_RGB32));
	const int imgHeight{image.height()};
	const int imgWidth{image.width()};
	QRgb pixel;
	for (uint y{0}; y < imgHeight; ++y) {
		QRgb *scanLine{reinterpret_cast<QRgb*>(image.scanLine(y))};
		for (uint x{0}; x < imgWidth; ++x) {
			pixel = *scanLine;
			const uint ci{static_cast<uint>(qGray(pixel))};
			*scanLine = qRgba(ci, ci, ci, qAlpha(pixel)/3);
			++scanLine;
		}
	}
}

inline TPMediaControls::controlInfo *TPMediaControls::controlFromMouseClick(const QPointF& mouse_pos) const
{
	for (const auto ci : std::as_const(m_controls)) {
		if (static_cast<int>(mouse_pos.x() >= ci->rect.x()) && static_cast<int>(mouse_pos.x() <= ci->rect.x() + ci->rect.width()))
			return ci;
	}
	return nullptr;
}

TPMediaControls::controlInfo *TPMediaControls::controlFromType(const ControlType type) const
{
	auto ci_itr{std::find_if(m_controls.cbegin(), m_controls.cend(), [type] (const auto ci) {
		return ci->type == type;
	})};
	return ci_itr != m_controls.cend() ? *ci_itr : nullptr;
}
