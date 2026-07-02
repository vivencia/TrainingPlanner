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
Q_PROPERTY(QString subdirForAddedFile READ subdirForAddedFile WRITE setSubdirForAddedFile NOTIFY subdirForAddedFileChanged FINAL)
Q_PROPERTY(QUrl fileURL READ fileURL WRITE setFileURL NOTIFY fileNameChanged FINAL)
Q_PROPERTY(QSize controlSize READ controlSize WRITE setControlSize NOTIFY controlSizeChanged FINAL)
Q_PROPERTY(QQuickItem* parentPage READ parentPage WRITE setParentPage NOTIFY parentPageChanged FINAL)
Q_PROPERTY(int mesoIdx READ mesoIdx WRITE setMesoIdx NOTIFY mesoIdxChanged FINAL)
Q_PROPERTY(int workoutCalendarDay READ workoutCalendarDay WRITE setWorkoutCalendarDay NOTIFY workoutCalendarDayChanged FINAL)
Q_PROPERTY(int tpFileSectionCount READ tpFileSectionCount NOTIFY tpFileSectionCountChanged FINAL)
Q_PROPERTY(int addFileFilters READ addFileFilters WRITE setAddFileFilters NOTIFY addFileFiltersChanged FINAL)
Q_PROPERTY(bool canDownloadOrGenerate READ canDownloadOrGenerate WRITE setCanDownloadOrGenerate NOTIFY canDownloadOrGenerateChanged FINAL)
Q_PROPERTY(bool canAddFile READ canAddFile WRITE setCanAddFile NOTIFY canAddFileChanged FINAL)
Q_PROPERTY(bool restrictedFileType READ restrictedFileType WRITE setRestrictedFileType NOTIFY restrictedFileTypeChanged FINAL)
Q_PROPERTY(bool useControls READ useControls WRITE setUseControls NOTIFY useControlsChanged FINAL)
Q_PROPERTY(bool fileIsOK READ fileIsOK NOTIFY fileIsOKChanged FINAL)
Q_PROPERTY(bool isTPFile READ isTPFile NOTIFY fileTypeChanged FINAL)
Q_PROPERTY(bool isMediaFile READ isMediaFile NOTIFY fileTypeChanged FINAL)
Q_PROPERTY(bool isKnownFile READ isKnownFile NOTIFY fileTypeChanged FINAL)
Q_PROPERTY(bool isDocumentFile READ isDocumentFile NOTIFY fileTypeChanged FINAL)

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

	explicit TPFileOps(QQuickItem *visual_parent = nullptr);
	~TPFileOps() { qDebug() << "~TPFileOps()" + m_filename.fileName(); }
	void paint(QPainter *painter) override;

	inline TPUtils::FILE_TYPE fileType() const { return m_filetype; }
	void setFileType(TPUtils::FILE_TYPE new_type);
	inline QString fileName() const { return m_filename.toString(); }
	inline const TPFilePath &tpFileName() const { return m_filename; }
	void setFileName(const QString &filename, const bool file_added = false);
	void setFileName(TPFilePath &&tp_filename);
	const QString &subdirForAddedFile() const { return m_subdir; }
	//subdir here is TPFilePath::targetUser + / + TPFilePath::subdir. addFile() will split subdir into those two parts
	void setSubdirForAddedFile(const QString &subdir) { m_subdir = subdir; emit subdirForAddedFileChanged(); }
	//TODO: Android URLs
	inline QUrl fileURL() const { return QString{"file://"_L1 % m_filename.toString()}; }
	void setFileURL(const QUrl &url);
	inline void setSuggestedFileNameGenerator(const std::function<TPFilePathPtr(const QString&)> &func)
	{
		m_suggestNameFunc = func;
	}

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
	inline int addFileFilters() const { return m_addFileFilters; }
	inline void setAddFileFilters(const int filters ) { m_addFileFilters = filters; emit addFileFiltersChanged(); }
	inline bool canDownloadOrGenerate() const { return m_downloadOrGenerate; }
	void setCanDownloadOrGenerate(const bool can_do);
	inline bool canAddFile() const { return m_canAddFile; }
	void setCanAddFile(const bool can_add);
	inline bool restrictedFileType() const { return m_restrictedFileType; }
	inline void setRestrictedFileType(const bool restricted) { m_restrictedFileType = restricted; emit restrictedFileTypeChanged(); }
	inline bool useControls() const { return m_useControls; }
	inline void setUseControls(const  bool use_controls) {
		if (use_controls != m_useControls) {
			if (m_filetype != TPUtils::FT_NO_TYPE_SET) {
				if (use_controls && !m_controls[OT_FullScreen])
					createControls();
				else if (!use_controls && m_controls[OT_FullScreen])
					clearControls();
			}
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

	inline bool isTPFile() const { return m_filetype & TPUtils::FT_ANY_TPFILE; }
	inline bool isMediaFile() const { return m_filetype & TPUtils::FT_MEDIA; }
	inline bool isKnownFile() const { return m_filetype > TPUtils::FT_UNKNOWN && m_filetype < TPUtils::FT_OTHER; }
	inline bool isDocumentFile() const { return m_filetype & TPUtils::FT_DOCUMENTS; }
	inline bool isViewableFile() const { return m_filetype & TPUtils::FT_VIEWBLE_FILE; }
	inline bool isOpenedExternally() const { return m_filetype & TPUtils::FT_VIEWABLE_OUTSIDE || m_filetype == TPUtils::FT_OTHER; }

	//Only changes the filename(both internally and of the actual file if it exists). It does not change paths or moves the file
	void renameFile(const QString &new_name);
	void removeFile(const bool bypass_confirmation, const bool remove_local, const bool remove_remote);
	void exportTPFile(const TPFilePath &tp_filename);
	Q_INVOKABLE QString openFileDialog(const int file_type, const QString &suggested_save_name = QString{});
	Q_INVOKABLE void attemptToCreateOrGetFile();
	Q_INVOKABLE void setEnabled(TPFileOps::OpType type, const bool enabled, const bool call_update = true);
	Q_INVOKABLE QString getFileTypeIcon(const QSize &preferred_size = QSize{}, const bool thumbnail = true) const;
	Q_INVOKABLE inline void doFileOperation(const int op) { _doFileOperation(static_cast<OpType>(op)); }
	Q_INVOKABLE inline QString tpFileSectionTitle(const int section) { return m_tpFileInfo.value(section).first; }
	Q_INVOKABLE inline QString tpFileSection(const int section) { return m_tpFileInfo.value(section).second; }
	Q_INVOKABLE inline void setWorkingTextDocument(QQuickTextDocument *text_doc) { m_textDocument = text_doc->textDocument(); }
	Q_INVOKABLE void setWorkingDocumentCursorPosition(const int cursor_position);
	Q_INVOKABLE QString getFileText(const bool preview_text) const;
	Q_INVOKABLE inline void repaintControls() { update(); }

public slots:
	void importSlot(const bool accepted);
	void sendFileTo(const int handle = 1, const QStringList &userids = QStringList{}, const QString &message = QString{},
																						const bool present_dialog = false);

signals:
	void fileTypeChanged();
	void fileNameChanged();
	void subdirForAddedFileChanged();
	void showFullScreen();
	void multimediaKeyPressed(const int key);
	void multimediaKeyReleased(const int key);
	void fileAdded(const QString &filepath);
	void fileAcquired(const int ret_code);
	void fileSent(const int success);
	void fileRemovalRequested();
	void mesoIdxChanged();
	void controlSizeChanged();
	void parentPageChanged();
	void workoutCalendarDayChanged();
	void tpFileSectionCountChanged();
	void addFileFiltersChanged();
	void canDownloadOrGenerateChanged();
	void canAddFileChanged();
	void restrictedFileTypeChanged();
	void useControlsChanged();
	void fileIsOKChanged();
	void setCursorPorsition(const int cursor_pos);
	void insertString(const QString &ch, const int pos);
	void _sendFileDialogCreated();

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
	TPUtils::FILE_TYPE m_filetype{TPUtils::FT_NO_TYPE_SET};
	QList<std::pair<QString,QString>> m_tpFileInfo;
	bool m_fullscreen{false}, m_canAddFile{false}, m_downloadOrGenerate{false}, m_restrictedFileType{false},
															m_fileIsOK{false}, m_useControls{false}, m_usews{false};
	int m_mesoIdx{-1}, m_workoutCalendarDay{-1}, m_cursorPostion{-1};
	uint  m_tpfileSections{0}, m_addFileFilters{0};
	QTextDocument *m_textDocument{nullptr};
	TPFilePath m_filename;
	QString m_subdir;
	QFileDialog *m_fileDialog{nullptr};
	QQmlComponent *m_sendFileDialogComponent{nullptr};
	QObject *m_sendFileDialog{nullptr};
	QQuickItem *m_parentPage{nullptr};
	std::function<TPFilePathPtr(const QString&)> m_suggestNameFunc{nullptr};

	void _setFileName(const bool file_added);
	void _doFileOperation(const OpType type);
	void generateFileFromType(const bool formatted);
	void doFullScreen();
	void addFile();
	void saveFileAs();
	void shareFile();
	void downloadOrCopyFile();
	void sendFileToUsers(const QStringList &users, const QString &message);
	void sendFileDirectly(const QStringList &users);
	void openFile();
	void setButtonCondition(const OpType type, std::optional<bool> visible = std::nullopt, bool do_update = false);
	void createControls();
	void clearControls();
	void resizeControl();
	void recalculateButtonsRect();
	void colorizeImage(QImage &image);
	void disableImage(QImage &image);
	controlInfo *controlFromMouseClick(const QPointF& mouse_pos) const;
	controlInfo *controlFromType(const OpType type) const;
	QString getImagePreviewFile(QSize preferred_size = QSize{}) const;
	QString getPDFPreviewFile(QSize preferred_size = QSize{}) const;
	void _setEnabled(controlInfo *ci, const bool enabled);
	void _getDefaultImage(controlInfo *ci);
	void readTPFile();
	void openTPFile();
	void textDocumentKeyNavigation(const int key);
	void createSendFileDialog();

	Q_DISABLE_COPY(TPFileOps)
};
