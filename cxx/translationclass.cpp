#include "translationclass.h"
#include "tpsettings.h"
#include "tputils.h"
#include "qmlitemmanager.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QSettings>
#include <QTranslator>

TranslationClass* TranslationClass::app_tr(nullptr);

TranslationClass::TranslationClass(QObject* parent)
	: QObject{parent}, mbOK(true)
{
	app_tr = this;
	mTranslator = new QTranslator{this};
	selectLanguage();
}

TranslationClass::~TranslationClass()
{
	delete mTranslator;
}

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
	{
		mbOK = mTranslator->load(u"tplanner.%1.qm"_s.arg(strLocale), u":/translations/"_s, u"qm"_s);
		if (mbOK)
			qApp->installTranslator(mTranslator);
	}
	appUtils()->setAppLocale(mbOK ? strLocale : u"en_US"_s);
}

void TranslationClass::switchToLanguage(const QString& language)
{
	QCoreApplication::removeTranslator(mTranslator);
	const bool bEnglish(language == u"en_US"_s);
	mbOK = bEnglish;
	if (!bEnglish)
		mbOK = mTranslator->load(u"tplanner.%1.qm"_s.arg(language), u":/translations/"_s, u"qm"_s);
	if (mbOK)
	{
		appUtils()->setAppLocale(language);
		if (!bEnglish)
			qApp->installTranslator(mTranslator);
		appQmlEngine()->retranslate();
	}
}
