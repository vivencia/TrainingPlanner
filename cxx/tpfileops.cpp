#include "tpfileops.h"
#include "dbusermodel.h"
#include "osinterface.h"
#include "pageslistmodel.h"
#include "qmlitemmanager.h"
#include "tpimage.h"
#include "tpsettings.h"

#include <QPainter>
#include <QtPdf/QPdfDocument>
#include <QQuickWindow>

// FNV-1a constants
const uint32_t FNV_PRIME_32{0x01000193};
const uint32_t FNV_OFFSET_BASIS_32{0x811c9dc5};

inline uint32_t fnv1a_hash(const QString& s) {
	uint32_t hash{FNV_OFFSET_BASIS_32};
	for (const auto c : s.toStdU16String()) {
		hash ^= static_cast<uint8_t>(c);
		hash *= FNV_PRIME_32;
	}
	return hash;
}

TPFileOps::TPFileOps(QQuickItem *parent)
	: QQuickPaintedItem{parent}, m_currentControl{nullptr}, m_qml_control_spacing{5}, m_qml_control_extra_height{10}, m_controls{nullptr}
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
	default:
		Q_UNREACHABLE();
	}
	m_pressedColor.setAlpha(100);
}

void TPFileOps::paint(QPainter *painter)
{
	if (painter->clipBoundingRect().width() == m_controlSize.width() && m_currentControl)
		painter->drawImage(painter->clipBoundingRect(), *(m_currentControl->current_image));
	else {
		for (const auto &ci : std::as_const(m_controls)) {
			if (ci)
				painter->drawImage(ci->rect, *(ci->current_image));
		}
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
	uint32_t ft{static_cast<uint32_t>(appUtils()->getFileType(filename)) & ~TPUtils::FT_TP_FORMATTED};
	switch (ft) {
	case TPUtils::FT_TP_USER_PROFILE:	return "user_preview.png"_L1;
	case TPUtils::FT_TP_PROGRAM:		return "meso_preview.png"_L1;
	case TPUtils::FT_TP_WORKOUT_A:
	case TPUtils::FT_TP_WORKOUT_B:
	case TPUtils::FT_TP_WORKOUT_C:
	case TPUtils::FT_TP_WORKOUT_D:
	case TPUtils::FT_TP_WORKOUT_E:
	case TPUtils::FT_TP_WORKOUT_F:		return "workout_preview.png"_L1;
	case TPUtils::FT_TP_EXERCISES:		return "exerciselist_preview.png"_L1;
	case TPUtils::FT_IMAGE:				return thumbnail ? getImagePreviewFile(filename, preferred_size) : "image_preview.png"_L1;
	case TPUtils::FT_VIDEO:				return "video_preview.png"_L1;
	case TPUtils::FT_PDF:				return thumbnail ? getPDFPreviewFile(filename, preferred_size) : "pdf_preview.png"_L1;;
	case TPUtils::FT_TEXT:				return "genreric_preview.png"_L1;
	case TPUtils::FT_OPEN_DOCUMENT:		return "odf_preview.png"_L1;
	case TPUtils::FT_MS_DOCUMENT:		return "docx_preview.png"_L1;
	case TPUtils::FT_OTHER:				return "generic_preview.png"_L1;
	case TPUtils::FT_UNKNOWN:
	default:							return "$error$"_L1;
	}
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
			QImage pdf_image{std::move(pdf_doc->render(0, preferred_size, pdf_opts))};
			pdf_doc->deleteLater();
			if (!pdf_image.isNull())
				pdf_image.save(preview_filename, "JPG", 10);
		}
		return preview_filename;
	}
	return QString{};
}

void TPFileOps::removeFileAnswer(const int button)
{
	if (button == 1)
		removeFile(false);
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
		ci->current_image = &ci->default_image;
		ci->pressed = false;
		update(ci->rect);

		switch (ci->type) {
		case OT_FullScreen:
			doFullScreen();
			break;
		case OT_Download:
			QMetaObject::invokeMethod(appMainWindow(), "chooseFolderToSave", Q_ARG(QString, appUtils()->getFileName(m_filename)));
			break;
		case OT_Share:
			appOsInterface()->shareFile(m_filename);
			break;
		case OT_Forward:
			{//TODO use TPMessagesManager
			QString userid;
			QMetaObject::invokeMethod(appMainWindow(), "getUsersList", Q_RETURN_ARG(QString, userid));
			appUserModel()->sendFileToUser(userid, m_filename);
			}
			break;
		case OT_ViewExternally:
			appUtils()->viewOrOpenFile(m_filename);
			break;
		case OT_Delete:
			removeFile();
			break;
		default:
			Q_UNREACHABLE();
		}
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
			emit multimediaKeyPressed(key_event->key());
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

void::TPFileOps::removeFile(const bool bypass_confirmation)
{
	if (!bypass_confirmation && appSettings()->alwaysAskConfirmation()) {
		connect(appMainWindow(), SIGNAL(generalMessagesPopupClicked(int)), this, SLOT(removeFileAnswer(int)), Qt::SingleShotConnection);
		QMetaObject::invokeMethod(appMainWindow(), "displayResultMessage", Q_ARG(QString, tr("Remove file?")), Q_ARG(QString, m_filename),
			Q_ARG(QString, getFileTypeIcon(m_filename, QSize{appSettings()->itemExtraLargeHeight(), appSettings()->itemExtraLargeHeight()})),
			Q_ARG(int, 0), Q_ARG(QString, tr("Yes")), Q_ARG(QString, tr("No")));
		return;
	}
	QFile::remove(m_filename);
	emit fileRemovalRequested();
}

void TPFileOps::createControls()
{
	if (width() > 0 && height() > 0) {
		const int new_height{static_cast<int>(height()) - m_qml_control_extra_height};
		if (new_height < 0 || new_height == m_controlSize.height())
			return;
		int new_width{qFloor((width() - 5 - static_cast<int>(OT_TypeCount) * m_qml_control_spacing) / static_cast<int>(OT_TypeCount))};
		if (new_width < 0)
			return;
		if (new_height < new_width) {
			new_width = new_height;
			m_qml_control_spacing = qFloor((width() - 5 - (static_cast<int>(OT_TypeCount) * new_width)) / static_cast<int>(OT_TypeCount));
		}
		else if (new_height > new_width) {
			new_width = new_height;
			m_qml_control_extra_height = height() - new_height;
		}

		m_controlSize.rwidth() = new_width;
		m_controlSize.rheight() = new_height;
		int control_x{qCeil((width() - static_cast<int>(OT_TypeCount) * (m_qml_control_spacing + new_width))/2) + qCeil(m_qml_control_spacing / 2)};
		for (int i{OT_FullScreen}; i < OT_TypeCount; ++i) {
			controlInfo *ci{m_controls[i]};
			if (!m_controls[i]) {
				ci = new controlInfo;
				m_controls[i] = ci;
				ci->type = static_cast<OpType>(i);
			}

			_getDefaultImage(ci);
			ci->default_image = std::move(ci->default_image.scaled(m_controlSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
			ci->current_image = &ci->default_image;
			ci->rect = QRect{control_x, m_qml_control_extra_height / 2, m_controlSize.width(), m_controlSize.height()};
			control_x += m_controlSize.width() + m_qml_control_spacing;
		}
		update();
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
	const QString &str_image_source{":/images/%1_"_L1 % appSettings()->indexColorSchemeToColorSchemeName() % ".png"_L1};
	switch (ci->type) {
	case OT_FullScreen: ci->default_image.load(str_image_source.arg("fullscreen")); break;
	case OT_Download: ci->default_image.load(str_image_source.arg("download")); break;
	case OT_Share: ci->default_image.load(str_image_source.arg("share")); break;
	case OT_Forward: ci->default_image.load(str_image_source.arg("forward")); break;
	case OT_ViewExternally: ci->default_image.load(":/images/" % getFileTypeIcon(m_filename, m_controlSize, false)); break;
	case OT_Delete: ci->default_image.load(":/images/remove.png"_L1); break;
	default: Q_UNREACHABLE();
	}
}
