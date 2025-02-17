#include "translationclass.h"
#include "tpsettings.h"
#include "tputils.h"
#include "qmlitemmanager.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QSettings>
#include <QTranslator>

using namespace Qt::Literals::StringLiterals;

TranslationClass* TranslationClass::app_tr{nullptr};

void TranslationClass::selectLanguage()
{
	QString strLocale{appSettings()->appLocale()};
	const bool bConfigEmpty{strLocale.isEmpty()};
	if (bConfigEmpty)
	{
		#ifndef Q_OS_ANDROID
		const QString &sysLocale{std::setlocale(LC_NAME, "")};
		strLocale = sysLocale.first(sysLocale.indexOf('.'));
		#else
		strLocale = QLocale::system().name();
		#endif
	}
	if (strLocale != "en_US"_L1)
		switchToLanguage(strLocale);
	else
		appUtils()->setAppLocale("en_US"_L1, false);
}

void TranslationClass::switchToLanguage(const QString& language)
{
	if (language == appUtils()->strLocale())
		return;

	if (mTranslator)
	{
		QCoreApplication::removeTranslator(mTranslator);
		delete mTranslator;
		mTranslator = nullptr;
	}
	const bool bEnglish{language == "en_US"_L1};
	mbOK = bEnglish;
	if (!bEnglish)
	{
		mTranslator = new QTranslator{this};
		mbOK = mTranslator->load("tplanner.%1.qm"_L1.arg(language), ":/translations/"_L1, "qm"_L1);
		if (mbOK)
			qApp->installTranslator(mTranslator);	
	}
	if (appQmlEngine())
		appQmlEngine()->retranslate();
	emit applicationLanguageChanged();
	appUtils()->setAppLocale(language, mbOK);
}
