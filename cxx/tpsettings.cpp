#include "tpsettings.h"

#include "tputils.h"

#include <QColor>
#include <QFontInfo>
#include <QScreen>

using namespace Qt::Literals::StringLiterals;

TPSettings *TPSettings::app_settings{nullptr};
UserSettings *TPSettings::user_settings{nullptr};

TPSettings::TPSettings(QObject *parent) : QSettings{parent}
{
	TPSettings::app_settings = this;
	m_propertyNames.insert(APP_VERSION_INDEX, std::move("appVersion"_L1));
	m_propertyNames.insert(CURRENT_USER, std::move("currentUser"_L1));
	m_propertyNames.insert(SERVER_ADDRESS, std::move("serverAddress"_L1));
	getScreenMeasures();
	changeUserSettings(!currentUser().isEmpty() ? currentUser() : "default"_L1);
}

void TPSettings::changeUserSettings(const QString &userid)
{
	const QString &settings_dir{appUtils()->localAppFilesDir() + userid};
	bool ok{false};
	if (currentUser() == "default"_L1)
		ok = appUtils()->rename(appUtils()->localAppFilesDir() + "default"_L1, settings_dir, false);
	else
		ok = appUtils()->mkdir(settings_dir);
	if (ok)
	{
		if (user_settings)
			delete user_settings;
		user_settings = new UserSettings(userid, this);
		setCurrentUser(userid);
	}
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
	return QStringList{} << std::move("en_US"_L1) << std::move("pt_BR"_L1) << std::move("de_DE"_L1);
}

QString TPSettings::userDir(const QString &userid) const
{
	return appUtils()->localAppFilesDir() + userid + '/';
}
