#include "tpimageprovider.h"

#include <QPainter>
#include <QGraphicsColorizeEffect>

#include <math.h>

static QImage convertToGrayScale(const QImage &srcImage) {
	 // Convert to 32bit pixel format
	 QImage dstImage = srcImage.convertToFormat(srcImage.hasAlphaChannel() ?
			  QImage::Format_ARGB32 : QImage::Format_RGB32);

	 unsigned int *data = (unsigned int*)dstImage.bits();
	 int pixelCount = dstImage.width() * dstImage.height();

	 // Convert each pixel to grayscale
	 for(int i = 0; i < pixelCount; ++i) {
		int val = qGray(*data);
		*data = qRgba(val, val, val, qAlpha(*data));
		++data;
	 }

	 return dstImage;
  }

  #include <QGraphicsScene>
#include <QGraphicsPixmapItem>
 static QImage applyEffectToImage(QImage& src, QGraphicsEffect *effect, int extent=0)
{
	if(src.isNull()) return QImage();   //No need to do anything else!
	if(!effect) return src;             //No need to do anything else!
	QGraphicsScene scene;
	QGraphicsPixmapItem item;
	item.setPixmap(QPixmap::fromImage(src));
	item.setGraphicsEffect(effect);
	scene.addItem(&item);
	QImage res(src.size()+QSize(extent*2, extent*2), QImage::Format_ARGB32);
	res.fill(Qt::transparent);
	QPainter ptr(&res);
	scene.render(&ptr, QRectF(), QRectF( -extent, -extent, src.width()+extent*2, src.height()+extent*2 ) );
	return res;
}

QPixmap TPImageProvider::requestPixmap(const QString& strid, QSize* size, const QSize&)
{
	bool bOK(false);
	static_cast<void>(strid.toUInt(&bOK));
	if (bOK)
	{
		const uint avatarWidth(140);
		const uint avatarHeight(140);
		if (size)
			*size = QSize(avatarWidth, avatarHeight);

		QPixmap allAvatars(u":/images/avatars.png"_qs);
		const int id(strid.toInt());
		if (id == -1)
			return allAvatars;

		const uint x((id % 5) * avatarWidth);
		const uint y(::floor(id / 5) * avatarHeight);
		return allAvatars.copy(QRect(x, y, avatarWidth, avatarHeight));
	}

	QString strSource;
	if (strid.contains(u"png"_qs))
		strSource = u":/images/"_qs + strid;
	else
		strSource = u":/images/"_qs + strid + u".png"_qs;

	QImage srcImg(strSource);
	if (size)
		*size = QSize(srcImg.size());
	qDebug() << srcImg.size();
	QGraphicsDropShadowEffect *e = new QGraphicsDropShadowEffect;
	e->setColor(QColor(40,40,40,245));
	e->setOffset(0,10);
	e->setBlurRadius(50);
	return QPixmap::fromImage(applyEffectToImage(srcImg, e, 40));

}

