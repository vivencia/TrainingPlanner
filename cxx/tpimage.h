#ifndef TPIMAGE_H
#define TPIMAGE_H

#include <QQmlEngine>
#include <QQuickPaintedItem>
#include <QImage>

class TPImage : public QQuickPaintedItem
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
Q_PROPERTY(bool dropShadow READ dropShadow WRITE setDropShadow FINAL)
Q_PROPERTY(int imgSize READ imgSize WRITE setImgSize NOTIFY imgSizeChanged)

public:
	TPImage(QQuickItem* parent = nullptr);

	inline QString source() const { return mSource; }
	void setSource(const QString& source);
	inline bool dropShadow() const { return mDropShadow; }
	void setDropShadow(const bool drop_shadow);
	inline int imgSize() const { return mSize.width(); }
	void setImgSize(const int size);

	void paint(QPainter* painter);

public slots:
	void checkEnabled(const bool bCallUpdate = true);

signals:
	void sourceChanged();
	void imgSizeChanged();

private:
	QString mSource;
	QImage mImage;
	QImage mImageDisabled;
	QImage mImageShadow;
	QImage* m_imageToPaint;
	QSize mSize;
	bool mDropShadow;
	bool mbCanUpdate;

	void scaleImage();
	void convertToGrayScale();
	void createDropShadowImage();
};

#endif // TPIMAGE_H
