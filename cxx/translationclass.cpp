#include "translationclass.h"

TranslationClass::TranslationClass (const QSettings& settingsObj )
{
	mTranslator = new QTranslator ( this );
	mSettingsObj = const_cast<QSettings*>( &settingsObj );
}

TranslationClass::~TranslationClass()
{
	delete mTranslator;
}

void TranslationClass::selectLanguage()
{
	const QString strLocale( mSettingsObj->value( "appLocale" ).toString() );
	if ( strLocale != QStringLiteral( "en_US" ) ) {
		if ( mTranslator->load( QStringLiteral( "tplanner.%1.qm" ).arg(strLocale), QStringLiteral(":/translations/"), QStringLiteral("qm"))) {
			qApp->installTranslator ( mTranslator );
			emit languageChanged();
		}
	}
}
