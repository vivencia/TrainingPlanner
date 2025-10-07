#include "translationclass.h"

#include "qmlitemmanager.h"
#include "tpsettings.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QTranslator>

using namespace Qt::Literals::StringLiterals;

TranslationClass *TranslationClass::app_tr{nullptr};

void TranslationClass::selectLanguage()
{
	QString strLocale{appSettings()->userLocale()};
	if (strLocale.isEmpty())
	{
		#ifndef Q_OS_ANDROID
		const QString &sysLocale{std::setlocale(LC_NAME, "")};
		strLocale = std::move(sysLocale.first(sysLocale.indexOf('.')));
		#else
		strLocale = std::move(QLocale::system().name());
		#endif
	}
	if (strLocale != "en_US"_L1)
		switchToLanguage(strLocale, false);
	else
		appSettings()->setUserLocale("en_US"_L1, false);
}

void TranslationClass::switchToLanguage(const QString &language, const bool write_config)
{
	if (mTranslator)
	{
		QCoreApplication::removeTranslator(mTranslator);
		delete mTranslator;
		mTranslator = nullptr;
	}
	if (language != "en_US"_L1)
	{
		mTranslator = new QTranslator{this};
		if (mTranslator->load("tplanner.%1.qm"_L1.arg(language), ":/translations/"_L1, "qm"_L1))
			qApp->installTranslator(mTranslator);	
	}
	if (appQmlEngine())
		appQmlEngine()->retranslate();
	appSettings()->setUserLocale(language, write_config);
	emit applicationLanguageChanged();
}

QString TranslationClass::language() const
{
	return mTranslator->language();
}
