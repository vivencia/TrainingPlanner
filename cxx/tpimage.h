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
Q_PROPERTY(bool keepAspectRatio READ keepAspectRatio WRITE setKeepAspectRatio NOTIFY keepAspectRatioChanged FINAL)
Q_PROPERTY(bool imageSizeFollowControlSize READ imageSizeFollowControlSize WRITE setImageSizeFollowControlSize NOTIFY imageSizeFollowControlSizeChanged FINAL)
Q_PROPERTY(bool fullWindowView READ fullWindowView WRITE setFullWindowView NOTIFY fullWindowViewChanged FINAL)
Q_PROPERTY(double preferredWidth READ preferredWidth CONSTANT FINAL)
Q_PROPERTY(double preferredHeight READ preferredHeight CONSTANT FINAL)
Q_PROPERTY(double wScale READ wScale WRITE setWScale NOTIFY scaleChanged FINAL)
Q_PROPERTY(double hScale READ hScale WRITE setHScale NOTIFY scaleChanged FINAL)

public:
	explicit TPImage(QQuickItem *parent = nullptr);

	inline QString source() const { return m_source; }
	void setSource(const QString &source);

	inline bool dropShadow() const { return m_dropShadow; }
	void setDropShadow(const bool drop_shadow);
	inline bool keepAspectRatio() const { return m_aspectRatioMode == Qt::KeepAspectRatio; }
	void setKeepAspectRatio(const bool keep_ar);
	inline bool imageSizeFollowControlSize() const { return m_imageFollowControl.has_value() ? m_imageFollowControl.value() : false; }
	void setImageSizeFollowControlSize(const bool follow);
	inline bool fullWindowView() const { return m_fullWindowView.has_value() ? m_fullWindowView.value() : false; }
	void setFullWindowView(const bool fullview);

	double preferredWidth() const;
	double preferredHeight() const;
	inline double wScale() const { return m_wscale; }
	void setWScale(const double new_wscale);
	inline double hScale() const { return m_hscale; }
	void setHScale(const double new_hscale);

	void saveToDisk(const QString &filename);
	void paint(QPainter *painter);

public slots:
	void checkEnabled();

signals:
	void sourceChanged();
	void dropShadowChanged();
	void keepAspectRatioChanged();
	void imageSizeFollowControlSizeChanged();
	void fullWindowViewChanged();
	void scaleChanged();

private:
	QString m_source;
	QImage m_image, m_imageDisabled, m_imageShadow;
	QImage *m_imageToPaint;
	QSize m_imageSize;
	bool m_dropShadow, m_canColorize;
	std::optional<bool> m_imageFollowControl, m_fullWindowView;
	std::optional<Qt::AspectRatioMode> m_aspectRatioMode;
	double m_wscale, m_hscale;

	void scaleImage();
	void convertToGrayScale();
	void createDropShadowImage();
	void grayScale(QImage &dstImg, const QImage &srcImg);
	void colorize(QImage &dstImg, const QImage &srcImg);
	void applyEffectToImage(QImage &dstImg, const QImage &srcImg, QGraphicsEffect *effect, const int extent = 0);
};
