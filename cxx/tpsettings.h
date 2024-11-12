#ifndef TPSETTINGS_H
#define TPSETTINGS_H

#include <QGeoCoordinate>
#include <QSettings>

#define APP_VERSION_INDEX 0
#define APP_LOCALE_INDEX 1
#define THEME_STYLE_INDEX 2
#define COLOR_INDEX 3
#define LIGHT_COLOR_INDEX 4
#define DARK_COLOR_INDEX 5
#define PANE_COLOR_INDEX 6
#define SELECTED_COLOR_INDEX 7
#define LISTS_COLOR_1_INDEX 8
#define LISTS_COLOR_2_INDEX 9
#define FONT_COLOR_INDEX 10
#define DISABLED_FONT_COLOR_INDEX 11
#define ICON_FOLDER_INDEX 12
#define WEIGHT_UNIT_INDEX 13
#define EXERCISES_VERSION_INDEX 14

#define MESO_IDX_INDEX 15
#define WINDOW_WIDTH_INDEX 16
#define WINDOW_HEIGHT_INDEX 17
#define PAGE_WIDTH_INDEX 18
#define PAGE_HEIGHT_INDEX 19
#define HEIGHT_TO_WIDTH_RATIO_INDEX 20
#define FONT_SIZE_INDEX 21
#define SMALLFONT_SIZE_INDEX 22
#define LARGEFONT_SIZE_INDEX 23
#define EXTRALARGEFONT_SIZE_INDEX 24
#define COLOR_SCHEME_INDEX 25

#define ASK_CONFIRMATION_INDEX 26
#define USER_INDEX 27

#define WEATHER_CITIES_INDEX 28

#define SETTINGS_FIELD_COUNT WEATHER_CITIES_INDEX + 1

class TPSettings : public QSettings
{

Q_OBJECT

Q_PROPERTY(uint windowWidth READ windowWidth CONSTANT)
Q_PROPERTY(uint windowHeight READ windowHeight CONSTANT)
Q_PROPERTY(uint pageWidth READ pageWidth CONSTANT)
Q_PROPERTY(uint pageHeight READ pageHeight CONSTANT)
Q_PROPERTY(uint heightToWidthRatio READ heightToWidthRatio CONSTANT)

Q_PROPERTY(uint fontSize READ fontSize WRITE setFontSize NOTIFY fontSizeChanged)
Q_PROPERTY(uint smallFontSize READ smallFontSize NOTIFY fontSizeChanged)
Q_PROPERTY(uint largeFontSize READ largeFontSize NOTIFY fontSizeChanged)
Q_PROPERTY(uint extraLargeFontSize READ extraLargeFontSize NOTIFY fontSizeChanged)
Q_PROPERTY(uint colorScheme READ colorScheme WRITE setColorScheme NOTIFY colorChanged)

Q_PROPERTY(QString appVersion READ appVersion CONSTANT)
Q_PROPERTY(QString appLocale READ appLocale WRITE setAppLocale NOTIFY appLocaleChanged)
Q_PROPERTY(QString themeStyle READ themeStyle WRITE setThemeStyle NOTIFY themeStyleChanged)
Q_PROPERTY(QString primaryColor READ primaryColor NOTIFY colorChanged)
Q_PROPERTY(QString primaryDarkColor READ primaryDarkColor NOTIFY colorChanged)
Q_PROPERTY(QString primaryLightColor READ primaryLightColor NOTIFY colorChanged)
Q_PROPERTY(QString paneBackgroundColor READ paneBackgroundColor NOTIFY colorChanged)
Q_PROPERTY(QString entrySelectedColor READ entrySelectedColor NOTIFY colorChanged)
Q_PROPERTY(QString listEntryColor1 READ listEntryColor1 NOTIFY colorChanged)
Q_PROPERTY(QString listEntryColor2 READ listEntryColor2 NOTIFY colorChanged)
Q_PROPERTY(QString fontColor READ fontColor NOTIFY colorChanged)
Q_PROPERTY(QString disabledFontColor READ disabledFontColor NOTIFY colorChanged)
Q_PROPERTY(QString iconFolder READ iconFolder NOTIFY colorChanged)
Q_PROPERTY(QString weightUnit READ weightUnit WRITE setWeightUnit NOTIFY weightUnitChanged)

Q_PROPERTY(uint lastViewedMesoIdx READ lastViewedMesoIdx WRITE setLastViewedMesoIdx NOTIFY lastViewedMesoIdxChanged)
Q_PROPERTY(uint weatherCitiesCount READ weatherCitiesCount NOTIFY weatherCitiesCountChanged)

Q_PROPERTY(bool alwaysAskConfirmation READ alwaysAskConfirmation WRITE setAlwaysAskConfirmation NOTIFY alwaysAskConfirmationChanged)
Q_PROPERTY(bool mainUserConfigured READ mainUserConfigured WRITE setMainUserConfigured NOTIFY mainUserConfiguredChanged)

public:
	explicit TPSettings(QObject* parent = nullptr);

	inline QString appVersion() const { return value(m_propertyNames.value(APP_VERSION_INDEX), m_defaultValues.at(APP_VERSION_INDEX)).toString(); }

	inline QString appLocale() const { return value(m_propertyNames.value(APP_LOCALE_INDEX), m_defaultValues.at(APP_LOCALE_INDEX)).toString(); }
	inline void setAppLocale(const QString& new_value) { changeValue(APP_LOCALE_INDEX, new_value); emit appLocaleChanged(); }

	inline QString themeStyle() const { return value(m_propertyNames.value(THEME_STYLE_INDEX), m_defaultValues.at(THEME_STYLE_INDEX)).toString(); }
	inline void setThemeStyle(const QString& new_value) { changeValue(THEME_STYLE_INDEX, new_value); emit themeStyleChanged(); }

	inline uint windowWidth() const { return m_defaultValues.at(WINDOW_WIDTH_INDEX).toUInt(); }
	inline uint windowHeight() const { return m_defaultValues.at(WINDOW_HEIGHT_INDEX).toUInt(); }
	inline uint pageWidth() const { return m_defaultValues.at(PAGE_WIDTH_INDEX).toUInt(); }
	inline uint pageHeight() const { return m_defaultValues.at(PAGE_HEIGHT_INDEX).toUInt(); }
	inline uint heightToWidthRatio() const { return m_defaultValues.at(HEIGHT_TO_WIDTH_RATIO_INDEX).toUInt(); }

	void setColorScheme(const uint new_value, const bool bFromQml = true);
	inline uint colorScheme() const { return m_defaultValues.at(COLOR_SCHEME_INDEX).toUInt(); }
	inline QString primaryColor() const { return m_defaultValues.at(COLOR_INDEX); }
	inline QString primaryLightColor() const { return m_defaultValues.at(LIGHT_COLOR_INDEX); }
	inline QString primaryDarkColor() const { return m_defaultValues.at(DARK_COLOR_INDEX); }
	inline QString paneBackgroundColor() const { return m_defaultValues.at(PANE_COLOR_INDEX); }
	inline QString entrySelectedColor() const { return m_defaultValues.at(SELECTED_COLOR_INDEX); }
	inline QString listEntryColor1() const { return m_defaultValues.at(LISTS_COLOR_1_INDEX); }
	inline QString listEntryColor2() const { return m_defaultValues.at(LISTS_COLOR_2_INDEX); }
	inline QString fontColor() const { return m_defaultValues.at(FONT_COLOR_INDEX); }
	inline QString disabledFontColor() const { return m_defaultValues.at(DISABLED_FONT_COLOR_INDEX); }
	inline QString iconFolder() const { return m_defaultValues.at(ICON_FOLDER_INDEX); }

	Q_INVOKABLE QString colorForScheme(const uint scheme) const;
	Q_INVOKABLE QString lightColorForScheme(const uint scheme) const;
	Q_INVOKABLE QString darkColorForScheme(const uint scheme) const;

	void setFontSize(const uint new_value, const bool bFromQml = true);
	inline uint fontSize() const { return m_defaultValues.at(FONT_SIZE_INDEX).toUInt(); }
	inline uint smallFontSize() const { return m_defaultValues.at(SMALLFONT_SIZE_INDEX).toUInt(); }
	inline uint largeFontSize() const { return m_defaultValues.at(LARGEFONT_SIZE_INDEX).toUInt(); }
	inline uint extraLargeFontSize() const { return m_defaultValues.at(EXTRALARGEFONT_SIZE_INDEX).toUInt(); }

	inline QString weightUnit() const { return value(m_propertyNames.value(WEIGHT_UNIT_INDEX), m_defaultValues.at(WEIGHT_UNIT_INDEX)).toString(); }
	inline void setWeightUnit(const QString& new_value) { changeValue(WEIGHT_UNIT_INDEX, new_value); emit weightUnitChanged(); }
	inline QString exercisesListVersion() const { return value(m_propertyNames.value(EXERCISES_VERSION_INDEX), m_defaultValues.at(EXERCISES_VERSION_INDEX)).toString(); }
	inline void setExercisesListVersion(const QString& new_value) { changeValue(EXERCISES_VERSION_INDEX, new_value); }
	inline uint lastViewedMesoIdx() const { return value(m_propertyNames.value(MESO_IDX_INDEX), m_defaultValues.at(MESO_IDX_INDEX).toUInt()).toUInt(); }
	inline void setLastViewedMesoIdx(const uint new_value) { changeValue(MESO_IDX_INDEX, QString::number(new_value)); emit lastViewedMesoIdxChanged(); }

	inline uint weatherCitiesCount() const { return m_weatherLocations.count(); }
	void addWeatherCity(const QString& city, const QString& latitude, const QString& longitude);
	Q_INVOKABLE void removeWeatherCity(const uint idx);
	Q_INVOKABLE QString weatherCity(const uint idx);
	Q_INVOKABLE QGeoCoordinate weatherCityCoordinates(const uint idx);

	inline bool alwaysAskConfirmation() const { return value(m_propertyNames.value(ASK_CONFIRMATION_INDEX), m_defaultValues.at(ASK_CONFIRMATION_INDEX)).toBool(); }
	inline void setAlwaysAskConfirmation(const bool new_value) { changeValue(ASK_CONFIRMATION_INDEX, QString::number(new_value)); emit alwaysAskConfirmationChanged(); }

	inline bool mainUserConfigured() const { return value(m_propertyNames.value(USER_INDEX), m_defaultValues.at(USER_INDEX)).toBool(); }
	inline void setMainUserConfigured(const bool new_value) { changeValue(USER_INDEX, QString::number(static_cast<uint>(new_value))); emit mainUserConfiguredChanged(); }

signals:
	void appLocaleChanged();
	void themeStyleChanged();
	void colorChanged();
	void weightUnitChanged();
	void fontSizeChanged();
	void lastViewedMesoIdxChanged();
	void weatherCitiesCountChanged();
	void alwaysAskConfirmationChanged();
	void mainUserConfiguredChanged();

private:
	QMap<uint,QString> m_propertyNames;
	QStringList m_defaultValues;
	QStringList m_weatherLocations;

	inline void changeValue(const uint index, const QVariant& new_value)
	{
		setValue(m_propertyNames.value(index), new_value);
		sync();
	}

	void getScreenMeasures();
	static TPSettings* app_settings;
	friend TPSettings* appSettings();
};

inline TPSettings* appSettings() { return TPSettings::app_settings; }
#endif // TPSETTINGS_H