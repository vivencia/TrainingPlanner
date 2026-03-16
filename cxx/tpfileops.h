#pragma once

#include "tputils.h"

#include <QColor>
#include <QImage>
#include <QList>
#include <QObject>
#include <QQmlEngine>
#include <QQuickPaintedItem>
#include <QQuickTextDocument>
#include <QRect>
#include <QSize>

QT_FORWARD_DECLARE_CLASS(QGraphicsEffect)
QT_FORWARD_DECLARE_CLASS(QPainter)
QT_FORWARD_DECLARE_CLASS(QTextDocument)

class TPFileOps : public QQuickPaintedItem
{

Q_OBJECT
QML_ELEMENT
QML_VALUE_TYPE(FileOperations)

Q_PROPERTY(TPUtils::FILE_TYPE fileType READ fileType WRITE setFileType NOTIFY fileTypeChanged FINAL)
Q_PROPERTY(QString fileName READ fileName WRITE setFileName NOTIFY fileNameChanged FINAL)
Q_PROPERTY(int mesoIdx READ mesoIdx WRITE setMesoIdx NOTIFY mesoIdxChanged FINAL)
Q_PROPERTY(int tpFileSectionCount READ tpFileSectionCount NOTIFY tpFileSectionCountChanged FINAL)
Q_PROPERTY(int operationsCount READ operationsCount CONSTANT FINAL)
Q_PROPERTY(QStringList operationsList READ operationsList CONSTANT FINAL)

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
	void setFileName(const QString &filename);
	inline int mesoIdx() const { return m_mesoIdx; }
	inline void setMesoIdx(const int meso_idx) { m_mesoIdx = meso_idx; emit mesoIdxChanged(); }
	inline int tpFileSectionCount() const { return m_tpfileSections; }
	inline int operationsCount() const { return
#ifdef Q_OS_ANDROID
		4;
#else
		3;
#endif
	}
	inline QStringList operationsList() const
	{
		return QStringList{} << tr("Save as") <<
#ifdef Q_OS_ANDROID
			   tr("Share") <<
#endif
			   tr("Send to") << tr("Open");
	}

	Q_INVOKABLE void setEnabled(TPFileOps::OpType type, const bool enabled, const bool call_update = true);
	Q_INVOKABLE QString getFileTypeIcon(const QString &filename, const QSize &preferred_size = QSize{}, const bool thumbnail = true) const;
	Q_INVOKABLE void doFileOperation(const int op);
	Q_INVOKABLE void saveFileAs();
	Q_INVOKABLE void shareFile();
	Q_INVOKABLE void sendFileTo(QString userid = QString{});
	Q_INVOKABLE void openFile();
	Q_INVOKABLE inline QString tpFileSectionTitle(const int section) { return m_tpFileInfo.value(section).first; }
	Q_INVOKABLE inline QString tpFileSection(const int section) { return m_tpFileInfo.value(section).second; }
	Q_INVOKABLE inline void setWorkingTextDocument(QQuickTextDocument *text_doc) { m_textDocument = text_doc->textDocument(); }
	Q_INVOKABLE void setWorkingDocumentCursorPosition(const int cursor_position);

public slots:
	void exportSlot(const QString &filePath = QString{});
	void removeFileAnswer(const int button);

signals:
	void fileTypeChanged();
	void fileNameChanged();
	void showFullScreen();
	void multimediaKeyPressed(const int key);
	void multimediaKeyReleased(const int key);
	void fileRemovalRequested();
	void mesoIdxChanged();
	void tpFileSectionCountChanged();
	void setCursorPorsition(const int cursor_pos);
	void insertString(const QString &ch, const int pos);

protected:
	void mousePressEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	bool eventFilter(QObject *obj, QEvent *event) override;

private:

	struct controlInfo {
		OpType type;
		QImage default_image;
		QImage pressed_image;
		QImage *current_image{nullptr};
		bool pressed{false}, enabled{true};
		QRect rect;
	};

	controlInfo *m_controls[OT_TypeCount]{nullptr};
	controlInfo *m_currentControl{nullptr};
	QSize m_controlSize;
	QColor m_pressedColor;
	int8_t m_qml_control_spacing{5};
	int8_t m_qml_control_extra_height{10};
	TPUtils::FILE_TYPE m_filetype;
	QString m_filename;
	QHash<int,std::pair<QString,QString>> m_tpFileInfo;
	bool m_fullscreen{false};
	int m_mesoIdx{-1}, m_cursorPostion{-1};
	uint  m_tpfileSections{0};
	QTextDocument *m_textDocument{nullptr};

	void _doFileOperation(const OpType type);
	void doFullScreen();
	void removeFile(const bool bypass_confirmation = false);
	void createControls();
	void colorizeImage(QImage &image);
	void disableImage(QImage &image);
	controlInfo *controlFromMouseClick(const QPointF& mouse_pos) const;
	controlInfo *controlFromType(const OpType type) const;
	QString getImagePreviewFile(const QString &image_filename, QSize preferred_size = QSize{}) const;
	QString getPDFPreviewFile(const QString &pdf_filename, QSize preferred_size = QSize{}) const;
	void _setEnabled(controlInfo *ci, const bool enabled);
	void _getDefaultImage(controlInfo *ci);
	void readTPFile();
	void textDocumentKeyNavigation(const int key);

	Q_DISABLE_COPY(TPFileOps)
};
