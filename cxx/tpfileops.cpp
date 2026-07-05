#include "tpfileops.h"
#include "dbexerciseslistmodel.h"
#include "dbexercisesmodel.h"
#include "dbmesocyclesmodel.h"
#include "dbusermodel.h"
#include "osinterface.h"
#include "pageslistmodel.h"
#include "qmlitemmanager.h"
#include "tpfilepath.h"
#include "tpimage.h"
#include "tpsettings.h"
#include "online_services/tponlineservices.h"
#include "online_services/tpmessagesmanager.h"
#include "online_services/websocketserver.h"

#ifdef Q_OS_ANDROID
#include "osinterface.h"
#endif

#include <QPainter>
#include <QFileDialog>
#include <QtPdf/QPdfDocument>
#include <QQmlApplicationEngine>
#include <QQuickTextDocument>
#include <QQuickWindow>
#include <QTextBlock>
#include <QTextDocument>

constexpr int8_t buttons_padding{5};

#include <filesystem>
namespace fs = std::filesystem;

inline bool _isFileOk(const QString &file)
{
	std::error_code ec;
	// Non-throwing usage
	const std::uintmax_t size{fs::file_size(file.toStdString(), ec)};
	return !ec && size > 0;
}

TPFileOps::TPFileOps(QQuickItem *visual_parent)
	: QQuickPaintedItem{visual_parent}
{
	setAcceptTouchEvents(true);
	setAcceptedMouseButtons(Qt::LeftButton);
	if (appUserModel()->actualMesoModel()) {
		connect(appUserModel()->actualMesoModel(), &DBMesocyclesModel::mesoIdxChanged, this, [this]
																	(const uint old_meso_idx, const uint new_meso_idx) {
			if (old_meso_idx == m_mesoIdx)
				setMesoIdx(new_meso_idx);
		});
	}
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
	default:
		m_pressedColor.setRgb(0, 0, m_pressedColor.black());
		break;
	}
	m_pressedColor.setAlpha(100);
	m_buttonSize.rwidth() = appSettings()->itemDefaultHeight();
	m_buttonSize.rheight() = appSettings()->itemDefaultHeight();
}

void TPFileOps::paint(QPainter *painter)
{
	if (painter->clipBoundingRect().width() == m_buttonSize.width() && m_currentControl) {
		painter->drawImage(painter->clipBoundingRect(), *(m_currentControl->current_image));
	} else {
		for (const auto &ci : std::as_const(m_controls)) {
			if (ci && ci->visible)
				painter->drawImage(ci->rect, *(ci->current_image));
		}
	}
}

void TPFileOps::setFileType(TPUtils::FILE_TYPE new_type)
{
	if (m_filetype != new_type) {
		m_filetype = new_type;
		emit fileTypeChanged();
		if (m_useControls) {
			if (m_controls[0]) {
				for (int i{OT_FullScreen}; i <= OT_TypeCount - 2; ++i)
					setButtonCondition(static_cast<OpType>(i));
				setButtonCondition(static_cast<OpType>(OT_TypeCount - 1), std::nullopt, true);
			} else {
				createControls();
			}
		}
	}
}

void TPFileOps::setFileName(const QString &filename, const bool file_added)
{
	m_filename = filename;
	if (!QFile::exists(m_filename.toString()) && !canDownloadOrGenerate()) {
		m_filename = std::move(TPFilePath{});
		setFileType(TPUtils::FT_UNKNOWN);
		setFileIsOK(false);
	} else {
		_setFileName(file_added);
	}
}

void TPFileOps::setFileName(TPFilePath &&tp_filename)
{
	if (tp_filename.isOK()) {
		m_filename = std::forward<TPFilePath>(tp_filename);
		_setFileName(false);
	}
}

void TPFileOps::setFileURL(const QUrl &url)
{
	setFileName(appUtils()->getCorrectPath(url));
}

void TPFileOps::setCanDownloadOrGenerate(const bool can_do)
{
	if (can_do != m_downloadOrGenerate) {
		m_downloadOrGenerate = can_do;
		emit canDownloadOrGenerateChanged();
		setButtonCondition(OT_Download, m_downloadOrGenerate, true);
	}
}

void TPFileOps::setCanAddFile(const bool can_add)
{
	if (can_add != m_canAddFile) {
		m_canAddFile = can_add;
		emit canAddFileChanged();
		setButtonCondition(OT_AddFile, m_canAddFile, true);
	}
}

void TPFileOps::renameFile(const QString &new_name)
{
 	const QString &correct_new_name{appUtils()->getFileName(new_name, true) %
															appUtils()->getFileExtension(m_filename.fileName(), true)};
	if (fileIsOK())
		QFile::rename(m_filename.toString(), m_filename.filePath() % correct_new_name);
	m_filename.setFileName(correct_new_name, true);
}

void TPFileOps::removeFile(const bool bypass_confirmation, const bool remove_local, const bool remove_remote)
{
	if (!bypass_confirmation || appSettings()->alwaysAskConfirmation()) {
		connect(appItemManager(), &QmlItemManager::generalMessagesPopupClicked, this, [=,this] (const uint8_t button) {
			if (button == 1)
				removeFile(true, remove_local, remove_remote);
		}, Qt::SingleShotConnection);
		appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE,
			appUtils()->string_strings({tr("Remove file?"), m_filename.toString()}, record_separator),
									Qt::AlignCenter, getFileTypeIcon(QSize{appSettings()->itemExtraLargeHeight(),
													appSettings()->itemExtraLargeHeight()}), -1, tr("Yes"), tr("No"));
		return;
	}
	if (remove_local)
		QFile::remove(m_filename.toString());
	if (remove_remote)
		appUserModel()->removeFileFromServer(m_filename);
	emit fileRemovalRequested();
}

void TPFileOps::exportTPFile(const TPFilePath &tp_filename)
{
	int ret_code(TP_RET_CODE_EXPORT_FAILED);
	QString message;
	if (tp_filename.isOK()) {
		uint32_t ft{static_cast<uint>(m_filetype) & static_cast<uint>(~TPUtils::FT_TP_FORMATTED)};
		switch (ft) {
		case TPUtils::FT_TP_USER_PROFILE:
			ret_code = appUserModel()->exportToFormattedFile(0, tp_filename);
			break;
		case TPUtils::FT_TP_PROGRAM:
			connect(appUserModel()->actualMesoModel(), &DBMesocyclesModel::mesoExported, this, [this]
					(const uint meso_idx, const TPFilePath &filename, const int return_code) {
						exportTPFile(return_code == TP_RET_CODE_EXPORT_OK ? filename : TPFilePath{});
					}, Qt::SingleShotConnection);
			appUserModel()->actualMesoModel()->exportToFormattedFile(m_mesoIdx, tp_filename);
			return;
		case TPUtils::FT_TP_WORKOUT_A:
		case TPUtils::FT_TP_WORKOUT_B:
		case TPUtils::FT_TP_WORKOUT_C:
		case TPUtils::FT_TP_WORKOUT_D:
		case TPUtils::FT_TP_WORKOUT_E:
		case TPUtils::FT_TP_WORKOUT_F:
			//TODO
			break;
		case TPUtils::FT_TP_EXERCISES:
			ret_code = appExercisesList()->exportToFormattedFile(tp_filename);
			break;
		default:
#ifndef QT_NO_DEBUG
			qDebug() << "Error! Trying to save/export a not TPApp file - TPFileOps::exportSlot(" << m_filename.toString() << ")";
#endif
			return;
		}
		if (ret_code == TP_RET_CODE_EXPORT_OK)
			message = std::move(tp_filename.fileName());
		else
			message = std::move(tr("Could not save to: ") % tp_filename.fileName());
	}
	else
		message = std::move(tr("Operation canceled"));
	appItemManager()->displayMessageOnAppWindow(ret_code, message);
}

QString TPFileOps::openFileDialog(const int file_type, const QString &suggested_save_name)
{
	if (!m_fileDialog)
		m_fileDialog = new QFileDialog{nullptr, Qt::Dialog};
	const TPUtils::FILE_TYPE f_type{static_cast<TPUtils::FILE_TYPE>(file_type)};
	const bool open_dialog{suggested_save_name.isEmpty()};
	m_fileDialog->setDirectory(appUtils()->standardPathForFileType(f_type));
	m_fileDialog->setNameFilters(appUtils()->extensionsListForType(f_type));
	m_fileDialog->setAcceptMode(open_dialog ? QFileDialog::AcceptOpen : QFileDialog::AcceptSave);
	m_fileDialog->setFileMode(open_dialog ? QFileDialog::AnyFile : QFileDialog::Directory);
	if (!open_dialog)
		m_fileDialog->selectFile(suggested_save_name);
	return m_fileDialog->exec() == QDialog::Accepted ? m_fileDialog->selectedFiles().at(0) : QString{};
}

void TPFileOps::attemptToCreateOrGetFile()
{
	if (fileIsOK()) {
		emit fileAcquired(TP_RET_CODE_NO_CHANGES_SUCCESS);
		return;
	} else if (!canDownloadOrGenerate()) {
		emit fileAcquired(TP_RET_CODE_INVALID_REQUEST_METHOD);
		return;
	}

	connect(this, &TPFileOps::fileAcquired, this, [this] (const int ret_code) {
		appItemManager()->displayMessageOnAppWindow(ret_code);
		if (fileIsOK() && isTPFile()) {
			if (m_filetype & TPUtils::FT_TP_FORMATTED)
				readTPFile();
		}
	}, Qt::SingleShotConnection);
	if (isTPFile())
		generateFileFromType(true);
	else
		downloadOrCopyFile();
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

QString TPFileOps::getFileTypeIcon(const QSize &preferred_size, const bool thumbnail) const
{
	uint32_t ft{static_cast<uint>(m_filetype) & static_cast<uint>(~TPUtils::FT_TP_FORMATTED)};
	switch (ft) {
	case TPUtils::FT_TP_USER_PROFILE:	return "user_preview"_L1;
	case TPUtils::FT_TP_PROGRAM:		return "meso_preview"_L1;
	case TPUtils::FT_TP_WORKOUT_A:
	case TPUtils::FT_TP_WORKOUT_B:
	case TPUtils::FT_TP_WORKOUT_C:
	case TPUtils::FT_TP_WORKOUT_D:
	case TPUtils::FT_TP_WORKOUT_E:
	case TPUtils::FT_TP_WORKOUT_F:		return "workout_preview"_L1;
	case TPUtils::FT_TP_EXERCISES:		return "exerciselist_preview"_L1;
	case TPUtils::FT_IMAGE:				return thumbnail ? getImagePreviewFile(preferred_size) : "image_preview"_L1;
	case TPUtils::FT_VIDEO:				return "video_preview"_L1;
	case TPUtils::FT_PDF:				return thumbnail ? getPDFPreviewFile(preferred_size) : "pdf_preview"_L1;;
	case TPUtils::FT_TEXT:				return "text_preview"_L1;
	case TPUtils::FT_OPEN_DOCUMENT:		return "odf_preview"_L1;
	case TPUtils::FT_MS_DOCUMENT:		return "docx_preview"_L1;
	case TPUtils::FT_OTHER:				return "generic_preview"_L1;
	case TPUtils::FT_UNKNOWN:
	default:							return "no-image"_L1;
	}
}

void TPFileOps::setWorkingDocumentCursorPosition(const int cursor_position)
{
	m_cursorPostion = cursor_position;
}

QString TPFileOps::getFileText(const bool preview_text) const
{
	if (m_filetype == TPUtils::FT_TEXT && QFile::exists(m_filename.toString())) {
		QFile *text_file{appUtils()->openFile(m_filename.toString())};
		if (text_file) {
			QString text_line{1024, QChar{0}};
			QTextStream stream{text_file};
			std::pair<QString,QString> section_info;
			const uint max_lines{preview_text ? 10: UINT_MAX};
			uint line{0};
			QString file_text;
			while (stream.readLineInto(&text_line)) {
				if (!text_line.isEmpty())
					file_text += std::move(text_line + (preview_text ? QChar{'\n'} : QChar{0x2029}));
				else
					file_text += std::move(preview_text ? QChar{'\n'} : QChar{0x2029});
				if (++line == max_lines)
					break;
			}
			text_file->close();
			delete text_file;
			return file_text;
		}
	}
	return QString{};
}

inline bool fileStillInUse(const QString &filename)
{
	QFileInfo fi{filename};
	if (fi.exists()) {
		const QDateTime &f_time{fi.lastModified()};
		if (f_time.date() == QDate::currentDate())
			return appUtils()->calculateTimeDifferenceInSecs(f_time.time(), QTime::currentTime()) <= 6;
	}
	return false;
}

void TPFileOps::importSlot(const bool accepted)
{
	if (!accepted)
		return;
	uint32_t ft{m_filetype};
	const bool formatted{(ft & TPUtils::FT_TP_FORMATTED) == TPUtils::FT_TP_FORMATTED};
	if (formatted)
		ft &= ~TPUtils::FT_TP_FORMATTED;

	switch (ft) {
	case TPUtils::FT_TP_USER_PROFILE:
		appUserModel()->newUserFromFile(m_filename, formatted);
		break;
	case TPUtils::FT_TP_PROGRAM:
		appUserModel()->actualMesoModel()->newMesoFromFile(m_filename, false, formatted);
		break;
	case TPUtils::FT_TP_WORKOUT_A:
	case TPUtils::FT_TP_WORKOUT_B:
	case TPUtils::FT_TP_WORKOUT_C:
	case TPUtils::FT_TP_WORKOUT_D:
	case TPUtils::FT_TP_WORKOUT_E:
	case TPUtils::FT_TP_WORKOUT_F:
		appUserModel()->actualMesoModel()->newWorkoutFromFile(m_filename, formatted,
			appUserModel()->actualMesoModel()->idxFromFieldValue(DBExercisesModel::workoutFileName_mesoName(m_filename),
							DBMesocyclesModel::MESO_FIELD_NAME), DBExercisesModel::workoutFileName_splitLetter(m_filename));
		break;
	case TPUtils::FT_TP_EXERCISES:
		appExercisesList()->newExerciseFromFile(m_filename, formatted);
		break;
	}
}

void TPFileOps::sendFileTo(const int handle, const QStringList& userids, const QString &message, const bool present_dialog)
{
	if (userids.isEmpty() || present_dialog) {
		if (!m_sendFileDialog) {
			connect(this, &TPFileOps::_sendFileDialogCreated, this, [=,this] { sendFileTo(handle, userids, message, present_dialog); });
			createSendFileDialog();
			return;
		}
		m_sendFileDialog->setProperty("handle", std::move(QVariant{handle}));
		m_sendFileDialog->setProperty("message", std::move(QVariant{message}));
		m_sendFileDialog->setProperty("selectedUsers", std::move(QVariant{userids}));
		appPagesListModel()->openPopup(m_sendFileDialog, m_parentPage);
	} else {
		switch (handle) {
		case TPUtils::MH_TPCHAT:
			sendFileDirectly(userids);
			if (m_usews)
				appWSServer()->sendTextMessage(message);
			else
				appOnlineServices()->sendChatMessage(-1, userids.at(0), message);
			break;
		case TPUtils::MH_TPMESSAGES_MANAGER:
			sendFileToUsers(userids, message);
			break;
		case TPUtils::MH_DIRECT_FILE_TRANSFER:
			sendFileDirectly(userids);
			break;
		default: //Cancel or dialog closed
			break;
		}
	}
}

void TPFileOps::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == acceptedMouseButtons()) {
		event->setAccepted(true);
		controlInfo* ci{controlFromMouseClick(event->position())};
        if (Q_LIKELY(ci)) {
			m_currentControl = ci;
			if (!ci->pressed) {
				if (ci->pressed_image.isNull()) {
					ci->pressed_image = ci->default_image.copy();
					TPImage::colorizeImage(ci->pressed_image, m_pressedColor);
				}
				ci->current_image = &ci->pressed_image;
				ci->pressed = true;
			} else {
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
    if (Q_LIKELY(ci == m_currentControl)) {
		ci->current_image = &ci->default_image;
		ci->pressed = false;
		update(ci->rect);
		_doFileOperation(ci->type);
	}
}

bool TPFileOps::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::KeyPress) {
		QKeyEvent *key_event{static_cast<QKeyEvent*>(event)};
		switch (key_event->key()) {
		case Qt::Key_Space:
		case Qt::Key_Left:
		case Qt::Key_Right:
		case Qt::Key_Up:
		case Qt::Key_Down:
			if (!m_textDocument)
				emit multimediaKeyPressed(key_event->key());
			else
				textDocumentKeyNavigation(key_event->key());
			break;
		default:
			return false;
		}
		return true; // Return true to stop the event from propagating
	} else if (event->type() == QEvent::KeyRelease) {
		QKeyEvent *key_event{static_cast<QKeyEvent*>(event)};
		switch (key_event->key()) {
		case Qt::Key_Space:
		case Qt::Key_Left:
		case Qt::Key_Right:
		case Qt::Key_Up:
		case Qt::Key_Down:
			emit multimediaKeyReleased(key_event->key());
			break;
		case Qt::Key_Escape:
			doFullScreen();
			break;
		case Qt::Key_Delete:
			removeFile(false, true, true);
			break;
		default:
			return false;
		}
		return true; // Return true to stop the event from propagating
	} else {
		return QObject::eventFilter(obj, event);
	}
}

void TPFileOps::_setFileName(const bool file_added)
{
	emit fileNameChanged();
	setFileIsOK(_isFileOk(m_filename.toString()));
	if (file_added)
		emit fileAdded(m_filename.toString());
	const TPUtils::FILE_TYPE file_type{appUtils()->getFileType(m_filename.toString())};
	setFileType(file_type);
	if (fileIsOK() && isTPFile()) {
		if (file_type & TPUtils::FT_TP_FORMATTED)
			readTPFile();
		else
			setEnabled(OT_FullScreen, false);
	}
}

void TPFileOps::_doFileOperation(const OpType type)
{
	switch (type) {
	case OT_AddFile:	addFile();				return;
	case OT_FullScreen:	doFullScreen();			return;
	case OT_Download:	downloadOrCopyFile();	return;
	default:									break;
	}

	if (QFile::exists(m_filename.toString())) {
		switch (type) {
		case OT_Share:			shareFile();						break;
		case OT_Forward:		sendFileTo();						break;
		case OT_ViewExternally:	openFile();							break;
		case OT_Delete:			removeFile(false, true, true);		break;
		default:													break;
		}
		return;
	} else {
		connect(this, &TPFileOps::fileAcquired, this, [this,type] (const int ret_code) {
			if (ret_code == TP_RET_CODE_SUCCESS || ret_code == TP_RET_CODE_NO_CHANGES_SUCCESS)
				_doFileOperation(type);
		}, Qt::SingleShotConnection);
		attemptToCreateOrGetFile();
	}
}

void TPFileOps::generateFileFromType(const bool formatted)
{
	int ret{TP_RET_CODE_UNKNOWN_ERROR};
	switch (fileType()) {
	case TPUtils::FT_TP_PROGRAM: {
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(appUserModel()->actualMesoModel(), &DBMesocyclesModel::mesoExported, this, [this,conn]
												(const uint meso_idx, const TPFilePath& filename, const int return_code) {
			if (meso_idx == m_mesoIdx) {
				disconnect(*conn);
				if (return_code == TP_RET_CODE_EXPORT_OK) {
					m_filetype |= TPUtils::FT_TP_FORMATTED;
					setFileIsOK(true);
				}
				emit fileAcquired(return_code);
			}
		});
		if (!formatted)
			appUserModel()->actualMesoModel()->exportToFile(m_mesoIdx, m_filename);
		else
			appUserModel()->actualMesoModel()->exportToFormattedFile(m_mesoIdx, m_filename);
		}
		break;
	case TPUtils::FT_TP_EXERCISES:
		if (!formatted)
			ret = appExercisesList()->exportToFile(m_filename);
		else
			ret = appExercisesList()->exportToFormattedFile(m_filename);
		break;
	case TPUtils::FT_TP_WORKOUT_A:
	case TPUtils::FT_TP_WORKOUT_B:
	case TPUtils::FT_TP_WORKOUT_C:
	case TPUtils::FT_TP_WORKOUT_D:
	case TPUtils::FT_TP_WORKOUT_E:
	case TPUtils::FT_TP_WORKOUT_F: {
			DBExercisesModel *model{appUserModel()->actualMesoModel()->workoutForDay(m_mesoIdx, m_workoutCalendarDay)};
			if (model) {
				if (!formatted)
					ret = model->exportToFile(m_filename);
				else
					ret = model->exportToFormattedFile(m_filename);
			}
		}
		break;
	default:
		qDebug() << "ERROR!!! File type set as " << fileType() << " but neither filename as given, nor a method provided to create the file";
	}
	if (ret == TP_RET_CODE_EXPORT_OK) {
		m_filetype |= TPUtils::FT_TP_FORMATTED;
		setFileIsOK(true);
	}
	emit fileAcquired(ret);
}

void TPFileOps::doFullScreen()
{
	m_fullscreen = !m_fullscreen;
	if (m_fullscreen) {
		appPagesListModel()->removeEventFilter();
		qApp->installEventFilter(this);
	} else {
		qApp->removeEventFilter(this);
		appPagesListModel()->reinstallEventFilter();
	}
	emit showFullScreen();
}

void TPFileOps::addFile()
{
	QString filepath{std::move(openFileDialog(m_restrictedFileType ? m_filetype : (m_addFileFilters > 0 ?
					m_addFileFilters : (m_filetype == TPUtils::FT_UNKNOWN ? TPUtils::FT_ANY_TYPE : m_filetype))))};
	if (!filepath.isEmpty()) {
		if (!m_filename.isOK()) {
			if (m_suggestNameFunc) {
				m_filename = std::move(*m_suggestNameFunc(appUtils()->getFileName(filepath)));
				if (appUtils()->getFileExtension(m_filename.toString()).isEmpty())
					m_filename.filename().append(appUtils()->getFileExtension(filepath, true));
			} else if (!m_subdir.isEmpty()) {
				m_filename = appUserModel()->userId(0) % '/' % m_subdir % appUtils()->getFileName(filepath);
			} else {
				qDebug() << "Error! Cannot add a file to TPFileOps that does not have a local filename set or suggested."_L1;
				return;
			}
			if (appUtils()->copyFile(filepath, m_filename.toString(), true))
				_setFileName(true);
		} else {
			static_cast<void>(appUtils()->copyFile(filepath, m_filename.toString(), true));
		}
	}
}

void TPFileOps::saveFileAs()
{
	QString new_name{std::move(openFileDialog(static_cast<int>(m_filetype), m_filename.fileName()))};
	if (!new_name.isEmpty()) {
		appUtils()->rename(m_filename.toString(), new_name, true);
		m_filename = new_name;
	}
}

void TPFileOps::shareFile()
{
#ifdef Q_OS_ANDROID
	appOsInterface()->shareFile(m_filename);
#else
	saveFileAs();
#endif
}

void TPFileOps::downloadOrCopyFile()
{
	if (!fileIsOK()) {
		if (canDownloadOrGenerate()) {
			if (m_suggestNameFunc)
				m_filename = std::move(*m_suggestNameFunc(QString{}));
			if (m_filename.isOK()) {
				if (appUserModel()->canConnectToServer()) {
					auto conn{std::make_shared<QMetaObject::Connection>()};
					const int request_id{appUserModel()->downloadFileFromServer(m_filename)};
					*conn = connect(appUserModel(), &DBUserModel::fileDownloaded, this, [this,conn,request_id]
							(const bool success, const uint requestid, const TPFilePath &tp_filepath) {
						if (requestid == request_id) {
							disconnect(*conn);
							setFileIsOK(success);
							emit fileAcquired(success ? TP_RET_CODE_SUCCESS : TP_RET_CODE_DOWNLOAD_FAILED);
						}
					});
					return;
				}
			}
		}
		emit fileAcquired(TP_RET_CODE_INVALID_REQUEST_METHOD);
	} else {
		emit fileAcquired(TP_RET_CODE_NO_CHANGES_SUCCESS);
	}
}

void TPFileOps::createSendFileDialog()
{
	if (!m_sendFileDialogComponent) {
		m_sendFileDialogComponent = new QQmlComponent{appQmlEngine(), "TpQml.Dialogs"_L1, "SendFileToDialog"_L1, QQmlComponent::Asynchronous};
		connect(m_sendFileDialogComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status status) {
			createSendFileDialog();
		});
	} else {
		if (!m_sendFileDialog) {
			switch (m_sendFileDialogComponent->status()) {
			case QQmlComponent::Ready:
				m_sendFileDialogComponent->disconnect();
				m_sendFileDialog = m_sendFileDialogComponent->create(appQmlEngine()->rootContext());
#ifndef QT_NO_DEBUG
				if (!m_sendFileDialog) {
					qDebug() << m_sendFileDialogComponent->errorString();
					return;
				}
#endif
				appQmlEngine()->setObjectOwnership(m_sendFileDialog, QQmlEngine::CppOwnership);
				m_sendFileDialog->setProperty("parent", QVariant::fromValue(appItemManager()->appHomePage()));
				connect(m_sendFileDialog, SIGNAL(selectedOptions(int,QStringList,QString,bool)), this, SLOT(sendFileTo(int,QStringList,QString,bool)));
				emit _sendFileDialogCreated();
				break;
			case QQmlComponent::Loading:
				return;
			case QQmlComponent::Null:
			case QQmlComponent::Error:
#ifndef QT_NO_DEBUG
				qDebug() << m_sendFileDialogComponent->errorString();
#endif
				return;
			}
		}
	}
}

void TPFileOps::sendFileToUsers(const QStringList &users, const QString &message)
{
	const auto ws_send_func = [this] (const TPFilePath &new_tppath, const QString &message) -> void {
		bool ws_sentok{appWSServer()->sendBinaryMessage(m_filename, new_tppath)};
		if (ws_sentok)
			ws_sentok = appWSServer()->sendTextMessage(message);
		emit fileSent(ws_sentok);
	};
	TPFilePath new_path{std::as_const(m_filename)};
	new_path.swapUsers();
	const QString &str_ctime{appUtils()->formatDateTime(QDateTime::currentDateTime())};
	for (const auto &user : users) {
		new_path.setOwnerUser(user);
		const QString &encoded_message{appUtils()->makeEncodedMessage(TPUtils::tpmessage_prefix, m_filename.ownerUser(),
													user, str_ctime, QString{}, message, new_path.relativeFilePath())};
		if ((m_usews = appWSServer()->isConnectionOK(user, true))) {
			ws_send_func(new_path, encoded_message);
			return;
		}
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(appWSServer(), &WSServer::connectionAttemptResult, this, [=,this]
																	(const bool established, const QString &userid) {
			if (userid == user) {
				disconnect(*conn);
				m_usews = established;
				if (established) {
					ws_send_func(new_path, encoded_message);
					return;
				}
				std::function<void(int)> failureMsg = [this] (const int ret_code) -> void {
					appItemManager()->displayMessageOnAppWindow(ret_code, m_filename.fileName());
					emit fileSent(false);
				};
				const auto request_id{appUserModel()->sendFileToServer(m_filename)};
				switch (request_id) {
				case TP_RET_CODE_SERVER_UNREACHABLE:
				case TP_RET_CODE_USER_OFFLINE:
				case TP_RET_CODE_FILE_TOO_BIG:
					failureMsg(request_id);
					return;
				default: break;
				}

				*conn = connect(appUserModel(), &DBUserModel::fileUploaded, this,
									[=,this] (const bool success, const uint requestid, const int ret_code) {
					if (requestid == request_id) {
						disconnect(*conn);
						if (!success) {
							failureMsg(ret_code);
							return;
						}
						*conn = connect(appMessagesManager(), &TPMessagesManager::TPMessageSent, this,
							[this,request_id,conn] (const int requestid, const bool success) {
							if (request_id == requestid) {
								disconnect(*conn);
								if (success)
									appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_SUCCESS,
									appUtils()->string_strings({tr("File sent!"), m_filename.fileName() % " -> "_L1 %
									appUserModel()->userNameFromId(m_filename.targetUser())}, record_separator));
								emit fileSent(success);
							}
						});
						appMessagesManager()->sendTPMessage(user, encoded_message, request_id);
					}
				});
			}
		});
	}
}

void TPFileOps::sendFileDirectly(const QStringList &users)
{
	const auto ws_send_func = [this] (const TPFilePath &target_filename) -> void {
		const bool ws_sentok{appWSServer()->sendBinaryMessage(m_filename, target_filename)};
		emit fileSent(ws_sentok);
	};
	TPFilePath sent_filepath{m_filename.fileName(), QString{}, m_filename.ownerUser(), {m_filename.subdirs()}};

	for (const auto &user : users) {
		sent_filepath.setOwnerUser(user);
		if (appWSServer()->isConnectionOK(user, true)) {
			ws_send_func(sent_filepath);
			return;
		}
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(appWSServer(), &WSServer::connectionAttemptResult, this, [=,this]
																(const bool established, const QString &userid) {
			if (userid == user) {
				disconnect(*conn);
				m_usews = established;
				if (established) {
					ws_send_func(sent_filepath);
					return;
				}
				std::function<void(int)> failureMsg = [this] (const int ret_code) -> void {
					qDebug() << "Error sending file ("_L1 << ret_code << "): "_L1 << m_filename.fileName();
					emit fileSent(false);
				};
				const auto request_id{appUserModel()->sendFileToServer(m_filename)};
				if (request_id < TP_RET_CODE_CUSTOM_WARNING) {
					failureMsg(request_id);
					return;
				}
				*conn = connect(appUserModel(), &DBUserModel::fileUploaded, this,
										[=,this] (const bool success, const uint requestid, const int ret_code) {
						if (requestid == request_id) {
							disconnect(*conn);
							if (!success)
								failureMsg(ret_code);
							else
								emit fileSent(true);
						}
				});
			}
		});
	}
}

void TPFileOps::openFile()
{
	if (isTPFile())
		openTPFile();
	else if (isOpenedExternally())
		appOsInterface()->viewExternalFile(m_filename.toString());
}

void TPFileOps::setButtonCondition(const OpType type, std::optional<bool> visible, bool do_update)
{
	controlInfo *ci{m_controls[type]};
	if (!ci)
		return;
	if (!visible.has_value()) {
		switch (type) {
		case OT_AddFile:
			visible = canAddFile();
			break;
		case OT_FullScreen:
			visible = isViewableFile();
			break;
		case OT_Download:
			visible = canDownloadOrGenerate();
			ci->enabled = !fileIsOK();
			break;
		case OT_Share:
			visible = isKnownFile();
			ci->enabled = fileIsOK();
			break;
		case OT_ViewExternally:
			if ((visible = fileIsOK()))
				_getDefaultImage(ci);
			break;
		default:
			visible = isKnownFile();
		}
	}
	ci->visible = visible.value();
	if (do_update) {
		resizeControl();
		recalculateButtonsRect();
		update();
	}
}

void TPFileOps::createControls()
{
	int button_x{buttons_padding};
	for (int i{OT_AddFile}; i < OT_TypeCount; ++i) {
		controlInfo *ci{m_controls[i]};
		if (!m_controls[i]) {
			ci = new controlInfo;
			m_controls[i] = ci;
			ci->type = static_cast<OpType>(i);
			setButtonCondition(ci->type);
		}
		_getDefaultImage(ci);
		ci->default_image = std::move(ci->default_image.scaled(m_buttonSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
		ci->current_image = &ci->default_image;
		if (ci->visible) {
			ci->rect = QRect{button_x, buttons_padding, m_buttonSize.width(), m_buttonSize.height()};
			button_x += m_buttonSize.width() + buttons_padding;
		}
	}
	resizeControl();
	update();
}

void TPFileOps::clearControls()
{
	for (int i{OT_AddFile}; i < OT_TypeCount; ++i) {
		controlInfo *ci{m_controls[i]};
		if (ci) {
			delete ci;
			ci = nullptr;
		}
	}
	setControlSize(QSize{0, 0});
}

void TPFileOps::resizeControl()
{
	int n_visible_controls{0};
	for (int i{OT_AddFile}; i < OT_TypeCount; ++i) {
		controlInfo *ci{m_controls[i]};
		if (ci->visible) ++n_visible_controls;
	}
	if (n_visible_controls >= 1)
		setControlSize(QSize{n_visible_controls * (appSettings()->itemDefaultHeight() + buttons_padding)
								 + buttons_padding, appSettings()->itemDefaultHeight() + (2 * buttons_padding)});
	else
		setControlSize(QSize{0,0});
}

void TPFileOps::recalculateButtonsRect()
{
	int button_x{buttons_padding};
	for (int i{OT_AddFile}; i < OT_TypeCount; ++i) {
		controlInfo *ci{m_controls[i]};
		if (ci->visible) {
			ci->rect = QRect{button_x, buttons_padding, m_buttonSize.width(), m_buttonSize.height()};
			button_x += m_buttonSize.width() + buttons_padding;
		}
	}
}

inline TPFileOps::controlInfo *TPFileOps::controlFromMouseClick(const QPointF& mouse_pos) const
{
	for (const auto ci : std::as_const(m_controls)) {
		if (ci->visible && ci->enabled && static_cast<int>(mouse_pos.x() >= ci->rect.x()) &&
													static_cast<int>(mouse_pos.x() <= ci->rect.x() + ci->rect.width()))
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

static inline QString previewFilename(const QString &source_filename, const QSize &preview_size)
{
	constexpr QLatin1StringView size_template{"_%1x%2"};
	return appUserModel()->mainUserDir() % TPUtils::previewImagesSubDir % QString::number(fnv1a_hash(source_filename)) %
		   size_template.arg(QString::number(preview_size.width()), QString::number(preview_size.height())) % ".jpg"_L1;
}

QString TPFileOps::getImagePreviewFile(QSize preferred_size) const
{
	if (!fileIsOK())
		return QString{};

	if (preferred_size.isNull()) {
		preferred_size.rwidth() = m_controlSize.width();
		preferred_size.rheight() = m_controlSize.width() * 1.4;
	}
	const QString &preview_filename{previewFilename(m_filename.fileName(), preferred_size)};
	if (!QFile::exists(preview_filename)) {
		QImage thumbnail{m_filename.toString()};
		thumbnail = std::move(thumbnail.scaled(preferred_size));
		thumbnail.save(preview_filename, "JPG", 10);
	}
	return preview_filename;
}

static QImage composeImages(const QImage& image1, const QImage& image2, const QPoint& position = QPoint(0, 0))
{
	// 1. Create a destination QImage (ensure it has an alpha channel for blending)
	QImage resultImage(image1.size(), QImage::Format_ARGB32_Premultiplied);
	// Start with a transparent background
	resultImage.fill(Qt::transparent);
	// 2. Initialize a QPainter on the result image
	QPainter painter(&resultImage);
	// 3. Draw the first image (base)
	painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
	painter.drawImage(0, 0, image1);
	// 4. Set the composition mode for the second image
	// QPainter::CompositionMode_SourceOver blends the source (image2) over the destination (image1),
	// respecting the alpha channel of image2. This is the most common blending mode.
	painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
	// 5. Draw the second image (overlay) at a specific position
	painter.drawImage(position, image2);
	// End painting
	painter.end();
	return resultImage;
}


QString TPFileOps::getPDFPreviewFile(QSize preferred_size) const
{
	if (fileIsOK()) {
		if (preferred_size.isNull()) {
			preferred_size.rwidth() = m_controlSize.width();
			preferred_size.rheight() = m_controlSize.width() * 1.4;
		}
		const QString &preview_filename{previewFilename(m_filename.toString(), preferred_size)};
		if (!QFile::exists(preview_filename)) {
			QPdfDocument *pdf_doc{new QPdfDocument{}};
			pdf_doc->load(m_filename.toString());
			QPdfDocumentRenderOptions pdf_opts;
			pdf_opts.setRenderFlags(QPdfDocumentRenderOptions::RenderFlag::TextAliased | QPdfDocumentRenderOptions::RenderFlag::ImageAliased |
									QPdfDocumentRenderOptions::RenderFlag::PathAliased | QPdfDocumentRenderOptions::RenderFlag::OptimizedForLcd);
			QImage background_image{preferred_size, QImage::Format_ARGB32_Premultiplied};
			background_image.fill(Qt::white);
			const QImage &pdf_image{composeImages(background_image, pdf_doc->render(0, preferred_size, pdf_opts))};
			if (!pdf_image.isNull())
				pdf_image.save(preview_filename, "JPG", 10);
			pdf_doc->deleteLater();
		}
		return preview_filename;
	}
	return QString{};
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
	const QString &str_image_source{":/images/%1_"_L1 % appSettings()->indexColorSchemeToColorSchemeName() % ".png"_L1};
	switch (ci->type) {
	case OT_AddFile: ci->default_image.load(str_image_source.arg("add-new")); break;
	case OT_FullScreen: ci->default_image.load(str_image_source.arg("fullscreen")); break;
	case OT_Download: ci->default_image.load(str_image_source.arg("download")); break;
	case OT_Share: ci->default_image.load(str_image_source.arg("share")); break;
	case OT_Forward: ci->default_image.load(str_image_source.arg("forward")); break;
	case OT_ViewExternally: ci->default_image.load(":/images/" % getFileTypeIcon(m_buttonSize, false)); break;
	case OT_Delete: ci->default_image.load(":/images/remove.png"_L1); break;
	default: break;
	}
}

void TPFileOps::readTPFile()
{
	QFile *in_file{appUtils()->openFile(m_filename.toString())};
	if (!in_file)
		return;

	const QString *identifier{nullptr};
	QString extra_identifier;
	const uint32_t ft{static_cast<uint>(m_filetype) & static_cast<uint>(~TPUtils::FT_TP_FORMATTED)};

	switch (ft) {
	case TPUtils::FT_TP_USER_PROFILE:
		identifier = &appUtils()->userFileIdentifier;
		break;
	case TPUtils::FT_TP_PROGRAM:
		identifier = &appUtils()->mesoFileIdentifier;
		break;
	case TPUtils::FT_TP_WORKOUT_A:
		extra_identifier = appUtils()->workoutFileIdentifier % "A"_L1;
		identifier = &extra_identifier;
		break;
	case TPUtils::FT_TP_WORKOUT_B:
		extra_identifier = appUtils()->workoutFileIdentifier % "B"_L1;
		identifier = &extra_identifier;
		break;
	case TPUtils::FT_TP_WORKOUT_C:
		extra_identifier = appUtils()->workoutFileIdentifier % "C"_L1;
		identifier = &extra_identifier;
		break;
	case TPUtils::FT_TP_WORKOUT_D:
		extra_identifier = appUtils()->workoutFileIdentifier % "D"_L1;
		identifier = &extra_identifier;
		break;
	case TPUtils::FT_TP_WORKOUT_E:
		extra_identifier = appUtils()->workoutFileIdentifier % "E"_L1;
		identifier = &extra_identifier;
		break;
	case TPUtils::FT_TP_WORKOUT_F:
		extra_identifier = appUtils()->workoutFileIdentifier % "F"_L1;
		identifier = &extra_identifier;
		break;
	case TPUtils::FT_TP_EXERCISES:
		identifier = &appUtils()->exercisesListFileIdentifier;
		break;
		default: Q_UNREACHABLE();
	}

	QString line{64, QChar{0}};
	QTextStream stream{in_file};
	std::pair<QString,QString> section_info;
	m_tpfileSections = 0;
	m_tpFileInfo.clear();

	while (stream.readLineInto(&line)) {
		if (line.isEmpty())
			section_info.second.append(QChar{0x2029});
		if (line.contains("##"_L1)) {
			if (line.contains(*identifier)) {
				section_info.first = std::move(line.right(line.length() - identifier->length() -
																	TPUtils::STR_START_FORMATTED_EXPORT.length() - 1));
				section_info.second.clear();
			} else if (line.startsWith(TPUtils::STR_END_FORMATTED_EXPORT)) {
				m_tpFileInfo.insert(m_tpfileSections, section_info);
				++m_tpfileSections;
				if (identifier == &appUtils()->mesoFileIdentifier)
					identifier =  &appUtils()->splitFileIdentifier;
			}
		} else {
			section_info.second.append(line % QChar{0x2029});
		}
	}
	if (!m_tpFileInfo.isEmpty())
		emit tpFileSectionCountChanged();
	in_file->close();
	delete in_file;
}

void TPFileOps::openTPFile()
{
	QString str_type, str_details, str_image;
	const QString &sender_client{m_filename.ownerUser()};
	const int user_idx{appUserModel()->userIdxFromFieldValue(DBUserModel::USER_FIELD_ID, sender_client)};
	const bool is_coach{appUserModel()->isCoach(user_idx)};
	const QString &client_name{appUserModel()->userName(user_idx)};
	const uint32_t ft{static_cast<uint>(m_filetype) & static_cast<uint>(~TPUtils::FT_TP_FORMATTED)};
	switch (ft) {
	case TPUtils::FT_TP_USER_PROFILE:
		str_type = std::move(is_coach ? tr("data for a new coach") : tr("data for a new client"));
		str_details = client_name;
		str_image = std::move(is_coach ? "manage-coaches"_L1 : "manage-clients"_L1);
		break;
	case TPUtils::FT_TP_PROGRAM:
		str_type = std::move(tr("program"));
		str_details = std::move(tr("A complete exercises program from coach ") % client_name);
		str_image = std::move("meso_preview"_L1);
		break;
	case TPUtils::FT_TP_WORKOUT_A:
	case TPUtils::FT_TP_WORKOUT_B:
	case TPUtils::FT_TP_WORKOUT_C:
	case TPUtils::FT_TP_WORKOUT_D:
	case TPUtils::FT_TP_WORKOUT_E:
	case TPUtils::FT_TP_WORKOUT_F: {
		const int meso_idx{appUserModel()->actualMesoModel()->idxFromFieldValue(
						DBExercisesModel::workoutFileName_mesoName(m_filename), DBMesocyclesModel::MESO_FIELD_NAME)};
		if (meso_idx < 0) return;
		const QChar &splitletter{DBExercisesModel::workoutFileName_splitLetter(m_filename)};
		str_type = std::move(tr("workout"));
		str_details = std::move(tr("An extra workout from ") % client_name % tr(" for the program: ") %
								appUserModel()->actualMesoModel()->name(meso_idx) % tr(" for the next time you train ") %
								appUserModel()->actualMesoModel()->muscularGroup(meso_idx, splitletter));
		str_image = std::move("workout_preview"_L1);
		}
		break;
	case TPUtils::FT_TP_EXERCISES:
		str_type = std::move(tr("Excercise Description"));
		str_details = std::move(tr("A new exercise for the exercises database from ") % client_name);
		str_image = std::move("exerciselist_preview"_L1);
		break;
	default:
		Q_UNREACHABLE();
	}
	connect(appMainWindow(), SIGNAL(tpFileOpenInquiryResult(bool)), this, SLOT(importSlot(bool)), Qt::SingleShotConnection);
	QMetaObject::invokeMethod(appMainWindow(), "confirmTPFileOpening", Q_ARG(QString, str_type),
														Q_ARG(QString, str_details), Q_ARG(QString, str_image));
}

void TPFileOps::textDocumentKeyNavigation(const int key)
{
	int other_line;
	switch (key) {
	case Qt::Key_Space: emit insertString("&nbsp"_L1, m_cursorPostion); return;
	case Qt::Key_Left: emit setCursorPorsition(--m_cursorPostion); return;
	case Qt::Key_Right: emit setCursorPorsition(++m_cursorPostion); return;
	case Qt::Key_Up: other_line = -1; break;
	case Qt::Key_Down: other_line = 1; break;
	}
	const QTextBlock &tb{m_textDocument->findBlock(m_cursorPostion)};
	const auto pos_in_line{m_cursorPostion - tb.position()};
	const QTextBlock &tb2{m_textDocument->findBlockByNumber(tb.blockNumber() + other_line)};
	if (pos_in_line >= tb.position() + tb.length()) {
		emit setCursorPorsition(tb2.position() + tb2.length());
	} else {
		auto line_length2{tb2.length()};
		if (line_length2 >= pos_in_line)
			emit setCursorPorsition(tb2.position() + pos_in_line);
		else
			emit setCursorPorsition(tb2.position() + tb2.length() - 1);
	}
}
