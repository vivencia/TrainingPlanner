#include "tpsettings.h"
#include "tpglobals.h"

constexpr uint QML_PROPERTIES(25);

TPSettings* TPSettings::app_settings(nullptr);

TPSettings::TPSettings(QObject* parent) : QSettings{parent}
{
	TPSettings::app_settings = this;
	m_propertyNames.reserve(QML_PROPERTIES);
	m_propertyNames << u"appVersion"_s << u"appLocale"_s << u"widgetTheme"_s << u"colorScheme"_s << u"primaryColor"_s << u"primaryDarkColor"_s <<
		u"primaryLightColor"_s << u"paneBackgroundColor"_s << u"entrySelectedColor"_s << u"listEntryColor1"_s << u"listEntryColor2"_s <<
		u"fontColor"_s << u"disabledFontColor"_s << u"weightUnit"_s << u"exercisesListVersion"_s << u"iconFolder"_s << u"fontSize"_s <<
		u"fontSizeText"_s << u"fontSizeLists"_s << u"fontSizeTitle"_s << u"lastViewedMesoIdx"_s << u"pageWidth"_s << u"pageHeight"_s <<
		u"alwaysAskConfirmation"_s << u"mainUserConfigured"_s;

	m_defaultValues.reserve(QML_PROPERTIES);
	m_defaultValues << TP_APP_VERSION << u""_s << u"Material"_s << u"Blue"_s << u"#25b5f3"_s << u"#1976d2"_s << u"#bbdefb"_s <<
		 u"#1976d2"_s << u"#6495ed"_s << u"#c8e3f0"_s << u"#c3cad5"_s << u"white"_s << u"darkgray"_s << u"(kg)"_s << u"0"_s  << u"white/"_s <<
		 FONT_POINT_SIZE << FONT_POINT_SIZE_TEXT << FONT_POINT_SIZE_LISTS << FONT_POINT_SIZE_TITLE << STR_MINUS_ONE << u"300"_s << u"600"_s <<
		 STR_ONE << STR_ZERO;
}
