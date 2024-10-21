#ifndef TRASLATIONCLASS_H
#define TRASLATIONCLASS_H

#include <QObject>

class QTranslator;

class TranslationClass : public QObject
{
Q_OBJECT

public:
	explicit TranslationClass(QObject* parent = nullptr);
	inline TranslationClass(const TranslationClass& other)
		: QObject(other.parent()), mTranslator(other.mTranslator), mbOK(other.mbOK) {}
	~TranslationClass();

	Q_INVOKABLE inline bool translatorOK() const { return mbOK; }
	Q_INVOKABLE void switchToLanguage(const QString& language);
	void selectLanguage();

private:
	QTranslator* mTranslator;
	bool mbOK;

	static TranslationClass* app_tr;
	friend TranslationClass* appTr();
};
Q_DECLARE_METATYPE(TranslationClass*)

inline TranslationClass* appTr() { return TranslationClass::app_tr; }

#endif // TRASLATIONCLASS_H
