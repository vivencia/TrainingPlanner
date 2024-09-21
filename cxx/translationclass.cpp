#include "translationclass.h"
#include "tpappcontrol.h"
#include "tputils.h"
#include "qmlitemmanager.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QSettings>
#include <QTranslator>

TranslationClass::TranslationClass ()
{
	mbOK = true;
	mTranslator = new QTranslator(this);
}

TranslationClass::~TranslationClass()
{
	delete mTranslator;
}

void TranslationClass::selectLanguage()
{
	QString strLocale(appSettings()->value("appLocale").toString());
	const bool bConfigEmpty(strLocale.isEmpty());
	if (bConfigEmpty)
		strLocale = QLocale::system().name();
	if (strLocale != u"en_US"_qs)
	{
		mbOK = mTranslator->load(QStringLiteral("tplanner.%1.qm").arg(strLocale), QStringLiteral(":/translations/"), QStringLiteral("qm"));
		if (mbOK)
			qApp->installTranslator(mTranslator);
	}
	if (mbOK)
	{
		appUtils()->setAppLocale(strLocale);
		if (bConfigEmpty)
			appSettings()->setValue("appLocale", strLocale);
	}
	else
		appUtils()->setAppLocale(u"en_US"_qs); //If any part of the program calls RunCommands::appLocale() we will hava an error
}

void TranslationClass::switchToLanguage(const QString& language)
{
	QCoreApplication::removeTranslator(mTranslator);
	const bool bEnglish(language == u"en_US"_qs);
	mbOK = bEnglish;
	if (!bEnglish)
		mbOK = mTranslator->load(QStringLiteral("tplanner.%1.qm").arg(language), QStringLiteral(":/translations/"), QStringLiteral("qm"));
	if (mbOK)
	{
		appUtils()->setAppLocale(language);
		appSettings()->setValue("appLocale", language);
		if (!bEnglish)
			qApp->installTranslator(mTranslator);
		appQmlEngine()->retranslate();
	}
}
