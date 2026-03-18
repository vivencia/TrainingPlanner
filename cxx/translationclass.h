#pragma once

#include "qml_singleton.h"

#include <QObject>

QT_FORWARD_DECLARE_CLASS(QTranslator)

class TranslationClass : public QObject
{

Q_OBJECT

public:
	explicit TranslationClass(QObject *parent = nullptr);

	Q_INVOKABLE void switchToLanguage(const QString &language, const bool write_config);
	Q_INVOKABLE QString language() const;

signals:
	void applicationLanguageChanged();

private:
	QTranslator *mTranslator;

	void selectLanguage();
	static TranslationClass *_app_tr;
	friend TranslationClass *appTr();
};

inline TranslationClass *appTr() { return TranslationClass::_app_tr; }

DECLARE_QML_NAMED_SINGLETON(TranslationClass, AppTr)
