#include "tpimageprovider.h"

#include <math.h>

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

	const int id(strid.toInt());
	if (id == -1)
		return mAllAvatars;
	else
		return getAvatar(static_cast<uint>(id));
}

QImage TPImageProvider::getAvatar(const QString& imagePath)
{
	const QString avatarId(imagePath.right(imagePath.length() - imagePath.lastIndexOf('/') - 1));
	bool bOK(false);
	const int id(avatarId.toUInt(&bOK));
	return bOK ? getAvatar(id) : QImage();
}

QImage TPImageProvider::getAvatar(const uint id)
{
	const uint x((id % 5) * avatarWidth);
	const uint y(::floor(id / 5) * avatarHeight);
	return mAllAvatars.copy(QRect(x, y, avatarWidth, avatarHeight));
}
