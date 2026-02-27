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

TPImage::TPImage(QQuickItem *parent)
	: QQuickPaintedItem{parent}, m_imageToPaint{nullptr}, m_wscale{1.0}, m_hscale{1.0}
{
	connect(this, &QQuickItem::enabledChanged, this, [&] () {
		if (m_image.isNull())
			return;
		//Under some circumstances(only noted when the app was quitting and the TPImage was the sourceComponent of
		//a Loader on a TPButton) the QML engine would update the property of an already deleted TPImage because it
		//was deleted after QML engine sent the signal
		if (!appSettings()->appExiting())
			checkEnabled();
	});
	connect(this, &QQuickItem::heightChanged, this, [this] () {
		if (!m_image.isNull())
			scaleImage();
	});
	connect(this, &QQuickItem::widthChanged, this, [this] () {
		if (!m_image.isNull() && !m_imageSize.isValid())
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
	if (!source.isEmpty())
	{
		QFileInfo img_file{source};
		if (img_file.isFile() && img_file.isReadable())
		{
			m_source = source;
			if (m_image.load(m_source))
			{
				m_canColorize = false;
				m_dropShadow = false;
				emit sourceChanged();
				if (m_imageFollowControl.has_value() && m_fullWindowView.has_value() && m_aspectRatioMode.has_value())
					scaleImage();
			}
		}
		else
		{
			m_aspectRatioMode = Qt::KeepAspectRatio;
			m_imageFollowControl = true;
			m_fullWindowView = false;
			m_canColorize = false;
			m_dropShadow = true;
			if (source.startsWith("image://tpimageprovider"_L1))
			{
				m_image = std::move(tpImageProvider()->getAvatar(source));
				if (!m_image.isNull())
				{
					m_source = source;
					scaleImage();
					emit sourceChanged();
				}
				return;
			}
			else
			{
				if (source.endsWith("png"_L1))
				{
					m_canColorize = true;
					m_source = std::move(":/images/flat/"_L1 % source);
				}
				else if (source.endsWith("svg"_L1))
				{
					m_canColorize = true;
					m_dropShadow = false;
					m_source = std::move(":/images/"_L1 % source);
				}
				else if (source.endsWith('_'))
					m_source = std::move(":/images/"_L1 % source % appSettings()->indexColorSchemeToColorSchemeName() % ".png"_L1);
				else
					m_source = std::move(":/images/"_L1 % source % ".png"_L1);
			}
			if (m_image.load(m_source))
			{
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
	m_aspectRatioMode = (keep_ar ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio);
	emit keepAspectRatioChanged();
	if (m_fullWindowView.has_value() && m_imageFollowControl && !m_image.isNull())
		scaleImage();
}

void TPImage::setImageSizeFollowControlSize(const bool follow)
{
	m_imageFollowControl = follow;
	emit imageSizeFollowControlSizeChanged();
	if (m_fullWindowView.has_value() && m_aspectRatioMode.has_value() && !m_image.isNull())
		scaleImage();
}

void TPImage::setFullWindowView(const bool fullview)
{
	m_fullWindowView = fullview;
	emit fullWindowViewChanged();
	if (m_imageFollowControl.has_value() && m_aspectRatioMode.has_value() && !m_image.isNull())
		scaleImage();
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
	if (new_wscale != m_wscale)
	{
		m_wscale = new_wscale;
		emit scaleChanged();
	}
}

void TPImage::setHScale(const double new_hscale)
{
	if (new_hscale != m_hscale)
	{
		m_hscale = new_hscale;
		emit scaleChanged();
	}
}

void TPImage::saveToDisk(const QString &filename)
{
	if (m_image.isNull())
		return;
	QFileInfo img_info{filename};
	if (img_info.exists())
	{
		if (!QFile::remove(filename))
			return;
	}
	static_cast<void>(m_image.save(filename));
}

void TPImage::paint(QPainter *painter)
{
	if (m_imageToPaint)
		painter->drawImage(QPoint{0, 0}, *m_imageToPaint);
}

void TPImage::scaleImage()
{
	if (imageSizeFollowControlSize())
	{
		if (height() <= 0 || width() <= 0)
			return;

		m_imageSize = QSize{static_cast<int>(width()), static_cast<int>(height())};
		if (m_dropShadow)
			m_imageSize -= QSize{DROP_SHADOW_EXTENT, DROP_SHADOW_EXTENT};

		if (wScale() != 1.0)
			m_imageSize.rwidth() *= m_wscale;
		if (hScale() != 1.0)
			m_imageSize.rheight() *= m_hscale;
		m_image = std::move(m_image.scaled(m_imageSize, m_aspectRatioMode.value(), Qt::SmoothTransformation));
	}
	else
	{
		if (!fullWindowView())
			m_imageSize = QSize{m_image.width(), m_image.height()};
		else
		{
			const QScreen *screen{QGuiApplication::primaryScreen()};
			const QRect &screenGeometry{screen->availableGeometry()};
			const int s_width{screenGeometry.width()};
			const int s_height{screenGeometry.height()};
			if (m_image.width() > m_image.height())
			{
				if (s_height > s_width)
					m_image = std::move(QImage{m_source}.transformed(QTransform{}.rotate(-270), Qt::SmoothTransformation));
			}
			m_imageSize = QSize{s_width, s_height};
			m_image = std::move(m_image.scaled(m_imageSize, m_aspectRatioMode.value(), Qt::SmoothTransformation));
			m_imageToPaint = &m_image;
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
	if (isEnabled())
	{
		if (m_canColorize)
			colorize(m_image, m_image, appSettings()->fontColor());
		if (!m_dropShadow)
			m_imageToPaint = &m_image;
		else
		{
			if (m_imageShadow.isNull())
				createDropShadowImage();
			m_imageToPaint = &m_imageShadow;
		}
	}
	else
	{
		if (m_imageDisabled.isNull())
			convertToGrayScale();
		m_imageToPaint = &m_imageDisabled;
	}
	update();
}

void TPImage::convertToGrayScale()
{
	if (!m_image.isNull())
		grayScale(m_imageDisabled, m_image);
}

void TPImage::createDropShadowImage()
{
	if (!m_image.isNull())
	{
		QGraphicsDropShadowEffect *shadowEffect{new QGraphicsDropShadowEffect};
		shadowEffect->setOffset(DROP_SHADOW_EXTENT, DROP_SHADOW_EXTENT);
		shadowEffect->setBlurRadius(DROP_SHADOW_EXTENT);
		applyEffectToImage(m_imageShadow, m_image, shadowEffect, DROP_SHADOW_EXTENT);
	}
}

void TPImage::applyEffectToImage(QImage &dstImg, const QImage &srcImg, QGraphicsEffect *effect, const int extent)
{
	QGraphicsScene scene;
	QGraphicsPixmapItem item;
	item.setPixmap(QPixmap::fromImage(srcImg));
	item.setGraphicsEffect(effect);
	scene.addItem(&item);
	dstImg = std::move(srcImg.scaled(m_imageSize + QSize{extent * 2, extent * 2}, m_aspectRatioMode.value(), Qt::SmoothTransformation));
	dstImg.reinterpretAsFormat(QImage::Format_ARGB32);
	dstImg.fill(Qt::transparent);
	QPainter ptr{&dstImg};
	scene.render(&ptr, QRectF(-extent, -extent, dstImg.width(), dstImg.height()),
								QRectF(-extent, -extent, dstImg.width(), dstImg.height()));
}

void TPImage::grayScale(QImage &dstImg, const QImage &srcImg)
{		
	dstImg = std::move(srcImg.convertToFormat(srcImg.hasAlphaChannel() ? QImage::Format_ARGB32 : QImage::Format_RGB32));
	const int imgHeight{dstImg.height()};
	const int imgWidth{dstImg.width()};
	QRgb pixel;
	for (uint y{0}; y < imgHeight; ++y)
	{
		QRgb *scanLine{reinterpret_cast<QRgb*>(dstImg.scanLine(y))};
		for (uint x{0}; x < imgWidth; ++x)
		{
			pixel = *scanLine;
			const uint ci{static_cast<uint>(qGray(pixel))};
			*scanLine = qRgba(ci, ci, ci, qAlpha(pixel)/3);
			++scanLine;
		}
	}
}

void TPImage::colorize(QImage &dstImg, const QImage &srcImg, const QColor& color)
{
	if (!srcImg.isNull())
	{
		QGraphicsColorizeEffect *colorEffect{new QGraphicsColorizeEffect};
		colorEffect->setColor(color);
		applyEffectToImage(dstImg, srcImg, colorEffect);
	}
}

/*void TPImage::blurred(QImage &dstImg, const QImage &srcImg, const QRect& rect, const int radius, const bool alphaOnly)
{
	dstImg = srcImg.convertToFormat(QImage::Format_ARGB32_Premultiplied);

	const uint tab[] = { 14, 10, 8, 6, 5, 5, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2 };
	const uint alpha((radius < 1)  ? 16 : (radius > 17) ? 1 : tab[radius-1]);
	const uint r1(rect.top());
	const uint r2(rect.bottom());
	const uint c1(rect.left());
	const uint c2(rect.right());
	const uint bpl(dstImg.bytesPerLine());

	uint rgba[4];
	unsigned char* p(nullptr);
	uint i1(0);
	uint i2(3);
	uint row(0), col(0), i(0), j(0);

	if (alphaOnly)
		i1 = i2 = (QSysInfo::ByteOrder == QSysInfo::BigEndian ? 0 : 3);

	for (col = c1; col <= c2; col++)
	{
		p = dstImg.scanLine(r1) + col * 4;
		for (i = i1; i <= i2; i++)
			rgba[i] = p[i] << 4;

		p += bpl;
		for (j = r1; j < r2; j++, p += bpl)
			for (i = i1; i <= i2; i++)
				p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
	}

	for (row = r1; row <= r2; row++)
	{
		p = dstImg.scanLine(row) + c1 * 4;
		for (i = i1; i <= i2; i++)
			rgba[i] = p[i] << 4;

		p += 4;
		for (j = c1; j < c2; j++, p += 4)
			for (i = i1; i <= i2; i++)
				p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
	}

	for (col = c1; col <= c2; col++)
	{
		p = dstImg.scanLine(r2) + col * 4;
		for (i = i1; i <= i2; i++)
			rgba[i] = p[i] << 4;

		p -= bpl;
		for (j = r1; j < r2; j++, p -= bpl)
			for (i = i1; i <= i2; i++)
				p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
	}

	for (row = r1; row <= r2; row++)
	{
		p = dstImg.scanLine(row) + c2 * 4;
		for (i = i1; i <= i2; i++)
			rgba[i] = p[i] << 4;

		p -= 4;
		for (j = c1; j < c2; j++, p -= 4)
			for (i = i1; i <= i2; i++)
				p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
	}
}*/
