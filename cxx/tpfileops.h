#pragma once

#include "tputils.h"

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

class TPFileOps : public QQuickPaintedItem
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(TPUtils::FILE_TYPE fileType READ fileType WRITE setFileType NOTIFY fileTypeChanged FINAL)
Q_PROPERTY(QString fileName READ fileName WRITE setFileName NOTIFY fileNameChanged FINAL)

public:

	enum OpType {
		OT_FullScreen,
		OT_Download,
		OT_Share,
		OT_Forward,
		OT_ViewExternally,
		OT_TypeCount
	};
	Q_ENUM(OpType)

	explicit TPFileOps(QQuickItem *parent = nullptr);
	void paint(QPainter *painter) override;

	inline TPUtils::FILE_TYPE fileType() const { return m_filetype; }
	inline void setFileType(TPUtils::FILE_TYPE new_type) { m_filetype = new_type; emit fileTypeChanged(); }
	inline QString fileName() const { return m_filename; }
	inline void setFileName(const QString &filename) { m_filename = filename; emit fileNameChanged(); }
	Q_INVOKABLE void setEnabled(TPFileOps::OpType type, const bool enabled, const bool call_update = true);

signals:
	void fileTypeChanged();
	void fileNameChanged();
	void showFullScreen();

protected:
	void mousePressEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;

private:

	struct controlInfo {
		OpType type;
		QImage default_image;
		QImage pressed_image;
		QImage *current_image;
		bool pressed, enabled;
		QRect rect;

		inline controlInfo() : current_image{nullptr}, pressed{false}, enabled{true} {}
	};

	controlInfo *m_controls[OT_TypeCount];
	controlInfo *m_currentControl;
	QSize m_controlSize;
	QColor m_pressedColor;
	int8_t m_qml_control_spacing;
	int8_t m_qml_control_extra_height;
	TPUtils::FILE_TYPE m_filetype;
	QString m_filename;

	void createControls();
	void colorizeImage(QImage &image);
	void disableImage(QImage &image);
	controlInfo *controlFromMouseClick(const QPointF& mouse_pos) const;
	controlInfo *controlFromType(const OpType type) const;
	void _setEnabled(controlInfo *ci, const bool enabled);
	void _getDefaultImage(controlInfo *ci);
};
