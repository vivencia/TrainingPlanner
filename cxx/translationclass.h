#ifndef TRASLATIONCLASS_H
#define TRASLATIONCLASS_H

#include <QObject>
#include <QtGui>
#include <QSettings>

class QQmlEngine;

class TranslationClass : public QObject
{
Q_OBJECT

public:
	explicit TranslationClass(const QSettings& settingsObj);
	virtual ~TranslationClass() override;

	void selectLanguage();
	Q_INVOKABLE void switchToLanguage(const QString& language);
	inline void setQMLEngine(QQmlEngine* engine) { mQMLEngine = engine; }

private:
	QTranslator* mTranslator;
	QSettings* mSettingsObj;
	QQmlEngine* mQMLEngine;

	static TranslationClass* app_tr;
	friend TranslationClass* appTr();
};

inline TranslationClass* appTr() { return TranslationClass::app_tr; }

#endif // TRASLATIONCLASS_H
