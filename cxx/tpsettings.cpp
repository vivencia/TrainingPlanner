#include "tpsettings.h"

#include "tputils.h"
#include "tpglobals.h"

#include <QScreen>
#include <QFontInfo>

constexpr uint QML_PROPERTIES(25);

TPSettings* TPSettings::app_settings(nullptr);

TPSettings::TPSettings(QObject* parent) : QSettings{parent}
{
	TPSettings::app_settings = this;

	m_propertyNames.insert(APP_VERSION_INDEX, std::move(u"appVersion"_s));
	m_propertyNames.insert(APP_LOCALE_INDEX, std::move(u"appLocale"_s));
	m_propertyNames.insert(THEME_STYLE_INDEX, std::move(u"widgetTheme"_s));
	m_propertyNames.insert(COLOR_SCHEME_INDEX, std::move(u"colorScheme"_s));
	m_propertyNames.insert(FONT_SIZE_INDEX, std::move(u"fontPixelSize"_s));
	m_propertyNames.insert(WEIGHT_UNIT_INDEX, std::move(u"weightUnit"_s));
	m_propertyNames.insert(EXERCISES_VERSION_INDEX, std::move(u"exercisesListVersion"_s));
	m_propertyNames.insert(MESO_IDX_INDEX, std::move(u"lastViewedMesoIdx"_s));
	m_propertyNames.insert(ASK_CONFIRMATION_INDEX, std::move(u"alwaysAskConfirmation"_s));
	m_propertyNames.insert(USER_INDEX, std::move(u"mainUserConfigured"_s));
	m_propertyNames.insert(WEATHER_CITIES_INDEX, std::move(u"weatherLocations"_s));
	m_propertyNames.insert(USEGPS_INDEX, std::move(u"useGPS"_s));

	m_defaultValues.reserve(QML_PROPERTIES);
	for(uint i(APP_VERSION_INDEX); i < SETTINGS_FIELD_COUNT; ++i)
		m_defaultValues.append(QString());

	m_defaultValues[APP_VERSION_INDEX] = std::move(TP_APP_VERSION);
	m_defaultValues[THEME_STYLE_INDEX] = std::move(u"Material"_s);
	m_defaultValues[WEIGHT_UNIT_INDEX] = std::move(u"(kg)"_s);
	m_defaultValues[ASK_CONFIRMATION_INDEX] = STR_ONE;
	m_defaultValues[USER_INDEX] = STR_ZERO;
	m_defaultValues[USEGPS_INDEX] = STR_ZERO;

	getScreenMeasures();
	const QFontInfo fi{QGuiApplication::font()};
	setFontSize(value(m_propertyNames.value(FONT_SIZE_INDEX), fi.pixelSize()).toUInt(), false);
	setColorScheme(value(m_propertyNames.value(COLOR_SCHEME_INDEX), 0).toUInt(), false);
}

void TPSettings::getScreenMeasures()
{
	QString screenWidth, screenHeight, heightToWidthScreenRatio, qmlPageHeight;

#ifdef Q_OS_ANDROID
	const QScreen* screen(QGuiApplication::primaryScreen());
	const QRect& screenGeometry = screen->availableGeometry();
	const uint sWidth{static_cast<uint>(screenGeometry.width())};
	const uint sHeight{static_cast<uint>(screenGeometry.height())};
	screenWidth = std::move(QString::number(sWidth));
	screenHeight = std::move(QString::number(sHeight));
	heightToWidthScreenRatio = std::move(QString::number(static_cast<float>(sHeight/sWidth), 'f', 2));
	qmlPageHeight = QString::number(qCeil(0.92*sHeight));
#else
	screenWidth = std::move(u"300"_s);
	screenHeight = std::move(u"640"_s);
	heightToWidthScreenRatio = std::move(u"2"_s);
	qmlPageHeight = std::move(u"600"_s);
#endif

	m_defaultValues[WINDOW_WIDTH_INDEX] = std::move(screenWidth);
	m_defaultValues[WINDOW_HEIGHT_INDEX] = std::move(screenHeight);
	m_defaultValues[PAGE_WIDTH_INDEX] = m_defaultValues.at(WINDOW_WIDTH_INDEX);
	m_defaultValues[PAGE_HEIGHT_INDEX] = std::move(qmlPageHeight);
	m_defaultValues[HEIGHT_TO_WIDTH_RATIO_INDEX] = std::move(heightToWidthScreenRatio);
}

void TPSettings::setColorScheme(const uint new_value, const bool bFromQml)
{
	QString colorLight, color, colorDark, paneBackColor, entrySelColor;
	QString fntColor{std::move("#ffffff")};
	QString disabledfntColor{std::move("#dcdcdc")};
	QString strIconFolder{std::move("white/")};

	color = std::move(colorForScheme(new_value));
	colorLight = std::move(lightColorForScheme(new_value));
	colorDark = std::move(darkColorForScheme(new_value));

	switch (new_value)
	{
		case 0: //Blue
			paneBackColor = std::move(u"#1976d2"_s);
			entrySelColor = std::move(u"#6495ed"_s);
		break;
		case 1: //Green
			paneBackColor = std::move(u"#60d219"_s);
			entrySelColor = std::move(u"#228b22"_s);
		break;
		case 2: //Red
			paneBackColor = std::move(u"#d21a45"_s);
			entrySelColor = std::move(u"#228b22"_s);
		break;
		case 3: //Dark
			paneBackColor = std::move(u"#3e3d48"_s);
			entrySelColor = std::move(u"#228b22"_s);
			disabledfntColor = std::move(u"e8e8e8"_s);
		break;
		case 4: //Light
			paneBackColor = std::move(u"#929299"_s);
			entrySelColor = std::move(u"#228b22"_s);
			fntColor = std::move(u"000000"_s);
			disabledfntColor = std::move(u"a8a8a8"_s);
			strIconFolder = std::move("black/");
		break;
	}

	m_defaultValues[COLOR_SCHEME_INDEX] = std::move(QString::number(new_value));
	m_defaultValues[COLOR_INDEX] = std::move(color);
	m_defaultValues[LIGHT_COLOR_INDEX] = std::move(colorLight);
	m_defaultValues[DARK_COLOR_INDEX] = std::move(colorDark);
	m_defaultValues[PANE_COLOR_INDEX] = std::move(paneBackColor);
	m_defaultValues[SELECTED_COLOR_INDEX] = std::move(entrySelColor);
	m_defaultValues[LISTS_COLOR_1_INDEX] = std::move(u"#c8e3f0"_s);
	m_defaultValues[LISTS_COLOR_2_INDEX] = std::move(u"#c3cad5"_s);
	m_defaultValues[FONT_COLOR_INDEX] = std::move(fntColor);
	m_defaultValues[DISABLED_FONT_COLOR_INDEX] = std::move(disabledfntColor);
	m_defaultValues[ICON_FOLDER_INDEX] = std::move(strIconFolder);

	changeValue(COLOR_SCHEME_INDEX, new_value);
	emit colorChanged();
}

QString TPSettings::colorForScheme(const uint scheme) const
{
	switch (scheme)
	{
		case 0: return std::move(u"#25b5f3"_s); break;
		case 1: return std::move(u"#97dd81"_s); break;
		case 2: return std::move(u"#fd9ab1"_s); break;
		case 3: return std::move(u"#9ea6a3"_s); break;
		default: return std::move(u"#cccccc"_s); break;
	}
}

QString TPSettings::lightColorForScheme(const uint scheme) const
{
	switch (scheme)
	{
		case 0: return std::move(u"#bbdefb"_s); break;
		case 1: return std::move(u"#d4fdc0"_s); break;
		case 2: return std::move(u"#ebafc7"_s); break;
		case 3: return std::move(u"#d7e2de"_s); break;
		default: return std::move(u"#f3f3f3"_s); break;
	}
}

QString TPSettings::darkColorForScheme(const uint scheme) const
{
	switch (scheme)
	{
		case 0: return std::move(u"#1976d2"_s); break;
		case 1: return std::move(u"#12a35a"_s); break;
		case 2: return std::move(u"#fd1c20"_s); break;
		case 3: return std::move(u"#000000"_s); break;
		default: return std::move(u"#c1c1c1"_s); break;
	}
}

void TPSettings::setFontSize(const uint new_value, const bool bFromQml)
{
	const uint fontSizeSmall{static_cast<uint>(qFloor(static_cast<float>(new_value*0.8)))};
	const uint fontSizeLarge{static_cast<uint>(qFloor(static_cast<float>(new_value*1.5)))};
	const uint fontSizeExtraLarge{static_cast<uint>(new_value*2)};

	m_defaultValues[FONT_SIZE_INDEX] = std::move(QString::number(new_value));
	m_defaultValues[SMALLFONT_SIZE_INDEX] = std::move(QString::number(fontSizeSmall));
	m_defaultValues[LARGEFONT_SIZE_INDEX] = std::move(QString::number(fontSizeLarge));
	m_defaultValues[EXTRALARGEFONT_SIZE_INDEX] = std::move(QString::number(fontSizeExtraLarge));

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

void TPSettings::addWeatherCity(const QString& city, const QString& latitude, const QString& longitude)
{
	for(uint i(0); i < m_weatherLocations.count(); ++i)
	{
		if (m_weatherLocations.at(i).contains(city, Qt::CaseInsensitive))
			return;
	}
	m_weatherLocations.append(city + record_separator + latitude + record_separator + longitude);
	changeValue(WEATHER_CITIES_INDEX, QVariant::fromValue(m_weatherLocations));
	emit weatherCitiesCountChanged();
}

void TPSettings::removeWeatherCity(const uint idx)
{
	m_weatherLocations.removeAt(idx);
	changeValue(WEATHER_CITIES_INDEX, QVariant::fromValue(m_weatherLocations)) ;
	emit weatherCitiesCountChanged();
}
