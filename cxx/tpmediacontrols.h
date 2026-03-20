#pragma once

#include "tpfileops.h"

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
QML_VALUE_TYPE(MediaControls)

Q_PROPERTY(QList<int> availableControls READ availableControls WRITE setAvailableControls NOTIFY availableControlsChanged FINAL)
Q_PROPERTY(TPFileOps* fileOps READ fileOps WRITE setFileOps NOTIFY fileOpsChanged FINAL)

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
		CT_VolumeUp,
		CT_VolumeDown,
		CT_Mute,
	};
	Q_ENUM(ControlType)

	explicit TPMediaControls(QQuickItem *parent = nullptr);
	void paint(QPainter *painter) override;

	inline QList<int> availableControls() const { return m_types; }
	inline void setAvailableControls(const QList<int> &types_list)
	{
		m_types.clear();
		m_types.reserve(types_list.count());
		for (const auto type : types_list)
			m_types.append(type);
		emit availableControlsChanged();
		createControls();
	}

	inline TPFileOps *fileOps() const { return m_fileOps; }
	void setFileOps(TPFileOps *fileops);
	Q_INVOKABLE void setEnabled(TPMediaControls::ControlType type, const bool enabled);
	Q_INVOKABLE void controlReachedLimit(TPMediaControls::ControlType type);
	Q_INVOKABLE inline void emulateControlClick(TPMediaControls::ControlType type)
	{
		controlInfo *ci{controlFromType(type)};
		pressEvent(ci);
		releaseEvent(ci);
	}

signals:
	void availableControlsChanged();
	void controlClicked(TPMediaControls::ControlType type);
	void controlPressed(TPMediaControls::ControlType type);
	void controlReleased(TPMediaControls::ControlType type);
	void fileOpsChanged();

protected:
	void mousePressEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;

private:

	struct controlInfo {
		ControlType type;
		QImage default_image;
		QImage pressed_image;
		QImage *current_image;
		bool pressed, enabled;
		QRect rect;

		inline controlInfo() : current_image{nullptr}, pressed{false} {}
	};

	QList<controlInfo*> m_controls;
	controlInfo *m_currentControl{nullptr};
	QList<int> m_types;
	QSize m_controlSize;
	QColor m_pressedColor;
	int8_t m_qml_control_spacing{5};
	int8_t m_qml_control_extra_height{10};
	TPFileOps *m_fileOps{nullptr};
	static QImage img_all_controls;

	void pressEvent(controlInfo *ci);
	void releaseEvent(controlInfo *ci);
	QImage getControlImageFromSource(ControlType type);
	void createControls();
	controlInfo *controlFromMouseClick(const QPointF& mouse_pos) const;
	controlInfo *controlFromType(const ControlType type) const;
	controlInfo *controlFromKey(const int key) const;
	void _controlClicked(controlInfo *ci);
	void _setEnabled(controlInfo *ci, const bool enabled);
};
