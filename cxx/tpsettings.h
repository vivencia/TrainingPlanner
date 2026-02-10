#pragma once

#include <QSettings>
#include <QTimer>

//--------------------------------------------GLOBAL SETTINGS---------------------------------------------//
using namespace Qt::Literals::StringLiterals;

constexpr QLatin1StringView TP_APP_VERSION{"v20260123 Build 1"};
constexpr QLatin1StringView GLOBAL_GROUP{"app"};
constexpr QLatin1StringView DEFAULT_USER{"default"};

enum GlobalSettingFields {
	APP_VERSION_INDEX,
	WINDOW_WIDTH_INDEX,
	WINDOW_HEIGHT_INDEX,
	PAGE_WIDTH_INDEX,
	PAGE_HEIGHT_INDEX,
	HEIGHT_TO_WIDTH_RATIO_INDEX,
	FONT_RATIO,
	CURRENT_USER,
	SERVER_ADDRESS,
	APP_SETTINGS_FIELD_COUNT
};
//--------------------------------------------GLOBAL SETTINGS---------------------------------------------//

//--------------------------------------------USER   SETTINGS---------------------------------------------//
enum UserSettingFields {
	USER_LOCALE_INDEX,
	THEME_STYLE_INDEX,
	COLOR_INDEX,
	LIGHT_COLOR_INDEX,
	DARK_COLOR_INDEX,
	PANE_COLOR_INDEX,
	SELECTED_COLOR_INDEX,
	LISTS_COLOR_1_INDEX,
	LISTS_COLOR_2_INDEX,
	FONT_COLOR_INDEX,
	DISABLED_FONT_COLOR_INDEX,
	WEIGHT_UNIT_INDEX,
	FONT_SIZE_INDEX,
	SMALLFONT_SIZE_INDEX,
	LARGEFONT_SIZE_INDEX,
	EXTRALARGEFONT_SIZE_INDEX,
	COLOR_SCHEME_INDEX,
	ITEM_DEFAULT_HEIGHT,
	ASK_CONFIRMATION_INDEX,
	USER_SETTINGS_FIELD_COUNT
};
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
Q_PROPERTY(uint userLocaleIdx READ userLocaleIdx NOTIFY userLocaleChanged FINAL)

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
Q_PROPERTY(QString settingsBackground READ settingsBackground NOTIFY colorChanged FINAL)
Q_PROPERTY(QString userBackground READ userBackground NOTIFY colorChanged FINAL)
Q_PROPERTY(QString coachesBackground READ coachesBackground NOTIFY colorChanged FINAL)
Q_PROPERTY(QString clientsBackground READ clientsBackground NOTIFY colorChanged FINAL)
Q_PROPERTY(QString weatherBackground READ weatherBackground NOTIFY colorChanged FINAL)
Q_PROPERTY(bool alwaysAskConfirmation READ alwaysAskConfirmation WRITE setAlwaysAskConfirmation NOTIFY alwaysAskConfirmationChanged)
Q_PROPERTY(QStringList colorSchemes READ colorSchemes FINAL CONSTANT)
//--------------------------------------------USER   SETTINGS---------------------------------------------//

public:
	explicit TPSettings(QObject *parent = nullptr);

//--------------------------------------------GLOBAL SETTINGS---------------------------------------------//
	void globalSettingsInit();
	QString userConfigFileName(const bool fullpath, const QString &userid = QString{}) const;
	void importFromUserConfig(const QString &userid);
	bool exportToUserConfig(const QString &userid);
#ifndef QT_NO_QDEBUG
#ifndef Q_OS_ANDROID
	void setReadOnlyGroup(const QString &group, const bool read_only);
#endif
#endif

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

	Q_INVOKABLE QString availableLanguagesLabel(const uint language_idx) const;
	QStringList availableLanguages() const;

	inline const QString &localAppFilesDir() const { return m_localAppFilesDir; }
	inline QString currentUserDir() const { return m_localAppFilesDir + currentUser() + '/'; }

	inline bool appExiting() const { return m_appExiting; }

signals:
	void currentUserChanged();
	void serverAddressChanged();

private:
	QMap<uint,QLatin1StringView> m_globalPropertyNames;
	qreal m_ratioFont;
	uint m_windowWidth, m_windowHeight, m_qmlPageHeight;
	double m_HeightToWidth;
	QString m_localAppFilesDir;
	QTimer m_timer;
	bool m_appExiting;

#ifndef QT_NO_QDEBUG
#ifndef Q_OS_ANDROID
	QHash<QString,QVariant> m_readOnlyValues;
	QStringList m_readOnlyGroups;
	inline bool isGroupReadOnly(const QString &group) const { return m_readOnlyGroups.contains(group); }

	inline QVariant getValue(const QString &group, const uint index, const QVariant &default_value = QVariant{}) const
	{
		return getValue(group, (group == GLOBAL_GROUP ? m_globalPropertyNames.value(index) :
															m_userPropertyNames.value(index)), default_value);
	}
	inline QVariant getValue(const QString &group, const QString &field_name, const QVariant &default_value = QVariant{}) const
	{
		if (!isGroupReadOnly(group))
			return value(group + '/' + field_name, default_value);
		else
		{
			const QVariant &read_only_value{m_readOnlyValues.value(group + '/' + field_name)};
			return !read_only_value.isNull() ? read_only_value : value(group + '/' + field_name, default_value);
		}
	}
#endif
#else
	inline QVariant getValue(const QString &group, const uint index, const QVariant &default_value = QVariant{}) const
	{
		return getValue(group, (group == GLOBAL_GROUP ? m_globalPropertyNames.value(index) :
															m_userPropertyNames.value(index)), default_value);
	}
	inline QVariant getValue(const QString &group, const QString &field_name, const QVariant &default_value = QVariant{}) const
	{
		return value(group + '/' + field_name, default_value);
	}
#endif
	inline void changeValue(const QString &group, const uint index, const QVariant &new_value, const bool dosync = true)
	{
		changeValue(group, group == GLOBAL_GROUP ? m_globalPropertyNames.value(index) : m_userPropertyNames.value(index),
									new_value, dosync);
	}
	void changeValue(const QString &group, const QString &field_name, const QVariant &new_value, const bool dosync = true);
	void getScreenMeasures();
	static TPSettings* app_settings;
	friend TPSettings* appSettings();
//--------------------------------------------GLOBAL SETTINGS---------------------------------------------//

//--------------------------------------------USER   SETTINGS---------------------------------------------//
public:
	void userSettingsInit();

	inline QString userLocale() const { return getValue(currentUser(), USER_LOCALE_INDEX, m_defaultValues.at(USER_LOCALE_INDEX)).toString(); }
	void setUserLocale(const QString &locale, const bool write_to_file);
	void setUserLocale(const uint language_idx, const bool write_to_file);

	inline QString themeStyle() const { return getValue(currentUser(), THEME_STYLE_INDEX, m_defaultValues.at(THEME_STYLE_INDEX)).toString(); }
	inline void setThemeStyle(const QString &new_value) { changeValue(currentUser(), THEME_STYLE_INDEX, new_value); emit themeStyleChanged(); }

	QStringList colorSchemes() const { return m_colorSchemes; }
	void setColorScheme(const uint new_value, const bool bFromQml = true);
	inline uint colorScheme() const { return getValue(currentUser(), COLOR_SCHEME_INDEX, m_defaultValues.at(COLOR_SCHEME_INDEX)).toUInt(); }
	inline QString primaryColor() const { return colorForScheme(colorScheme()); }
	void setPrimaryColor(const QColor &color);
	inline QString primaryLightColor() const { return lightColorForScheme(colorScheme()); }
	void setPrimaryLightColor(const QColor &color);
	inline QString primaryDarkColor() const { return darkColorForScheme(colorScheme()); }
	void setPrimaryDarkColor(const QColor &color);
	inline QString paneBackgroundColor() const { return paneColorForScheme(colorScheme()); }
	inline QString entrySelectedColor() const { return selectedColorForScheme(colorScheme()); };
	inline QString listEntryColor1() const { { return listColor1ForScheme(colorScheme()); } }
	inline QString listEntryColor2() const { { return listColor2ForScheme(colorScheme()); } }
	inline QString fontColor() const { return fontColorForScheme(colorScheme()); }
	void setFontColor(const QColor &color);
	inline QString disabledFontColor() const { return disabledFontColorForScheme(colorScheme()); }
	void setDisabledFontColor(const QColor &color);

	Q_INVOKABLE QString colorForScheme(const uint scheme) const;
	Q_INVOKABLE QString lightColorForScheme(const uint scheme) const;
	Q_INVOKABLE QString darkColorForScheme(const uint scheme) const;
	QString listColor1ForScheme(const uint scheme) const;
	QString listColor2ForScheme(const uint scheme) const;
	QString paneColorForScheme(const uint scheme) const;
	QString selectedColorForScheme(const uint scheme) const;
	QString fontColorForScheme(const uint scheme) const;
	QString disabledFontColorForScheme(const uint scheme) const;

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
	inline uint userLocaleIdx() const { return m_languageIdx; }

	QString indexColorSchemeToColorSchemeName() const;
	inline QString settingsBackground() const
	{
		return ":/images/backgrounds/settings_"_L1 + indexColorSchemeToColorSchemeName() + ".jpg"_L1;
	}
	inline QString userBackground() const
	{
		return ":/images/backgrounds/user_"_L1 + indexColorSchemeToColorSchemeName() + ".jpg"_L1;
	}
	inline QString coachesBackground() const
	{
		return ":/images/backgrounds/coaches_"_L1 + indexColorSchemeToColorSchemeName() + ".jpg"_L1;
	}
	inline QString clientsBackground() const
	{
		return ":/images/backgrounds/clients_"_L1 + indexColorSchemeToColorSchemeName() + ".jpg"_L1;
	}
	inline QString weatherBackground() const
	{
		return ":/images/backgrounds/weather_"_L1 + indexColorSchemeToColorSchemeName() + ".jpg"_L1;
	}

	inline bool alwaysAskConfirmation() const { return getValue(currentUser(), ASK_CONFIRMATION_INDEX, m_defaultValues.at(ASK_CONFIRMATION_INDEX)).toBool(); }
	inline void setAlwaysAskConfirmation(const bool new_value) { changeValue(currentUser(), ASK_CONFIRMATION_INDEX, QString::number(new_value)); emit alwaysAskConfirmationChanged(); }

	inline QVariant getCustomValue(const QLatin1StringView &value_name, const QVariant &default_value = QVariant{}) const
	{
		return getValue(currentUser(), value_name, default_value);
	}
	inline void setCustomValue(const QLatin1StringView &value_name, const QVariant &value)
	{
		changeValue(currentUser(), value_name, value);
	}

	void userSwitchingActions();

signals:
	void userLocaleChanged();
	void themeStyleChanged();
	void colorChanged();
	void weightUnitChanged();
	void fontSizeChanged();
	void lastViewedMesoIdxChanged();
	void alwaysAskConfirmationChanged();

private:
	QHash<uint,QLatin1StringView> m_userPropertyNames;
	QStringList m_defaultValues;
	QStringList m_colorSchemes;
	QString m_userId;
	int m_languageIdx, m_prevColorScheme;


//--------------------------------------------USER   SETTINGS---------------------------------------------//
};

inline TPSettings* appSettings() { return TPSettings::app_settings; }
