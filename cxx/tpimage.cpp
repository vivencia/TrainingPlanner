#include "tpimage.h"

#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsDropShadowEffect>

TPImage::TPImage(QQuickItem* parent)
	: QQuickPaintedItem(parent), mDropShadow(false), mbCanUpdate(true)
{}

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
			if (mSize.isNull())
				setImgSize(20);
			else
				scaleImage();
			emit sourceChanged();
		}
		mbCanUpdate = true;
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
	if (mImage.isNull())
		return;

	if (isEnabled())
	{
		if (mDropShadow)
		{
			if (mImageEffect.isNull())
			{
				QGraphicsDropShadowEffect* e(new QGraphicsDropShadowEffect());
				e->setColor(QColor(40,40,40,245));
				e->setOffset(0,10);
				e->setBlurRadius(50);
				applyEffectToImage(e, 40);
			}
			m_imageToPaint = &mImageEffect;
		}
		else
			m_imageToPaint = &mImage;
	}
	else
	{
		if (mImageDisabled.isNull())
			//convertToGrayScale();
			mImageDisabled = mImage.convertedTo(mImage.format(), Qt::MonoOnly);
		m_imageToPaint = &mImageDisabled;
	}

	QPointF center(boundingRect().center() - m_imageToPaint->rect().center());

	if (center.x() < 0)
		center.setX(0);
	if (center.y() < 0)
		center.setY(0);
	painter->drawImage(center, *m_imageToPaint);
}

void TPImage::convertToGrayScale()
{
	// Convert to 32bit pixel format
	mImageDisabled = mImage.convertToFormat(mImage.hasAlphaChannel() ? QImage::Format_ARGB32 : QImage::Format_RGB32);

	unsigned int *data(reinterpret_cast<unsigned int*>(mImageDisabled.bits()));
	const uint pixelCount(mImageDisabled.width() * mImageDisabled.height());
	int val(0);

	// Convert each pixel to grayscale
	for(uint i(0); i < pixelCount; ++i)
	{
		val = qGray(*data);
		*data = qRgba(val, val, val, qAlpha(*data));
		++data;
	}
}

void TPImage::scaleImage()
{
	if (!mImage.isNull())
	{
	mbCanUpdate = false;
	mImage = mImage.scaled(mSize, Qt::KeepAspectRatio, Qt::FastTransformation);
	mImageDisabled = QImage();
	mImageEffect = QImage();
	mbCanUpdate = true;
	update();
	}
}

void TPImage::applyEffectToImage(QGraphicsEffect* effect, int extent)
{
	QGraphicsScene scene;
	QGraphicsPixmapItem item;
	item.setPixmap(QPixmap::fromImage(mImage));
	item.setGraphicsEffect(effect);
	scene.addItem(&item);
	QImage res(mImage.size() + QSize(extent*2, extent*2), QImage::Format_ARGB32);
	res.fill(Qt::transparent);
	QPainter ptr(&res);
	scene.render(&ptr, QRectF(), QRectF(-extent, -extent, mImage.width() + extent*2, mImage.height() + extent*2));
	mImageEffect = res;
}
