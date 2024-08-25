#include "tpimage.h"
#include "tpimageprovider.h"

#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsEffect>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

TPImage::TPImage(QQuickItem* parent)
	: QQuickPaintedItem(parent), mSize(20, 20), mDropShadow(true), mbCanUpdate(true)
{
	connect(this, &QQuickItem::enabledChanged, this, [&] () { checkEnabled(); });
}

void TPImage::setSource(const QString& source)
{
	if (!source.isEmpty() && mSource != source)
	{
		if (source.contains(u"png"_qs))
			mSource = u":/images/"_qs + source;
		else
		{
			if (source.contains(u"provider"_qs))
			{
				mSource = source;
				mImage = tpImageProvider()->getAvatar(mSource);
				scaleImage(true);
				emit sourceChanged();
				return;
			}
			else
				mSource = u":/images/"_qs + source + u".png"_qs;
		}
		if (mImage.load(mSource))
		{
			scaleImage(true);
			emit sourceChanged();
		}
	}
}

void TPImage::setDropShadow(const bool drop_shadow)
{
	mDropShadow = drop_shadow;
	checkEnabled(false);
}

void TPImage::setImgSize(const int size)
{
	if (size != mSize.width())
	{
		mSize.setWidth(size);
		mSize.setHeight(size);
		scaleImage(false);
		emit imgSizeChanged();
	}
}

void TPImage::paint(QPainter* painter)
{
	if (!mbCanUpdate)
		return;

	QPointF center(boundingRect().center() - m_imageToPaint->rect().center());

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
	if (!mImage.isNull())
	{
		if (mSize != mImage.size())
		{
			mbCanUpdate = false;
			mImage = mImage.scaled(mSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
			mImageDisabled = QImage();
			mImageShadow = QImage();
			mbCanUpdate = true;
		}
		checkEnabled(bCallUpdate);
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
		QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect();
		shadowEffect->setOffset(5, 5);
		shadowEffect->setBlurRadius(5);
		applyEffectToImage(mImageShadow, mImage, shadowEffect, 5);
	}
}

void TPImage::applyEffectToImage(QImage& dstImg, const QImage& srcImg, QGraphicsEffect* effect, const int extent)
{
	QGraphicsScene scene;
	QGraphicsPixmapItem item;
	item.setPixmap(QPixmap::fromImage(srcImg));
	item.setGraphicsEffect(effect);
	scene.addItem(&item);
	dstImg = srcImg.scaled(srcImg.size()+QSize(extent*2, extent*2), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	dstImg.reinterpretAsFormat(QImage::Format_ARGB32);
	dstImg.fill(Qt::transparent);
	QPainter ptr(&dstImg);
	scene.render(&ptr, QRectF(), QRectF( -extent, -extent, dstImg.width(), dstImg.height()));
}

void TPImage::grayScale(QImage& dstImg, const QImage& srcImg)
{
	dstImg = srcImg.convertToFormat(srcImg.hasAlphaChannel() ? QImage::Format_ARGB32 : QImage::Format_RGB32);
	const uint imgHeight(dstImg.height());
	const uint imgWidth(dstImg.width());
	uint x(0), y(0), ci(0);
	QRgb pixel, *scanLine;
	for (; y < imgHeight; ++y)
	{
		scanLine = reinterpret_cast<QRgb*>(dstImg.scanLine(y));
		for (x = 0; x < imgWidth; ++x)
		{
			pixel = *scanLine;
			ci = static_cast<uint>(qGray(pixel));
			*scanLine = qRgba(ci, ci, ci, qAlpha(pixel)/3);
			++scanLine;
		}
	}
}

/*void TPImage::blurred(QImage& dstImg, const QImage& srcImg, const QRect& rect, const int radius, const bool alphaOnly)
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
