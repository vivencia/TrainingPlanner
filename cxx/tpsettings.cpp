#include "tpsettings.h"

#include "dbusermodel.h"
#include "tputils.h"

#include <QColor>
#include <QFontInfo>
#include <QScreen>
#include <QStandardPaths>

using namespace Qt::Literals::StringLiterals;

TPSettings *TPSettings::app_settings{nullptr};

TPSettings::TPSettings(QObject *parent)
	: QSettings{parent},
	  m_localAppFilesDir{std::move(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)) + QLatin1Char('/')}
{
	TPSettings::app_settings = this;
	globalSettingsInit();
	userSettingsInit();
}

//--------------------------------------------GLOBAL SETTINGS---------------------------------------------//
void TPSettings::globalSettingsInit()
{
	m_propertyNames.insert(APP_VERSION_INDEX, std::move("appVersion"_L1));
	m_propertyNames.insert(CURRENT_USER, std::move("currentUser"_L1));
	m_propertyNames.insert(SERVER_ADDRESS, std::move("serverAddress"_L1));
	m_propertyNames.insert(EXERCISES_VERSION_INDEX, std::move("exercisesListVersion"_L1));
	getScreenMeasures();
}

QString TPSettings::userConfigFileName(const bool fullpath, const QString &userid) const
{
	const QString &configfile{"config.ini"_L1};
	if (!fullpath)
		return configfile;
	else
		return userDir(userid) + configfile;
}

void TPSettings::importFromUserConfig(const QString &userid)
{
	QFile *cfg_file{appUtils()->openFile(userConfigFileName(true, userid), QIODeviceBase::ReadOnly|QIODeviceBase::Text)};
	if (cfg_file)
	{
		QString line{512, QChar{0}};
		QTextStream stream{cfg_file};
		while (stream.readLineInto(&line))
		{
			const uint key{m_userPropertyNames.key(QLatin1StringView{line.section('=', 0, 0).toLatin1()}, 200)};
			if (key != 200)
				changeValue(userid, key, line.section('=', 1, 1), false);
		}
		sync();
		cfg_file->close();
		delete cfg_file;
	}
	setCurrentUser(userid);
	userSwitchingActions();
}

bool TPSettings::exportToUserConfig(const QString &userid)
{
	QFile *cfg_file{appUtils()->openFile(userConfigFileName(true, userid), QIODeviceBase::WriteOnly|QIODeviceBase::Truncate|QIODeviceBase::Text)};
	if (cfg_file)
	{
		QTextStream stream{cfg_file};
		uint i{0};
		for (const auto &field_name : std::as_const(m_userPropertyNames))
		{
			const QString &field_value{getValue(userid, i++, QString{'!'}).toString()};
			if (field_value != '!')
				stream << field_name << '=' << field_value << '\n';
		}
		cfg_file->close();
		delete cfg_file;
		return true;
	}
	return false;
}

void TPSettings::getScreenMeasures()
{
	const QScreen *screen{QGuiApplication::primaryScreen()};
	const QRect &screenGeometry{screen->availableGeometry()};
	const uint s_width{static_cast<uint>(screenGeometry.width())};
	const uint s_height{static_cast<uint>(screenGeometry.height())};
#ifdef Q_OS_ANDROID
	m_HeightToWidth = static_cast<double>(s_height)/s_width;
	m_windowWidth = s_width;
	m_windowHeight = s_height;
	m_qmlPageHeight = qCeil(0.92 * s_height);
#else
	m_HeightToWidth = static_cast<double>(s_width)/s_height;
	m_windowWidth = s_width/4;
	m_windowHeight = qCeil(m_windowWidth * m_HeightToWidth);
	m_qmlPageHeight = qCeil(0.95 * (qCeil((s_width/4) * m_HeightToWidth)));
#endif

	constexpr qreal refDpi{100.};
	constexpr qreal refHeight{1920.};
	constexpr qreal refWidth{1080.};
	const qreal dpi{screen->logicalDotsPerInch()};
	const uint height{qMax(s_width, s_height)};
	const uint width{qMin(s_width, s_height)};
	m_ratioFont = qMin(static_cast<qreal>(height * refDpi)/(dpi * refHeight),
										static_cast<qreal>(width * refDpi)/(dpi * refWidth));
	if (m_ratioFont < 1.5)
		m_ratioFont = 1.5;
}

void TPSettings::changeValue(const QString &group, const uint index, const QVariant &new_value, const bool dosync)
{
	beginGroup(group);
	setValue(group == GLOBAL_GROUP ? m_propertyNames.value(index) : m_userPropertyNames.value(index), new_value);
	endGroup();
	if (dosync)
	{
		sync();
		if (group != GLOBAL_GROUP && group != DEFAULT_USER)
		{
			if (exportToUserConfig(group))
				appUserModel()->sendFileToServer(userConfigFileName(true, group), nullptr, QString{}, QString{}, group);
		}
	}
}

QString TPSettings::availableLanguagesLabel(const uint language_idx) const
{
	switch (language_idx)
	{
		case 0: return "Application Language: English"_L1;
		case 1: return u"Linguagem do aplicativo: Português do Brasil"_s;
		case 2: return "Sprache des Apps: Deutsch von Deutschland"_L1;
	}
	return QString{};
}

QStringList TPSettings::availableLanguages() const
{
	return QStringList{} << std::move("en_US"_L1) << std::move("pt_BR"_L1) << std::move("de_DE"_L1);
}

QString TPSettings::userDir(const QString &userid) const
{
	return m_localAppFilesDir + userid + '/';
}
//--------------------------------------------GLOBAL SETTINGS---------------------------------------------//

//--------------------------------------------USER   SETTINGS---------------------------------------------//
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

void TPSettings::userSettingsInit()
{
	m_userPropertyNames.insert(USER_LOCALE_INDEX, std::move("userLocale"_L1));
	m_userPropertyNames.insert(THEME_STYLE_INDEX, std::move("widgetTheme"_L1));
	m_userPropertyNames.insert(COLOR_INDEX, std::move("mainColor"_L1));
	m_userPropertyNames.insert(LIGHT_COLOR_INDEX, std::move("lightColor"_L1));
	m_userPropertyNames.insert(DARK_COLOR_INDEX, std::move("darkColor"_L1));
	m_userPropertyNames.insert(PANE_COLOR_INDEX, std::move("paneColor"_L1));
	m_userPropertyNames.insert(SELECTED_COLOR_INDEX, std::move("selectedColor"_L1));
	m_userPropertyNames.insert(FONT_COLOR_INDEX, std::move("fontColor"_L1));
	m_userPropertyNames.insert(DISABLED_FONT_COLOR_INDEX, std::move("disabledFontColor"_L1));
	m_userPropertyNames.insert(COLOR_SCHEME_INDEX, std::move("colorScheme"_L1));
	m_userPropertyNames.insert(FONT_SIZE_INDEX, std::move("fontPixelSize"_L1));
	m_userPropertyNames.insert(WEIGHT_UNIT_INDEX, std::move("weightUnit"_L1));
	m_userPropertyNames.insert(MESO_IDX_INDEX, std::move("lastViewedMesoIdx"_L1));
	m_userPropertyNames.insert(ASK_CONFIRMATION_INDEX, std::move("alwaysAskConfirmation"_L1));
	m_userPropertyNames.insert(WEATHER_CITIES_INDEX, std::move("weatherLocations"_L1));

	m_defaultValues.reserve(USER_SETTINGS_FIELD_COUNT);
	for(uint i{USER_LOCALE_INDEX}; i < USER_SETTINGS_FIELD_COUNT; ++i)
	{
		switch (i)
		{
			case THEME_STYLE_INDEX: m_defaultValues.append(std::move("Material"_L1)); break;
			case WEIGHT_UNIT_INDEX: m_defaultValues.append(std::move("(kg)"_L1)); break;
			case MESO_IDX_INDEX: m_defaultValues.append(std::move("-1"_L1)); break;
			case ASK_CONFIRMATION_INDEX: m_defaultValues.append(std::move(QString{'1'})); break;
			case COLOR_SCHEME_INDEX: m_defaultValues.append(std::move(QString{'3'})); break;
			default: m_defaultValues.append(QString{}); break;
		}
	}

	m_colorSchemes.reserve(7);
	m_colorSchemes << std::move(tr("Custom(click on the regions of the rectangle on the right to change colors)")) <<
					std::move(tr("Dark")) << std::move(tr("Light")) << std::move(tr("Blue")) <<
					std::move(tr("Green")) << std::move(tr("Red")) << std::move(tr("Gray"));

	const QFontInfo fi{QGuiApplication::font()};
	setFontSize(getValue(currentUser(), FONT_SIZE_INDEX,
								applyFontRatio(fi.pixelSize(), appSettings()->fontRatio())).toUInt(), false);
	setColorScheme(getValue(currentUser(), COLOR_SCHEME_INDEX, m_defaultValues.at(COLOR_SCHEME_INDEX)).toUInt(), false);
}

void TPSettings::setUserLocale(const QString &locale, const bool write_to_file)
{
	const qsizetype language_idx{appSettings()->availableLanguages().indexOf(locale)};
	if (language_idx >= 0)
	{
		appUtils()->setAppLocale(locale);
		if (locale != userLocale())
			setUserLocale(language_idx, write_to_file);
	}
}

void TPSettings::setUserLocale(const uint language_idx, const bool write_to_file)
{
	m_languageIdx = language_idx;
	if (write_to_file)
		changeValue(currentUser(), USER_LOCALE_INDEX, appSettings()->availableLanguages().at(language_idx));
	emit userLocaleChanged();
}

void TPSettings::setColorScheme(const uint new_value, const bool bFromQml)
{
	switch (new_value)
	{
		case Custom:
		break;
		case Dark:
			m_defaultValues[FONT_COLOR_INDEX] = std::move("#ffffff"_L1);
			m_defaultValues[DISABLED_FONT_COLOR_INDEX] = std::move("#b9b9b9"_L1);
		break;
		case Light:
			m_defaultValues[FONT_COLOR_INDEX] = std::move("#1a28e7"_L1);
			m_defaultValues[DISABLED_FONT_COLOR_INDEX] = std::move("#cccaff"_L1);
		break;
		case Gray:
			m_defaultValues[FONT_COLOR_INDEX] = std::move("#000000"_L1);
			m_defaultValues[DISABLED_FONT_COLOR_INDEX] = std::move("#a8a8a8"_L1);
		break;
		default:
			m_defaultValues[FONT_COLOR_INDEX] = std::move("#ffffff"_L1);
			m_defaultValues[DISABLED_FONT_COLOR_INDEX] = std::move("#dcdcdc"_L1);
	}

	m_defaultValues[COLOR_SCHEME_INDEX] = std::move(QString::number(new_value));
	m_defaultValues[COLOR_INDEX] = std::move(colorForScheme(new_value));
	m_defaultValues[LIGHT_COLOR_INDEX] = std::move(lightColorForScheme(new_value));
	m_defaultValues[DARK_COLOR_INDEX] = std::move(darkColorForScheme(new_value));
	m_defaultValues[LISTS_COLOR_1_INDEX] = std::move(listColor1ForScheme(new_value));
	m_defaultValues[LISTS_COLOR_2_INDEX] = std::move(listColor2ForScheme(new_value));
	m_defaultValues[PANE_COLOR_INDEX] = std::move(paneColorForScheme(new_value));
	m_defaultValues[SELECTED_COLOR_INDEX] = std::move(selectedColorForScheme(new_value));

	if (bFromQml)
	{
		changeValue(currentUser(), COLOR_SCHEME_INDEX, new_value);
		emit colorChanged();
	}
}

void TPSettings::setPrimaryColor(const QColor &color)
{
	changeValue(currentUser(), COLOR_INDEX, color.name());
	emit colorChanged();
}

void TPSettings::setPrimaryLightColor(const QColor &color)
{
	changeValue(currentUser(), LIGHT_COLOR_INDEX, color.name());
	emit colorChanged();
}

void TPSettings::setPrimaryDarkColor(const QColor &color)
{
	changeValue(currentUser(), DARK_COLOR_INDEX, color.name());
	changeValue(currentUser(), PANE_COLOR_INDEX, color.lightness() <= 180 ? color.lighter(125).name() : color.darker(280).name());
	changeValue(currentUser(), SELECTED_COLOR_INDEX, color.lightness() <= 180 ? color.lighter(115).name() : color.darker(250).name());
	const QColor white {"#ffffff"_L1};
	const QColor black {"#000000"_L1};
	changeValue(currentUser(), FONT_COLOR_INDEX, color.lightness() <= 127 ? white.name() : black.name());
	changeValue(currentUser(), DISABLED_FONT_COLOR_INDEX, color.lightness() <= 127 ? white.darker(150).name() : black.lighter(115).name());
	emit colorChanged();
}

void TPSettings::setFontColor(const QColor &color)
{
	changeValue(currentUser(), FONT_COLOR_INDEX, color.name());
	emit colorChanged();
}

void TPSettings::setDisabledFontColor(const QColor &color)
{
	changeValue(currentUser(), DISABLED_FONT_COLOR_INDEX, color.name());
	emit colorChanged();
}

QString TPSettings::colorForScheme(const uint scheme) const
{
	switch (scheme)
	{
		case Custom: return getValue(currentUser(), COLOR_INDEX, m_defaultValues[COLOR_INDEX]).toString(); break;
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
		case Custom: return getValue(currentUser(), LIGHT_COLOR_INDEX, m_defaultValues[LIGHT_COLOR_INDEX]).toString(); break;
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
		case Custom: return getValue(currentUser(), DARK_COLOR_INDEX, m_defaultValues[DARK_COLOR_INDEX]).toString(); break;
		case Dark: return "#000000"_L1; break;
		case Light: return "#e9e9ea"_L1; break;
		case Blue: return "#1e344a"_L1; break;
		case Green: return "#2a6949"_L1; break;
		case Red: return "#e34c3f"_L1; break;
		case Gray: return "#c1c1c1"_L1; break;
	}
	return QString{};
}

QString TPSettings::listColor1ForScheme(const uint scheme) const
{
	switch (scheme)
	{
		case Custom: return getValue(currentUser(), LISTS_COLOR_1_INDEX, m_defaultValues[LISTS_COLOR_1_INDEX]).toString(); break;
		case Dark: return "#65756e"_L1; break;
		case Light: return "#95aaa6"_L1; break;
		case Blue: return "#25415c"_L1; break;
		case Green: return "#319355"_L1; break;
		case Red: return "#953229"_L1; break;
		case Gray: return "#9a9fa8"_L1; break;
	}
	return QString{};
}

QString TPSettings::listColor2ForScheme(const uint scheme) const
{
	switch (scheme)
	{
		case Custom: return getValue(currentUser(), LISTS_COLOR_2_INDEX, m_defaultValues[LISTS_COLOR_2_INDEX]).toString(); break;
		case Dark: return "#7a8d84"_L1; break;
		case Light: return "#bfdad5"_L1; break;
		case Blue: return "#4d6e98"_L1; break;
		case Green: return "#3cb367"_L1; break;
		case Red: return "#c84438"_L1; break;
		case Gray: return "#b6bbc6"_L1; break;
	}
	return QString{};
}

QString TPSettings::paneColorForScheme(const uint scheme) const
{
	switch (scheme)
	{
		case Custom: return getValue(currentUser(), PANE_COLOR_INDEX, m_defaultValues[PANE_COLOR_INDEX]).toString(); break;
		case Dark: return "#000000"_L1; break;
		case Light: return "#535354"_L1; break;
		case Blue: return "#638dc3"_L1; break;
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
		case Custom: return getValue(currentUser(), SELECTED_COLOR_INDEX, m_defaultValues[SELECTED_COLOR_INDEX]).toString(); break;
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
	const uint fontSizeSmall{static_cast<uint>(qFloor(static_cast<float>(new_value) * 0.8))};
	const uint fontSizeLarge{static_cast<uint>(qFloor(static_cast<float>(new_value) * 1.5))};
	const uint fontSizeExtraLarge{static_cast<uint>(new_value * 2)};

	m_defaultValues[SMALLFONT_SIZE_INDEX] = std::move(QString::number(fontSizeSmall));
	m_defaultValues[LARGEFONT_SIZE_INDEX] = std::move(QString::number(fontSizeLarge));
	m_defaultValues[EXTRALARGEFONT_SIZE_INDEX] = std::move(QString::number(fontSizeExtraLarge));
	m_defaultValues[ITEM_DEFAULT_HEIGHT] =
				std::move(QString::number(static_cast<uint>(qCeil(static_cast<float>(new_value) * 1.5))));

	if (bFromQml)
	{
		changeValue(currentUser(), FONT_SIZE_INDEX, QString::number(new_value));
		emit fontSizeChanged();
	}
	else
		m_defaultValues[FONT_SIZE_INDEX] = std::move(QString::number(new_value));
}

QString TPSettings::weatherCity(const uint idx)
{
	if (m_weatherLocations.isEmpty())
		m_weatherLocations = std::move(value(m_userPropertyNames.value(WEATHER_CITIES_INDEX)).value<QStringList>());

	return idx < m_weatherLocations.count() ? appUtils()->getCompositeValue(0, m_weatherLocations.at(idx), record_separator) : QString();
}

QGeoCoordinate TPSettings::weatherCityCoordinates(const uint idx)
{
	if (m_weatherLocations.isEmpty())
		m_weatherLocations = std::move(value(m_userPropertyNames.value(WEATHER_CITIES_INDEX)).value<QStringList>());

	QGeoCoordinate coord;
	if (idx < m_weatherLocations.count())
	{
		coord.setLatitude(appUtils()->getCompositeValue(1, m_weatherLocations.at(idx), record_separator).toDouble());
		coord.setLongitude(appUtils()->getCompositeValue(2, m_weatherLocations.at(idx), record_separator).toDouble());
	}
	return coord;
}

void TPSettings::addWeatherCity(const QString &city, const QString &latitude, const QString &longitude)
{
	for (const auto &location: std::as_const(m_weatherLocations))
	{
		if (location.contains(city, Qt::CaseInsensitive))
			return;
	}
	m_weatherLocations.append(appUtils()->string_strings({city, latitude, longitude}, record_separator));
	changeValue(currentUser(), WEATHER_CITIES_INDEX, QVariant::fromValue(m_weatherLocations));
	emit weatherCitiesCountChanged();
}

void TPSettings::removeWeatherCity(const uint idx)
{
	m_weatherLocations.removeAt(idx);
	changeValue(currentUser(), WEATHER_CITIES_INDEX, QVariant::fromValue(m_weatherLocations));
	emit weatherCitiesCountChanged();
}

void TPSettings::userSwitchingActions()
{
	setFontSize(getValue(currentUser(), FONT_SIZE_INDEX, m_defaultValues.at(FONT_SIZE_INDEX)).toUInt(), false);
	setColorScheme(getValue(currentUser(), COLOR_SCHEME_INDEX, m_defaultValues.at(COLOR_SCHEME_INDEX)).toUInt(), false);
	emit fontSizeChanged();
	emit userLocaleChanged();
	emit themeStyleChanged();
	emit colorChanged();
	emit weightUnitChanged();
	emit lastViewedMesoIdxChanged();
	emit weatherCitiesCountChanged();
	emit alwaysAskConfirmationChanged();
}
//--------------------------------------------USER   SETTINGS---------------------------------------------//
