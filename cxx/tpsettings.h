#pragma once

#include <QGeoCoordinate>
#include <QSettings>

//--------------------------------------------GLOBAL SETTINGS---------------------------------------------//
using namespace Qt::Literals::StringLiterals;

constexpr QLatin1StringView TP_APP_VERSION("v20250922 Build 20"_L1);
constexpr QLatin1StringView GLOBAL_GROUP("app"_L1);
constexpr QLatin1StringView DEFAULT_USER{"default"_L1};

#define APP_VERSION_INDEX 0
#define WINDOW_WIDTH_INDEX 1
#define WINDOW_HEIGHT_INDEX 2
#define PAGE_WIDTH_INDEX 3
#define PAGE_HEIGHT_INDEX 4
#define HEIGHT_TO_WIDTH_RATIO_INDEX 5
#define FONT_RATIO 6
#define CURRENT_USER 7
#define SERVER_ADDRESS 8
#define EXERCISES_VERSION_INDEX 9
#define APP_SETTINGS_FIELD_COUNT EXERCISES_VERSION_INDEX + 1
//--------------------------------------------GLOBAL SETTINGS---------------------------------------------//

//--------------------------------------------USER   SETTINGS---------------------------------------------//
#define USER_LOCALE_INDEX 0
#define THEME_STYLE_INDEX 1
#define COLOR_INDEX 2
#define LIGHT_COLOR_INDEX 3
#define DARK_COLOR_INDEX 4
#define PANE_COLOR_INDEX 5
#define SELECTED_COLOR_INDEX 6
#define LISTS_COLOR_1_INDEX 7
#define LISTS_COLOR_2_INDEX 8
#define FONT_COLOR_INDEX 9
#define DISABLED_FONT_COLOR_INDEX 10
#define WEIGHT_UNIT_INDEX 11
#define MESO_IDX_INDEX 12
#define FONT_SIZE_INDEX 13
#define SMALLFONT_SIZE_INDEX 14
#define LARGEFONT_SIZE_INDEX 15
#define EXTRALARGEFONT_SIZE_INDEX 16
#define COLOR_SCHEME_INDEX 17
#define ITEM_DEFAULT_HEIGHT 18
#define ASK_CONFIRMATION_INDEX 19
#define WEATHER_CITIES_INDEX 20
#define USER_SETTINGS_FIELD_COUNT WEATHER_CITIES_INDEX + 1
//--------------------------------------------USER   SETTINGS---------------------------------------------//

class TPSettings : public QSettings
{

Q_OBJECT

//--------------------------------------------GLOBAL SETTINGS---------------------------------------------//
Q_PROPERTY(uint windowWidth READ windowWidth CONSTANT)
Q_PROPERTY(uint windowHeight READ windowHeight CONSTANT)
Q_PROPERTY(uint pageWidth READ pageWidth CONSTANT)
Q_PROPERTY(uint pageHeight READ pageHeight CONSTANT)
Q_PROPERTY(double heightToWidthRatio READ heightToWidthRatio CONSTANT)
Q_PROPERTY(QString appVersion READ appVersion CONSTANT)
Q_PROPERTY(QString currentUser READ currentUser WRITE setCurrentUser NOTIFY currentUserChanged FINAL)
Q_PROPERTY(QString serverAddress READ serverAddress WRITE setServerAddress NOTIFY serverAddressChanged FINAL)
Q_PROPERTY(QStringList availableLanguages READ availableLanguages CONSTANT FINAL)
//--------------------------------------------GLOBAL SETTINGS---------------------------------------------//

//--------------------------------------------USER   SETTINGS---------------------------------------------//
Q_PROPERTY(uint fontSize READ fontSize WRITE setFontSize NOTIFY fontSizeChanged)
Q_PROPERTY(uint smallFontSize READ smallFontSize NOTIFY fontSizeChanged)
Q_PROPERTY(uint largeFontSize READ largeFontSize NOTIFY fontSizeChanged)
Q_PROPERTY(uint extraLargeFontSize READ extraLargeFontSize NOTIFY fontSizeChanged)
Q_PROPERTY(uint colorScheme READ colorScheme WRITE setColorScheme NOTIFY colorChanged)
Q_PROPERTY(uint itemDefaultHeight READ itemDefaultHeight NOTIFY fontSizeChanged FINAL)
Q_PROPERTY(uint itemSmallHeight READ itemSmallHeight NOTIFY fontSizeChanged FINAL)
Q_PROPERTY(uint itemLargeHeight READ itemLargeHeight NOTIFY fontSizeChanged FINAL)
Q_PROPERTY(uint itemExtraLargeHeight READ itemExtraLargeHeight NOTIFY fontSizeChanged FINAL)

Q_PROPERTY(QString userLocale READ userLocale NOTIFY userLocaleChanged)
Q_PROPERTY(QString themeStyle READ themeStyle WRITE setThemeStyle NOTIFY themeStyleChanged)
Q_PROPERTY(QString primaryColor READ primaryColor WRITE setPrimaryColor NOTIFY colorChanged)
Q_PROPERTY(QString primaryLightColor READ primaryLightColor WRITE setPrimaryLightColor NOTIFY colorChanged)
Q_PROPERTY(QString primaryDarkColor READ primaryDarkColor WRITE setPrimaryDarkColor NOTIFY colorChanged)
Q_PROPERTY(QString paneBackgroundColor READ paneBackgroundColor NOTIFY colorChanged)
Q_PROPERTY(QString entrySelectedColor READ entrySelectedColor NOTIFY colorChanged)
Q_PROPERTY(QString listEntryColor1 READ listEntryColor1 NOTIFY colorChanged)
Q_PROPERTY(QString listEntryColor2 READ listEntryColor2 NOTIFY colorChanged)
Q_PROPERTY(QString fontColor READ fontColor WRITE setFontColor NOTIFY colorChanged)
Q_PROPERTY(QString disabledFontColor READ disabledFontColor WRITE setDisabledFontColor NOTIFY colorChanged)
Q_PROPERTY(QString weightUnit READ weightUnit WRITE setWeightUnit NOTIFY weightUnitChanged)

Q_PROPERTY(int lastViewedMesoIdx READ lastViewedMesoIdx WRITE setLastViewedMesoIdx NOTIFY lastViewedMesoIdxChanged)
Q_PROPERTY(uint userLocaleIdx READ userLocaleIdx NOTIFY userLocaleChanged FINAL)
Q_PROPERTY(uint weatherCitiesCount READ weatherCitiesCount NOTIFY weatherCitiesCountChanged)
Q_PROPERTY(bool alwaysAskConfirmation READ alwaysAskConfirmation WRITE setAlwaysAskConfirmation NOTIFY alwaysAskConfirmationChanged)

Q_PROPERTY(QStringList colorSchemes READ colorSchemes FINAL CONSTANT)
//--------------------------------------------USER   SETTINGS---------------------------------------------//

public:
	explicit TPSettings(QObject *parent = nullptr);

//--------------------------------------------GLOBAL SETTINGS---------------------------------------------//
	void globalSettingsInit();
	QString userConfigFileName(const bool fullpath, const QString &userid = QString{}) const;
	bool importFromUserConfig(const QString &userid);
	bool exportToUserConfig(const QString &userid);

	inline QString appVersion() const { return getValue(GLOBAL_GROUP, APP_VERSION_INDEX, TP_APP_VERSION).toString(); }
	inline uint windowWidth() const { return m_windowWidth; }
	inline uint windowHeight() const { return m_windowHeight; }
	inline uint pageWidth() const { return m_windowWidth; }
	inline uint pageHeight() const { return m_qmlPageHeight; }
	inline double heightToWidthRatio() const { return m_HeightToWidth; }
	inline qreal fontRatio() const { return m_ratioFont; }
	inline QString currentUser() const { return getValue(GLOBAL_GROUP, CURRENT_USER, DEFAULT_USER).toString(); }
	inline void setCurrentUser(const QString &new_value) { changeValue(GLOBAL_GROUP, CURRENT_USER, new_value); emit currentUserChanged(); }
	inline QString serverAddress() const { return getValue(GLOBAL_GROUP, SERVER_ADDRESS).toString(); }
	inline void setServerAddress(const QString &new_value) { changeValue(GLOBAL_GROUP, SERVER_ADDRESS, new_value); emit serverAddressChanged(); }
	inline QString exercisesListVersion() const { return getValue(GLOBAL_GROUP, EXERCISES_VERSION_INDEX).toString(); }
	inline void setExercisesListVersion(const QString &new_value) { changeValue(GLOBAL_GROUP, EXERCISES_VERSION_INDEX, new_value); }

	Q_INVOKABLE QString availableLanguagesLabel(const uint language_idx) const;
	QStringList availableLanguages() const;

	inline const QString &localAppFilesDir() const { return m_localAppFilesDir; }
	QString userDir(const QString &userid) const;
	inline QString currentUserDir() const { return userDir(currentUser()); }
	void changeUserSettings(const QString &userid);

signals:
	void currentUserChanged();
	void serverAddressChanged();

private:
	QMap<uint,QLatin1StringView> m_propertyNames;
	qreal m_ratioFont;
	uint m_windowWidth, m_windowHeight, m_qmlPageHeight;
	double m_HeightToWidth;
	QString m_localAppFilesDir;

	void getScreenMeasures();
	static TPSettings* app_settings;
	friend TPSettings* appSettings();

	inline QVariant getValue(const QString &group, const uint index, const QVariant &default_value = QVariant{}) const
	{
		return value(group + '/' + (group == GLOBAL_GROUP ? m_propertyNames.value(index) :
										m_userPropertyNames.value(index)), default_value);
	}

	void changeValue(const QString &group, const uint index, const QVariant &new_value, const bool dosync = true);
//--------------------------------------------GLOBAL SETTINGS---------------------------------------------//

//--------------------------------------------USER   SETTINGS---------------------------------------------//
public:
	void userSettingsInit();

	inline QString userLocale() const { return getValue(currentUser(), USER_LOCALE_INDEX, m_defaultValues.at(USER_LOCALE_INDEX)).toString(); }
	void setUserLocale(const QString &locale, const bool write_to_file);
	Q_INVOKABLE void setUserLocale(const uint language_idx, const bool write_to_file);

	inline QString themeStyle() const { return getValue(currentUser(), THEME_STYLE_INDEX, m_defaultValues.at(THEME_STYLE_INDEX)).toString(); }
	inline void setThemeStyle(const QString &new_value) { changeValue(currentUser(), THEME_STYLE_INDEX, new_value); emit themeStyleChanged(); }

	QStringList colorSchemes() const { return m_colorSchemes; }
	void setColorScheme(const uint new_value, const bool bFromQml = true);
	inline uint colorScheme() const { return getValue(currentUser(), COLOR_SCHEME_INDEX, m_defaultValues.at(COLOR_SCHEME_INDEX)).toUInt(); }
	inline QString primaryColor() const { return getValue(currentUser(), COLOR_INDEX, colorForScheme(colorScheme())).toString(); }
	void setPrimaryColor(const QColor &color);
	inline QString primaryLightColor() const { return getValue(currentUser(), LIGHT_COLOR_INDEX, lightColorForScheme(colorScheme())).toString(); }
	void setPrimaryLightColor(const QColor &color);
	inline QString primaryDarkColor() const { return getValue(currentUser(), DARK_COLOR_INDEX, darkColorForScheme(colorScheme())).toString(); }
	void setPrimaryDarkColor(const QColor &color);
	inline QString paneBackgroundColor() const { return getValue(currentUser(), PANE_COLOR_INDEX, paneColorForScheme(colorScheme())).toString(); }
	inline QString entrySelectedColor() const { return getValue(currentUser(), SELECTED_COLOR_INDEX, selectedColorForScheme(colorScheme())).toString(); };
	inline QString listEntryColor1() const { { return getValue(currentUser(), LISTS_COLOR_1_INDEX, listColor1ForScheme(colorScheme())).toString(); } }
	inline QString listEntryColor2() const { { return getValue(currentUser(), LISTS_COLOR_2_INDEX, listColor2ForScheme(colorScheme())).toString(); } }
	inline QString fontColor() const { return getValue(currentUser(), FONT_COLOR_INDEX, m_defaultValues.at(FONT_COLOR_INDEX)).toString(); }
	void setFontColor(const QColor &color);
	inline QString disabledFontColor() const { return getValue(currentUser(), DISABLED_FONT_COLOR_INDEX, m_defaultValues.at(DISABLED_FONT_COLOR_INDEX)).toString(); }
	void setDisabledFontColor(const QColor &color);

	Q_INVOKABLE QString colorForScheme(const uint scheme) const;
	Q_INVOKABLE QString lightColorForScheme(const uint scheme) const;
	Q_INVOKABLE QString darkColorForScheme(const uint scheme) const;
	QString listColor1ForScheme(const uint scheme) const;
	QString listColor2ForScheme(const uint scheme) const;
	QString paneColorForScheme(const uint scheme) const;
	QString selectedColorForScheme(const uint scheme) const;

	void setFontSize(const uint new_value, const bool bFromQml = true);
	inline uint fontSize() const { return getValue(currentUser(), FONT_SIZE_INDEX, m_defaultValues.at(FONT_SIZE_INDEX)).toUInt(); }
	inline uint smallFontSize() const { return m_defaultValues.at(SMALLFONT_SIZE_INDEX).toUInt(); }
	inline uint largeFontSize() const { return m_defaultValues.at(LARGEFONT_SIZE_INDEX).toUInt(); }
	inline uint extraLargeFontSize() const { return m_defaultValues.at(EXTRALARGEFONT_SIZE_INDEX).toUInt(); }
	inline uint itemDefaultHeight() const { return m_defaultValues.at(ITEM_DEFAULT_HEIGHT).toUInt(); }
	inline uint itemSmallHeight() const { return static_cast<uint>(qFloor(static_cast<float>(m_defaultValues.at(ITEM_DEFAULT_HEIGHT).toUInt()) * 0.8)); }
	inline uint itemLargeHeight() const { return static_cast<uint>(qFloor(static_cast<float>(m_defaultValues.at(ITEM_DEFAULT_HEIGHT).toUInt()) * 1.2)); }
	inline uint itemExtraLargeHeight() const { return static_cast<uint>(qFloor(static_cast<float>(m_defaultValues.at(ITEM_DEFAULT_HEIGHT).toUInt()) * 1.5)); }

	inline QString weightUnit() const { return getValue(currentUser(), WEIGHT_UNIT_INDEX, m_defaultValues.at(WEIGHT_UNIT_INDEX)).toString(); }
	inline void setWeightUnit(const QString &new_value) { changeValue(currentUser(), WEIGHT_UNIT_INDEX, new_value); emit weightUnitChanged(); }

	inline int lastViewedMesoIdx() const { return getValue(currentUser(), MESO_IDX_INDEX, m_defaultValues.at(MESO_IDX_INDEX)).toInt(); }
	inline void setLastViewedMesoIdx(const int new_value) { changeValue(currentUser(), MESO_IDX_INDEX, QString::number(new_value)); emit lastViewedMesoIdxChanged(); }

	inline uint userLocaleIdx() const { return m_languageIdx; }
	inline uint weatherCitiesCount() const { return m_weatherLocations.count(); }
	void addWeatherCity(const QString &city, const QString &latitude, const QString &longitude);
	Q_INVOKABLE void removeWeatherCity(const uint idx);
	Q_INVOKABLE QString weatherCity(const uint idx);
	Q_INVOKABLE QGeoCoordinate weatherCityCoordinates(const uint idx);

	inline bool alwaysAskConfirmation() const { return getValue(currentUser(), ASK_CONFIRMATION_INDEX, m_defaultValues.at(ASK_CONFIRMATION_INDEX)).toBool(); }
	inline void setAlwaysAskConfirmation(const bool new_value) { changeValue(currentUser(), ASK_CONFIRMATION_INDEX, QString::number(new_value)); emit alwaysAskConfirmationChanged(); }

	void userSwitchingActions();

signals:
	void userLocaleChanged();
	void themeStyleChanged();
	void colorChanged();
	void weightUnitChanged();
	void fontSizeChanged();
	void lastViewedMesoIdxChanged();
	void weatherCitiesCountChanged();
	void alwaysAskConfirmationChanged();

private:
	QMap<uint,QLatin1StringView> m_userPropertyNames;
	QStringList m_defaultValues;
	QStringList m_weatherLocations;
	QStringList m_colorSchemes;
	QString m_userId;
	int m_languageIdx;
//--------------------------------------------USER   SETTINGS---------------------------------------------//
};

inline TPSettings* appSettings() { return TPSettings::app_settings; }
