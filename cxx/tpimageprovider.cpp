#include "tpimageprovider.h"

#include <QPainter>

#include <math.h>

QPixmap TPImageProvider::requestPixmap(const QString& strid, QSize* size, const QSize& requestedSize)
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
