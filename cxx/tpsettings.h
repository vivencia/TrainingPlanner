#ifndef TPSETTINGS_H
#define TPSETTINGS_H

#include <QSettings>

#define APP_VERSION_INDEX 0
#define APP_LOCALE_INDEX 1
#define THEME_STYLE_INDEX 2
#define COLOR_SCHEME_INDEX 3
#define COLOR_INDEX 4
#define DARK_COLOR_INDEX 5
#define LIGHT_COLOR_INDEX 6
#define PANE_COLOR_INDEX 7
#define SELECTED_COLOR_INDEX 8
#define LISTS_COLOR_1_INDEX 9
#define LISTS_COLOR_2_INDEX 10
#define FONT_COLOR_INDEX 11
#define DISABLED_FONT_COLOR_INDEX 12
#define WEIGHT_UNIT_INDEX 13
#define EXERCISES_VERSION_INDEX 14
#define ICON_FOLDER_INDEX 15

#define FONT_SIZE_INDEX 16
#define TEXT_FONT_SIZE_INDEX 17
#define LIST_FONT_SIZE_INDEX 18
#define TITLE_FONT_SIZE_INDEX 19
#define MESO_IDX_INDEX 20
#define PAGE_WIDTH_INDEX 21
#define PAGE_HEIGHT_INDEX 22
#define ITEM_MAX_WIDTH 23

#define ASK_CONFIRMATION_INDEX 24
#define USER_INDEX 25

#define WEATHER_CITIES_INDEX 26

class TPSettings : public QSettings
{

Q_OBJECT

Q_PROPERTY(QString appVersion READ appVersion CONSTANT)
Q_PROPERTY(QString appLocale READ appLocale WRITE setAppLocale NOTIFY appLocaleChanged)
Q_PROPERTY(QString themeStyle READ themeStyle WRITE setThemeStyle NOTIFY themeStyleChanged)
Q_PROPERTY(QString colorScheme READ colorScheme WRITE setColorScheme NOTIFY colorSchemeChanged)
Q_PROPERTY(QString primaryColor READ primaryColor WRITE setPrimaryColor NOTIFY primaryColorChanged)
Q_PROPERTY(QString primaryDarkColor READ primaryDarkColor WRITE setPrimaryDarkColor NOTIFY primaryDarkColorChanged)
Q_PROPERTY(QString primaryLightColor READ primaryLightColor WRITE setPrimaryLightColor NOTIFY primaryLightColorChanged)
Q_PROPERTY(QString paneBackgroundColor READ paneBackgroundColor WRITE setPaneBackgroundColor NOTIFY paneBackgroundColorChanged)
Q_PROPERTY(QString entrySelectedColor READ entrySelectedColor WRITE setEntrySelectedColor NOTIFY entrySelectedColorChanged)
Q_PROPERTY(QString listEntryColor1 READ listEntryColor1 WRITE setListEntryColor2 NOTIFY listEntryColor1Changed)
Q_PROPERTY(QString listEntryColor2 READ listEntryColor2 WRITE setListEntryColor2 NOTIFY listEntryColor2Changed)
Q_PROPERTY(QString fontColor READ fontColor WRITE setFontColor NOTIFY fontColorChanged)
Q_PROPERTY(QString disabledFontColor READ disabledFontColor WRITE setDisabledFontColor NOTIFY disabledFontColorChanged)
Q_PROPERTY(QString weightUnit READ weightUnit WRITE setWeightUnit NOTIFY weightUnitChanged)
Q_PROPERTY(QString iconFolder READ iconFolder WRITE setIconFolder NOTIFY iconFolderChanged)

Q_PROPERTY(uint fontSize READ fontSize WRITE setFontSize NOTIFY fontSizeChanged)
Q_PROPERTY(uint fontSizeText READ fontSizeText WRITE setFontSizeText NOTIFY fontSizeTextChanged)
Q_PROPERTY(uint fontSizeLists READ fontSizeLists WRITE setFontSizeLists NOTIFY fontSizeListsChanged)
Q_PROPERTY(uint fontSizeTitle READ fontSizeTitle WRITE setFontSizeTitle NOTIFY fontSizeTitleChanged)
Q_PROPERTY(uint lastViewedMesoIdx READ lastViewedMesoIdx WRITE setLastViewedMesoIdx NOTIFY lastViewedMesoIdxChanged)
Q_PROPERTY(uint pageWidth READ pageWidth CONSTANT)
Q_PROPERTY(uint pageHeight READ pageHeight CONSTANT)
Q_PROPERTY(uint weatherCitiesCount READ weatherCitiesCount NOTIFY weatherCitiesCountChanged)
Q_PROPERTY(uint itemMaxWidth READ itemMaxWidth CONSTANT)

Q_PROPERTY(bool alwaysAskConfirmation READ alwaysAskConfirmation WRITE setAlwaysAskConfirmation NOTIFY alwaysAskConfirmationChanged)
Q_PROPERTY(bool mainUserConfigured READ mainUserConfigured WRITE setMainUserConfigured NOTIFY mainUserConfiguredChanged)

public:
	explicit TPSettings(QObject* parent = nullptr);

	inline QString appVersion() const { return value(m_propertyNames.at(APP_VERSION_INDEX), m_defaultValues.at(APP_VERSION_INDEX)).toString(); }

	inline QString appLocale() const { return value(m_propertyNames.at(APP_LOCALE_INDEX), m_defaultValues.at(APP_LOCALE_INDEX)).toString(); }
	inline void setAppLocale(const QString& new_value) { changeValue(APP_LOCALE_INDEX, new_value); emit appLocaleChanged(); }

	inline QString themeStyle() const { return value(m_propertyNames.at(THEME_STYLE_INDEX), m_defaultValues.at(THEME_STYLE_INDEX)).toString(); }
	inline void setThemeStyle(const QString& new_value) { changeValue(THEME_STYLE_INDEX, new_value); emit themeStyleChanged(); }

	inline QString colorScheme() const { return value(m_propertyNames.at(COLOR_SCHEME_INDEX), m_defaultValues.at(COLOR_SCHEME_INDEX)).toString(); }
	inline void setColorScheme(const QString& new_value) { changeValue(COLOR_SCHEME_INDEX, new_value); emit colorSchemeChanged(); }

	inline QString primaryColor() const { return value(m_propertyNames.at(COLOR_INDEX), m_defaultValues.at(COLOR_INDEX)).toString(); }
	inline void setPrimaryColor(const QString& new_value) { changeValue(COLOR_INDEX, new_value); emit primaryColorChanged(); }

	inline QString primaryDarkColor() const { return value(m_propertyNames.at(DARK_COLOR_INDEX), m_defaultValues.at(DARK_COLOR_INDEX)).toString(); }
	inline void setPrimaryDarkColor(const QString& new_value) { changeValue(DARK_COLOR_INDEX, new_value); emit primaryDarkColorChanged(); }

	inline QString primaryLightColor() const { return value(m_propertyNames.at(LIGHT_COLOR_INDEX), m_defaultValues.at(LIGHT_COLOR_INDEX)).toString(); }
	inline void setPrimaryLightColor(const QString& new_value) { changeValue(LIGHT_COLOR_INDEX, new_value); emit primaryLightColorChanged(); }

	inline QString paneBackgroundColor() const { return value(m_propertyNames.at(PANE_COLOR_INDEX), m_defaultValues.at(PANE_COLOR_INDEX)).toString(); }
	inline void setPaneBackgroundColor(const QString& new_value) { changeValue(PANE_COLOR_INDEX, new_value); emit paneBackgroundColorChanged(); }

	inline QString entrySelectedColor() const { return value(m_propertyNames.at(SELECTED_COLOR_INDEX), m_defaultValues.at(SELECTED_COLOR_INDEX)).toString(); }
	inline void setEntrySelectedColor(const QString& new_value) { changeValue(SELECTED_COLOR_INDEX, new_value); emit entrySelectedColorChanged(); }

	inline QString listEntryColor1() const { return value(m_propertyNames.at(LISTS_COLOR_1_INDEX), m_defaultValues.at(LISTS_COLOR_1_INDEX)).toString(); }
	inline void setListEntryColor1(const QString& new_value) { changeValue(LISTS_COLOR_1_INDEX, new_value); emit listEntryColor1Changed(); }

	inline QString listEntryColor2() const { return value(m_propertyNames.at(LISTS_COLOR_2_INDEX), m_defaultValues.at(LISTS_COLOR_2_INDEX)).toString(); }
	inline void setListEntryColor2(const QString& new_value) { changeValue(LISTS_COLOR_2_INDEX, new_value); emit listEntryColor2Changed(); }

	inline QString fontColor() const { return value(m_propertyNames.at(FONT_COLOR_INDEX), m_defaultValues.at(FONT_COLOR_INDEX)).toString(); }
	inline void setFontColor(const QString& new_value) { changeValue(FONT_COLOR_INDEX, new_value); emit fontColorChanged(); }

	inline QString disabledFontColor() const { return value(m_propertyNames.at(DISABLED_FONT_COLOR_INDEX), m_defaultValues.at(DISABLED_FONT_COLOR_INDEX)).toString(); }
	inline void setDisabledFontColor(const QString& new_value) { changeValue(DISABLED_FONT_COLOR_INDEX, new_value); emit disabledFontColorChanged(); }

	inline QString weightUnit() const { return value(m_propertyNames.at(WEIGHT_UNIT_INDEX), m_defaultValues.at(WEIGHT_UNIT_INDEX)).toString(); }
	inline void setWeightUnit(const QString& new_value) { changeValue(WEIGHT_UNIT_INDEX, new_value); emit weightUnitChanged(); }

	inline QString exercisesListVersion() const { return value(m_propertyNames.at(EXERCISES_VERSION_INDEX), m_defaultValues.at(EXERCISES_VERSION_INDEX)).toString(); }
	inline void setExercisesListVersion(const QString& new_value) { changeValue(EXERCISES_VERSION_INDEX, new_value); }

	inline QString iconFolder() const { return value(m_propertyNames.at(ICON_FOLDER_INDEX), m_defaultValues.at(ICON_FOLDER_INDEX)).toString(); }
	inline void setIconFolder(const QString& new_value) { changeValue(ICON_FOLDER_INDEX, new_value); emit iconFolderChanged(); }

	inline uint fontSize() const { return value(m_propertyNames.at(FONT_SIZE_INDEX), m_defaultValues.at(FONT_SIZE_INDEX).toUInt()).toUInt(); }
	inline void setFontSize(const uint new_value) { changeValue(FONT_SIZE_INDEX, QString::number(new_value)); emit fontSizeChanged(); }

	inline uint fontSizeText() const { return value(m_propertyNames.at(TEXT_FONT_SIZE_INDEX), m_defaultValues.at(TEXT_FONT_SIZE_INDEX).toUInt()).toUInt(); }
	inline void setFontSizeText(const uint new_value) { changeValue(TEXT_FONT_SIZE_INDEX, QString::number(new_value)); emit fontSizeTextChanged(); }

	inline uint fontSizeLists() const { return value(m_propertyNames.at(LIST_FONT_SIZE_INDEX), m_defaultValues.at(LIST_FONT_SIZE_INDEX).toUInt()).toUInt(); }
	inline void setFontSizeLists(const uint new_value) { changeValue(LIST_FONT_SIZE_INDEX, QString::number(new_value)); emit fontSizeListsChanged(); }

	inline uint fontSizeTitle() const { return value(m_propertyNames.at(TITLE_FONT_SIZE_INDEX), m_defaultValues.at(TITLE_FONT_SIZE_INDEX).toUInt()).toUInt(); }
	inline void setFontSizeTitle(const uint new_value) { changeValue(TITLE_FONT_SIZE_INDEX, QString::number(new_value)); emit fontSizeTitleChanged(); }

	inline uint lastViewedMesoIdx() const { return value(m_propertyNames.at(MESO_IDX_INDEX), m_defaultValues.at(MESO_IDX_INDEX).toUInt()).toUInt(); }
	inline void setLastViewedMesoIdx(const uint new_value) { changeValue(MESO_IDX_INDEX, QString::number(new_value)); emit lastViewedMesoIdxChanged(); }

	inline uint pageWidth() const { return value(m_propertyNames.at(PAGE_WIDTH_INDEX), m_defaultValues.at(PAGE_WIDTH_INDEX).toUInt()).toUInt(); }

	//mainWindow.height(640) - NavBar.height(40)
	inline uint pageHeight() const { return value(m_propertyNames.at(PAGE_HEIGHT_INDEX), m_defaultValues.at(PAGE_HEIGHT_INDEX).toUInt()).toUInt(); }

	inline uint itemMaxWidth() const { return value(m_propertyNames.at(ITEM_MAX_WIDTH), m_defaultValues.at(ITEM_MAX_WIDTH).toUInt()).toUInt(); }

	inline uint weatherCitiesCount() const { return m_weatherCities.count(); }
	void setCurrentWeatherCity(const QString& city);
	Q_INVOKABLE void removeWeatherCity(const int index);
	Q_INVOKABLE inline QString weatherCity(const int idx) const
	{
		return value(m_propertyNames.at(WEATHER_CITIES_INDEX), m_defaultValues.at(WEATHER_CITIES_INDEX)).value<QStringList>().at(idx);
	}

	inline bool alwaysAskConfirmation() const { return value(m_propertyNames.at(ASK_CONFIRMATION_INDEX), m_defaultValues.at(ASK_CONFIRMATION_INDEX).toUInt()).toBool(); }
	inline void setAlwaysAskConfirmation(const bool new_value) { changeValue(ASK_CONFIRMATION_INDEX, QString::number(new_value)); emit alwaysAskConfirmationChanged(); }

	inline bool mainUserConfigured() const { return value(m_propertyNames.at(USER_INDEX), m_defaultValues.at(USER_INDEX)).toUInt() != 0; }
	inline void setMainUserConfigured(const bool new_value) { changeValue(USER_INDEX, QString::number(static_cast<uint>(new_value))); emit mainUserConfiguredChanged(); }

signals:
	void appLocaleChanged();
	void themeStyleChanged();
	void colorSchemeChanged();
	void primaryColorChanged();
	void primaryDarkColorChanged();
	void primaryLightColorChanged();
	void paneBackgroundColorChanged();
	void entrySelectedColorChanged();
	void listEntryColor1Changed();
	void listEntryColor2Changed();
	void fontColorChanged();
	void disabledFontColorChanged();
	void weightUnitChanged();
	void iconFolderChanged();
	void fontSizeChanged();
	void fontSizeListsChanged();
	void fontSizeTextChanged();
	void fontSizeTitleChanged();
	void lastViewedMesoIdxChanged();
	void weatherCitiesCountChanged();
	void alwaysAskConfirmationChanged();
	void mainUserConfiguredChanged();

private:
	QStringList m_propertyNames;
	QStringList m_defaultValues;
	QStringList m_weatherCities;

	inline void changeValue(const uint index, const QVariant& new_value)
	{
		setValue(m_propertyNames.at(index), new_value);
		sync();
	}

	static TPSettings* app_settings;
	friend TPSettings* appSettings();
};

inline TPSettings* appSettings() { return TPSettings::app_settings; }
#endif // TPSETTINGS_H
