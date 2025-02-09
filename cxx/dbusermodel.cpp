#include "dbusermodel.h"

#include "dbinterface.h"
#include "osinterface.h"
#include "qmlitemmanager.h"
#include "tpglobals.h"
#include "tpimage.h"
#include "tpsettings.h"
#include "tputils.h"
#include "translationclass.h"
#include "online_services/tponlineservices.h"

#include <QDateTime>
#include <QFile>
#include <QStandardPaths>

#include <utility>

DBUserModel* DBUserModel::_appUserModel(nullptr);

static const QLatin1StringView userProfileFileName{"profile.txt"_L1};
static const QLatin1StringView userLocalDataFileName{"user.data"_L1};
static const QString &tpNetworkTitle{qApp->tr("TP Network")};

DBUserModel::DBUserModel(QObject *parent, const bool bMainUserModel)
	: TPListModel{parent}, m_searchRow{-1}
{
	setObjectName(DBUserObjectName);
	m_tableId = USER_TABLE_ID;

	if (bMainUserModel)
	{
		_appUserModel = this;
		m_exportName = std::move(tr("Coach information"));

		mColumnNames.reserve(USER_TOTAL_COLS);
		for (uint i{0}; i < USER_TOTAL_COLS; ++i)
			mColumnNames.append(std::move(QString{}));

		connect(appTr(), &TranslationClass::applicationLanguageChanged, this, [this] () {
			updateColumnNames();
			emit labelsChanged();
		});
		updateColumnNames();

		mb_mainUserConfigured = appSettings()->mainUserConfigured();
		m_appDataPath = std::move(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/Files/"_L1);
		connect(this, &DBUserModel::userModified, this, [this] (const uint user_row, const uint) {
			appDBInterface()->saveUser(user_row);
		});
	}
}

void DBUserModel::updateColumnNames()
{
	mColumnNames[USER_COL_NAME] = std::move(tr("Name: "));
	mColumnNames[USER_COL_BIRTHDAY] = std::move(tr("Birthday: "));
	mColumnNames[USER_COL_SEX] = std::move(tr("Sex: "));
	mColumnNames[USER_COL_PHONE] = std::move(tr("Phone: "));
	mColumnNames[USER_COL_EMAIL] = std::move("e-mail: "_L1);
	mColumnNames[USER_COL_SOCIALMEDIA] = std::move(tr("Social Media: "));
	mColumnNames[USER_COL_USERROLE] = std::move(tr("Your are: "));
	mColumnNames[USER_COL_COACHROLE] = std::move(tr("Professional job: "));
	mColumnNames[USER_COL_GOAL] = std::move(tr("Goal: "));
}

QString DBUserModel::newUserLabel() const
{
	return !m_modeldata.isEmpty() && !userName(0).isEmpty() ? tr("Continue Setup") : tr("Create a new user");
}

QString DBUserModel::existingUserLabel() const { return tr("User already registered"); }
QString DBUserModel::invalidEmailLabel() const { return tr("Invalid email address"); }
QString DBUserModel::checkEmailLabel() const { return tr("Check"); }
QString DBUserModel::importUserLabel() const { return tr("Import"); }

void DBUserModel::createMainUser()
{
	if (m_modeldata.isEmpty())
	{
		m_modeldata.insert(0, std::move(QStringList{} << std::move(generateUniqueUserId()) << QString{} << std::move("2424151"_L1) <<
			"2"_L1 << QString{} << QString{} << QString{} << QString{} << QString{} << QString{} << QString{} <<
			QString::number(APP_USE_MODE_SINGLE_USER) << STR_ZERO << STR_ZERO));
		emit userModified(0);
	}
}

int DBUserModel::addUser(const bool bCoach)
{
	uint use_mode{APP_USE_MODE_SINGLE_USER};
	int cur_coach{-1};
	int cur_client{-1};
	if (!m_modeldata.isEmpty())
	{
		switch (appUseMode(0))
		{
			case APP_USE_MODE_SINGLE_USER:
				return -1;
			case APP_USE_MODE_SINGLE_USER_WITH_COACH:
				if (!bCoach)
					return -1;
				use_mode = APP_USE_MODE_SINGLE_COACH;
				cur_coach = findLastUser(true);
			break;

			case APP_USE_MODE_SINGLE_COACH:
				if (bCoach)
					return -1;
				use_mode = APP_USE_MODE_CLIENTS;
				cur_client = findLastUser(false);
			break;

			case APP_USE_MODE_COACH_USER_WITH_COACH:
				if (bCoach)
				{
					use_mode = APP_USE_MODE_SINGLE_COACH;
					cur_coach = findLastUser(true);
				}
				else
				{
					use_mode = APP_USE_MODE_CLIENTS;
					cur_client = findLastUser(false);
				}
			break;
		}
	}
	appendList_fast(std::move(QStringList{} << std::move(generateUniqueUserId()) << QString{} << std::move("2424151"_L1) << "2"_L1 << QString{} <<
		QString{} << QString{} << QString{} << QString{} << QString{} << std::move("image://tpimageprovider/m5"_L1) <<
		QString::number(use_mode) << QString::number(cur_coach) << QString::number(cur_client)));
	return m_modeldata.count() - 1;
}

void DBUserModel::removeMainUser()
{
	if (!m_modeldata.isEmpty())
		m_modeldata.removeFirst();
}

uint DBUserModel::removeUser(const int row, const bool bCoach)
{
	if (row >= 1 && row < m_modeldata.count())
	{
		removeRow(row);
		emit userAddedOrRemoved(row, false);
		return findNextUser(bCoach);
	}
	return row;
}

int DBUserModel::findFirstUser(const bool bCoach)
{
	int searchRow(1);
	m_searchRow = -1;
	for (; searchRow < m_modeldata.count(); ++searchRow)
	{
		if (m_modeldata.at(searchRow).at(USER_COL_APP_USE_MODE) == (bCoach ? "2"_L1 : "0"_L1))
		{
			m_searchRow = searchRow;
			break;
		}
	}
	return m_searchRow;
}

int DBUserModel::findNextUser(const bool bCoach)
{
	if (m_searchRow == m_modeldata.count() - 1)
		return m_searchRow;
	else if (m_searchRow <= 0)
		return findFirstUser(bCoach);

	int searchRow(m_searchRow + 1);
	for (; searchRow < m_modeldata.count(); ++searchRow)
	{
		if (m_modeldata.at(searchRow).at(USER_COL_APP_USE_MODE) == (bCoach ? "2"_L1 : "0"_L1))
		{
			m_searchRow = searchRow;
			break;
		}
	}
	return m_searchRow;
}

int DBUserModel::findPrevUser(const bool bCoach)
{
	if (m_searchRow <= 1)
		return findFirstUser(bCoach);

	int searchRow(m_searchRow - 1);
	for (; searchRow >= 0; --searchRow)
	{
		if (m_modeldata.at(searchRow).at(USER_COL_APP_USE_MODE) == (bCoach ? "2"_L1 : "0"_L1))
		{
			m_searchRow = searchRow;
			break;
		}
	}
	return m_searchRow;
}

int DBUserModel::findLastUser(const bool bCoach)
{
	int searchRow(m_modeldata.count() - 1);
	for (; searchRow >= 0; --searchRow)
	{
		if (m_modeldata.at(searchRow).at(USER_COL_APP_USE_MODE) == (bCoach ? "2"_L1 : "0"_L1))
		{
			m_searchRow = searchRow;
			break;
		}
	}
	return m_searchRow;
}

const int DBUserModel::getRowByCoachName(const QString &coachname) const
{
	for (uint i{0}; i < m_modeldata.count(); ++i)
	{
		if (m_modeldata.at(i).at(USER_COL_NAME) == coachname)
		{
			if (isCoach(i))
				return i;
		}
	}
	return -1;
}

QStringList DBUserModel::getCoaches() const
{
	QStringList coaches;
	for (uint i{0}; i < m_modeldata.count(); ++i)
	{
		if (isCoach(i))
			coaches.append(m_modeldata.at(i).at(USER_COL_NAME));
	}
	return coaches;
}

QStringList DBUserModel::getClients() const
{
	QStringList clients;
	for (uint i{0}; i < m_modeldata.count(); ++i)
	{
		if (isUser(i))
			clients.append(m_modeldata.at(i).at(USER_COL_NAME));
	}
	return clients;
}

uint DBUserModel::userRow(const QString &userName) const
{
	for (uint i{0}; i < m_modeldata.count(); ++i)
	{
		if (m_modeldata.at(i).at(USER_COL_NAME) == userName)
			return i;
	}
	return 0; //Should never reach here
}

void DBUserModel::setAvatar(const int row, const QString &new_avatar, const bool upload)
{
	const QString &avatar_str{m_appDataPath + _userId(row) + "_avatar.png"_L1};
	m_modeldata[row][USER_COL_AVATAR] = std::move(avatar_str);
	emit userModified(row, USER_COL_AVATAR);

	if (upload) {
		TPImage img{nullptr};
		img.setSource(new_avatar);
		img.saveToDisk(avatar_str);

		if (row == 0)
		{
			if (!onlineCheckIn())
			{
				connect(this, &DBUserModel::mainUserOnlineCheckInChanged, this, [this] () {
					sendAvatarToServer();
				}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
				return;
			}
			sendAvatarToServer();
		}
	}
}

void DBUserModel::checkUserOnline(const QString &email)
{
	if (appOsInterface()->tpServerOK())
	{
		connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this] (const int ret_code, const QString &ret_string) {
			if (ret_code == 0)
				m_onlineUserId = ret_string;
			emit userOnlineCheckResult(ret_code == 0);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		appOnlineServices()->checkOnlineUser("email="_L1 + email);
	}
}

void DBUserModel::importFromOnlineServer()
{
	if (appOsInterface()->tpServerOK())
	{
		connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this] (const int ret_code, const QString &ret_string) {
			if (ret_code == 0)
			{
				removeMainUser();
				if (importFromString(ret_string))
				{
					downloadAvatarFromServer(m_modeldata.count() - 1);
					emit userOnlineImportFinished(true);
				}
			}
			else
				emit userOnlineImportFinished(false);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		appOnlineServices()->getOnlineUserData(m_onlineUserId);
	}
}

void DBUserModel::setCoachPublicStatus(const bool bPublic)
{
	if (isCoach(0)) //Only applicable to the main user that is a coach
	{
		if (!onlineCheckIn())
		{
			connect(this, &DBUserModel::mainUserOnlineCheckInChanged, this, [this,bPublic] () {
				setCoachPublicStatus(bPublic);
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
			return;
		}

		appOnlineServices()->addOrRemoveCoach(_userId(0), getUserPassword(), bPublic);
		connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,bPublic] (const int ret_code, const QString &ret_string) {
			appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, tr("Coach registration") + record_separator + ret_string);
			mb_coachRegistered = ret_code == 0 && bPublic;
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	}
}

void DBUserModel::isCoachAlreadyRegisteredOnline()
{
	if (isCoach(0)) //Only applicable to the main user that is a coach
	{
		if (!onlineCheckIn())
		{
			connect(this, &DBUserModel::mainUserOnlineCheckInChanged, this, [this] () {
				isCoachAlreadyRegisteredOnline();
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
			return;
		}

		if (!mb_coachRegistered)
		{
			connect(this, &DBUserModel::coachesListReceived, this, [this] (const QStringList &coaches_list) {
				mb_coachRegistered = coaches_list.contains(_userId(0));
				emit coachOnlineStatus(mb_coachRegistered == true);
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
			getOnlineCoachesList();
		}
	}
}

void DBUserModel::uploadResume(const QString &resumeFileName)
{
	if (isCoach(0)) //Only applicable to the main user that is a coach
	{
		if (!onlineCheckIn())
		{
			connect(this, &DBUserModel::mainUserOnlineCheckInChanged, this, [this,resumeFileName] () {
				uploadResume(resumeFileName);
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
			return;
		}

		const QString &resumeFileName_ok{appUtils()->getCorrectPath(resumeFileName)};
		QFileInfo fi{resumeFileName_ok};
		if (fi.isReadable())
		{
			const qsizetype idx{resumeFileName_ok.lastIndexOf('.')};
			const QString &extension{idx > 0 ? resumeFileName_ok.last(resumeFileName_ok.length() - idx) : QString{}};
			const QString &localResumeFileName{m_appDataPath + "resume"_L1 + extension};
			if (QFile::copy(resumeFileName_ok, localResumeFileName))
			{
				QFile *resume{new QFile{localResumeFileName, this}};
				if (resume->open(QIODeviceBase::ReadOnly))
				{
					connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,resume] (const int ret_code, const QString &ret_string) {
						appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, tr("Résumé uploading") + record_separator + ret_string);
						resume->close();
						resume->remove();
						delete resume;
					}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
					appOnlineServices()->sendFile(_userId(0), getUserPassword(), resume);
					return;
				}
				else
				{
					resume->remove();
					delete resume;
				}
			}
		}
		appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, tr("Résumé uploading") + record_separator + tr("Failed to open file"));
	}
}

void DBUserModel::downloadResume(const uint coach_index)
{
	if (!onlineCheckIn())
	{
		connect(this, &DBUserModel::mainUserOnlineCheckInChanged, this, [this,coach_index] () {
			downloadResume(coach_index);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		return;
	}

	if (coach_index < m_onlineUserInfo.count())
	{
		connect(appOnlineServices(), &TPOnlineServices::binaryFileReceived, this, [this]
						(const int ret_code, const QString &filename, const QByteArray &contents) {
			if (ret_code == 0)
			{
				const QString &localResumeFileName{m_appDataPath + filename};
				QFile *resume{new QFile{localResumeFileName, this}};
				static_cast<void>(resume->remove());
				if (resume->open(QIODeviceBase::WriteOnly|QIODeviceBase::NewOnly))
				{
					resume->write(contents);
					resume->close();
					appOsInterface()->openURL(localResumeFileName);
				}
				delete resume;
			}
			else
				appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_ERROR, filename + contents);
		});
		appOnlineServices()->getBinFile(_userId(0), getUserPassword(), "resume"_L1, m_onlineUserInfo.at(coach_index).at(USER_COL_ID));
	}
}

void DBUserModel::mainUserConfigurationFinished()
{
	mb_mainUserConfigured = true;
	emit mainUserConfigurationFinishedSignal();

	if (!onlineCheckIn())
	{
		connect(this, &DBUserModel::mainUserOnlineCheckInChanged, this, [this] () {
			mainUserConfigurationFinished();
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		return;
	}
	if (!mainUserRegistered())
	{
		connect(this, &DBUserModel::mainUserOnlineCheckInChanged, this, [this] () {
			mainUserConfigurationFinished();
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		registerUserOnline();
	}
	else
	{
		sendProfileToServer();
		sendUserInfoToServer();
	}
}

void DBUserModel::sendRequestToCoaches(const QList<bool>& selectedCoaches)
{
	if (!onlineCheckIn())
	{
		connect(this, &DBUserModel::mainUserOnlineCheckInChanged, this, [this,selectedCoaches] () {
			sendRequestToCoaches(selectedCoaches);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		return;
	}

	for (uint i{0}; i < selectedCoaches.count(); ++i)
	{
		if (selectedCoaches.at(i))
		{
			connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this] (const int ret_code, const QString &ret_string) {
				if (ret_code == 0)
					appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, tr("Coach contacting") + record_separator + ret_string);
				else
					appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_ERROR, ret_string);
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
			appOnlineServices()->sendRequestToCoach(_userId(0), getUserPassword(), m_onlineUserInfo.at(i).at(USER_COL_ID));
		}
	}
}

void DBUserModel::getOnlineCoachesList()
{
	if (onlineCheckIn())
	{
		connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this] (const int ret_code, const QString &ret_string) {
			const QStringList &coaches{ret_string.split(' ')};
			if (!coaches.isEmpty() && !coaches.first().contains("does not"_L1))
			{
				connect(this, &DBUserModel::userProfileAcquired, this, [this] {
					QStringList coaches{m_onlineUserInfo.count()};
					for (uint i{0}; i < m_onlineUserInfo.count(); ++i)
						coaches.append(m_onlineUserInfo.at(i).at(USER_COL_NAME));
					emit coachesListReceived(coaches);
				}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
				m_onlineUserInfo.clear();
				for (uint i{0}; i < coaches.count(); ++i)
					getUserOnlineProfile(coaches.at(i));
			}
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		appOnlineServices()->getCoachesList(_userId(0), getUserPassword());
	}
}

void DBUserModel::getUserOnlineProfile(const QString &netName, uint n_max_profiles)
{
	if (!onlineCheckIn())
	{
		connect(this, &DBUserModel::mainUserOnlineCheckInChanged, this, [this,netName,n_max_profiles] () {
			getUserOnlineProfile(netName, n_max_profiles);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		return;
	}

	connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,netName,n_max_profiles]
						(const int ret_code, const QString &ret_string) {
		if (ret_code != 1)
		{
			const QString &temp_profile_filename{m_appDataPath + "temp_profile.txt"_L1};
			QFile *temp_profile{new QFile{temp_profile_filename, this}};
			if (temp_profile->open(QIODeviceBase::WriteOnly|QIODeviceBase::Truncate|QIODeviceBase::Text))
			{
				temp_profile->write(ret_string.toUtf8().constData());
				temp_profile->close();
				if (_importFromFile(temp_profile_filename, m_onlineUserInfo) == APPWINDOW_MSG_READ_FROM_FILE_OK)
					m_onlineUserInfo.last()[USER_COL_ID] = netName;
				static_cast<void>(temp_profile->remove());
			}
			delete temp_profile;
			if (n_max_profiles == m_onlineUserInfo.count())
				emit userProfileAcquired();
		}
	}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	appOnlineServices()->getFile(_userId(0), getUserPassword(), userProfileFileName, netName);
}

bool DBUserModel::updateFromModel(TPListModel *model)
{
	addUser_fast(std::move(model->m_modeldata[0]));
	emit userModified(m_modeldata.count() - 1);
	return true;
}

bool DBUserModel::importFromString(const QString &user_data)
{
	QStringList modeldata{std::move(user_data.split('\n'))};
	if (modeldata.count() < USER_TOTAL_COLS)
		return false;
	if (modeldata.count() > USER_TOTAL_COLS)
		modeldata.removeLast();
	m_modeldata.append(std::move(modeldata));
	emit userModified(m_modeldata.count() - 1);
	return true;
}

QString DBUserModel::formatFieldToExport(const uint field, const QString &fieldValue) const
{
	switch (field)
	{
		case USER_COL_BIRTHDAY:
			return appUtils()->formatDate(QDate::fromJulianDay(fieldValue.toInt()));
		case USER_COL_SEX:
			return fieldValue == STR_ZERO ? std::move(tr("Male")) : std::move(tr("Female"));
		case USER_COL_SOCIALMEDIA:
		{
			QString strSocial{fieldValue};
			return strSocial.replace(record_separator, fancy_record_separator1);
		}
		default: return QString{};
	}
}

QString DBUserModel::formatFieldToImport(const uint field, const QString &fieldValue) const
{
	switch (field)
	{
		case USER_COL_BIRTHDAY:
			return QString::number(appUtils()->getDateFromStrDate(fieldValue).toJulianDay());
		case USER_COL_SEX:
			return fieldValue == tr("Male") ? STR_ZERO : STR_ONE;
		case USER_COL_SOCIALMEDIA:
		{
			QString strSocial{fieldValue};
			return strSocial.replace(fancy_record_separator1, record_separator);
		}
		default: return QString{};
	}
}

bool DBUserModel::onlineCheckIn()
{
	if (!appOsInterface()->tpServerOK())
	{
		auto conn = std::make_shared<QMetaObject::Connection>();
		*conn =  connect(appOsInterface(), &OSInterface::networkStatusChanged, this, [this,conn] () {
			if (appOsInterface()->tpServerOK())
			{
				disconnect(*conn);
				appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, tpNetworkTitle + record_separator + tr("Connected to server"));
				onlineCheckIn();
			}
		});
		appOsInterface()->checkInternetConnection();
		appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_ERROR, tpNetworkTitle + record_separator + tr("Server unreachable"));
		return false;
	}

	registerUserOnline();
	emit mainUserOnlineCheckInChanged();
	return mb_userRegistered == true;
}

void DBUserModel::registerUserOnline()
{
	if (!mb_userRegistered && mb_mainUserConfigured)
	{
		appOnlineServices()->checkUser(_userId(0), _userId(0));
		connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this] (const int ret_code, const QString &ret_string) {
			switch (ret_code)
			{
				case 0:
					mb_userRegistered = true;
				break;
				case 6: //User does not exist in the online database
					connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this] (const int ret_code, const QString &ret_string) {
						mb_userRegistered = true;
						emit mainUserOnlineCheckInChanged();
						appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, tpNetworkTitle + record_separator + tr("User information updated"));
					}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
					appOnlineServices()->registerUser(_userId(0), getUserPassword());
				break;
				default:
					appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_ERROR, ret_string);
					mb_userRegistered = false;
				break;
			}
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	}
}

inline QString DBUserModel::generateUniqueUserId() const
{
	return QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());
}

QString DBUserModel::getUserPassword() const
{
	return _userId(0);
}

void DBUserModel::sendProfileToServer()
{
	const QString &localProfile{m_appDataPath + userProfileFileName};
	if (exportToFile(localProfile, true, true, false) == APPWINDOW_MSG_EXPORT_OK)
	{
		QFile *profile{new QFile{localProfile, this}};
		if (profile->open(QIODeviceBase::ReadOnly))
		{
			connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,profile] (const int ret_code, const QString &ret_string) {
				profile->close();
				delete profile;
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
			appOnlineServices()->sendFile(_userId(0), getUserPassword(), profile);
		}
		else
			delete profile;
	}
}

void DBUserModel::sendUserInfoToServer()
{
	const QString &localUserData{m_appDataPath + userLocalDataFileName};
	if (exportContentsOnlyToFile(localUserData))
	{
		QFile *userdata{new QFile{localUserData, this}};
		if (userdata->open(QIODeviceBase::ReadOnly))
		{
			connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,userdata] (const int ret_code, const QString &ret_string) {
				userdata->close();
				delete userdata;
				if (ret_code == 0)
					appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, tpNetworkTitle + record_separator + tr("Online user information updated"));
				else
					appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_ERROR, tpNetworkTitle + record_separator + ret_string);
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
			appOnlineServices()->updateOnlineUserInfo(_userId(0), getUserPassword(), userdata);
		}
		else
			delete userdata;
	}
}

void DBUserModel::sendAvatarToServer()
{
	QFile *avatar_file{new QFile{avatar(0), this}};
	if (avatar_file->open(QIODeviceBase::ReadOnly))
	{
		connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,avatar_file] (const int ret_code, const QString &ret_string) {
			avatar_file->close();
			delete avatar_file;
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		appOnlineServices()->sendFile(_userId(0), getUserPassword(), avatar_file);
	}
	else
		delete avatar_file;
}

void DBUserModel::downloadAvatarFromServer(const uint row)
{
	connect(appOnlineServices(), &TPOnlineServices::binaryFileReceived, this, [this,row]
						(const int ret_code, const QString &filename, const QByteArray &contents) {
		if (ret_code == 0)
		{
			const QString &localAvatarFileName{m_appDataPath + filename};
			QFile *avatarImg{new QFile{localAvatarFileName, this}};
			if (!avatarImg->exists() || avatarImg->remove())
			{
				if (avatarImg->open(QIODeviceBase::WriteOnly))
				{
					avatarImg->write(contents);
					avatarImg->close();
				}
			}
			delete avatarImg;
			setAvatar(row, localAvatarFileName, false);
		}
	});
	const QString& avatarFileName{_userId(row) + "_avatar"};
	appOnlineServices()->getBinFile(_userId(row), _userId(row), avatarFileName, _userId(row));
}

int DBUserModel::_importFromFile(const QString &filename, QList<QStringList> &targetModel)
{
	QFile *inFile{new QFile{filename, this}};
	if (!inFile->open(QIODeviceBase::ReadOnly|QIODeviceBase::Text))
	{
		delete inFile;
		return APPWINDOW_MSG_OPEN_FAILED;
	}

	char buf[128];
	qint64 lineLength{0};
	uint col{USER_COL_NAME};
	QString value;
	bool bFoundModelInfo{false};
	const QString tableIdStr{"0x000"_L1 + QString::number(USER_TABLE_ID)};
	QStringList modeldata{USER_TOTAL_COLS};
	modeldata[0] = STR_MINUS_ONE;

	while ((lineLength = inFile->readLine(buf, sizeof(buf))) != -1)
	{
		if (strstr(buf, STR_END_EXPORT.toLatin1().constData()) == NULL)
		{
			if (lineLength > 10)
			{
				if (!bFoundModelInfo)
					bFoundModelInfo = strstr(buf, tableIdStr.toLatin1().constData()) != NULL;
				else
				{
					if (col < USER_COL_APP_USE_MODE)
					{
						if (col != USER_COL_AVATAR)
						{
							value = buf;
							value = value.remove(0, value.indexOf(':') + 2).simplified();
							if (!isFieldFormatSpecial(col))
								modeldata[col] = std::move(value);
							else
								modeldata[col] = std::move(formatFieldToImport(col, value));
						}
						++col;
					}
					else if (col == USER_COL_APP_USE_MODE)
					{
						modeldata[USER_COL_APP_USE_MODE] = QString::number(APP_USE_MODE_SINGLE_COACH);
						modeldata[USER_COL_CURRENT_COACH] = "0"_L1;
						modeldata[USER_COL_CURRENT_CLIENT] = "0"_L1;
						break;
					}
				}
			}
		}
		else
			break;
	}
	inFile->close();
	delete inFile;
	if (bFoundModelInfo)
		targetModel.append(std::move(modeldata));
	return col >= USER_COL_APP_USE_MODE ? APPWINDOW_MSG_READ_FROM_FILE_OK : APPWINDOW_MSG_UNKNOWN_FILE_FORMAT;
}
