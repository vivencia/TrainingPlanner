#include "usersettings.h"

#include "tpsettings.h"
#include "tputils.h"

#include <QColor>
#include <QFontInfo>
#include <QScreen>

enum ColorSchemes {
	Custom = 0,
	Dark,
	Light,
	Blue,
	Green,
	Red,
	Gray
};

const auto applyFontRatio = [] (const uint value, const qreal ratio) { return value * ratio; };

UserSettings::UserSettings(const QString &userid, QObject *parent)
	: QSettings{appSettings()->userDir(userid) + "config.ini"_L1, QSettings::NativeFormat, parent}, m_userId{userid}
{
	m_propertyNames.insert(USER_LOCALE_INDEX, std::move("userLocale"_L1));
	m_propertyNames.insert(THEME_STYLE_INDEX, std::move("widgetTheme"_L1));
	m_propertyNames.insert(COLOR_INDEX, std::move("mainColor"_L1));
	m_propertyNames.insert(LIGHT_COLOR_INDEX, std::move("lightColor"_L1));
	m_propertyNames.insert(DARK_COLOR_INDEX, std::move("darkColor"_L1));
	m_propertyNames.insert(PANE_COLOR_INDEX, std::move("paneColor"_L1));
	m_propertyNames.insert(SELECTED_COLOR_INDEX, std::move("selectedColor"_L1));
	m_propertyNames.insert(FONT_COLOR_INDEX, std::move("fontColor"_L1));
	m_propertyNames.insert(DISABLED_FONT_COLOR_INDEX, std::move("disabledFontColor"_L1));
	m_propertyNames.insert(COLOR_SCHEME_INDEX, std::move("colorScheme"_L1));
	m_propertyNames.insert(FONT_SIZE_INDEX, std::move("fontPixelSize"_L1));
	m_propertyNames.insert(WEIGHT_UNIT_INDEX, std::move("weightUnit"_L1));
	m_propertyNames.insert(EXERCISES_VERSION_INDEX, std::move("exercisesListVersion"_L1));
	m_propertyNames.insert(MESO_IDX_INDEX, std::move("lastViewedMesoIdx"_L1));
	m_propertyNames.insert(ASK_CONFIRMATION_INDEX, std::move("alwaysAskConfirmation"_L1));
	m_propertyNames.insert(WEATHER_CITIES_INDEX, std::move("weatherLocations"_L1));

	m_defaultValues.reserve(USER_SETTINGS_FIELD_COUNT);
	for(uint i{USER_LOCALE_INDEX}; i < USER_SETTINGS_FIELD_COUNT; ++i)
		m_defaultValues.append(QString{});

	m_defaultValues[APP_VERSION_INDEX] = std::move(TP_APP_VERSION);
	m_defaultValues[THEME_STYLE_INDEX] = std::move("Material"_L1);
	m_defaultValues[WEIGHT_UNIT_INDEX] = std::move("(kg)"_L1);
	m_defaultValues[MESO_IDX_INDEX] = std::move("-1"_L1);
	m_defaultValues[ASK_CONFIRMATION_INDEX] = std::move("1"_L1);

	m_colorSchemes.reserve(7);
	m_colorSchemes << std::move(tr("Custom(click on the regions of the rectangle on the right to change colors)")) <<
					std::move(tr("Dark")) << std::move(tr("Light")) << std::move(tr("Blue")) <<
					std::move(tr("Green")) << std::move(tr("Red")) << std::move(tr("Gray"));

	const QFontInfo fi{QGuiApplication::font()};
	setFontSize(value(m_propertyNames.value(FONT_SIZE_INDEX), applyFontRatio(fi.pixelSize(), appSettings()->fontRatio())).toUInt(), false);
	setColorScheme(value(m_propertyNames.value(COLOR_SCHEME_INDEX), 3).toUInt(), false);
}

void UserSettings::setUserLocale(const QString &locale)
{
	const qsizetype language_idx{appSettings()->availableLanguages().indexOf(locale)};
	if (language_idx >= 0)
	{
		setUserLocale(language_idx);
		appUtils()->setAppLocale(locale);
	}
}

void UserSettings::setUserLocale(const uint language_idx)
{
	m_languageIdx = language_idx;
	changeValue(USER_LOCALE_INDEX, appSettings()->availableLanguages().at(language_idx));
	emit userLocaleChanged();
}

void UserSettings::setColorScheme(const uint new_value, const bool bFromQml)
{
	QString colorLight, color, colorDark, paneBackColor, entrySelColor;

	m_defaultValues[LISTS_COLOR_1_INDEX] = std::move("#b4ccd8"_L1);
	m_defaultValues[LISTS_COLOR_2_INDEX] = std::move("#a0b7c1"_L1);
	color = std::move(colorForScheme(new_value));
	colorLight = std::move(lightColorForScheme(new_value));
	colorDark = std::move(darkColorForScheme(new_value));
	paneBackColor = std::move(paneColorForScheme(new_value));
	entrySelColor = std::move(selectedColorForScheme(new_value));

	switch (new_value)
	{
		case Custom:
		break;
		case Dark:
			setDisabledFontColor("e8e8e8"_L1);
			m_defaultValues[LISTS_COLOR_1_INDEX] = std::move("#c8e3f0"_L1);
			m_defaultValues[LISTS_COLOR_2_INDEX] = std::move("#d4f1ff"_L1);
		break;
		case Light:
			setFontColor("#1a28e7"_L1);
			setDisabledFontColor("#bdcae6"_L1);
		break;
		case Gray:
			setFontColor("000000"_L1);
			setDisabledFontColor("a8a8a8"_L1);
		break;
		default:
			setFontColor("#ffffff"_L1);
			setDisabledFontColor("#dcdcdc"_L1);
	}

	m_defaultValues[COLOR_SCHEME_INDEX] = std::move(QString::number(new_value));
	m_defaultValues[COLOR_INDEX] = std::move(color);
	m_defaultValues[LIGHT_COLOR_INDEX] = std::move(colorLight);
	m_defaultValues[DARK_COLOR_INDEX] = std::move(colorDark);
	m_defaultValues[PANE_COLOR_INDEX] = std::move(paneBackColor);
	m_defaultValues[SELECTED_COLOR_INDEX] = std::move(entrySelColor);

	changeValue(COLOR_SCHEME_INDEX, new_value);
	emit colorChanged();
}

QString UserSettings::primaryColor() const
{
	return colorScheme() == Custom ? value(m_propertyNames.value(COLOR_INDEX)).toString() : colorForScheme(colorScheme());
}

void UserSettings::setPrimaryColor(const QColor &color)
{
	changeValue(COLOR_INDEX, color.name());
	emit colorChanged();
}

QString UserSettings::primaryLightColor() const
{
	return colorScheme() == Custom ? value(m_propertyNames.value(LIGHT_COLOR_INDEX)).toString() : lightColorForScheme(colorScheme());
}

void UserSettings::setPrimaryLightColor(const QColor &color)
{
	changeValue(LIGHT_COLOR_INDEX, color.name());
	emit colorChanged();
}

QString UserSettings::primaryDarkColor() const
{
	return colorScheme() == Custom ? value(m_propertyNames.value(DARK_COLOR_INDEX)).toString() : darkColorForScheme(colorScheme());
}

void UserSettings::setPrimaryDarkColor(const QColor &color)
{
	changeValue(DARK_COLOR_INDEX, color.name());
	changeValue(PANE_COLOR_INDEX, color.lightness() <= 180 ? color.lighter(125).name() : color.darker(280).name());
	changeValue(SELECTED_COLOR_INDEX, color.lightness() <= 180 ? color.lighter(115).name() : color.darker(250).name());
	const QColor white {"#ffffff"_L1};
	const QColor black {"#000000"_L1};
	changeValue(FONT_COLOR_INDEX, color.lightness() <= 127 ? white.name() : black.name());
	changeValue(DISABLED_FONT_COLOR_INDEX, color.lightness() <= 127 ? white.darker(150).name() : black.lighter(115).name());
	emit colorChanged();
}

QString UserSettings::paneBackgroundColor() const
{
	return colorScheme() == Custom ? value(m_propertyNames.value(PANE_COLOR_INDEX)).toString() : paneColorForScheme(colorScheme());
}

QString UserSettings::entrySelectedColor() const
{
	return colorScheme() == Custom ? value(m_propertyNames.value(SELECTED_COLOR_INDEX)).toString() : selectedColorForScheme(colorScheme());
}

void UserSettings::setFontColor(const QColor &color)
{
	changeValue(FONT_COLOR_INDEX, color.name());
	emit colorChanged();
}

void UserSettings::setDisabledFontColor(const QColor &color)
{
	changeValue(DISABLED_FONT_COLOR_INDEX, color.name());
	emit colorChanged();
}

QString UserSettings::colorForScheme(const uint scheme) const
{
	switch (scheme)
	{
		case Custom: return value(m_propertyNames.value(COLOR_INDEX), m_defaultValues[COLOR_INDEX]).toString(); break;
		case Dark: return "#5d615f"_L1; break;
		case Light: return "#b8bec1"_L1; break;
		case Blue: return "#5c83a6"_L1; break;
		case Green: return "#698c5e"_L1; break;
		case Red: return "#cc695f"_L1; break;
		case Gray: return "#cccccc"_L1; break;
	}
	return QString {};
}

QString UserSettings::lightColorForScheme(const uint scheme) const
{
	switch (scheme)
	{
		case Custom: return value(m_propertyNames.value(LIGHT_COLOR_INDEX), m_defaultValues[LIGHT_COLOR_INDEX]).toString(); break;
		case Dark: return "#959595"_L1; break;
		case Light: return "#b1acac"_L1; break;
		case Blue: return "#76b0e1"_L1; break;
		case Green: return "#82a572"_L1; break;
		case Red: return "#df7460"_L1; break;
		case Gray: return "#f3f3f3"_L1; break;
	}
	return QString {};
}

QString UserSettings::darkColorForScheme(const uint scheme) const
{
	switch (scheme)
	{
		case Custom: return value(m_propertyNames.value(DARK_COLOR_INDEX), m_defaultValues[DARK_COLOR_INDEX]).toString(); break;
		case Dark: return "#000000"_L1; break;
		case Light: return "#e9e9ea"_L1; break;
		case Blue: return "#1e344a"_L1; break;
		case Green: return "#2a6949"_L1; break;
		case Red: return "#e34c3f"_L1; break;
		case Gray: return "#c1c1c1"_L1; break;
	}
	return QString{};
}

QString UserSettings::paneColorForScheme(const uint scheme) const
{
	switch (scheme)
	{
		case Custom: return value(m_propertyNames.value(PANE_COLOR_INDEX), m_defaultValues[PANE_COLOR_INDEX]).toString(); break;
		case Dark: return "#000000"_L1; break;
		case Light: return "#535354"_L1; break;
		case Blue: return "#25415c"_L1; break;
		case Green: return "#34835b"_L1; break;
		case Red: return "#ff7064"_L1; break;
		case Gray: return "#929299"_L1; break;
	}
	return QString{};
}

QString UserSettings::selectedColorForScheme(const uint scheme) const
{
	switch (scheme)
	{
		case Custom: return value(m_propertyNames.value(SELECTED_COLOR_INDEX), m_defaultValues[SELECTED_COLOR_INDEX]).toString(); break;
		case Dark: return "#6c6f73"_L1; break;
		case Light: return "#5d5d5e"_L1; break;
		case Blue: return "#223c55"_L1; break;
		case Green: return "#307954"_L1; break;
		case Red: return "#ff5b4d"_L1; break;
		case Gray: return "#65696c"_L1; break;
	}
	return QString{};
}

void UserSettings::setFontSize(const uint new_value, const bool bFromQml)
{
	const uint fontSizeSmall{static_cast<uint>(qFloor(static_cast<float>(new_value)*0.8))};
	const uint fontSizeLarge{static_cast<uint>(qFloor(static_cast<float>(new_value)*1.5))};
	const uint fontSizeExtraLarge{static_cast<uint>(new_value*2)};

	m_defaultValues[FONT_SIZE_INDEX] = std::move(QString::number(new_value));
	m_defaultValues[SMALLFONT_SIZE_INDEX] = std::move(QString::number(fontSizeSmall));
	m_defaultValues[LARGEFONT_SIZE_INDEX] = std::move(QString::number(fontSizeLarge));
	m_defaultValues[EXTRALARGEFONT_SIZE_INDEX] = std::move(QString::number(fontSizeExtraLarge));
	m_defaultValues[ITEM_DEFAULT_HEIGHT] = std::move(QString::number(static_cast<uint>(qCeil(static_cast<float>(new_value)*1.5))));

	if (bFromQml)
	{
		changeValue(FONT_SIZE_INDEX, QString::number(new_value));
		emit fontSizeChanged();
	}
}

QString UserSettings::weatherCity(const uint idx)
{
	if (m_weatherLocations.isEmpty())
		m_weatherLocations = std::move(value(m_propertyNames.value(WEATHER_CITIES_INDEX)).value<QStringList>());

	return idx < m_weatherLocations.count() ? appUtils()->getCompositeValue(0, m_weatherLocations.at(idx), record_separator) : QString();
}

QGeoCoordinate UserSettings::weatherCityCoordinates(const uint idx)
{
	if (m_weatherLocations.isEmpty())
		m_weatherLocations = std::move(value(m_propertyNames.value(WEATHER_CITIES_INDEX)).value<QStringList>());

	QGeoCoordinate coord;
	if (idx < m_weatherLocations.count())
	{
		coord.setLatitude(appUtils()->getCompositeValue(1, m_weatherLocations.at(idx), record_separator).toDouble());
		coord.setLongitude(appUtils()->getCompositeValue(2, m_weatherLocations.at(idx), record_separator).toDouble());
	}
	return coord;
}

void UserSettings::addWeatherCity(const QString &city, const QString &latitude, const QString &longitude)
{
	for (const auto &location: std::as_const(m_weatherLocations))
	{
		if (location.contains(city, Qt::CaseInsensitive))
			return;
	}
	m_weatherLocations.append(appUtils()->string_strings({city, latitude, longitude}, record_separator));
	changeValue(WEATHER_CITIES_INDEX, QVariant::fromValue(m_weatherLocations));
	emit weatherCitiesCountChanged();
}

void UserSettings::removeWeatherCity(const uint idx)
{
	m_weatherLocations.removeAt(idx);
	changeValue(WEATHER_CITIES_INDEX, QVariant::fromValue(m_weatherLocations));
	emit weatherCitiesCountChanged();
}
