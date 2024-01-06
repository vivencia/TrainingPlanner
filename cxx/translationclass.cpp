#include "translationclass.h"

TranslationClass::TranslationClass (const QSettings& settingsObj )
{
	mTranslator = new QTranslator ( this );
	mSettingsObj = const_cast<QSettings*>( &settingsObj );
}

TranslationClass::~TranslationClass()
{
	delete mTranslator;
	this->QObject::~QObject();
}
void TranslationClass::selectLanguage()
{
	const QString strLocale( mSettingsObj->value( "appLocale" ).toString() );
	if ( strLocale != QString( "en_US" ) ) {
		if ( mTranslator->load( QString( "tplanner.%1.qm" ).arg(strLocale), ":/translations/", "qm")) {
			qApp->installTranslator ( mTranslator );
			emit languageChanged();
		}
	}
}
