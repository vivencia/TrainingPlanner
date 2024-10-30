#include "tpsettings.h"
#include "tpglobals.h"

constexpr uint QML_PROPERTIES(25);

TPSettings* TPSettings::app_settings(nullptr);

TPSettings::TPSettings(QObject* parent) : QSettings{parent}
{
	TPSettings::app_settings = this;
	m_propertyNames.reserve(QML_PROPERTIES);
	m_propertyNames << std::move(u"appVersion"_s) << std::move(u"appLocale"_s) << std::move(u"widgetTheme"_s) << std::move(u"colorScheme"_s) <<
		std::move(u"primaryColor"_s) << std::move(u"primaryDarkColor"_s) << std::move(u"primaryLightColor"_s) << std::move(u"paneBackgroundColor"_s) <<
		std::move(u"entrySelectedColor"_s) << std::move(u"listEntryColor1"_s) << std::move(u"listEntryColor2"_s) << std::move(u"fontColor"_s) <<
		std::move(u"disabledFontColor"_s) << std::move(u"weightUnit"_s) << std::move(u"exercisesListVersion"_s) << std::move(u"iconFolder"_s) <<
		std::move(u"fontSize"_s) << std::move(u"fontSizeText"_s) << std::move(u"fontSizeLists"_s) << std::move(u"fontSizeTitle"_s) <<
		std::move(u"lastViewedMesoIdx"_s) << std::move(u"pageWidth"_s) << std::move(u"pageHeight"_s) << std::move(u"alwaysAskConfirmation"_s) <<
		std::move(u"mainUserConfigured"_s) << std::move(u"weatherCities"_s);

	m_defaultValues.reserve(QML_PROPERTIES);
	m_defaultValues << std::move(TP_APP_VERSION) << std::move(u""_s) << std::move(u"Material"_s) << std::move(u"Blue"_s) <<
		std::move(u"#25b5f3"_s) << std::move(u"#1976d2"_s) << std::move(u"#bbdefb"_s) << std::move(u"#1976d2"_s) << std::move(u"#6495ed"_s) <<
		std::move(u"#c8e3f0"_s) << std::move(u"#c3cad5"_s) << std::move(u"#ffffff"_s) << std::move(u"#dcdcdc"_s) << std::move(u"(kg)"_s) <<
		std::move(u"0"_s ) << std::move(u"white/"_s) << FONT_POINT_SIZE << FONT_POINT_SIZE_TEXT << FONT_POINT_SIZE_LISTS <<
		FONT_POINT_SIZE_TITLE << STR_MINUS_ONE << std::move(u"360"_s) << std::move(u"600"_s) << STR_ONE << STR_ZERO << std::move(u"São Paulo"_s);

	m_weatherCities = std::move(value(m_propertyNames.at(WEATHER_CITIES_INDEX)).value<QStringList>());
}

void TPSettings::setCurrentWeatherCity(const QString& city)
{
	const int idx(m_weatherCities.indexOf(city));
	if (idx == -1)
		m_weatherCities.prepend(city);
	else if (idx != 0)
		m_weatherCities.move(idx, 0);
	changeValue(WEATHER_CITIES_INDEX, QVariant::fromValue(m_weatherCities));
	emit weatherCitiesCountChanged();
}

void TPSettings::removeWeatherCity(const int index)
{
	if (index >= 0 && index < m_weatherCities.count())
	{
		m_weatherCities.removeAt(index);
		changeValue(WEATHER_CITIES_INDEX, QVariant::fromValue(m_weatherCities)) ;
		emit weatherCitiesCountChanged();
	}
}
