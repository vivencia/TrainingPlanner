#ifndef TRASLATIONCLASS_H
#define TRASLATIONCLASS_H

#include <QObject>
#include <QtGui>
#include <QSettings>

class TranslationClass : public QObject
{
Q_OBJECT
Q_PROPERTY( QString emptyString READ getEmptyString NOTIFY languageChanged )

public:
	explicit TranslationClass( const QSettings& settingsObj );
	virtual ~TranslationClass () override;

	inline QString getEmptyString() { return QString(); }
	Q_INVOKABLE void selectLanguage();

signals:
	void languageChanged();

private:
	QTranslator *mTranslator;
	QSettings *mSettingsObj;
};

#endif // TRASLATIONCLASS_H
