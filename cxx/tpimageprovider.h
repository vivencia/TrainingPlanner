#ifndef TPIMAGEPROVIDER_H
#define TPIMAGEPROVIDER_H

#include <qqmlextensionplugin.h>
#include <qqmlengine.h>
#include <qquickimageprovider.h>
#include <QImage>
#include <QPainter>

class TPImageProvider : public QQuickImageProvider
{

public:
	TPImageProvider() : QQuickImageProvider(QQuickImageProvider::Pixmap, QQmlImageProviderBase::ForceAsynchronousImageLoading) {}

	QPixmap requestPixmap(const QString& strid, QSize* size, const QSize& requestedSize) override;
};

class ImageProviderExtensionPlugin : public QQmlEngineExtensionPlugin
{

Q_OBJECT
Q_PLUGIN_METADATA(IID QQmlEngineExtensionInterface_iid)

public:
	void initializeEngine(QQmlEngine* engine, const char* uri) override
	{
		Q_UNUSED(uri);
		engine->addImageProvider("TPImageProvider", new TPImageProvider);
	}
};

#endif // TPIMAGEPROVIDER_H
