#ifndef TRASLATIONCLASS_H
#define TRASLATIONCLASS_H

#include <QObject>
#include <QtGui>
#include <QSettings>

class TranslationClass : public QObject
{
Q_OBJECT

public:
	explicit TranslationClass();
	virtual ~TranslationClass() override;

	Q_INVOKABLE inline bool translatorOK() const { return mbOK; }
	void selectLanguage();
	Q_INVOKABLE void switchToLanguage(const QString& language);

private:
	QTranslator* mTranslator;

	bool mbOK;
	static TranslationClass* app_tr;
	friend TranslationClass* appTr();
};

inline TranslationClass* appTr() { return TranslationClass::app_tr; }

#endif // TRASLATIONCLASS_H
