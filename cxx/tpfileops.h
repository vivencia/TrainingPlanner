#pragma once

#include "tputils.h"
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
		OT_Delete,
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

	Q_INVOKABLE QString getFileTypeIcon(const QString &filename, const QSize &preferred_size = QSize{}, const bool thumbnail = true) const;
	QString getImagePreviewFile(const QString &image_filename, QSize preferred_size = QSize{}) const;
	QString getPDFPreviewFile(const QString &pdf_filename, QSize preferred_size = QSize{}) const;

public slots:
	void removeFileAnswer(const int button);

signals:
	void fileTypeChanged();
	void fileNameChanged();
	void showFullScreen();
	void multimediaKeyPressed(const int key);
	void multimediaKeyReleased(const int key);
	void fileRemovalRequested();

protected:
	void mousePressEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	bool eventFilter(QObject *obj, QEvent *event) override;

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
	TPBool m_fullscreen;

	void doFullScreen();
	void removeFile(const bool bypass_confirmation = false);
	void createControls();
	void colorizeImage(QImage &image);
	void disableImage(QImage &image);
	controlInfo *controlFromMouseClick(const QPointF& mouse_pos) const;
	controlInfo *controlFromType(const OpType type) const;
	void _setEnabled(controlInfo *ci, const bool enabled);
	void _getDefaultImage(controlInfo *ci);

	Q_DISABLE_COPY(TPFileOps)
};
