#include "tpimage.h"

#include <QPainter>

TPImage::TPImage(QQuickItem* parent)
	: QQuickPaintedItem(parent), mDropShadow(true), mbCanUpdate(true)
{
	connect(this, &QQuickItem::enabledChanged, this, [&] () { checkEnabled(); });
}

void TPImage::checkEnabled(const bool bCallUpdate)
{
	if (isEnabled())
	{
		m_imageToPaint = &mImage;
		if (mDropShadow && mImageShadow.isNull())
			createDropShadowImage();
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

void TPImage::setSource(const QString& source)
{
	if (mSource != source)
	{
		mbCanUpdate = false;
		if (source.contains(u"png"_qs))
			mSource = u":/images/"_qs + source;
		else
			mSource = u":/images/"_qs + source + u".png"_qs;
		if (mImage.load(mSource))
		{
			m_imageToPaint = &mImage;
			if (mSize.isNull())
				setImgSize(20);
			else
				scaleImage();
			emit sourceChanged();
		}
		mbCanUpdate = true;
	}
}

void TPImage::setDropShadow(const bool drop_shadow)
{
	mDropShadow = drop_shadow;
	if (mDropShadow)
	{
		if (mImageShadow.isNull())
			createDropShadowImage();
	}
}

void TPImage::setImgSize(const int size)
{
	if (size != mSize.width())
	{
		mSize.setWidth(size);
		mSize.setHeight(size);
		scaleImage();
		emit imgSizeChanged();

	}
}

void TPImage::paint(QPainter* painter)
{
	if (!mbCanUpdate)
		return;
	if (m_imageToPaint->isNull())
		return;

	QPointF center(boundingRect().center() - m_imageToPaint->rect().center());

	if (center.x() < 0)
		center.setX(0);
	if (center.y() < 0)
		center.setY(0);
	painter->drawImage(center, *m_imageToPaint);
	if (isEnabled())
	{
		if (mDropShadow)
			painter->drawImage(center + QPointF(5, 5), mImageShadow);
	}
}

void TPImage::convertToGrayScale()
{
	mImageDisabled = mImage.convertToFormat(mImage.hasAlphaChannel() ? QImage::Format_ARGB32 : QImage::Format_RGB32);
	const uint imgHeight(mImageDisabled.height());
	const uint imgWidth(mImageDisabled.width());
	uint x(0), y(0), ci(0);
	QRgb pixel, *scanLine;
	for (; y < imgHeight; ++y)
	{
		scanLine = reinterpret_cast<QRgb*>(mImageDisabled.scanLine(y));
		for (x = 0; x < imgWidth; ++x)
		{
			pixel = *scanLine;
			ci = static_cast<uint>(qGray(pixel));
			*scanLine = qRgba(ci, ci, ci, qAlpha(pixel)/3);
			++scanLine;
		}
	}
}

void TPImage::scaleImage()
{
	if (!mImage.isNull())
	{
		mbCanUpdate = false;
		mImage = mImage.scaled(mSize, Qt::KeepAspectRatio, Qt::FastTransformation);
		mImageDisabled = QImage();
		mImageShadow = QImage();
		mbCanUpdate = true;
		checkEnabled(false);
	}
}

static void blurred(QImage& dstImg, const QImage& srcImg, const QRect& rect, int radius, bool alphaOnly = false)
{
	int tab[] = { 14, 10, 8, 6, 5, 5, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2 };
	int alpha = (radius < 1)  ? 16 : (radius > 17) ? 1 : tab[radius-1];

	dstImg = srcImg.convertToFormat(QImage::Format_ARGB32_Premultiplied);
	int r1 = rect.top();
	int r2 = rect.bottom();
	int c1 = rect.left();
	int c2 = rect.right();

	int bpl = dstImg.bytesPerLine();
	int rgba[4];
	unsigned char* p;

	int i1 = 0;
	int i2 = 3;

	if (alphaOnly)
		i1 = i2 = (QSysInfo::ByteOrder == QSysInfo::BigEndian ? 0 : 3);

	for (int col = c1; col <= c2; col++)
	{
		p = dstImg.scanLine(r1) + col * 4;
		for (int i = i1; i <= i2; i++)
			rgba[i] = p[i] << 4;

		p += bpl;
		for (int j = r1; j < r2; j++, p += bpl)
			for (int i = i1; i <= i2; i++)
				p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
	}

	for (int row = r1; row <= r2; row++)
	{
		p = dstImg.scanLine(row) + c1 * 4;
		for (int i = i1; i <= i2; i++)
			rgba[i] = p[i] << 4;

		p += 4;
		for (int j = c1; j < c2; j++, p += 4)
			for (int i = i1; i <= i2; i++)
				p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
	}

	for (int col = c1; col <= c2; col++)
	{
		p = dstImg.scanLine(r2) + col * 4;
		for (int i = i1; i <= i2; i++)
			rgba[i] = p[i] << 4;

		p -= bpl;
		for (int j = r1; j < r2; j++, p -= bpl)
			for (int i = i1; i <= i2; i++)
				p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
	}

	for (int row = r1; row <= r2; row++)
	{
		p = dstImg.scanLine(row) + c2 * 4;
		for (int i = i1; i <= i2; i++)
			rgba[i] = p[i] << 4;

		p -= 4;
		for (int j = c1; j < c2; j++, p -= 4)
			for (int i = i1; i <= i2; i++)
				p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
	}
}

void TPImage::createDropShadowImage()
{
	//blurred(mImageShadow, mImage, mImage.rect(), 5);
	mImageShadow = mImage.convertToFormat(mImage.hasAlphaChannel() ? QImage::Format_ARGB32 : QImage::Format_RGB32);
	const uint imgHeight(mImageShadow.height());
	const uint imgWidth(mImageShadow.width());
	uint x(0), y(0), ci(0), alpha(0);
	QRgb pixel, *scanLine;
	qDebug() << "-------------------------------";
	for (; y < imgHeight; ++y)
	{
		scanLine = reinterpret_cast<QRgb*>(mImageShadow.scanLine(y));
		for (x = 0; x < imgWidth; ++x)
		{
			if (x < imgWidth - 5 && y < imgHeight - 5)
				*scanLine = qRgba(0, 0, 0, 0);
			else
			{
				pixel = *scanLine;
				ci = static_cast<uint>(qGray(pixel));
				if (x == imgWidth - 5 || x == imgWidth - 1)
					alpha = qAlpha(pixel);
				else if (x == imgWidth - 4 || x == imgWidth - 2)
					alpha = qAlpha(pixel)/5;
				else if (x == imgWidth - 3)
					alpha = qAlpha(pixel)/10;
				else if (y == imgHeight - 5 || x == imgHeight - 1)
					alpha = qAlpha(pixel);
				else if (y == imgHeight - 4 || x == imgHeight - 2)
					alpha = qAlpha(pixel)/5;
				else
					alpha = qAlpha(pixel)/10;
				qDebug() << alpha;
				*scanLine = qRgba(ci, ci, ci, alpha);
			}
			++scanLine;
		}
	}
	qDebug() << "-------------------------------";
}
