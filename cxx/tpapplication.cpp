#include "tpapplication.h"

#include <QFileOpenEvent>
#include <QFile>

bool TPApplication::event(QEvent* event)
{
	//qDebug() << "#######   Caught event: " << event->type() << "   #######";
	if (event->type() == QEvent::FileOpen)
	{
		const QFileOpenEvent* openEvent(static_cast<QFileOpenEvent *>(event));
		const QUrl url(openEvent->url());
		if (url.isLocalFile())
			emit openFileRequested(url.toLocalFile());
	}
	return QApplication::event(event);
}
