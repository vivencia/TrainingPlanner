#pragma once

#include "usersettings.h"

QT_FORWARD_DECLARE_CLASS(UserSettings)

#define APP_VERSION_INDEX 0
#define WINDOW_WIDTH_INDEX 1
#define WINDOW_HEIGHT_INDEX 2
#define PAGE_WIDTH_INDEX 3
#define PAGE_HEIGHT_INDEX 4
#define HEIGHT_TO_WIDTH_RATIO_INDEX 5
#define FONT_RATIO 6
#define CURRENT_USER 7
#define SERVER_ADDRESS 8

#define APP_SETTINGS_FIELD_COUNT SERVER_ADDRESS + 1

static const QString &TP_APP_VERSION(Qt::Literals::StringLiterals::operator""_L1("v20250915 Build 1", 17));

class TPSettings : public QSettings
{

Q_OBJECT

Q_PROPERTY(uint windowWidth READ windowWidth CONSTANT)
Q_PROPERTY(uint windowHeight READ windowHeight CONSTANT)
Q_PROPERTY(uint pageWidth READ pageWidth CONSTANT)
Q_PROPERTY(uint pageHeight READ pageHeight CONSTANT)
Q_PROPERTY(double heightToWidthRatio READ heightToWidthRatio CONSTANT)
Q_PROPERTY(QString appVersion READ appVersion CONSTANT)
Q_PROPERTY(QString currentUser READ currentUser WRITE setCurrentUser NOTIFY currentUserChanged FINAL)
Q_PROPERTY(QString serverAddress READ serverAddress WRITE setServerAddress NOTIFY serverAddressChanged FINAL)

public:
	explicit TPSettings(QObject *parent = nullptr);
	inline QString configFileName() const { return fileName(); }

	inline QString appVersion() const { return value(m_propertyNames.value(APP_VERSION_INDEX), TP_APP_VERSION).toString(); }

	inline uint windowWidth() const { return m_windowWidth; }
	inline uint windowHeight() const { return m_windowHeight; }
	inline uint pageWidth() const { return m_windowWidth; }
	inline uint pageHeight() const { return m_qmlPageHeight; }
	inline double heightToWidthRatio() const { return m_HeightToWidth; }
	inline qreal fontRatio() const { return m_ratioFont; }
	inline QString currentUser() const { return value(m_propertyNames.value(CURRENT_USER)).toString(); }
	inline void setCurrentUser(const QString &new_value) { changeValue(CURRENT_USER, new_value); emit currentUserChanged(); }
	inline QString serverAddress() const { return value(m_propertyNames.value(SERVER_ADDRESS)).toString(); }
	inline void setServerAddress(const QString &new_value) { changeValue(SERVER_ADDRESS, new_value); emit serverAddressChanged(); }

	Q_INVOKABLE QString availableLanguagesLabel(const uint language_idx) const;
	Q_INVOKABLE QStringList availableLanguages() const;

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

	inline void changeValue(const uint index, const QVariant &new_value)
	{
		setValue(m_propertyNames.value(index), new_value);
		sync();
	}

	void getScreenMeasures();
	static TPSettings* app_settings;
	friend TPSettings* appSettings();
	static UserSettings *user_settings;
	friend UserSettings *userSettings();
};

inline TPSettings* appSettings() { return TPSettings::app_settings; }
inline UserSettings *userSettings() { return TPSettings::user_settings; }
