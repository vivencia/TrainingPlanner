#include "translationclass.h"
#include "runcommands.h"

#include <QQmlEngine>

TranslationClass* TranslationClass::app_tr(nullptr);

TranslationClass::TranslationClass (const QSettings& settingsObj)
{
	app_tr = this;
	mTranslator = new QTranslator (this);
	mSettingsObj = const_cast<QSettings*>(&settingsObj);
}

TranslationClass::~TranslationClass()
{
	delete mTranslator;
}

void TranslationClass::selectLanguage()
{
	const QString strLocale(mSettingsObj->value("appLocale").toString());
	if (strLocale != QStringLiteral("en_US"))
	{
		if (mTranslator->load(QStringLiteral("tplanner.%1.qm").arg(strLocale), QStringLiteral(":/translations/"), QStringLiteral("qm")))
			qApp->installTranslator(mTranslator);
	}
}

void TranslationClass::switchToLanguage(const QString& language)
{
	QCoreApplication::removeTranslator(mTranslator);
	bool bEnglish(language == u"en_US"_qs);
	bool ok(bEnglish);
	if (!bEnglish)
		ok = mTranslator->load(QStringLiteral("tplanner.%1.qm").arg(language), QStringLiteral(":/translations/"), QStringLiteral("qm"));
	if (ok)
	{
		runCmd()->setAppLocale(language, true);
		if (!bEnglish)
			qApp->installTranslator(mTranslator);
		mQMLEngine->retranslate();
	}
}
