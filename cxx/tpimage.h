#pragma once

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
Q_PROPERTY(double wScale READ wScale WRITE setWScale NOTIFY scaleChanged FINAL)
Q_PROPERTY(double hScale READ hScale WRITE setHScale NOTIFY scaleChanged FINAL)

public:
	explicit TPImage(QQuickItem *parent = nullptr);

	inline QString source() const { return mSource; }
	void setSource(const QString &source);
	inline const QString &sourceExtension() const { return mSourceExtension; }

	inline bool dropShadow() const { return mDropShadow; }
	void setDropShadow(const bool drop_shadow);
	inline double wScale() const { return m_wscale; }
	void setWScale(const double new_wscale);
	inline double hScale() const { return m_hscale; }
	void setHScale(const double new_hscale);

	void saveToDisk(const QString &filename);
	void paint(QPainter *painter);

public slots:
	void checkEnabled(const bool bCallUpdate = true);
	void maybeResize(const bool bForceResize = false);

signals:
	void sourceChanged();
	void dropShadowChanged();
	void scaleChanged();

private:
	QString mSource, mSourceExtension;
	QImage mImage;
	QImage mImageDisabled;
	QImage mImageShadow;
	QImage *m_imageToPaint;
	QSize mSize, mNominalSize;
	Qt::AspectRatioMode m_arm;
	bool mDropShadow;
	bool mbCanUpdate;
	bool mbCanColorize;
	double m_wscale, m_hscale;

	void scaleImage(const bool bCallUpdate);
	void convertToGrayScale();
	void createDropShadowImage();
	void grayScale(QImage &dstImg, const QImage &srcImg);
	void colorize(QImage &dstImg, const QImage &srcImg);
	void applyEffectToImage(QImage &dstImg, const QImage &srcImg, QGraphicsEffect *effect, const int extent = 0);
};
