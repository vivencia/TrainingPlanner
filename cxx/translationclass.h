#ifndef TRASLATIONCLASS_H
#define TRASLATIONCLASS_H

#include <QObject>
#include <QtGui>
#include <QSettings>

class TraslationClass : public QObject
{
Q_OBJECT
Q_PROPERTY( QString emptyString READ getEmptyString NOTIFY languageChanged )

public:
	explicit TraslationClass( const QSettings& settingsObj );
	QString getEmptyString();
	Q_INVOKABLE void selectLanguage();

signals:
	void languageChanged();

private:
	QTranslator *mTranslator;
	QSettings *mSettingsObj;
};

#endif // TRASLATIONCLASS_H
