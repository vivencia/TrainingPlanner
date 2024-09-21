#include "tpimageprovider.h"

#include <QtMath>

static const uint avatarWidth(140);
static const uint avatarHeight(140);
static const QString avatarsFile(u":/images/avatars.png"_qs);
TPImageProvider* TPImageProvider::mtpImageProvider(nullptr);

TPImageProvider::TPImageProvider()
	: QQuickImageProvider(QQuickImageProvider::Image, QQmlImageProviderBase::ForceAsynchronousImageLoading)
{
	mAllAvatars.load(avatarsFile);
	mtpImageProvider = this;
}

QImage TPImageProvider::requestImage(const QString& strid, QSize* size, const QSize&)
{
	if (size)
		*size = QSize(avatarWidth, avatarHeight);

	const uint id(strid.right(strid.length() - 1).toUInt());
	return getAvatar(static_cast<uint>(id), strid.left(1));
}

QImage TPImageProvider::getAvatar(const QString& imagePath)
{
	const QString avatarId(imagePath.right(imagePath.length() - imagePath.lastIndexOf('/') - 1));
	bool bOK(false);
	const int id(avatarId.right(avatarId.length() - 1).toUInt(&bOK));
	return bOK ? getAvatar(id, avatarId.left(1)) : QImage();
}

QImage TPImageProvider::getAvatar(const uint id, const QString& strSex)
{
	const uint x((id % 5) * avatarWidth);
	uint y(0);
	if (strSex == u"m"_qs)
	{
		if (id >= 5)
			y = qFloor((id+10) / 5) * avatarHeight;
		else
			y = qFloor((id+5) / 5) * avatarHeight;
	}
	else
	{
		if (id >= 10)
			y = qFloor((id+10) / 5) * avatarHeight;
		else if (id >= 5)
			y = qFloor((id+5) / 5) * avatarHeight;
		else
			y = 0;
	}
	return mAllAvatars.copy(QRect(x, y, avatarWidth, avatarHeight));
}
