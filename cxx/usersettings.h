#pragma once

#include <QGeoCoordinate>
#include <QSettings>

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
#define EXERCISES_VERSION_INDEX 12
#define MESO_IDX_INDEX 13
#define FONT_SIZE_INDEX 14
#define SMALLFONT_SIZE_INDEX 15
#define LARGEFONT_SIZE_INDEX 16
#define EXTRALARGEFONT_SIZE_INDEX 17
#define COLOR_SCHEME_INDEX 18
#define ITEM_DEFAULT_HEIGHT 19
#define ASK_CONFIRMATION_INDEX 20
#define WEATHER_CITIES_INDEX 21

#define USER_SETTINGS_FIELD_COUNT WEATHER_CITIES_INDEX + 1

class UserSettings : public QSettings
{

Q_OBJECT

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

public:
	explicit UserSettings(const QString &userid, QObject *parent = nullptr);

	inline QString userLocale() const { return value(m_propertyNames.value(USER_LOCALE_INDEX), m_defaultValues.at(USER_LOCALE_INDEX)).toString(); }
	void setUserLocale(const QString &locale);
	Q_INVOKABLE void setUserLocale(const uint language_idx);

	inline QString themeStyle() const { return value(m_propertyNames.value(THEME_STYLE_INDEX), m_defaultValues.at(THEME_STYLE_INDEX)).toString(); }
	inline void setThemeStyle(const QString &new_value) { changeValue(THEME_STYLE_INDEX, new_value); emit themeStyleChanged(); }

	QStringList colorSchemes() const { return m_colorSchemes; }
	void setColorScheme(const uint new_value, const bool bFromQml = true);
	inline uint colorScheme() const { return m_defaultValues.at(COLOR_SCHEME_INDEX).toUInt(); }
	QString primaryColor() const;
	void setPrimaryColor(const QColor &color);
	QString primaryLightColor() const;
	void setPrimaryLightColor(const QColor &color);
	QString primaryDarkColor() const;
	void setPrimaryDarkColor(const QColor &color);
	QString paneBackgroundColor() const;
	QString entrySelectedColor() const;
	inline QString listEntryColor1() const { return m_defaultValues.at(LISTS_COLOR_1_INDEX); }
	inline QString listEntryColor2() const { return m_defaultValues.at(LISTS_COLOR_2_INDEX); }
	inline QString fontColor() const { return value(m_propertyNames.value(FONT_COLOR_INDEX), m_defaultValues.at(FONT_COLOR_INDEX)).toString(); }
	void setFontColor(const QColor &color);
	inline QString disabledFontColor() const { return value(m_propertyNames.value(DISABLED_FONT_COLOR_INDEX), m_defaultValues.at(DISABLED_FONT_COLOR_INDEX)).toString(); }
	void setDisabledFontColor(const QColor &color);

	Q_INVOKABLE QString colorForScheme(const uint scheme) const;
	Q_INVOKABLE QString lightColorForScheme(const uint scheme) const;
	Q_INVOKABLE QString darkColorForScheme(const uint scheme) const;
	QString paneColorForScheme(const uint scheme) const;
	QString selectedColorForScheme(const uint scheme) const;

	void setFontSize(const uint new_value, const bool bFromQml = true);
	inline uint fontSize() const { return m_defaultValues.at(FONT_SIZE_INDEX).toUInt(); }
	inline uint smallFontSize() const { return m_defaultValues.at(SMALLFONT_SIZE_INDEX).toUInt(); }
	inline uint largeFontSize() const { return m_defaultValues.at(LARGEFONT_SIZE_INDEX).toUInt(); }
	inline uint extraLargeFontSize() const { return m_defaultValues.at(EXTRALARGEFONT_SIZE_INDEX).toUInt(); }
	inline uint itemDefaultHeight() const { return m_defaultValues.at(ITEM_DEFAULT_HEIGHT).toUInt(); }
	inline uint itemSmallHeight() const { return static_cast<uint>(qFloor(static_cast<float>(m_defaultValues.at(ITEM_DEFAULT_HEIGHT).toUInt()) * 0.8)); }
	inline uint itemLargeHeight() const { return static_cast<uint>(qFloor(static_cast<float>(m_defaultValues.at(ITEM_DEFAULT_HEIGHT).toUInt()) * 1.2)); }
	inline uint itemExtraLargeHeight() const { return static_cast<uint>(qFloor(static_cast<float>(m_defaultValues.at(ITEM_DEFAULT_HEIGHT).toUInt()) * 1.5)); }

	inline QString weightUnit() const { return value(m_propertyNames.value(WEIGHT_UNIT_INDEX), m_defaultValues.at(WEIGHT_UNIT_INDEX)).toString(); }
	inline void setWeightUnit(const QString &new_value) { changeValue(WEIGHT_UNIT_INDEX, new_value); emit weightUnitChanged(); }
	inline QString exercisesListVersion() const { return value(m_propertyNames.value(EXERCISES_VERSION_INDEX), m_defaultValues.at(EXERCISES_VERSION_INDEX)).toString(); }
	inline void setExercisesListVersion(const QString &new_value) { changeValue(EXERCISES_VERSION_INDEX, new_value); }

	inline int lastViewedMesoIdx() const { return value(m_propertyNames.value(MESO_IDX_INDEX), m_defaultValues.at(MESO_IDX_INDEX)).toInt(); }
	inline void setLastViewedMesoIdx(const int new_value) { changeValue(MESO_IDX_INDEX, QString::number(new_value)); emit lastViewedMesoIdxChanged(); }

	inline uint userLocaleIdx() const { return m_languageIdx; }
	inline uint weatherCitiesCount() const { return m_weatherLocations.count(); }
	void addWeatherCity(const QString &city, const QString &latitude, const QString &longitude);
	Q_INVOKABLE void removeWeatherCity(const uint idx);
	Q_INVOKABLE QString weatherCity(const uint idx);
	Q_INVOKABLE QGeoCoordinate weatherCityCoordinates(const uint idx);

	inline bool alwaysAskConfirmation() const { return value(m_propertyNames.value(ASK_CONFIRMATION_INDEX), m_defaultValues.at(ASK_CONFIRMATION_INDEX)).toBool(); }
	inline void setAlwaysAskConfirmation(const bool new_value) { changeValue(ASK_CONFIRMATION_INDEX, QString::number(new_value)); emit alwaysAskConfirmationChanged(); }

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
	QMap<uint,QLatin1StringView> m_propertyNames;
	QStringList m_defaultValues;
	QStringList m_weatherLocations;
	QStringList m_colorSchemes;
	QString m_userId;
	int m_languageIdx;

	inline void changeValue(const uint index, const QVariant &new_value)
	{
		setValue(m_propertyNames.value(index), new_value);
		sync();
	}
};
