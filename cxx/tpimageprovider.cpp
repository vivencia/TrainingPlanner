#include "tpimageprovider.h"

#include <QPainter>

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

	uint x(0), y(0);
	switch (id)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
			y = 5; break;
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			y = 145; break;
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
			y = 285; break;
		case 15:
		case 16:
		case 17:
		case 18:
		case 19:
			y = 425; break;
		case 20:
		case 21:
		case 22:
		case 23:
		case 24:
			y = 565; break;
	}
	switch (id)
	{
		case 0:
		case 5:
		case 10:
		case 15:
		case 20:
			x = 5; break;
		case 1:
		case 6:
		case 11:
		case 16:
		case 21:
			x = 145; break;
		case 2:
		case 7:
		case 12:
		case 17:
		case 22:
			x = 285; break;
		case 3:
		case 8:
		case 13:
		case 18:
		case 23:
			x = 425; break;
		case 4:
		case 9:
		case 14:
		case 19:
		case 24:
			x = 565; break;
	}
	return allAvatars.copy(QRect(x, y, avatarWidth, avatarHeight));
}
