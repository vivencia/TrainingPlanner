#include "tpmediacontrols.h"
#include "tpimage.h"
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

void TPMediaControls::setEnabled(TPMediaControls::ControlType type, const bool enabled, const bool call_update)
{
	controlInfo *ci{controlFromType(type)};
	if (ci && ci->enabled != enabled) {
		_setEnabled(ci, enabled);
		if (call_update)
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
					else
						ci->pressed_image = std::move(getControlImageFromSource(CT_Pause));
					TPImage::colorizeImage(ci->pressed_image, m_pressedColor);
				}
				ci->current_image = &ci->pressed_image;
				ci->pressed = true;
			}
			else {
				ci->current_image = &ci->default_image;
				ci->pressed = false;
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
				_controlClicked(ci);
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
				ci->rect = QRect{control_x, m_qml_control_extra_height / 2, m_controlSize.width(), m_controlSize.height()};
				_setEnabled(ci, ci->type == CT_Play);
				control_x += m_controlSize.width() + m_qml_control_spacing;
				m_controls.append(ci);
			}
			update();
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

void TPMediaControls::_controlClicked(controlInfo *ci)
{
	bool revert_play{false};
	switch (ci->type) {
		case CT_Stop:
			setEnabled(CT_Equalizer, false);
			setEnabled(CT_Pause, false);
			setEnabled(CT_Rewind, false);
			setEnabled(CT_FastForward, false);
			setEnabled(CT_Stop, false);
			revert_play = true;
		break;
		case CT_Play:
			setEnabled(CT_Equalizer, true);
			setEnabled(CT_Stop, true);
			setEnabled(CT_Rewind, true);
			setEnabled(CT_FastForward, false);
		break;
		case CT_Pause:
			setEnabled(CT_Stop, false);
			revert_play = true;
			break;
		case CT_Prev:
			setEnabled(CT_Next, true);
		break;
		case CT_Next:
			setEnabled(CT_Prev, true);
		break;
		default:
			return;
	}
	if (revert_play) {
		controlInfo *ci_play{controlFromType(CT_Play)};
		ci_play->current_image = &ci_play->default_image;
		ci_play->pressed = false;
		ci_play->enabled = true;
		update(ci_play->rect);
	}
}

void TPMediaControls::_setEnabled(controlInfo *ci, const bool enabled)
{
	ci->enabled = enabled;
	if (enabled)
		ci->default_image = std::move(getControlImageFromSource(ci->type));
	else
		TPImage::grayScale(ci->default_image, ci->default_image);
	ci->current_image = &ci->default_image;
}
