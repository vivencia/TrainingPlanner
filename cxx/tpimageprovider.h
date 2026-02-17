#pragma once

#include <QImage>
#include <QQmlEngine>
#include <QQmlExtensionPlugin>
#include <QQuickImageProvider>
#include <QPainter>

using namespace Qt::Literals::StringLiterals;

class TPImageProvider : public QQuickImageProvider
{

public:
	explicit TPImageProvider();
	QImage requestImage(const QString &strid, QSize *size, const QSize &requestedSize) override;
	QImage getAvatar(const uint id, const QString &strSex);
	QImage getAvatar(const QString &imagePath);

private:
	QImage mAllAvatars;

	static TPImageProvider *mtpImageProvider;
	friend TPImageProvider *tpImageProvider();
};

class ImageProviderExtensionPlugin : public QQmlEngineExtensionPlugin
{

Q_OBJECT
Q_PLUGIN_METADATA(IID QQmlEngineExtensionInterface_iid)

public:
	void initializeEngine(QQmlEngine *engine, const char *uri) override
	{
		Q_UNUSED(uri);
		engine->addImageProvider("TPImageProvider"_L1, new TPImageProvider);
	}
};

inline TPImageProvider *tpImageProvider()
{
	return TPImageProvider::mtpImageProvider;
}
