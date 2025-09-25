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

using namespace Qt::Literals::StringLiterals;

#define DROP_SHADOW_EXTENT 5

TPImage::TPImage(QQuickItem *parent)
	: QQuickPaintedItem{parent}, m_imageToPaint{nullptr}, mDropShadow{true}, mbCanUpdate{true}, mbCanColorize{false}
{
	connect(this, &QQuickItem::enabledChanged, this, [&] () { checkEnabled(); });
	connect(this, &QQuickItem::heightChanged, this, [&] () { maybeResize(); });
	connect(appSettings(), &TPSettings::colorChanged, this, [&] () {
		if (mbCanColorize)
		{
			colorize(mImage, mImage);
			mbCanUpdate = true;
			checkEnabled(true);
		}
	});
}

void TPImage::setSource(const QString &source)
{
	if (!source.isEmpty())
	{
		mbCanColorize = false;
		QFileInfo img_file{source};
		if (img_file.isFile())
		{
			if (img_file.isReadable())
				mSource = source;
		}
		else if (source.startsWith("image://tpimageprovider"_L1))
		{
			mImage = std::move(tpImageProvider()->getAvatar(source));
			if (!mImage.isNull())
			{
				mSource = source;
				mSourceExtension = std::move("png"_L1);
				mNominalSize.setHeight(0);
				maybeResize(true);
				emit sourceChanged();
			}
			return;
		}
		else
		{
			mbCanColorize = true;
			if (source.endsWith("png"_L1))
				mSource = std::move(":/images/flat/"_L1 + source);
			else if (source.endsWith("svg"_L1))
			{
				mDropShadow = false;
				mSource = std::move(":/images/"_L1 + source);
			}
			else
			{
				mbCanColorize = false;
				mSource = std::move(":/images/"_L1 + source + ".png"_L1);
			}
		}

		if (mImage.load(mSource))
		{
			const qsizetype ext_idx{mSource.lastIndexOf('.')};
			mSourceExtension = ext_idx > 0 ? std::move(mSource.right(mSource.length()-ext_idx-1)) : std::move("png"_L1);
			if (mbCanColorize)
				colorize(mImage, mImage);
			maybeResize(true);
			emit sourceChanged();
		}
	}
}

void TPImage::setDropShadow(const bool drop_shadow)
{
	mDropShadow = drop_shadow;
	emit dropShadowChanged();
	checkEnabled(false);
}

void TPImage::saveToDisk(const QString &filename)
{
	if (mImage.isNull())
		return;
	QFileInfo img_info{filename};
	if (img_info.exists())
	{
		if (!QFile::remove(filename))
			return;
	}
	static_cast<void>(mImage.save(filename));
}

void TPImage::paint(QPainter *painter)
{
	if (!mbCanUpdate)
		return;
	if (!m_imageToPaint)
		return;

	QPointF center{boundingRect().center() - m_imageToPaint->rect().center()};

	if (center.x() < 0)
		center.setX(0);
	if (center.y() < 0)
		center.setY(0);

	painter->drawImage(center, *m_imageToPaint);
}

void TPImage::checkEnabled(const bool bCallUpdate)
{
	if (isEnabled())
	{
		if (!mDropShadow)
			m_imageToPaint = &mImage;
		else
		{
			if (mImageShadow.isNull())
				createDropShadowImage();
			m_imageToPaint = &mImageShadow;
		}
	}
	else
	{
		if (mImageDisabled.isNull())
			convertToGrayScale();
		m_imageToPaint = &mImageDisabled;
	}
	if (bCallUpdate)
		update();
}

void TPImage::scaleImage(const bool bCallUpdate)
{
	if (!mImage.isNull() && !mNominalSize.isEmpty())
	{
		mbCanUpdate = false;
		if (mDropShadow)
			mSize = mNominalSize - QSize{DROP_SHADOW_EXTENT, DROP_SHADOW_EXTENT};
		else
			mSize = mNominalSize - QSize{qCeil(mNominalSize.width()*0.05), qCeil(mNominalSize.height()*0.05)};
		mImage = std::move(mImage.scaled(mSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
		mImageDisabled = std::move(QImage{});
		mImageShadow = std::move(QImage{});
		mbCanUpdate = true;
		checkEnabled(bCallUpdate);
	}
}

void TPImage::maybeResize(const bool bForceResize)
{
	if (mSource.isEmpty())
		return;
	if (width() <= 0 || height() <= 0)
		return;

	if (mNominalSize.height() != height())
	{
		mNominalSize.setHeight(height());
		mNominalSize.setWidth(width());
		scaleImage(true);
	}
	else
	{
		if (bForceResize)
			scaleImage(true);
	}
}

void TPImage::convertToGrayScale()
{
	if (!mImage.isNull())
		grayScale(mImageDisabled, mImage);
}

void TPImage::createDropShadowImage()
{
	if (!mImage.isNull())
	{
		QGraphicsDropShadowEffect *shadowEffect{new QGraphicsDropShadowEffect()};
		shadowEffect->setOffset(DROP_SHADOW_EXTENT, DROP_SHADOW_EXTENT);
		shadowEffect->setBlurRadius(DROP_SHADOW_EXTENT);
		applyEffectToImage(mImageShadow, mImage, shadowEffect, DROP_SHADOW_EXTENT);
	}
}

void TPImage::applyEffectToImage(QImage &dstImg, const QImage &srcImg, QGraphicsEffect *effect, const int extent)
{
	QGraphicsScene scene;
	QGraphicsPixmapItem item;
	item.setPixmap(QPixmap::fromImage(srcImg));
	item.setGraphicsEffect(effect);
	scene.addItem(&item);
	dstImg = std::move(srcImg.scaled(mSize+QSize(extent*2, extent*2), Qt::KeepAspectRatio, Qt::SmoothTransformation));
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

void TPImage::colorize(QImage &dstImg, const QImage &srcImg)
{
	if (!srcImg.isNull())
	{
		mSize.setHeight(srcImg.height());
		mSize.setWidth(srcImg.width());
		const QColor color{appSettings()->fontColor()};
		QGraphicsColorizeEffect *colorEffect{new QGraphicsColorizeEffect()};
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
