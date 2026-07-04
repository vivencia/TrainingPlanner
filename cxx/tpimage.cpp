#include "tpimage.h"

#include "tpimageprovider.h"
#include "tpsettings.h"

#include <QFileInfo>
#include <QPainter>
#include <QGraphicsColorizeEffect>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsEffect>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QScreen>

using namespace Qt::Literals::StringLiterals;

#define DROP_SHADOW_EXTENT 5

TPImage::TPImage(QQuickItem *parent) : QQuickPaintedItem{parent}
{
	connect(this, &QQuickItem::enabledChanged, this, [&] () {
		if (m_image.isNull())
			return;
		//Under some circumstances(only noted when the app was quitting and the TPImage was the sourceComponent of
		//a Loader on a TPButton) the QML engine would update the property of an already deleted TPImage because it
		//was deleted after QML engine sent the signal, and before we received the signal
		if (!appSettings()->appExiting())
			checkEnabled();
	});
	connect(this, &QQuickItem::heightChanged, this, [this] () {
		if (!m_image.isNull() && imageSizeFollowControlSize())
			scaleImage();
	});
	connect(this, &QQuickItem::widthChanged, this, [this] () {
		if (!m_image.isNull() && imageSizeFollowControlSize() && !m_imageSize.isValid())
			scaleImage();
	});
	connect(appSettings(), &TPSettings::colorChanged, this, [&] () {
		if (m_image.isNull() || !m_canColorize)
			return;
		checkEnabled();
	});
}

void TPImage::operator=(const QImage &other)
{
	m_image = other;
}

void TPImage::setSource(const QString &source)
{
	if (!source.isEmpty()) {
		QFileInfo img_file{source};
		if (img_file.isFile() && img_file.isReadable()) {
			m_source = source;
			if (m_image.load(m_source)) {
				m_canColorize = false;
				m_dropShadow = false;
				emit sourceChanged();
				scaleImage();
			}
		} else {
			m_aspectRatioMode = Qt::KeepAspectRatio;
			m_imageFollowControl = true;
			m_fullWindowView = false;
			m_canColorize = false;
			m_dropShadow = true;
			if (source.startsWith("image://tpimageprovider"_L1)) {
				m_image = std::move(tpImageProvider()->getAvatar(source));
				if (!m_image.isNull()) {
					m_source = source;
					scaleImage();
					emit sourceChanged();
				}
				return;
			} else {
				if (!source.contains('.')) {
					if (source.endsWith('_'))
						m_source = std::move(":/images/"_L1 % source % appSettings()->indexColorSchemeToColorSchemeName() % ".png"_L1);
					else
						m_source = std::move(":/images/"_L1 % source % ".png"_L1);
				} else {
					if (source.endsWith("png"_L1)) {
						m_canColorize = true;
						m_source = std::move(":/images/flat/"_L1 % source);
					} else if (source.endsWith("svg"_L1)) {
						m_canColorize = true;
						m_dropShadow = false;
						m_source = std::move(":/images/"_L1 % source);
					}
				}
			}
			if (m_image.load(m_source)) {
				scaleImage();
				emit sourceChanged();
			}
		}
	}
}

void TPImage::setDropShadow(const bool drop_shadow)
{
	m_dropShadow = drop_shadow;
	emit dropShadowChanged();
	checkEnabled();
}

void TPImage::setKeepAspectRatio(const bool keep_ar)
{
	if (m_aspectRatioMode != keep_ar) {
		m_aspectRatioMode = (keep_ar ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio);
		emit keepAspectRatioChanged();
		scaleImage();
	}
}

void TPImage::setImageSizeFollowControlSize(const bool follow)
{
	if (m_imageFollowControl != follow) {
		m_imageFollowControl = follow;
		emit imageSizeFollowControlSizeChanged();
		scaleImage();
	}
}

void TPImage::setFullWindowView(const bool fullview)
{
	if (m_fullWindowView != fullview) {
		m_fullWindowView = fullview;
		emit fullWindowViewChanged();
		scaleImage();
	}
}

double TPImage::preferredWidth() const
{
	if (m_image.isNull())
		return 0;
	return QImage{m_source}.width();
}

double TPImage::preferredHeight() const
{
	if (m_image.isNull())
		return 0;
	return QImage{m_source}.height();
}

void TPImage::setWScale(const double new_wscale)
{
	if (new_wscale != m_wscale) {
		m_wscale = new_wscale;
		emit imageScaleChanged();
	}
}

void TPImage::setHScale(const double new_hscale)
{
	if (new_hscale != m_hscale) {
		m_hscale = new_hscale;
		emit imageScaleChanged();
	}
}

void TPImage::saveToDisk(const QString &filename)
{
	if (m_image.isNull())
		return;
	QFileInfo img_info{filename};
	if (img_info.exists()) {
		if (!QFile::remove(filename))
			return;
	}
	static_cast<void>(m_image.save(filename));
}

void TPImage::paint(QPainter *painter)
{
	if (m_imageToPaint)
		painter->drawImage(m_paintOrigin, *m_imageToPaint);
}

void TPImage::scaleImage()
{
	if (m_image.isNull())
		return;
	if (imageSizeFollowControlSize()) {
		if (height() <= 0 || width() <= 0)
			return;

		m_imageSize = QSize{static_cast<int>(width()), static_cast<int>(height())};
		if (m_dropShadow)
			m_imageSize -= QSize{DROP_SHADOW_EXTENT, DROP_SHADOW_EXTENT};

		if (wScale() != 1.0)
			m_imageSize.rwidth() *= m_wscale;
		if (hScale() != 1.0)
			m_imageSize.rheight() *= m_hscale;
		m_image = std::move(m_image.scaled(m_imageSize, m_aspectRatioMode, Qt::SmoothTransformation));
		m_paintOrigin.setX((width() - m_image.width()) / 2);
		m_paintOrigin.setY((height() - m_image.height()) / 2);
	} else {
		if (!fullWindowView()) {
			m_imageSize = QSize{m_image.width(), m_image.height()};
		} else {
			const QScreen *screen{QGuiApplication::primaryScreen()};
			const QRect &screenGeometry{screen->availableGeometry()};
			const int s_width{screenGeometry.width()};
			const int s_height{screenGeometry.height()};
			if (m_image.width() > m_image.height()) {
				if (s_height > s_width)
					m_image = std::move(QImage{m_source}.transformed(QTransform{}.rotate(-270), Qt::SmoothTransformation));
			}
			m_imageSize = QSize{s_width, s_height};
			m_image = std::move(m_image.scaled(m_imageSize, m_aspectRatioMode, Qt::SmoothTransformation));
			m_imageToPaint = &m_image;
			setWidth(s_width);
			setHeight(s_height);
			m_paintOrigin.setX((s_width - m_image.width()) / 2);
			m_paintOrigin.setY((s_height - m_image.height()) / 2);
			update();
			emit imageSizeChanged();
			return;

		}
	}
	emit imageSizeChanged();
	m_imageDisabled = std::move(QImage{});
	m_imageShadow = std::move(QImage{});
	checkEnabled();
}

void TPImage::checkEnabled()
{
	if (isEnabled()) {
		if (m_canColorize)
			colorize(m_image, m_image, appSettings()->fontColor());
		if (!m_dropShadow) {
			m_imageToPaint = &m_image;
		} else {
			if (m_imageShadow.isNull())
				createDropShadowImage(m_image, m_imageShadow);
			m_imageToPaint = &m_imageShadow;
		}
	} else {
		if (m_imageDisabled.isNull())
			convertToGrayScale();
		m_imageToPaint = &m_imageDisabled;
	}
	update();
}

void TPImage::convertToGrayScale()
{
	if (!m_image.isNull())
		grayScale(m_image, m_imageDisabled);
}

void TPImage::grayScale(const QImage &source_img, QImage &dest_img)
{		
	dest_img = std::move(source_img.convertToFormat(source_img.hasAlphaChannel() ? QImage::Format_ARGB32 : QImage::Format_RGB32));
	const int imgHeight{dest_img.height()};
	const int imgWidth{dest_img.width()};
	QRgb pixel;
	for (uint y{0}; y < imgHeight; ++y) {
		QRgb *scanLine{reinterpret_cast<QRgb*>(dest_img.scanLine(y))};
		for (uint x{0}; x < imgWidth; ++x) {
			pixel = *scanLine;
			const uint ci{static_cast<uint>(qGray(pixel))};
			*scanLine = qRgba(ci, ci, ci, qAlpha(pixel)/3);
			++scanLine;
		}
	}
}

void TPImage::colorizeImage(QImage &source_img, const QColor &color)
{
	QPainter painter(&source_img);
	painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
	painter.fillRect(source_img.rect(), color);
	painter.end();
}

void TPImage::applyEffectToImage(const QImage &source_img, QImage &dest_img, QGraphicsEffect *effect, const QSize &size, const int extent)
{
	QGraphicsScene scene;
	QGraphicsPixmapItem item;
	item.setPixmap(QPixmap::fromImage(source_img));
	item.setGraphicsEffect(effect);
	scene.addItem(&item);
	dest_img = std::move(source_img.scaled(size + QSize{extent * 2, extent * 2}, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	dest_img.reinterpretAsFormat(QImage::Format_ARGB32);
	dest_img.fill(Qt::transparent);
	QPainter ptr{&dest_img};
	scene.render(&ptr, QRectF(-extent, -extent, dest_img.width(), dest_img.height()),
																				QRectF(-extent, -extent, dest_img.width(), dest_img.height()));
}

void TPImage::createDropShadowImage(const QImage &source_img, QImage &dest_img)
{
	if (!source_img.isNull()) {
		QGraphicsDropShadowEffect *shadowEffect{new QGraphicsDropShadowEffect};
		shadowEffect->setOffset(DROP_SHADOW_EXTENT, DROP_SHADOW_EXTENT);
		shadowEffect->setBlurRadius(DROP_SHADOW_EXTENT);
		applyEffectToImage(source_img, dest_img, shadowEffect, source_img.size(), DROP_SHADOW_EXTENT);
	}
}

void TPImage::colorize(const QImage &source_img, QImage &dest_img, const QColor& color)
{
	if (!source_img.isNull()) {
		QGraphicsColorizeEffect *colorEffect{new QGraphicsColorizeEffect};
		colorEffect->setColor(color);
		applyEffectToImage(source_img, dest_img, colorEffect, source_img.size());
	}
}
