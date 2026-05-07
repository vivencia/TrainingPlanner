#include "tpfileops.h"
#include "dbexerciseslistmodel.h"
#include "dbexercisesmodel.h"
#include "dbmesocyclesmodel.h"
#include "dbusermodel.h"
#include "pageslistmodel.h"
#include "qmlitemmanager.h"
#include "tpimage.h"
#include "tpsettings.h"
#include "online_services/tpmessagesmanager.h"

#ifdef Q_OS_ANDROID
#include "osinterface.h"
#endif

#include <QPainter>
#include <QtPdf/QPdfDocument>
#include <QQuickTextDocument>
#include <QQuickWindow>
#include <QTextBlock>
#include <QTextDocument>

// FNV-1a constants
constexpr uint32_t FNV_PRIME_32{0x01000193};
constexpr uint32_t FNV_OFFSET_BASIS_32{0x811c9dc5};
constexpr int8_t buttons_padding{5};

inline uint32_t fnv1a_hash(const QString& s) {
	uint32_t hash{FNV_OFFSET_BASIS_32};
	for (const auto c : s.toStdU16String()) {
		hash ^= static_cast<uint8_t>(c);
		hash *= FNV_PRIME_32;
	}
	return hash;
}

TPFileOps::TPFileOps(QQuickItem *parent)
	: QQuickPaintedItem{parent}
{
	setAcceptTouchEvents(true);
	setAcceptedMouseButtons(Qt::LeftButton);
	connect(appUserModel()->actualMesoModel(), &DBMesocyclesModel::mesoIdxChanged, this, [this] (const uint old_meso_idx, const uint new_meso_idx) {
		if (old_meso_idx == m_mesoIdx)
			setMesoIdx(new_meso_idx);
	});
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
		Q_UNREACHABLE();
	}
	m_pressedColor.setAlpha(100);
	m_buttonSize.rwidth() = appSettings()->itemDefaultHeight();
	m_buttonSize.rheight() = appSettings()->itemDefaultHeight();
	createControls();
}

void TPFileOps::paint(QPainter *painter)
{
	if (painter->clipBoundingRect().width() == m_buttonSize.width() && m_currentControl)
		painter->drawImage(painter->clipBoundingRect(), *(m_currentControl->current_image));
	else {
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
		for (int i{OT_FullScreen}; i < OT_TypeCount; ++i) {
			controlInfo *ci{m_controls[i]};
			ci->visible = new_type != TPUtils::FT_UNKNOWN;
			switch (i) {
			case OT_FullScreen:
				ci->visible = new_type <= TPUtils::FT_TEXT;
				break;
			case OT_ViewExternally:
				_getDefaultImage(ci);
				break;
			}
		}
		resizeControl();
		recalculateButtonsRect();
		update();
	}
}

void TPFileOps::setFileName(const QString &filename)
{
	if (filename.isEmpty() || !QFile::exists(filename))
		setFileType(TPUtils::FT_UNKNOWN);
	else {
		m_filename = filename;
		emit fileNameChanged();
		const TPUtils::FILE_TYPE file_type{appUtils()->getFileType(filename)};
		setFileType(file_type);
		if (file_type < TPUtils::FT_IMAGE) {
			if (file_type & TPUtils::FT_TP_FORMATTED) {
				m_tpfileSections = 0;
				m_tpFileInfo.clear();
				readTPFile();
			}
			else
				setEnabled(OT_FullScreen, false);
		}
	}
}

void TPFileOps::setFileURL(const QUrl &url)
{
	setFileName(appUtils()->getCorrectPath(url));
}

void TPFileOps::setCanAddFile(const bool can_add)
{
	if (can_add != m_canAddFile) {
		m_canAddFile = can_add;
		emit canAddFileChanged();
		m_controls[OT_AddFile]->visible = canAddFile();
		resizeControl();
		recalculateButtonsRect();
		update();
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

QString TPFileOps::getFileTypeIcon(const QString &filename, const QSize &preferred_size, const bool thumbnail) const
{
	uint32_t ft{m_filetype & ~TPUtils::FT_TP_FORMATTED};
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
	case TPUtils::FT_IMAGE:				return thumbnail ? getImagePreviewFile(filename, preferred_size) : "image_preview"_L1;
	case TPUtils::FT_VIDEO:				return "video_preview"_L1;
	case TPUtils::FT_PDF:				return thumbnail ? getPDFPreviewFile(filename, preferred_size) : "pdf_preview"_L1;;
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
	if (m_filetype == TPUtils::FT_TEXT && QFile::exists(m_filename)) {
		QFile *text_file{appUtils()->openFile(m_filename)};
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

void TPFileOps::exportSlot(const QString &filepath)
{
	int ret_code(TP_RET_CODE_EXPORT_FAILED);
	QString export_filename{};
	if (!filepath.isEmpty()) {
		uint32_t ft{m_filetype & ~TPUtils::FT_TP_FORMATTED};
		export_filename = std::move("%1export_file_%2.txt"_L1.arg(appSettings()->currentUserDir(), QString::number(ft)));
		QFile export_file{export_filename};
		if (!fileStillInUse(export_filename))
			export_file.remove();

		switch (ft) {
		case TPUtils::FT_TP_USER_PROFILE:
			ret_code = appUserModel()->exportToFormattedFile(0, export_filename);
			break;
		case TPUtils::FT_TP_PROGRAM:
			if (QFile::exists(export_filename)) {
				if (export_filename.endsWith(QString::number(TPUtils::FT_TP_PROGRAM) % TPUtils::TP_FILE_EXTENSION)) {
					ret_code = TP_RET_CODE_EXPORT_OK;
					break;
				}
				else
					export_file.remove();
			}
			connect(appUserModel()->actualMesoModel(), &DBMesocyclesModel::mesoExported, this, [this,filepath]
															(const uint meso_idx, const QString& filename, const int return_code) {
				exportSlot(return_code == TP_RET_CODE_EXPORT_OK ? filepath : QString{});
			}, Qt::SingleShotConnection);
			appUserModel()->actualMesoModel()->exportToFormattedFile(m_mesoIdx, export_filename);
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
			ret_code = appExercisesList()->exportToFormattedFile(export_filename);
			break;
		default:
#ifndef QT_NO_DEBUG
			qDebug() << "Error! Trying to save/export a not TPApp file - TPFileOps::exportSlot(" << m_filename << ")";
#endif
			return;
		}
		if (ret_code == TP_RET_CODE_EXPORT_OK) {
			QFile file{filepath};
			if (!appUtils()->copyFile(export_filename, filepath, true, true))
				ret_code = TP_RET_CODE_EXPORT_FAILED;
		}
		if (ret_code == TP_RET_CODE_EXPORT_OK)
			export_filename = std::move(appUtils()->getFileName(filepath));
		else
			export_filename = std::move(tr("Could not save to: ") % appUtils()->getFileName(filepath));
	}
	else
		export_filename = std::move(tr("Operation canceled"));
	appItemManager()->displayMessageOnAppWindow(ret_code, export_filename);
}

void TPFileOps::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == acceptedMouseButtons()) {
		event->setAccepted(true);
		controlInfo* ci{controlFromMouseClick(event->position())};
		if (ci) {
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
	}
	else if (event->type() == QEvent::KeyRelease) {
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
			removeFile();
			break;
		default:
			return false;
		}
		return true; // Return true to stop the event from propagating
	}
	else
		return QObject::eventFilter(obj, event);
}

void TPFileOps::_doFileOperation(const OpType type)
{
	if (fileType() != TPUtils::FT_UNKNOWN && !QFile::exists(fileName())) {
		auto displayError = [this] (const int return_code) -> void {
			appItemManager()->displayMessageOnAppWindow(return_code);
#ifndef QT_NO_DEBUG
			qDebug() << "Failed to generate file " << fileName() << " from file type " << fileType();
#endif
		};

		const auto ret{generateFileFromType(type)};
		switch (ret) {
		case TP_RET_CODE_EXPORT_OK: break;
		case TP_RET_CODE_DEFERRED_ACTION:
		{
			std::shared_ptr<QMetaObject::Connection>conn{std::make_shared<QMetaObject::Connection>()};
			*conn = connect(this, &TPFileOps::_internalSignal, this, [this,type,ret,conn,displayError] (const int requestid, const int return_code) {
				if (requestid == ret) {
					disconnect(*conn);
					if (return_code == TP_RET_CODE_EXPORT_OK) {
						//file will not be generated this time, but everything else that this method does must still be carried out
						generateFileFromType(type);
						_doFileOperation(type);
					}
					else
						displayError(return_code);
				}
			});
			return;
		}
		default:
			displayError(ret);
			return;
		}
	}

	switch (type) {
	case OT_AddFile:			addFile();												break;
	case OT_FullScreen:			doFullScreen();											break;
	case OT_Download:			saveFileAs();											break;
	case OT_Share:				shareFile();											break;
	case OT_Forward:			sendFileTo(appUtils()->getFileName(fileName(), true));	break;
	case OT_ViewExternally:		openFile();												break;
	case OT_Delete:				removeFile();											break;
	default:																			break;
	}
}

int TPFileOps::generateFileFromType(const OpType type)
{
	int ret{TP_RET_CODE_EXPORT_FAILED};
	const bool formatted{type == OT_Download || type == OT_Share || type == OT_Forward};
	switch (fileType()) {
	case TPUtils::FT_TP_PROGRAM:
		m_filename = std::move(appUserModel()->actualMesoModel()->suggestedName(m_mesoIdx, formatted));
		if (!QFile::exists(fileName())) {
			ret = deferredActionId();
			auto conn{std::make_shared<QMetaObject::Connection>()};
			*conn = connect(appUserModel()->actualMesoModel(), &DBMesocyclesModel::mesoExported, this, [this,ret,conn]
															(const uint meso_idx, const QString& filename, const int return_code) {
				if (meso_idx == m_mesoIdx) {
					disconnect(*conn);
					emit _internalSignal(ret, return_code);
				}
			});
			if (!formatted)
				appUserModel()->actualMesoModel()->exportToFile(m_mesoIdx, fileName());
			else
				appUserModel()->actualMesoModel()->exportToFormattedFile(m_mesoIdx, fileName());
		}
		else
			ret = TP_RET_CODE_EXPORT_OK;
		break;
	case TPUtils::FT_TP_EXERCISES:
		m_filename = std::move(appExercisesList()->suggestedName(formatted));
		if (!QFile::exists(fileName())) {
			if (!formatted)
				ret = appExercisesList()->exportToFile(fileName());
			else
				ret = appExercisesList()->exportToFormattedFile(fileName());
		}
		else
			ret = TP_RET_CODE_EXPORT_OK;
		break;
	case TPUtils::FT_TP_WORKOUT_A:
	case TPUtils::FT_TP_WORKOUT_B:
	case TPUtils::FT_TP_WORKOUT_C:
	case TPUtils::FT_TP_WORKOUT_D:
	case TPUtils::FT_TP_WORKOUT_E:
	case TPUtils::FT_TP_WORKOUT_F:
		{
			DBExercisesModel *model{appUserModel()->actualMesoModel()->workoutForDay(m_mesoIdx, m_workoutCalendarDay)};
			if (model) {
				m_filename = std::move(model->suggestedName(formatted));
				if (!QFile::exists(fileName())) {
					if (!formatted)
						ret = model->exportToFile(fileName());
					else
						ret = model->exportToFormattedFile(fileName());
				}
				else
					ret = TP_RET_CODE_EXPORT_OK;
			}
		}
		break;
	default:
		qDebug() << "ERROR!!! File type set as " << fileType() << " but neither filename as given, nor a method provided to create the file";
	}
	if (ret == TP_RET_CODE_EXPORT_OK) {
		if (formatted)
			m_filetype |= TPUtils::FT_TP_FORMATTED;
		emit fileNameChanged();
	}
	return ret;
}

void TPFileOps::doFullScreen()
{
	m_fullscreen = !m_fullscreen;
	if (m_fullscreen) {
		appPagesListModel()->removeEventFilter();
		qApp->installEventFilter(this);
	}
	else {
		qApp->removeEventFilter(this);
		appPagesListModel()->reinstallEventFilter();
	}
	emit showFullScreen();
}

void TPFileOps::addFile()
{
	connect(appMainWindow(), SIGNAL(fileDialogClosed(QString)), this, SLOT(importSlot(QString)), Qt::SingleShotConnection);
	QMetaObject::invokeMethod(appMainWindow(), "chooseFileToOpen", Q_ARG(int, restrictedFileType() ? m_filetype : TPUtils::FT_OTHER));
}

void TPFileOps::saveFileAs()
{
	connect(appMainWindow(), SIGNAL(fileDialogClosed(QString)), this, SLOT(exportSlot(QString)), Qt::SingleShotConnection);
	QMetaObject::invokeMethod(appMainWindow(), "chooseFolderToSaveFile", Q_ARG(int, m_filetype), Q_ARG(QString, appUtils()->getFileName(fileName())));
}

void TPFileOps::shareFile()
{
#ifdef Q_OS_ANDROID
	appOsInterface()->shareFile(m_filename);
#else
	saveFileAs();
#endif
}

void TPFileOps::sendFileTo(const QString &message, QString userid)
{
	if (userid.isEmpty())
		QMetaObject::invokeMethod(appMainWindow(), "getUsersList", Q_RETURN_ARG(QString, userid));
	appMessagesManager()->sendFileTo(userid, fileName(), message);
}

void TPFileOps::openFile()
{
	appUtils()->viewOrOpenFile(m_filename);
}

void TPFileOps::removeFile(const bool bypass_confirmation)
{
	if (!bypass_confirmation || appSettings()->alwaysAskConfirmation()) {
		connect(appItemManager(), &QmlItemManager::generalMessagesPopupClicked, this, [this] (const uint8_t button) {
			if (button == 1)
				removeFile(true);
		}, Qt::SingleShotConnection);
		appItemManager()->displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE,
			appUtils()->string_strings({tr("Remove file?"), m_filename}, record_separator), Qt::AlignCenter,
			getFileTypeIcon(m_filename, QSize{appSettings()->itemExtraLargeHeight(),
														appSettings()->itemExtraLargeHeight()}), -1, tr("Yes"), tr("No"));
		return;
	}
	QFile::remove(m_filename);
	emit fileRemovalRequested();
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
		}
		_getDefaultImage(ci);
		ci->default_image = std::move(ci->default_image.scaled(m_buttonSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
		ci->current_image = &ci->default_image;
		ci->rect = QRect{button_x, buttons_padding, m_buttonSize.width(), m_buttonSize.height()};
		button_x += m_buttonSize.width() + buttons_padding;
	}
	resizeControl();
	update();
}

void TPFileOps::resizeControl()
{
	int n_visible_controls{0};
	for (int i{OT_AddFile}; i < OT_TypeCount; ++i) {
		controlInfo *ci{m_controls[i]};
		if (ci->visible) ++n_visible_controls;
	}
	setControlSize(QSize{n_visible_controls * (appSettings()->itemDefaultHeight() + buttons_padding) + buttons_padding,
						 appSettings()->itemDefaultHeight() + (2 * buttons_padding)});
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
	return appSettings()->currentUserDir() % TPUtils::previewImagesSubDir % QString::number(fnv1a_hash(source_filename)) %
		   size_template.arg(QString::number(preview_size.width()), QString::number(preview_size.height())) % ".jpg"_L1;
}

QString TPFileOps::getImagePreviewFile(const QString &image_filename, QSize preferred_size) const
{
	if (!QFile::exists(image_filename))
		return QString{};

	QImage thumbnail;
	if (preferred_size.isNull()) {
		thumbnail.load(image_filename);
		const QSize &img_size{thumbnail.size()};
		const int page_img_width_ratio{qFloor(img_size.width() / appSettings()->pageWidth())};
		if (page_img_width_ratio >= 1)
			preferred_size.setWidth(appSettings()->pageWidth() * 0.8);
		else
			preferred_size.setWidth(img_size.width());
		const auto ratio{static_cast<float>(img_size.height()) / img_size.width()};
		preferred_size.setHeight(preferred_size.width() * ratio);
	}
	const QString &preview_filename{previewFilename(image_filename, preferred_size)};
	if (!QFile::exists(preview_filename)) {
		if (thumbnail.isNull())
			thumbnail.load(image_filename);
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
	// QPainter::CompositionMode_SourceOver is often the default, but explicit is fine.
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


QString TPFileOps::getPDFPreviewFile(const QString &pdf_filename, QSize preferred_size) const
{
	if (QFile::exists(pdf_filename)) {
		if (preferred_size.isNull()) {
			preferred_size.setWidth(appSettings()->pageWidth());
			preferred_size.setHeight(appSettings()->pageHeight());
		}
		const QString &preview_filename{previewFilename(pdf_filename, preferred_size)};
		if (!QFile::exists(preview_filename)) {
			QPdfDocument *pdf_doc{new QPdfDocument{}};
			pdf_doc->load(pdf_filename);
			QPdfDocumentRenderOptions pdf_opts;
			pdf_opts.setRenderFlags(QPdfDocumentRenderOptions::RenderFlag::TextAliased | QPdfDocumentRenderOptions::RenderFlag::ImageAliased |
									QPdfDocumentRenderOptions::RenderFlag::PathAliased | QPdfDocumentRenderOptions::RenderFlag::OptimizedForLcd);
			QImage background_image{preferred_size, QImage::Format_ARGB32_Premultiplied};
			background_image.fill(Qt::white);
			const QImage &pdf_image{composeImages(background_image, pdf_doc->render(0, preferred_size, pdf_opts))};
			pdf_doc->deleteLater();
			if (!pdf_image.isNull())
				pdf_image.save(preview_filename, "JPG", 10);
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
	case OT_ViewExternally: ci->default_image.load(":/images/" % getFileTypeIcon(m_filename, m_buttonSize, false)); break;
	case OT_Delete: ci->default_image.load(":/images/remove.png"_L1); break;
	default: break;
	}
}

void TPFileOps::readTPFile()
{
	QFile *in_file{appUtils()->openFile(m_filename)};
	if (!in_file)
		return;

	const QString *identifier{nullptr};
	QString extra_identifier;

	uint32_t ft{m_filetype & ~TPUtils::FT_TP_FORMATTED};
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

	while (stream.readLineInto(&line)) {
		if (line.isEmpty())
			section_info.second.append(QChar{0x2029});
		if (line.contains("##"_L1)) {
			if (line.contains(*identifier)) {
				section_info.first = std::move(line.right(line.length() - identifier->length() -
																					TPUtils::STR_START_FORMATTED_EXPORT.length() - 1));
				section_info.second.clear();
			}
			else if (line.startsWith(TPUtils::STR_END_FORMATTED_EXPORT)) {
				m_tpFileInfo.insert(m_tpfileSections, section_info);
				++m_tpfileSections;
				if (identifier == &appUtils()->mesoFileIdentifier)
					identifier =  &appUtils()->splitFileIdentifier;
			}
		}
		else
			section_info.second.append(line % QChar{0x2029});
	}
	if (!m_tpFileInfo.isEmpty())
		emit tpFileSectionCountChanged();
	in_file->close();
	delete in_file;
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
	if (pos_in_line >= tb.position() + tb.length())
		emit setCursorPorsition(tb2.position() + tb2.length());
	else {
		auto line_length2{tb2.length()};
		if (line_length2 >= pos_in_line)
			emit setCursorPorsition(tb2.position() + pos_in_line);
		else
			emit setCursorPorsition(tb2.position() + tb2.length() - 1);
	}
}
