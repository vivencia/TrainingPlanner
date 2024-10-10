#include "tpsettings.h"
#include "tpglobals.h"

constexpr uint QML_PROPERTIES(25);

TPSettings* TPSettings::app_settings(nullptr);

TPSettings::TPSettings(QObject* parent) : QSettings{parent}
{
	TPSettings::app_settings = this;
	m_propertyNames.reserve(QML_PROPERTIES);
	m_propertyNames << u"appVersion"_qs << u"appLocale"_qs << u"widgetTheme"_qs << u"colorScheme"_qs << u"primaryColor"_qs << u"primaryDarkColor"_qs <<
		u"primaryLightColor"_qs << u"paneBackgroundColor"_qs << u"entrySelectedColor"_qs << u"listEntryColor1"_qs << u"listEntryColor2"_qs <<
		u"fontColor"_qs << u"disabledFontColor"_qs << u"weightUnit"_qs << u"exercisesListVersion"_qs << u"iconFolder"_qs << u"fontSize"_qs <<
		u"fontSizeText"_qs << u"fontSizeLists"_qs << u"fontSizeTitle"_qs << u"lastViewedMesoIdx"_qs << u"pageWidth"_qs << u"pageHeight"_qs <<
		u"alwaysAskConfirmation"_qs << u"mainUserConfigured"_qs;

	m_defaultValues.reserve(QML_PROPERTIES);
	m_defaultValues << TP_APP_VERSION << u""_qs << u"Material"_qs << u"Blue"_qs << u"#25b5f3"_qs << u"#1976d2"_qs << u"#bbdefb"_qs <<
		 u"#1976d2"_qs << u"#6495ed"_qs << u"#c8e3f0"_qs << u"#c3cad5"_qs << u"white"_qs << u"darkgray"_qs << u"(kg)"_qs << u"0"_qs  << u"white/"_qs <<
		 FONT_POINT_SIZE << FONT_POINT_SIZE_TEXT << FONT_POINT_SIZE_LISTS << FONT_POINT_SIZE_TITLE << STR_MINUS_ONE << u"300"_qs << u"600"_qs <<
		 STR_ONE << STR_ZERO;
}
