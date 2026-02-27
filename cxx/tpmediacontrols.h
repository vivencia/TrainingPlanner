#pragma once

#include "tpbool.h"

#include <QColor>
#include <QImage>
#include <QList>
#include <QObject>
#include <QQmlEngine>
#include <QQuickPaintedItem>
#include <QRect>
#include <QSize>

QT_FORWARD_DECLARE_CLASS(QGraphicsEffect)
QT_FORWARD_DECLARE_CLASS(QPainter)

class TPMediaControls : public QQuickPaintedItem
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(QList<int> availableControls READ availableControls WRITE setAvailableControls NOTIFY availableControlsChanged FINAL)

public:

enum ControlType {
	CT_Stop,
	CT_Play,
	CT_Pause,
	CT_Prev,
	CT_Next,
	CT_Equalizer,
	CT_Rewind,
	CT_FastForward,
	CT_Mute
};
Q_ENUM(ControlType)

	explicit TPMediaControls(QQuickItem *parent = nullptr);
	void paint(QPainter *painter) override;

	inline QList<int> availableControls() const { return m_types; }
	inline void setAvailableControls(const QList<int> &types_list)
	{
		m_types.reserve(types_list.count());
		for (const auto type : types_list)
			m_types.append(type);
		emit availableControlsChanged();
		createControls();
	}

	Q_INVOKABLE void setEnabled(TPMediaControls::ControlType type, const bool enabled);

signals:
	void availableControlsChanged();
	void controlSizeChanged();
	void controlClicked(TPMediaControls::ControlType type);
	void controlPressed(TPMediaControls::ControlType type);
	void controlReleased(TPMediaControls::ControlType type);

protected:
	void mousePressEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;

private:

	struct controlInfo {
		ControlType type;
		QImage default_image;
		QImage pressed_image;
		QImage *current_image;
		TPBool pressed, enabled;
		QRect rect;
	};
	QList<controlInfo*> m_controls;
	controlInfo *m_currentControl;
	QList<int> m_types;
	QSize m_controlSize;
	QColor m_pressedColor;
	int8_t m_qml_control_spacing;
	int8_t m_qml_control_extra_height;
	static QImage img_all_controls;

	QImage getControlImageFromSource(ControlType type);
	void createControls();
	void colorizeImage(QImage &image);
	void disableImage(QImage &image);
	controlInfo *controlFromMouseClick(const QPointF& mouse_pos) const;
	controlInfo *controlFromType(const ControlType type) const;
};

