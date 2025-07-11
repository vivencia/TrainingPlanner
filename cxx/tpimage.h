#ifndef TPIMAGE_H
#define TPIMAGE_H

#include <QQmlEngine>
#include <QQuickPaintedItem>
#include <QImage>

QT_FORWARD_DECLARE_CLASS(QGraphicsEffect);

class TPImage : public QQuickPaintedItem
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
Q_PROPERTY(bool dropShadow READ dropShadow WRITE setDropShadow NOTIFY dropShadowChanged FINAL)

public:
	explicit TPImage(QQuickItem *parent = nullptr);

	inline QString source() const { return mSource; }
	void setSource(const QString &source);
	inline const QString &sourceExtension() const { return mSourceExtension; }
	inline bool dropShadow() const { return mDropShadow; }
	void setDropShadow(const bool drop_shadow);

	void saveToDisk(const QString &filename);
	void paint(QPainter *painter);

public slots:
	void checkEnabled(const bool bCallUpdate = true);
	void maybeResize(const bool bForceResize = false);

signals:
	void sourceChanged();
	void dropShadowChanged();

private:
	QString mSource, mSourceExtension;
	QImage mImage;
	QImage mImageDisabled;
	QImage mImageShadow;
	QImage *m_imageToPaint;
	QSize mSize, mNominalSize;
	bool mDropShadow;
	bool mbCanUpdate;
	bool mbCanColorize;

	void scaleImage(const bool bCallUpdate);
	void convertToGrayScale();
	void createDropShadowImage();
	void grayScale(QImage &dstImg, const QImage &srcImg);
	void colorize(QImage &dstImg, const QImage &srcImg);
	void applyEffectToImage(QImage &dstImg, const QImage &srcImg, QGraphicsEffect *effect, const int extent = 0);
};

#endif // TPIMAGE_H
