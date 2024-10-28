#include "translationclass.h"
#include "tpsettings.h"
#include "tputils.h"
#include "qmlitemmanager.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QSettings>
#include <QTranslator>

TranslationClass* TranslationClass::app_tr(nullptr);

void TranslationClass::selectLanguage()
{
	QString strLocale{appSettings()->appLocale()};
	const bool bConfigEmpty(strLocale.isEmpty());
	if (bConfigEmpty)
	{
		#ifndef Q_OS_ANDROID
		const QString& sysLocale{std::setlocale(LC_NAME, "")};
		strLocale = sysLocale.first(sysLocale.indexOf('.'));
		#else
		strLocale = QLocale::system().name();
		#endif
	}
	if (strLocale != u"en_US"_s)
		switchToLanguage(strLocale);
	else
		appUtils()->setAppLocale(u"en_US"_s, false);
}

void TranslationClass::switchToLanguage(const QString& language)
{
	if (language == appUtils()->strLocale())
		return;

	if (mTranslator)
	{
		QCoreApplication::removeTranslator(mTranslator);
		delete mTranslator;
	}
	mTranslator = new QTranslator(this);

	const bool bEnglish(language == u"en_US"_s);
	mbOK = bEnglish;
	if (!bEnglish)
	{
		mbOK = mTranslator->load(u"tplanner.%1.qm"_s.arg(language), u":/translations/"_s, u"qm"_s);
		if (mbOK)
		{
			qApp->installTranslator(mTranslator);
			if (appQmlEngine())
				appQmlEngine()->retranslate();
			emit applicationLanguageChanged();
		}
	}
	appUtils()->setAppLocale(language, mbOK);
}
