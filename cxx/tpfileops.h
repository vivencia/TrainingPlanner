#pragma once

#include "tpfilepath.h"
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

QT_FORWARD_DECLARE_CLASS(QFileDialog)
QT_FORWARD_DECLARE_CLASS(QGraphicsEffect)
QT_FORWARD_DECLARE_CLASS(QPainter)
QT_FORWARD_DECLARE_CLASS(QQmlComponent)
QT_FORWARD_DECLARE_CLASS(QQuickItem)
QT_FORWARD_DECLARE_CLASS(QTextDocument)

class TPFileOps : public QQuickPaintedItem
{

Q_OBJECT
QML_ELEMENT
QML_VALUE_TYPE(FileOperations)

Q_PROPERTY(TPUtils::FILE_TYPE fileType READ fileType WRITE setFileType NOTIFY fileTypeChanged FINAL)
Q_PROPERTY(QString fileName READ fileName WRITE setFileName NOTIFY fileNameChanged FINAL)
Q_PROPERTY(QUrl fileURL READ fileURL WRITE setFileURL NOTIFY fileNameChanged FINAL)
Q_PROPERTY(QSize controlSize READ controlSize WRITE setControlSize NOTIFY controlSizeChanged FINAL)
Q_PROPERTY(QQuickItem* parentPage READ parentPage WRITE setParentPage NOTIFY parentPageChanged FINAL)
Q_PROPERTY(int mesoIdx READ mesoIdx WRITE setMesoIdx NOTIFY mesoIdxChanged FINAL)
Q_PROPERTY(int workoutCalendarDay READ workoutCalendarDay WRITE setWorkoutCalendarDay NOTIFY workoutCalendarDayChanged FINAL)
Q_PROPERTY(int tpFileSectionCount READ tpFileSectionCount NOTIFY tpFileSectionCountChanged FINAL)
Q_PROPERTY(bool canDownloadOrGenerate READ canDownloadOrGenerate WRITE setCanDownloadOrGenerate NOTIFY canDownloadOrGenerateChanged FINAL)
Q_PROPERTY(bool canAddFile READ canAddFile WRITE setCanAddFile NOTIFY canAddFileChanged FINAL)
Q_PROPERTY(bool restrictedFileType READ restrictedFileType WRITE setRestrictedFileType NOTIFY restrictedFileTypeChanged FINAL)
Q_PROPERTY(bool useControls READ useControls WRITE setUseControls NOTIFY useControlsChanged FINAL)
Q_PROPERTY(bool fileIsOK READ fileIsOK NOTIFY fileIsOKChanged FINAL)

public:

	enum OpType {
		OT_AddFile,
		OT_FullScreen,
		OT_Download,
		OT_Share,
		OT_Forward,
		OT_ViewExternally,
		OT_Delete,
		OT_TypeCount,
		OT_Custom_1,
		OT_Custom_2,
		OT_Custom_X,
	};
	Q_ENUM(OpType)

	explicit TPFileOps(QQuickItem *parent = nullptr);
	void paint(QPainter *painter) override;

	inline TPUtils::FILE_TYPE fileType() const { return m_filetype; }
	void setFileType(TPUtils::FILE_TYPE new_type);
	QString fileName() const { return m_filename.toString(); }
	inline const TPFilePath &tpFileName() const { return m_filename; }
	void setFileName(const QString &filename, const bool file_added = false);
	//TODO: Android URLs
	inline QUrl fileURL() const { return QString{"file://"_L1 % m_filename.toString()}; }
	void setFileURL(const QUrl &url);
	inline QSize controlSize() const { return m_controlSize; }
	inline void setControlSize(const QSize &new_size)
	{
		m_controlSize = new_size;
		setWidth(new_size.width());
		setHeight(new_size.height());
		emit controlSizeChanged();
	}
	inline QQuickItem *parentPage() const { return m_parentPage; }
	inline void setParentPage(QQuickItem *page) { m_parentPage = page; emit parentPageChanged(); }
	inline int mesoIdx() const { return m_mesoIdx; }
	inline void setMesoIdx(const int meso_idx) { m_mesoIdx = meso_idx; emit mesoIdxChanged(); }
	inline int workoutCalendarDay() const { return m_workoutCalendarDay; }
	inline void setWorkoutCalendarDay(const int workout_id) { m_workoutCalendarDay = workout_id; emit workoutCalendarDayChanged(); }
	inline int tpFileSectionCount() const { return m_tpfileSections; }
	inline bool canDownloadOrGenerate() const { return m_downloadOrGenerate; }
	void setCanDownloadOrGenerate(const bool can_do);
	inline bool canAddFile() const { return m_canAddFile; }
	void setCanAddFile(const bool can_add);
	inline bool restrictedFileType() const { return m_restrictedFileType; }
	inline void setRestrictedFileType(const bool restricted) { m_restrictedFileType = restricted; emit restrictedFileTypeChanged(); }
	inline bool useControls() const { return m_useControls; }
	inline void setUseControls(const  bool use_controls) {
		if (use_controls != m_useControls) {
			if (use_controls && !m_controls[OT_FullScreen])
				createControls();
			else if (!use_controls && m_controls[OT_FullScreen])
				clearControls();
			m_useControls = use_controls;
			emit useControlsChanged();
		}
	}
	inline bool fileIsOK() const { return m_fileIsOK; }
	inline void setFileIsOK(const bool ok)
	{
		if (m_fileIsOK != ok) {
			m_fileIsOK = ok;
			emit fileIsOKChanged();
		}
	}

	void removeFile(const bool bypass_confirmation, const bool remove_local, const bool remove_remote);
	static QString chooseFileDialog(const int file_type = TPUtils::FT_ANY_TYPE);
	Q_INVOKABLE QString openFileDialog(const int file_type, const QString &suggested_save_name = QString{});
	Q_INVOKABLE void attemptToCreateOrGetFile();
	Q_INVOKABLE void setEnabled(TPFileOps::OpType type, const bool enabled, const bool call_update = true);
	Q_INVOKABLE QString getFileTypeIcon(const QString &filename, const QSize &preferred_size = QSize{}, const bool thumbnail = true) const;
	Q_INVOKABLE inline void doFileOperation(const int op) { _doFileOperation(static_cast<OpType>(op)); }
	Q_INVOKABLE inline QString tpFileSectionTitle(const int section) { return m_tpFileInfo.value(section).first; }
	Q_INVOKABLE inline QString tpFileSection(const int section) { return m_tpFileInfo.value(section).second; }
	Q_INVOKABLE inline void setWorkingTextDocument(QQuickTextDocument *text_doc) { m_textDocument = text_doc->textDocument(); }
	Q_INVOKABLE void setWorkingDocumentCursorPosition(const int cursor_position);
	Q_INVOKABLE QString getFileText(const bool preview_text) const;
	Q_INVOKABLE inline void repaintControls() { update(); }

public slots:
	void exportSlot(const TPFilePath &tp_filename);
	void importSlot(const bool accepted);
	void sendFileTo(const int handle = 1, const QStringList &userids = QStringList{}, const QString &message = QString{});

signals:
	void fileTypeChanged();
	void fileNameChanged();
	void showFullScreen();
	void multimediaKeyPressed(const int key);
	void multimediaKeyReleased(const int key);
	void fileAdded(const QString &filepath);
	void fileAcquired(const int ret_code);
	void fileRemovalRequested();
	void mesoIdxChanged();
	void controlSizeChanged();
	void parentPageChanged();
	void workoutCalendarDayChanged();
	void tpFileSectionCountChanged();
	void canDownloadOrGenerateChanged();
	void canAddFileChanged();
	void restrictedFileTypeChanged();
	void useControlsChanged();
	void fileIsOKChanged();
	void setCursorPorsition(const int cursor_pos);
	void insertString(const QString &ch, const int pos);
	void _internalSignal(const int requestid, const int return_code);

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
		bool visible{true}, pressed{false}, enabled{true};
		QRect rect;
	};

	controlInfo *m_controls[OT_TypeCount]{nullptr};
	controlInfo *m_currentControl{nullptr};
	QSize m_controlSize, m_buttonSize;
	QColor m_pressedColor;
	TPUtils::FILE_TYPE m_filetype;
	QList<std::pair<QString,QString>> m_tpFileInfo;
	bool m_fullscreen{false}, m_canAddFile{false}, m_downloadOrGenerate{false}, m_restrictedFileType{false},
																			m_fileIsOK{false}, m_useControls{false};
	int m_mesoIdx{-1}, m_workoutCalendarDay{-1}, m_cursorPostion{-1};
	uint  m_tpfileSections{0};
	QTextDocument *m_textDocument{nullptr};
	TPFilePath m_filename;
	QFileDialog *m_fileDialog{nullptr};
	QQmlComponent *m_sendFileDialogComponent{nullptr};
	QObject *m_sendFileDialog{nullptr};
	QQuickItem *m_parentPage{nullptr};

	void _doFileOperation(const OpType type);
	int generateFileFromType(const bool formatted);
	void doFullScreen();
	void addFile();
	void saveFileAs();
	void shareFile();
	void downloadOrCopyFile();
	void sendFileToUsers(const QStringList &users, const QString &message);
	void openFile();
	void createControls();
	void clearControls();
	void resizeControl();
	void recalculateButtonsRect();
	void colorizeImage(QImage &image);
	void disableImage(QImage &image);
	controlInfo *controlFromMouseClick(const QPointF& mouse_pos) const;
	controlInfo *controlFromType(const OpType type) const;
	QString getImagePreviewFile(const QString &image_filename, QSize preferred_size = QSize{}) const;
	QString getPDFPreviewFile(const QString &pdf_filename, QSize preferred_size = QSize{}) const;
	QString getSubDir() const;
	void _setEnabled(controlInfo *ci, const bool enabled);
	void _getDefaultImage(controlInfo *ci);
	void readTPFile();
	void openTPFile();
	void textDocumentKeyNavigation(const int key);
	void createSendFileDialog(const int handle, const QStringList &userids, const QString &message);

	Q_DISABLE_COPY(TPFileOps)
};
