#pragma once

#include <QObject>

QT_FORWARD_DECLARE_CLASS(QTranslator)

class TranslationClass : public QObject
{

Q_OBJECT

public:
	explicit inline TranslationClass(QObject *parent = nullptr)
		: QObject{parent}, mTranslator{nullptr}, mbOK{false}
	{
		app_tr = this;
		selectLanguage();
	}

	Q_INVOKABLE inline bool translatorOK() const { return mbOK; }
	Q_INVOKABLE void switchToLanguage(const QString &language);
	Q_INVOKABLE QString language() const;

signals:
	void applicationLanguageChanged();

private:
	QTranslator *mTranslator;
	bool mbOK;

	void selectLanguage();
	static TranslationClass *app_tr;
	friend TranslationClass *appTr();
};

inline TranslationClass *appTr() { return TranslationClass::app_tr; }
