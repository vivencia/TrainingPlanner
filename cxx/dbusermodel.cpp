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
#include "online_services/onlineuserinfo.h"
#include "tpkeychain/tpkeychain.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QQuickWindow>
#include <QStandardPaths>
#include <QTimer>

#include <utility>

DBUserModel* DBUserModel::_appUserModel(nullptr);

static const QLatin1StringView& userProfileFileName{"profile.txt"_L1};
static const QLatin1StringView& userLocalDataFileName{"user.data"_L1};
static const QString &tpNetworkTitle{qApp->tr("TP Network")};
static const QString &profileFile_template{"%1%2.txt"};

#define POLLING_INTERVAL 300000 //5 minutes

DBUserModel::DBUserModel(QObject *parent, const bool bMainUserModel)
	: TPListModel{parent}, m_searchRow{-1}, m_availableCoaches{nullptr}, m_pendingClientRequests{nullptr}, m_pendingCoachesResponses{nullptr},
		m_clientsRequestsTimer{nullptr}, m_coachesAnswersTimer{nullptr}, m_tempRow{-1}
{
	setObjectName(DBUserObjectName);
	m_tableId = USERS_TABLE_ID;

	if (bMainUserModel)
	{
		_appUserModel = this;

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
		m_localAvatarFilePath = std::move(m_appDataPath + "%1_avatar.%2"_L1);
		m_onlineCoachesDir = std::move(m_appDataPath + "online_coaches/"_L1);
		m_dirForRequestedCoaches = std::move(m_appDataPath + "requested_coaches/"_L1);
		m_dirForClientsRequests = std::move(m_appDataPath + "clients_requests/"_L1);
		connect(this, &DBUserModel::userModified, this, [this] (const uint user_row, const uint) {
			appDBInterface()->saveUser(user_row);
		});

		/*connect(appKeyChain(), &TPKeyChain::keyRestored, this, [&] (const QString &key, const QString &value) {
			qDebug() << "Read key:";
			qDebug() << "key: " << key;
			qDebug() << "password: " << value;
			qDebug() << "";
		});
		connect(appKeyChain(), &TPKeyChain::keyStored, this, [&] (const QString &key) {
			qDebug() << "Write key:";
			qDebug() << "key: " << key;
			qDebug() << "";
		});*/
		//appKeyChain()->writeKey("1739296696780", "userpassword");
		//appKeyChain()->readKey("1739296696780");
		//appKeyChain()->writeKey("test", "testpassword");
		//appKeyChain()->readKey("test");
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
	mColumnNames[USER_COL_APP_USE_MODE] = std::move(tr("App use mode: "));
}

QString DBUserModel::passwordLabel() const { return tr("Password:"); }
QString DBUserModel::newUserLabel() const
{
	return !m_modeldata.isEmpty() && !userName(0).isEmpty() ? tr("Continue Setup") : tr("Create a new user");
}

QString DBUserModel::existingUserLabel() const { return tr("User already registered"); }
QString DBUserModel::invalidEmailLabel() const { return tr("Invalid email address"); }
QString DBUserModel::invalidPasswordLabel() const { return tr("Password must have 6 characters or more"); }
QString DBUserModel::checkEmailLabel() const { return tr("Check"); }
QString DBUserModel::importUserLabel() const { return tr("Import"); }

void DBUserModel::createMainUser()
{
	if (m_modeldata.isEmpty())
	{
		m_modeldata.insert(0, std::move(QStringList{} << std::move(generateUniqueUserId()) << QString{} << std::move("2424151"_L1) <<
			"2"_L1 << QString{} << QString{} << QString{} << QString{} << QString{} << QString{} << QString::number(APP_USE_MODE_SINGLE_USER)
			<< STR_ZERO << STR_ZERO));
		emit userModified(0);
	}
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
		if (isCoach(row))
			delCoach(row);
		if (isClient(row))
			delClient(row);
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

int DBUserModel::findUserByName(const QString &userName) const
{
	for (uint i{0}; i < m_modeldata.count(); ++i)
	{
		if (m_modeldata.at(i).at(USER_COL_NAME) == userName)
			return i;
	}
	return -1;
}

int DBUserModel::findUserById(const QString &userId) const
{
	for (uint i{0}; i < m_modeldata.count(); ++i)
	{
		if (m_modeldata.at(i).at(USER_COL_ID) == userId)
			return i;
	}
	return -1;
}

void DBUserModel::setPassword(const QString &password)
{
	appKeyChain()->writeKey(_userId(0), password);
}

void DBUserModel::getPassword()
{
	if (!m_modeldata.isEmpty())
	{
		connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this] (const QString &key, const QString &value) {
			emit userPasswordAvailable(value);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		appKeyChain()->readKey(_userId(0));
	}
}

QString DBUserModel::avatar(const int row) const
{
	if (row >= 0 && row < m_modeldata.count() && !_userId(row).isEmpty())
	{
		QFileInfo fi{getAvatarFile(_userId(row))};
		if (fi.exists())
			return fi.filePath();
	}
	return QString {};
}

void DBUserModel::setAvatar(const int row, const QString &new_avatar, const bool upload)
{
	if (upload) {
		TPImage img{nullptr};
		img.setSource(new_avatar);
		img.saveToDisk(m_localAvatarFilePath.arg(_userId(row), img.sourceExtension()));
		emit userModified(row, USER_COL_AVATAR);

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

int DBUserModel::getTemporaryUserInfo(OnlineUserInfo *tempUser, const int userInfoRow)
{
	if (!tempUser || userInfoRow == -1)
	{
		if (isRowTemp(m_tempRow))
		{
			m_modeldata.remove(m_tempRow);
			m_tempRow = -1;
		}
	}
	else if (userInfoRow < tempUser->count())
	{
		m_tempRow = m_modeldata.count();
		m_modeldata.append(tempUser->modeldata(userInfoRow));
		downloadAvatarFromServer(m_tempRow);
		tempUser->setCurrentRow(userInfoRow);
		setRowTemp(m_tempRow, true);
		return m_tempRow;
	}
	return -1;
}

void DBUserModel::addCoach(const uint row)
{
	appUtils()->setCompositeValue(m_coachesNames.count(), _userId(row), m_modeldata[row][USER_COL_COACHES], record_separator);
	emit userModified(row, USER_COL_COACHES);
	m_coachesNames.append(_userName(row));
	emit coachesNamesChanged();
}

void DBUserModel::delCoach(const uint coach_idx)
{
	if (coach_idx < m_coachesNames.count())
	{
		const qsizetype row{findUserByName(m_coachesNames.at(coach_idx))};
		if (row > 0)
		{
			appUtils()->removeFieldFromCompositeValue(coach_idx, m_modeldata[row][USER_COL_COACHES], record_separator);
			m_coachesNames.remove(coach_idx);
			emit coachesNamesChanged();
			emit userModified(row, USER_COL_COACHES);
		}
	}
}

void DBUserModel::addClient(const uint row)
{
	appUtils()->setCompositeValue(m_clientsNames.count(), _userId(row), m_modeldata[row][USER_COL_CLIENTS], record_separator);
	emit userModified(row, USER_COL_CLIENTS);
	m_clientsNames.append(_userName(row));
	emit clientsNamesChanged();
}

void DBUserModel::delClient(const uint client_idx)
{
	if (client_idx < m_clientsNames.count())
	{
		const qsizetype row{findUserByName(m_clientsNames.at(client_idx))};
		if (row > 0)
		{
			appUtils()->removeFieldFromCompositeValue(client_idx, m_modeldata[row][USER_COL_CLIENTS], record_separator);
			m_clientsNames.remove(client_idx);
			emit clientsNamesChanged();
			emit userModified(row, USER_COL_CLIENTS);
		}
	}
}

void DBUserModel::acceptUser(OnlineUserInfo *userInfo, const int userInfoRow)
{
	connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,userInfo,userInfoRow] (const QString &key, const QString &value) {
		const QString &user_id{userInfo->data(userInfoRow, USER_COL_ID)};
		if (_userId(m_tempRow) == user_id)
		{
			setRowTemp(m_tempRow, false);
			m_tempRow = -1;
		}
		else
		{
			addUser_fast(std::move(userInfo->modeldata(userInfoRow)));
			userInfo->removeUserInfo(userInfoRow, true);
		}
		const int new_app_use_mode{userInfo->data(userInfoRow, USER_COL_COACHES) == STR_ONE ? APP_USE_MODE_SINGLE_COACH : APP_USE_MODE_SINGLE_USER};
		m_modeldata.last()[USER_COL_COACHES] = QString{};
		m_modeldata.last()[USER_COL_APP_USE_MODE] = std::move(QString::number(new_app_use_mode));
		if (new_app_use_mode == APP_USE_MODE_SINGLE_COACH)
			addCoach(count()-1);
		else
			addClient(count()-1);
		appOnlineServices()->acceptClientRequest(key, value, user_id);
		//No need to emit the userModified signal here because either addCoach() or addClient() will
	}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	appKeyChain()->readKey(_userId(0));
}

void DBUserModel::rejectUser(OnlineUserInfo *userInfo, const int userInfoRow)
{
	connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,userInfo,userInfoRow] (const QString &key, const QString &value) {
		const QString &user_id{userInfo->data(userInfoRow, USER_COL_ID)};
		if (_userId(m_tempRow) == user_id)
			getTemporaryUserInfo(nullptr, -1);
		userInfo->removeUserInfo(userInfoRow, true);
		removeLocalAvatarFile(user_id);
		appOnlineServices()->rejectClientRequest(key, value, user_id);
	}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	appKeyChain()->readKey(_userId(0));
}

void DBUserModel::checkUserOnline(const QString &email, const QString &password)
{
	if (appOsInterface()->tpServerOK())
	{
		connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,password] (const int ret_code, const QString &ret_string) {
			if (ret_code == 0)
			{
				m_onlineUserId = ret_string;
				appKeyChain()->writeKey(_userId(0), password); //Password matches server's. Store it for the session
			}
			emit userOnlineCheckResult(ret_code == 0);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		appOnlineServices()->checkOnlineUser("email="_L1 + email, password);
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

		connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,bPublic] (const QString &key, const QString &value) {
			connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,bPublic] (const int ret_code, const QString &ret_string) {
				appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, tr("Coach registration") + record_separator + ret_string);
				mb_coachRegistered = ret_code == 0 && bPublic;
				if (mb_coachRegistered == true)
					startClientRequestsPolling();
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
			appOnlineServices()->addOrRemoveCoach(key, value, bPublic);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		appKeyChain()->readKey(_userId(0));
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
			const QString &localResumeFilePath{m_appDataPath + "resume"_L1 + extension};
			if (QFile::copy(resumeFileName_ok, localResumeFilePath))
			{
				QFile *resume{new QFile{localResumeFilePath, this}};
				if (resume->open(QIODeviceBase::ReadOnly))
				{
					connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,resume] (const QString &key, const QString &value) {
						connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,resume] (const int ret_code, const QString &ret_string) {
							appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, tr("Résumé uploading") + record_separator + ret_string);
							resume->close();
							resume->remove();
							delete resume;
						}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
						appOnlineServices()->sendFile(key, value, resume);
					}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
					appKeyChain()->readKey(_userId(0));
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

static QString getResumeFile(const QString &dir)
{
	QDir directory{dir};
	const QStringList &resume_types{directory.entryList(QStringList{} << "*.pdf"_L1 << "*.odt"_L1 << "*.docx"_L1, QDir::Files)};
	for (const auto &it : resume_types)
	{
		if (it.contains("resume"))
			return it;
	}
	return QString {};
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

	if (coach_index < m_availableCoaches->count())
	{
		connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,coach_index] (const QString &key, const QString &value) {
			QString localResumeFileName{std::move(getResumeFile(m_appDataPath))};
			const QString &localResumeFilePath{std::move(m_appDataPath + localResumeFileName)};
			connect(appOnlineServices(), &TPOnlineServices::fileReceived, this, [this,localResumeFilePath]
						(const int ret_code, const QString &filename, const QByteArray &contents) {
				switch (ret_code)
				{
					case 0: //file downloaded
					{
						const QString &resumeFileName{m_appDataPath + filename};
						QFile *resume{new QFile{resumeFileName, this}};
						static_cast<void>(resume->remove());
						if (resume->open(QIODeviceBase::WriteOnly|QIODeviceBase::NewOnly))
						{
							resume->write(contents);
							resume->close();
							appOsInterface()->openURL(resumeFileName);
						}
						delete resume;
					}
					break;
					case 1: //online file and local file are the same
						appOsInterface()->openURL(localResumeFilePath);
					break;
					default: //some error
						appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_ERROR, filename + contents);
				}
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
			QFileInfo fi{localResumeFilePath};
			QDateTime c_time;
			if (fi.exists())
				c_time = std::move(fi.birthTime());
			else
				localResumeFileName = std::move("resume"_L1);
			appOnlineServices()->getBinFile(key, value, localResumeFileName, m_availableCoaches->data(coach_index, USER_COL_ID), c_time);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		appKeyChain()->readKey(_userId(0));
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

void DBUserModel::sendRequestToCoaches()
{
	if (m_availableCoaches)
	{
		connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this] (const QString &key, const QString &value) {
			for (uint i{0}; i < m_availableCoaches->nSelected(); ++i)
			{
				if (m_availableCoaches->isSelected(i))
				{
					connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,i] (const int ret_code, const QString &ret_string) {
						if (ret_code == 0)
						{
							QDir requestsDir{m_dirForRequestedCoaches};
							if (!requestsDir.exists())
								requestsDir.mkpath(m_dirForRequestedCoaches);
							const QString &coach_id{m_availableCoaches->data(i, USER_COL_ID)};
							if (QFile::copy(profileFile_template.arg(m_onlineCoachesDir, coach_id), profileFile_template.arg(m_dirForRequestedCoaches, coach_id)))
							{
								appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, tr("Coach contacting") +
									record_separator + tr("Online coach contacted ") + m_availableCoaches->data(i, USER_COL_NAME));
								startCoachesAnswerPolling();
							}
						}
						else
							appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_ERROR, ret_string);
					}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
					appOnlineServices()->sendRequestToCoach(key, value, m_availableCoaches->data(i, USER_COL_ID));
				}
			}
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		appKeyChain()->readKey(_userId(0));
	}
}

void DBUserModel::getOnlineCoachesList(const bool get_list_only)
{
	if (onlineCheckIn())
	{
		QDir requestsDir{m_onlineCoachesDir};
		if (!requestsDir.exists())
			requestsDir.mkpath(m_onlineCoachesDir);
		connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,get_list_only] (const QString &key, const QString &value) {
			connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,get_list_only] (const int ret_code, const QString &ret_string) {
				if (ret_code == 0)
				{
					QStringList coaches{std::move(ret_string.split(' '))};
					if (get_list_only)
					{
						emit coachesListReceived(coaches);
						return;
					}
					if (!m_availableCoaches)
						m_availableCoaches = new OnlineUserInfo{this};
					if (m_availableCoaches->sanitize(coaches, USER_COL_NAME))
						emit availableCoachesChanged();

					//First pass
					for (qsizetype i{coaches.count()-1}; i >= 0; --i)
					{
						if (findUserById(coaches.at(i)) == -1)
						{
							const QString &coach_profile{m_onlineCoachesDir + coaches.at(i) + ".txt"_L1};
							if (QFile::exists(coach_profile))
							{
								addAvailableCoach(coaches.at(i));
								coaches.remove(i);
							}
						}
						else
							coaches.remove(i); //coach is already in the database, therefore not available
					}

					//Second pass
					qsizetype n_connections{coaches.count()};
					auto conn = std::make_shared<QMetaObject::Connection>();
					*conn = connect(this, &DBUserModel::userProfileAcquired, this, [this,conn,coaches,n_connections]
																				(const QString &userid, const bool success) mutable {
						if (--n_connections == 0)
							disconnect(*conn);
						if (success)
							addAvailableCoach(userid);
					});
					for (qsizetype x{0}; x < coaches.count(); ++x)
					{
						const QString &coach_profile{m_onlineCoachesDir + coaches.at(x) + ".txt"_L1};
						getUserOnlineProfile(coaches.at(x), coach_profile);
					}
				}
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
			appOnlineServices()->getCoachesList(key, value);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		appKeyChain()->readKey(_userId(0));
	}
}

//Only applicable to the main user that is a coach
void DBUserModel::checkIfCoachRegisteredOnline()
{
	if (isCoach(0))
	{
		if (!onlineCheckIn())
		{
			connect(this, &DBUserModel::mainUserOnlineCheckInChanged, this, [this] () {
				checkIfCoachRegisteredOnline();
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
			return;
		}

		if (!mb_coachRegistered)
		{
			connect(this, &DBUserModel::coachesListReceived, this, [this] (const QStringList &coaches_list) {
				mb_coachRegistered = coaches_list.contains(_userId(0));
				emit coachOnlineStatus(mb_coachRegistered == true);
				if (mb_coachRegistered == true)
					startClientRequestsPolling();
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
			getOnlineCoachesList(true);
		}
	}
}

void DBUserModel::getUserOnlineProfile(const QString &netID, const QString &save_as_filename)
{
	if (!onlineCheckIn())
	{
		connect(this, &DBUserModel::mainUserOnlineCheckInChanged, this, [this,netID,save_as_filename] () {
			getUserOnlineProfile(netID, save_as_filename);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		return;
	}

	connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,netID,save_as_filename] (const QString &key, const QString &value) {
		connect(appOnlineServices(), &TPOnlineServices::fileReceived, this, [this,netID,save_as_filename]
						(const int ret_code, const QString &filename, const QByteArray &contents) {
			if (ret_code == 0)
			{
				QFile *profile{new QFile{save_as_filename, this}};
				if (profile->open(QIODeviceBase::WriteOnly|QIODeviceBase::Truncate|QIODeviceBase::Text))
				{
					profile->write(contents);
					profile->close();
					emit userProfileAcquired(netID, true);
				}
				else
					emit userProfileAcquired(netID, false);
				delete profile;
			}
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		appOnlineServices()->getFile(key, value, userProfileFileName, netID);
	}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	appKeyChain()->readKey(_userId(0));
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
		modeldata.removeLast(); //remove the password field
	m_modeldata.append(std::move(modeldata));
	emit userModified(m_modeldata.count() - 1);
	return true;
}

void DBUserModel::getPasswordFromUserInput(const int resultCode, const QString &password)
{
	if (resultCode == 0)
	{
		connect(appKeyChain(), &TPKeyChain::keyStored, this, [this] (const QString &key) {
			registerUserOnline();
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		setPassword(password);
	}
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
		case USER_COL_APP_USE_MODE:
			switch (fieldValue.at(0).toLatin1())
			{
				case '1': return tr("User");
				case '2': return tr("Coach");
				case '3': return tr("Client");
				default: return tr("Coach and Client");
			}
		default: return QString{};
	}
}

QString DBUserModel::formatFieldToImport(const uint field, const QString &fieldValue) const
{
	switch (field)
	{
		case USER_COL_BIRTHDAY:
			return QString::number(appUtils()->getDateFromDateString(fieldValue).toJulianDay());
		case USER_COL_SEX:
			return fieldValue == tr("Male") ? STR_ZERO : STR_ONE;
		case USER_COL_SOCIALMEDIA:
		{
			QString strSocial{fieldValue};
			return strSocial.replace(fancy_record_separator1, record_separator);
		}
		case USER_COL_APP_USE_MODE:
			if (fieldValue == tr("User"))
				return "1"_L1;
			else if (fieldValue == tr("Coach"))
				return "2"_L1;
			else if (fieldValue == tr("Client"))
				return "3"_L1;
			else
				return "4"_L1;
		default: return QString{};
	}
}

bool DBUserModel::onlineCheckIn()
{
	if (!appOsInterface()->tpServerOK() && !appOsInterface()->internetConnectionCheckInPlace())
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
	return mb_userRegistered == true;
}

void DBUserModel::registerUserOnline()
{
	if (!mb_userRegistered && mb_mainUserConfigured)
	{
		connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this] (const QString &key, const QString &value) {
			connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,key,value] (const int ret_code, const QString &ret_string) {
				switch (ret_code)
				{
					case 0:
						mb_userRegistered = true;
						emit mainUserOnlineCheckInChanged();
					break;
					case 3:
						connect(appMainWindow(), SIGNAL(passwordDialogClosed(int,QString)), this, SLOT(getPasswordFromUserInput(int,QString)));
					break;
					case 6: //User does not exist in the online database
						connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this] (const int ret_code, const QString &ret_string) {
							if (ret_code == 0)
							{
								mb_userRegistered = true;
								emit mainUserOnlineCheckInChanged();
							}
							appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, tpNetworkTitle + record_separator + tr("User information updated"));
						}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
						appOnlineServices()->registerUser(key, value);
					break;
					default:
						appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_ERROR, ret_string);
						mb_userRegistered = false;
					break;
				}
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
			appOnlineServices()->checkUser(key, value);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		appKeyChain()->readKey(_userId(0));
	}
}

inline QString DBUserModel::generateUniqueUserId() const
{
	return QString::number(QDateTime::currentMSecsSinceEpoch());
}

void DBUserModel::sendProfileToServer()
{
	const QString &localProfile{m_appDataPath + userProfileFileName};
	if (exportToFile(localProfile, true, true, false) == APPWINDOW_MSG_EXPORT_OK)
	{
		QFile *profile{new QFile{localProfile, this}};
		if (profile->open(QIODeviceBase::ReadOnly))
		{
			connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,profile] (const QString &key, const QString &value) {
				connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,profile] (const int ret_code, const QString &ret_string) {
					profile->close();
					delete profile;
				}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
				appOnlineServices()->sendFile(key, value, profile);
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
			appKeyChain()->readKey(_userId(0));
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
			connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,userdata] (const QString &key, const QString &value) {
				connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,userdata] (const int ret_code, const QString &ret_string) {
					userdata->close();
					delete userdata;
					if (ret_code == 0)
						appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, tpNetworkTitle + record_separator + tr("Online user information updated"));
					else
						appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_ERROR, tpNetworkTitle + record_separator + ret_string);
				}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
				appOnlineServices()->updateOnlineUserInfo(key, value, userdata);
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
			appKeyChain()->readKey(_userId(0));
		}
		else
			delete userdata;
	}
}

QFileInfo DBUserModel::getAvatarFile(const QString &userid) const
{
	const QDir localFilesDir{m_appDataPath};
	const QFileInfoList &images{localFilesDir.entryInfoList(QDir::Files|QDir::NoDotAndDotDot|QDir::NoSymLinks)};
	auto itr{images.constBegin()};
	const auto itr_end{images.constEnd()};
	while (itr != itr_end)
	{
		if ((*itr).fileName().startsWith(userid))
			return *itr;
		++itr;
	}
	return QFileInfo{};
}

void DBUserModel::sendAvatarToServer()
{
	QFile *avatar_file{new QFile{avatar(0), this}};
	if (avatar_file->open(QIODeviceBase::ReadOnly))
	{
		connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,avatar_file] (const QString &key, const QString &value) {
			connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,avatar_file] (const int ret_code, const QString &ret_string) {
				avatar_file->close();
				delete avatar_file;
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
			appOnlineServices()->sendFile(key, value, avatar_file);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		appKeyChain()->readKey(_userId(0));
	}
	else
		delete avatar_file;
}

void DBUserModel::downloadAvatarFromServer(const uint row)
{
	connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,row] (const QString &key, const QString &value) {
		QFileInfo fi{getAvatarFile(_userId(row))};
		QDateTime m_time;
		if (fi.exists())
			m_time = std::move(fi.lastModified());

		connect(appOnlineServices(), &TPOnlineServices::fileReceived, this, [this,row]
						(const int ret_code, const QString &filename, const QByteArray &contents) {
			if (ret_code == 0)
			{
				const QString &imagefile{m_appDataPath + filename};
				QFile *avatarImg{new QFile{imagefile, this}};
				if (!avatarImg->exists() || avatarImg->remove())
				{
					if (avatarImg->open(QIODeviceBase::WriteOnly))
					{
						avatarImg->write(contents);
						avatarImg->close();
					}
				}
				delete avatarImg;
				setAvatar(row, imagefile, false);
			}
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		appOnlineServices()->getBinFile(key, value, m_time.isNull() ? _userId(row) + "_avatar"_L1 : fi.fileName(), _userId(row), m_time);
	}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	appKeyChain()->readKey(_userId(row));
}

inline void DBUserModel::removeLocalAvatarFile(const QString &user_id)
{
	QFileInfo fi{getAvatarFile(user_id)};
	if (fi.exists())
		static_cast<void>(QFile::remove(fi.filePath()));
}

void DBUserModel::startClientRequestsPolling()
{
	if (!m_clientsRequestsTimer)
	{
		m_clientsRequestsTimer = new QTimer{this};
		m_clientsRequestsTimer->setInterval(POLLING_INTERVAL);
		m_clientsRequestsTimer->callOnTimeout([this] () { pollClientsRequests(); });
		m_clientsRequestsTimer->start();

		QDir requestsDir{m_dirForClientsRequests};
		if (!requestsDir.exists())
			requestsDir.mkpath(m_dirForClientsRequests);
	}
	pollClientsRequests();
}

void DBUserModel::pollClientsRequests(const bool get_list_only)
{
	connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,get_list_only] (const QString &key, const QString &value) {
		connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,get_list_only,key,value] (const int ret_code, const QString &ret_string) {
			if (ret_code == 0)
			{
				QStringList requests_list{std::move(ret_string.split(' ', Qt::SkipEmptyParts))};
				if (get_list_only)
				{
					emit clientsListReceived(requests_list);
					return;
				}
				if (!m_pendingClientRequests)
					m_pendingClientRequests = new OnlineUserInfo{this};
				if (m_pendingClientRequests->sanitize(requests_list, USER_COL_NAME))
					emit pendingClientsRequestsChanged();

				//First pass
				for (qsizetype i{requests_list.count()-1}; i >= 0; --i)
				{
					if (findUserById(requests_list.at(i)) == -1)
					{
						const QString &client_profile{m_dirForClientsRequests + requests_list.at(i) + ".txt"_L1};
						if (QFile::exists(client_profile))
						{
							addPendingClient(requests_list.at(i));
							requests_list.remove(i);
						}
					}
					else
					{
						appOnlineServices()->removeClientRequest(key, value, requests_list.at(i));
						requests_list.remove(i);
					}
				}

				//Second pass
				qsizetype n_connections{requests_list.count()};
				auto conn = std::make_shared<QMetaObject::Connection>();
				*conn = connect(this, &DBUserModel::userProfileAcquired, this, [this,conn,requests_list,n_connections]
																				(const QString &userid, const bool success) mutable {
						if (--n_connections == 0)
							disconnect(*conn);
						if (success)
							addPendingClient(userid);
				});
				for (qsizetype x{0}; x < requests_list.count(); ++x)
				{
					const QString &client_profile{m_dirForClientsRequests + requests_list.at(x) + ".txt"_L1};
					getUserOnlineProfile(requests_list.at(x), client_profile);
				}
			}
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		appOnlineServices()->checkClientsRequests(key, value);
	}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	appKeyChain()->readKey(_userId(0));
}

void DBUserModel::addPendingClient(const QString &user_id)
{
	if (!m_pendingClientRequests->containsUser(user_id))
	{
		const QString &client_profile{profileFile_template.arg(m_dirForClientsRequests, user_id)};
		if (m_pendingClientRequests->dataFromFileSource(client_profile))
		{
			const qsizetype last_idx{m_pendingClientRequests->count()-1};
			m_pendingClientRequests->setData(last_idx, USER_COL_ID,user_id);
			//Indicate that the online user is a client. acceptUser will look for this info in order to setup the new user and the main user's field:
			//add the newly accpted user as a coach(USER_COL_COACHES) or as a client(USER_COL_CLIENTS)
			m_pendingClientRequests->setData(last_idx, USER_COL_COACHES, STR_ZERO);
			emit pendingClientsRequestsChanged();
		}
	}
}

void DBUserModel::startCoachesAnswerPolling()
{
	QDir coaches_dir{m_dirForRequestedCoaches};
	if (coaches_dir.exists() && !coaches_dir.isEmpty(QDir::Files|QDir::Readable|QDir::NoDotAndDotDot))
	{
		if (!m_coachesAnswersTimer)
		{
			m_coachesAnswersTimer = new QTimer{this};
			m_coachesAnswersTimer->setInterval(POLLING_INTERVAL);
			m_coachesAnswersTimer->callOnTimeout([this] () { pollCoachesAnswers(); });
			m_coachesAnswersTimer->start();
		}
		pollCoachesAnswers();
	}
}

void DBUserModel::pollCoachesAnswers()
{
	connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this] (const QString &key, const QString &value) {
		connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,key,value] (const int ret_code, const QString &ret_string) {
			if (ret_code == 0)
			{
				const QStringList &answers_list{ret_string.split(' ', Qt::SkipEmptyParts)};
				if (!m_pendingCoachesResponses)
					m_pendingCoachesResponses = new OnlineUserInfo{this};
				else
					m_pendingCoachesResponses->clear();

				for (uint i{0}; i < m_availableCoaches->count(); ++i)
				{
					if (answers_list.contains(m_availableCoaches->data(i, USER_COL_ID)))
						m_pendingCoachesResponses->setModelData(m_availableCoaches->modeldata(i));
				}
				emit pendingCoachesResponsesChanged();
			}
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		appOnlineServices()->checkCoachesAnswers(key, value);
	}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	appKeyChain()->readKey(_userId(0));
}

void DBUserModel::addAvailableCoach(const QString &user_id)
{
	if (!m_availableCoaches->containsUser(user_id))
	{
		const QString &coach_profile{profileFile_template.arg(m_onlineCoachesDir, user_id)};
		if (m_availableCoaches->dataFromFileSource(coach_profile))
		{
			const qsizetype last_idx{m_availableCoaches->count()-1};
			m_availableCoaches->setData(last_idx, USER_COL_ID, user_id);
			//Indicate that the online user is a coach. acceptUser will look for this info in order to setup the new user and the main user's field:
			//add the newly accepted user as a coach(USER_COL_COACHES) or as a client(USER_COL_CLIENTS)
			m_availableCoaches->setData(last_idx, USER_COL_COACHES, STR_ONE);
			emit availableCoachesChanged();
		}
	}
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
	const QString tableIdStr{"0x000"_L1 + QString::number(USERS_TABLE_ID)};
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
					if (col < USER_COL_COACHES)
					{
						value = buf;
						value = value.remove(0, value.indexOf(':') + 2).simplified();
						if (!isFieldFormatSpecial(col))
							modeldata[col] = std::move(value);
						else
							modeldata[col] = std::move(formatFieldToImport(col, value));
						++col;
					}
					else
						break;
				}
			}
		}
		else
			break;
	}
	inFile->close();
	delete inFile;
	if (bFoundModelInfo && col == USER_COL_COACHES)
	{
		targetModel.append(std::move(modeldata));
		return APPWINDOW_MSG_READ_FROM_FILE_OK;
	}
	else
		return APPWINDOW_MSG_UNKNOWN_FILE_FORMAT;
}
