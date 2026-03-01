#include "tpfileops.h"
#include "dbusermodel.h"
#include "osinterface.h"
#include "qmlitemmanager.h"
#include "tpimage.h"
#include "tpsettings.h"

#include <QPainter>
#include <QQuickWindow>

TPFileOps::TPFileOps(QQuickItem *parent)
	: QQuickPaintedItem{parent}, m_currentControl{nullptr}, m_qml_control_spacing{5}, m_qml_control_extra_height{10}
{
	setAcceptTouchEvents(true);
	setAcceptedMouseButtons(Qt::LeftButton);
	connect(this, &TPFileOps::widthChanged, [this] { createControls(); });
	connect(this, &TPFileOps::heightChanged, [this] { createControls(); });
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

void TPFileOps::paint(QPainter *painter)
{
	if (painter->clipBoundingRect().width() == m_controlSize.width() && m_currentControl)
		painter->drawImage(painter->clipBoundingRect(), *(m_currentControl->current_image));
	else {
		for (const auto &ci : std::as_const(m_controls))
			painter->drawImage(ci->rect, *(ci->current_image));
	}
}

void TPFileOps::setEnabled(TPFileOps::OpType type, const bool enabled, const bool call_update)
{
	controlInfo *ci{controlFromType(type)};
	if (ci && ci->enabled != enabled) {
		_setEnabled(ci, enabled);
		if (call_update)
			update(ci->rect);
	}
}

void TPFileOps::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == acceptedMouseButtons()) {
		event->setAccepted(true);
		controlInfo* ci{controlFromMouseClick(event->position())};
		if (ci && ci->enabled) {
			m_currentControl = ci;
			if (!ci->pressed) {
				if (ci->pressed_image.isNull()) {
					ci->pressed_image = ci->default_image.copy();
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

void TPFileOps::mouseReleaseEvent(QMouseEvent *event)
{
	if (!m_currentControl)
		return;

	controlInfo* ci{controlFromMouseClick(event->position())};
	if (ci == m_currentControl) {
		switch (ci->type) {
			case OT_FullScreen:
				emit showFullScreen();
			break;
			case OT_Download:
				QMetaObject::invokeMethod(appMainWindow(), "chooseFolderToSave", Q_ARG(QString, appUtils()->getFileName(m_filename)));
			break;
			case OT_Share:
				appOsInterface()->viewExternalFile(m_filename);
			break;
			case OT_Forward:
			{//TODO
				QString userid;
				QMetaObject::invokeMethod(appMainWindow(), "getUsersList", Q_RETURN_ARG(QString, userid));
				appUserModel()->sendFileToUser(userid, m_filename);
			}
			break;
			case OT_ViewExternally: appUtils()->viewOrOpenFile(m_filename); break;
			default: return;
		}
		ci->current_image = &ci->default_image;
		ci->pressed = false;
		update(ci->rect);
	}
}

void TPFileOps::createControls()
{
	if (width() > 0 && height() > 0) {
		int new_height{static_cast<int>(height()) - m_qml_control_extra_height};
		int new_width{qFloor((width() - 5 - (static_cast<int>(OT_TypeCount) * m_qml_control_spacing)) / static_cast<int>(OT_TypeCount))};
		if (new_height < new_width) {
			new_width = new_height;
			m_qml_control_spacing = qFloor((width() - 5 - (static_cast<int>(OT_TypeCount) * new_width)) / static_cast<int>(OT_TypeCount));
		}
		else if (new_height > new_width) {
			new_height = new_width;
			m_qml_control_extra_height = height() - new_height;
		}
		if (new_height != m_controlSize.height() || new_width != m_controlSize.width()) {
			m_controlSize.rwidth() = new_width;
			m_controlSize.rheight() = new_height;
			int control_x{qCeil((width() - static_cast<int>(OT_TypeCount) * (m_qml_control_spacing + new_width))/2) + qCeil(m_qml_control_spacing / 2)};
			for (int i{OT_FullScreen}; i < OT_TypeCount; ++i) {
				controlInfo *ci{new controlInfo};
				ci->type = static_cast<OpType>(i);
				_getDefaultImage(ci);
				ci->default_image = std::move(ci->default_image.scaled(m_controlSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
				ci->current_image = &ci->default_image;
				ci->rect = QRect{control_x, m_qml_control_extra_height / 2, m_controlSize.width(), m_controlSize.height()};
				control_x += m_controlSize.width() + m_qml_control_spacing;
				m_controls[i] = ci;
			}
			update();
		}
	}
}

inline TPFileOps::controlInfo *TPFileOps::controlFromMouseClick(const QPointF& mouse_pos) const
{
	for (const auto ci : std::as_const(m_controls)) {
		if (static_cast<int>(mouse_pos.x() >= ci->rect.x()) && static_cast<int>(mouse_pos.x() <= ci->rect.x() + ci->rect.width()))
			return ci;
	}
	return nullptr;
}

TPFileOps::controlInfo *TPFileOps::controlFromType(const OpType type) const
{
	for (controlInfo *ci{m_controls[0]}; ci != nullptr; ++ci) {
		if (ci->type == type)
			return ci;
	};
	return nullptr;
}

void TPFileOps::_setEnabled(controlInfo *ci, const bool enabled)
{
	ci->enabled = enabled;
	if (enabled)
		_getDefaultImage(ci);
	else
		TPImage::grayScale(ci->default_image, ci->default_image);
	ci->current_image = &ci->default_image;
}

void TPFileOps::_getDefaultImage(controlInfo *ci)
{
	const QString &str_image_source{":/images/%1_"_L1 % appSettings()->indexColorSchemeToColorSchemeName() % ".png"};
	switch (ci->type) {
		case OT_FullScreen: ci->default_image.load(str_image_source.arg("fullscreen")); break;
		case OT_Download: ci->default_image.load(str_image_source.arg("download")); break;
		case OT_Share: ci->default_image.load(str_image_source.arg("share")); break;
		case OT_Forward: ci->default_image.load(str_image_source.arg("forward")); break;
		case OT_ViewExternally: ci->default_image.load(":/images/" % appUtils()->getFileTypeIcon(m_filename, m_controlSize, false)); break;
		default: break;
	}
}
