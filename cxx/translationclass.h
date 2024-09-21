#ifndef TRASLATIONCLASS_H
#define TRASLATIONCLASS_H

#include <QObject>

class QTranslator;

class TranslationClass : public QObject
{
Q_OBJECT

public:
	explicit TranslationClass();
	virtual ~TranslationClass() override;

	Q_INVOKABLE inline bool translatorOK() const { return mbOK; }
	Q_INVOKABLE void switchToLanguage(const QString& language);
	void selectLanguage();

private:
	QTranslator* mTranslator;
	bool mbOK;
};

#endif // TRASLATIONCLASS_H
