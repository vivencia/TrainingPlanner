#include "tpsettings.h"

#include "tputils.h"
#include "tpglobals.h"

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

TPSettings *TPSettings::app_settings{nullptr};

TPSettings::TPSettings(QObject *parent) : QSettings{parent}
{
	TPSettings::app_settings = this;

	m_propertyNames.insert(APP_VERSION_INDEX, std::move("appVersion"_L1));
	m_propertyNames.insert(APP_LOCALE_INDEX, std::move("appLocale"_L1));
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
	m_propertyNames.insert(SERVER_ADDRESS, std::move("serverAddress"_L1));
	m_propertyNames.insert(ASK_CONFIRMATION_INDEX, std::move("alwaysAskConfirmation"_L1));
	m_propertyNames.insert(WEATHER_CITIES_INDEX, std::move("weatherLocations"_L1));

	m_defaultValues.reserve(SETTINGS_FIELD_COUNT);
	for(uint i{APP_VERSION_INDEX}; i < SETTINGS_FIELD_COUNT; ++i)
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

	getScreenMeasures();
	const QFontInfo fi{QGuiApplication::font()};
	setFontSize(value(m_propertyNames.value(FONT_SIZE_INDEX), applyFontRatio(fi.pixelSize(), m_ratioFont)).toUInt(), false);
	setColorScheme(value(m_propertyNames.value(COLOR_SCHEME_INDEX), 3).toUInt(), false);

	//Update of config sections
	/*if (value(m_propertyNames.value(APP_VERSION_INDEX)).isNull())
	{
		setValue(m_propertyNames.value(APP_VERSION_INDEX), TP_APP_VERSION);
		remove(m_propertyNames.value(WEATHER_CITIES_INDEX));
		sync();
	}*/
}

void TPSettings::setAppLocale(const QString &locale)
{
	const qsizetype language_idx{availableLanguages().indexOf(locale)};
	if (language_idx >= 0)
	{
		setAppLocale(language_idx);
		appUtils()->setAppLocale(locale);
	}
}

void TPSettings::getScreenMeasures()
{
	QString screenWidth, screenHeight, qmlPageHeight;
	double heightToWidthScreenRatio{0};

	const QScreen *screen{QGuiApplication::primaryScreen()};
	const QRect &screenGeometry{screen->availableGeometry()};
	const uint s_width{static_cast<uint>(screenGeometry.width())};
	const uint s_height{static_cast<uint>(screenGeometry.height())};
#ifdef Q_OS_ANDROID
	heightToWidthScreenRatio = static_cast<double>(s_height)/s_width;
	screenWidth = std::move(QString::number(s_width));
	screenHeight = std::move(QString::number(s_height));
	qmlPageHeight = std::move(QString::number(qCeil(0.92*s_height)));
#else
	heightToWidthScreenRatio = static_cast<double>(s_width)/s_height;
	screenWidth = std::move(QString::number(s_width/4));
	screenHeight = std::move(QString::number(qCeil((s_width/4) * heightToWidthScreenRatio)));
	qmlPageHeight = std::move(QString::number(qCeil(0.95 * (qCeil((s_width/4) * heightToWidthScreenRatio)))));
#endif

	constexpr qreal refDpi{100.};
	constexpr qreal refHeight{1920.};
	constexpr qreal refWidth{1080.};
	const qreal dpi{screen->logicalDotsPerInch()};
	const uint height{qMax(s_width, s_height)};
	const uint width{qMin(s_width, s_height)};
	m_ratioFont = qMin(static_cast<qreal>(height*refDpi)/(dpi*refHeight), static_cast<qreal>(width*refDpi)/(dpi*refWidth));
	if (m_ratioFont < 1.5)
		m_ratioFont = 1.5;

	m_defaultValues[WINDOW_WIDTH_INDEX] = std::move(screenWidth);
	m_defaultValues[WINDOW_HEIGHT_INDEX] = std::move(screenHeight);
	m_defaultValues[PAGE_WIDTH_INDEX] = m_defaultValues.at(WINDOW_WIDTH_INDEX);
	m_defaultValues[PAGE_HEIGHT_INDEX] = std::move(qmlPageHeight);
	m_defaultValues[HEIGHT_TO_WIDTH_RATIO_INDEX] = std::move(QString::number(heightToWidthScreenRatio, 'g', 2));
}

void TPSettings::setColorScheme(const uint new_value, const bool bFromQml)
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

QString TPSettings::primaryColor() const
{
	return colorScheme() == Custom ? value(m_propertyNames.value(COLOR_INDEX)).toString() : colorForScheme(colorScheme());
}

void TPSettings::setPrimaryColor(const QColor &color)
{
	changeValue(COLOR_INDEX, color.name());
	emit colorChanged();
}

QString TPSettings::primaryLightColor() const
{
	return colorScheme() == Custom ? value(m_propertyNames.value(LIGHT_COLOR_INDEX)).toString() : lightColorForScheme(colorScheme());
}

void TPSettings::setPrimaryLightColor(const QColor &color)
{
	changeValue(LIGHT_COLOR_INDEX, color.name());
	emit colorChanged();
}

QString TPSettings::primaryDarkColor() const
{
	return colorScheme() == Custom ? value(m_propertyNames.value(DARK_COLOR_INDEX)).toString() : darkColorForScheme(colorScheme());
}

void TPSettings::setPrimaryDarkColor(const QColor &color)
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

QString TPSettings::paneBackgroundColor() const
{
	return colorScheme() == Custom ? value(m_propertyNames.value(PANE_COLOR_INDEX)).toString() : paneColorForScheme(colorScheme());
}

QString TPSettings::entrySelectedColor() const
{
	return colorScheme() == Custom ? value(m_propertyNames.value(SELECTED_COLOR_INDEX)).toString() : selectedColorForScheme(colorScheme());
}

void TPSettings::setFontColor(const QColor &color)
{
	changeValue(FONT_COLOR_INDEX, color.name());
	emit colorChanged();
}

void TPSettings::setDisabledFontColor(const QColor &color)
{
	changeValue(DISABLED_FONT_COLOR_INDEX, color.name());
	emit colorChanged();
}

QString TPSettings::colorForScheme(const uint scheme) const
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

QString TPSettings::lightColorForScheme(const uint scheme) const
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

QString TPSettings::darkColorForScheme(const uint scheme) const
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

QString TPSettings::paneColorForScheme(const uint scheme) const
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

QString TPSettings::selectedColorForScheme(const uint scheme) const
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

void TPSettings::setFontSize(const uint new_value, const bool bFromQml)
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

QString TPSettings::weatherCity(const uint idx)
{
	if (m_weatherLocations.isEmpty())
		m_weatherLocations = std::move(value(m_propertyNames.value(WEATHER_CITIES_INDEX)).value<QStringList>());

	return idx < m_weatherLocations.count() ? appUtils()->getCompositeValue(0, m_weatherLocations.at(idx), record_separator) : QString();
}

QGeoCoordinate TPSettings::weatherCityCoordinates(const uint idx)
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

QString TPSettings::availableLanguagesLabel(const uint language_idx) const
{
	switch (language_idx)
	{
		case 0: return "Application Language: English"_L1;
		case 1: return "Linguagem do aplicativo: Português do Brasil"_L1;
		case 2: return "Sprache des Apps: Deutsch von Deutschland"_L1;
	}
	return QString{};
}

QStringList TPSettings::availableLanguages() const
{
	return QStringList{} << "en_US"_L1 << "pt_BR"_L1 << "de_DE"_L1;
}

void TPSettings::addWeatherCity(const QString &city, const QString &latitude, const QString &longitude)
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

void TPSettings::removeWeatherCity(const uint idx)
{
	m_weatherLocations.removeAt(idx);
	changeValue(WEATHER_CITIES_INDEX, QVariant::fromValue(m_weatherLocations));
	emit weatherCitiesCountChanged();
}
