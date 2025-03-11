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
#include <QMutex>
#include <QQuickWindow>
#include <QStandardPaths>
#include <QTimer>
#include <QWaitCondition>

#include <utility>

DBUserModel* DBUserModel::_appUserModel(nullptr);

static const QLatin1StringView& userProfileFileName{"profile.txt"_L1};
static const QLatin1StringView& userLocalDataFileName{"user.data"_L1};
static const QString &tpNetworkTitle{qApp->tr("TP Network")};
static const QString &profileFile_template{"%1%2.txt"};

//#define POLLING_INTERVAL 1000*60
#define POLLING_INTERVAL 1000*60*20

DBUserModel::DBUserModel(QObject *parent, const bool bMainUserModel)
	: TPListModel{parent}, m_searchRow{-1}, m_tempRow{-1}, m_availableCoaches{nullptr}, m_pendingClientRequests{nullptr},
		m_pendingCoachesResponses{nullptr}, mb_onlineCheckInInProgress{false}, m_mainTimer{nullptr}
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
		m_onlineCoachesDir = std::move(m_appDataPath + "online_coaches/"_L1);
		m_dirForRequestedCoaches = std::move(m_appDataPath + "requested_coaches/"_L1);
		m_dirForClientsRequests = std::move(m_appDataPath + "clients_requests/"_L1);
		m_dirForCurrentClients = std::move(m_appDataPath + "clients/"_L1);
		m_dirForCurrentCoaches = std::move(m_appDataPath + "coaches/"_L1);
		m_localProfileFile = std::move("%1/%2.txt"_L1);
		connect(this, &DBUserModel::userModified, this, [this] (const uint row, const uint field) {
			if (row == 0 || field == 100)
				appDBInterface()->saveUser(row);
		});
		connect(this, &DBUserModel::userRemoved, this, [this] (const uint row) {
			if (row > 0)
				appDBInterface()->removeUser(row);
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
	return !m_modeldata.isEmpty() && !_userName(0).isEmpty() ? tr("Continue Setup") : tr("Create a new user");
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

void DBUserModel::removeUser(const int row)
{
	if (row >= 1 && row < m_modeldata.count())
	{
		connect(appDBInterface(), &DBInterface::databaseReady, this, [this,row] (const uint) {
			removeRow(row);
			uint next_cur_row{0};
			if (m_modeldata.count() > 1)
				next_cur_row = row - 1;
			setCurrentRow(next_cur_row);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		emit userRemoved(row);
		if (isCoach(row))
			delCoach(_userId(row));
		if (isClient(row))
			delClient(_userId(row));
		emit userModified(0);
	}
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
		if (i != m_tempRow)
		{
			if (m_modeldata.at(i).at(USER_COL_NAME) == userName)
				return i;
		}
	}
	return -1;
}

int DBUserModel::findUserById(const QString &userId) const
{
	for (uint i{0}; i < m_modeldata.count(); ++i)
	{
		if (i != m_tempRow)
		{
			if (m_modeldata.at(i).at(USER_COL_ID) == userId)
				return i;
		}
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
		if (row != m_tempRow)
		{
			const QDir &localFilesDir{row == 0 ? m_appDataPath : isCoach(row) ? m_dirForCurrentCoaches : m_dirForCurrentClients};
			const QFileInfoList &images{localFilesDir.entryInfoList(QDir::Files|QDir::NoDotAndDotDot|QDir::NoSymLinks)};
			const auto &it = std::find_if(images.cbegin(), images.cend(), [] (auto image_fi) {
				return image_fi.fileName().contains("_avatar."_L1);
			});
			return it != images.cend() ? it->filePath() : QString{};
			/*for (const auto &it: images)
			{
				if (it.fileName().contains("_avatar."_L1))
					return it.filePath();
			}*/
		}
		else
		{
			if (m_tempRowUserInfo)
				return m_tempRowUserInfo->associatedFile(m_tempRowUserInfo->currentRow(), ASSOCIATED_FILES_AVATAR);
		}
	}
	return QString {};
}

void DBUserModel::setAvatar(const int row, const QString &new_avatar, const bool saveToDisk, const bool upload)
{
	if (saveToDisk)
	{
		TPImage img{nullptr};
		img.setSource(new_avatar);
		QString localAvatarFilePath{};
		if (row != m_tempRow)
		{
			const QDir &localFilesDir{row == 0 ? m_appDataPath : isCoach(row) ? m_dirForCurrentCoaches : m_dirForCurrentClients};
			localAvatarFilePath = std::move(_userId(row) + "_avatar."_L1 + img.sourceExtension());
		}
		static_cast<void>(QFile::remove(avatar(row)));
		img.saveToDisk(localAvatarFilePath);
	}
	emit userModified(row, USER_COL_AVATAR);

	if (row == 0 && upload)
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

int DBUserModel::getTemporaryUserInfo(OnlineUserInfo *tempUser, const int userInfoRow)
{
	if (!tempUser || userInfoRow == -1)
	{
		if (isRowTemp(m_tempRow))
		{
			m_modeldata.remove(m_tempRow);
			m_tempRow = -1;
			m_tempRowUserInfo = nullptr;
		}
	}
	else if (userInfoRow < tempUser->count())
	{
		m_tempRow = m_modeldata.count();
		m_modeldata.append(tempUser->modeldata(userInfoRow));
		m_tempRowUserInfo = tempUser;
		downloadAvatarFromServer(m_tempRow);
		tempUser->setCurrentRow(userInfoRow);
		setRowTemp(m_tempRow, true);
		return m_tempRow;
	}
	return -1;
}

void DBUserModel::copyTempUserFilesToFinalUserDir(const QString &destDir, OnlineUserInfo *userInfo, const int userInfoRow) const
{
	static_cast<void>(QFile::copy(userInfo->associatedFile(userInfoRow, ASSOCIATED_FILES_PROFILE), destDir + userProfileFileName));
	QString extension{userInfo->associatedFile(userInfoRow, ASSOCIATED_FILES_AVATAR)};
	qsizetype dot_idx{extension.lastIndexOf('.')};
	extension.remove(0, extension.length() - dot_idx);
	static_cast<void>(QFile::copy(userInfo->associatedFile(userInfoRow, ASSOCIATED_FILES_AVATAR), destDir + "avatar"_L1 + extension));
	extension = userInfo->associatedFile(userInfoRow, ASSOCIATED_FILES_RESUME);
	dot_idx = extension.lastIndexOf('.');
	extension.remove(0, extension.length() - dot_idx);
	static_cast<void>(QFile::copy(userInfo->associatedFile(userInfoRow, ASSOCIATED_FILES_RESUME), destDir + "resume"_L1 + extension));
}

void DBUserModel::clearUserDir(const QString &dir) const
{
	QDir directory{dir};
	const QFileInfoList &user_files{directory.entryInfoList(QStringList{} << "*.*"_L1, QDir::NoDotAndDotDot|QDir::Files)};
	for (const auto &it : user_files)
	{
		if (it.isFile())
			static_cast<void>(QFile::remove(it.filePath()));
	}
	static_cast<void>(directory.rmdir(dir));
}

void DBUserModel::addCoach(const uint row)
{
	appUtils()->setCompositeValue(m_coachesNames.count(), _userId(row), m_modeldata[row][USER_COL_COACHES], record_separator);
	emit userModified(0, USER_COL_COACHES);
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
			clearUserDir(m_coachesNames.at(coach_idx));
			m_coachesNames.remove(coach_idx);
			emit coachesNamesChanged();
			connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,row] (const QString &key, const QString &value) {
				appOnlineServices()->removeCoachFromClient(0, key, value, _userId(row));
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
			appKeyChain()->readKey(_userId(0));
		}
	}
}

void DBUserModel::addClient(const uint row)
{
	appUtils()->setCompositeValue(m_clientsNames.count(), _userId(row), m_modeldata[row][USER_COL_CLIENTS], record_separator);
	emit userModified(0, USER_COL_CLIENTS);
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
			clearUserDir(m_coachesNames.at(client_idx));
			m_clientsNames.remove(client_idx);
			emit clientsNamesChanged();
			static_cast<void>(QFile::remove(m_localProfileFile.arg(m_dirForCurrentClients, _userId(row))));
			connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,row] (const QString &key, const QString &value) {
				appOnlineServices()->removeClientFromCoach(0, key, value, _userId(row));
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
			appKeyChain()->readKey(_userId(0));
		}
	}
}

void DBUserModel::acceptUser(OnlineUserInfo *userInfo, const int userInfoRow)
{
	connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,userInfo,userInfoRow] (const QString &key, const QString &value) {
		const QString &user_id{userInfo->data(userInfoRow, USER_COL_ID)};
		const bool bIsCoach{userInfo->data(userInfoRow, USER_COL_COACHES) == STR_ONE};
		const int new_app_use_mode{bIsCoach ? APP_USE_MODE_SINGLE_COACH : APP_USE_MODE_SINGLE_USER};
		const QString &destDir{(bIsCoach ? m_dirForCurrentCoaches : m_dirForCurrentClients) + user_id + '/'};
		copyTempUserFilesToFinalUserDir(destDir, userInfo, userInfoRow);

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

		m_modeldata.last()[USER_COL_COACHES] = QString{};
		m_modeldata.last()[USER_COL_APP_USE_MODE] = std::move(QString::number(new_app_use_mode));
		if (bIsCoach)
		{
			addCoach(count()-1);
			appOnlineServices()->acceptClientRequest(0, key, value, user_id);
		}
		else
		{
			addClient(count()-1);
			appOnlineServices()->acceptCoachAnswer(0, key, value, user_id);

		}
		emit userModified(count() - 1);
	}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	appKeyChain()->readKey(_userId(0));
}

void DBUserModel::rejectUser(OnlineUserInfo *userInfo, const int userInfoRow)
{
	connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,userInfo,userInfoRow] (const QString &key, const QString &value) {
		const QString &user_id{userInfo->data(userInfoRow, USER_COL_ID)};
		if (_userId(m_tempRow) == user_id)
			getTemporaryUserInfo(nullptr, -1);
		if (userInfo->data(userInfoRow, USER_COL_COACHES) == STR_ONE)
			appOnlineServices()->rejectCoachAnswer(0, key, value, user_id);
		else
			appOnlineServices()->rejectClientRequest(0, key, value, user_id);
		userInfo->removeUserInfo(userInfoRow, true);

	}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	appKeyChain()->readKey(_userId(0));
}

void DBUserModel::removeCoach(const uint row)
{
	connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,row] (const QString &key, const QString &value) {
		appOnlineServices()->removeCoachFromClient(0, key, value, _userId(row));
		removeUser(row);
	}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	appKeyChain()->readKey(_userId(0));
}

void DBUserModel::removeClient(const uint row)
{
	connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,row] (const QString &key, const QString &value) {
		appOnlineServices()->removeClientFromCoach(0, key, value, _userId(row));
		removeUser(row);
	}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	appKeyChain()->readKey(_userId(0));
}

void DBUserModel::checkUserOnline(const QString &email, const QString &password)
{
	if (appOsInterface()->tpServerOK())
	{
		const int requestid{appUtils()->generateUniqueId("checkUserOnline"_L1)};
		auto conn = std::make_shared<QMetaObject::Connection>();
		*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid,password]
								(const int request_id, const int ret_code, const QString &ret_string) {
			if (request_id == requestid)
			{
				disconnect(*conn);
				if (ret_code == 0)
				{
					m_onlineUserId = ret_string;
					appKeyChain()->writeKey(_userId(0), password); //Password matches server's. Store it for the session
				}
				emit userOnlineCheckResult(ret_code == 0);
			}
		});
		appOnlineServices()->checkOnlineUser(requestid, "email="_L1 + email, password);
	}
}

void DBUserModel::importFromOnlineServer()
{
	if (appOsInterface()->tpServerOK())
	{
		const int requestid{appUtils()->generateUniqueId("importFromOnlineServer"_L1)};
		auto conn = std::make_shared<QMetaObject::Connection>();
		*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid]
								(const int request_id, const int ret_code, const QString &ret_string) {
			if (request_id == requestid)
			{
				disconnect(*conn);
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
			}
		});
		appOnlineServices()->getOnlineUserData(requestid, m_onlineUserId);
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
			const int requestid{appUtils()->generateUniqueId("setCoachPublicStatus"_L1)};
			auto conn = std::make_shared<QMetaObject::Connection>();
			*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid,bPublic]
								(const int request_id, const int ret_code, const QString &ret_string) {
				if (request_id == requestid)
				{
					disconnect(*conn);
					appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, tr("Coach registration") + record_separator + ret_string);
					mb_coachRegistered = ret_code == 0 && bPublic;
					emit coachOnlineStatus(mb_coachRegistered == true);
				}
			});
			appOnlineServices()->addOrRemoveCoach(requestid, key, value, bPublic);
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
						const int requestid{appUtils()->generateUniqueId("uploadResume"_L1)};
						auto conn = std::make_shared<QMetaObject::Connection>();
						*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid,resume]
											(const int request_id, const int ret_code, const QString &ret_string) {
							if (request_id == requestid)
							{
								disconnect(*conn);
								appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, tr("Résumé uploading") + record_separator + ret_string);
								resume->close();
								resume->remove();
								delete resume;
							}
						});
						appOnlineServices()->sendFile(requestid, key, value, resume);
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

void DBUserModel::downloadResume(const uint row)
{
	if (!onlineCheckIn())
	{
		connect(this, &DBUserModel::mainUserOnlineCheckInChanged, this, [this,row] () {
			downloadResume(row);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		return;
	}

	if (row < m_modeldata.count())
	{
		connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,row] (const QString &key, const QString &value) {
			const int requestid{appUtils()->generateUniqueId("downloadResume"_L1)};
			auto conn = std::make_shared<QMetaObject::Connection>();
			*conn = connect(appOnlineServices(), &TPOnlineServices::fileReceived, this, [this,conn,requestid,row]
							(const int request_id, const int ret_code, const QString &filename, const QByteArray &contents) {
				if (request_id == requestid)
				{
					disconnect(*conn);
					switch (ret_code)
					{
						case 0: //file downloaded
						{
							QString destDir;
							if (row == 0 )
								destDir = m_appDataPath;
							else if (row == m_tempRow && m_tempRowUserInfo)
							{
								destDir = m_tempRowUserInfo->sourcePath();
								m_tempRowUserInfo->setResume(m_tempRowUserInfo->currentRow(), filename);
							}
							else
								destDir = m_appDataPath + _userId(row) + '/';

							const QString &localResumeFile{destDir + filename};
							QFile *resume{new QFile{localResumeFile, this}};
							static_cast<void>(resume->remove());
							if (resume->open(QIODeviceBase::WriteOnly|QIODeviceBase::NewOnly))
							{
								resume->write(contents);
								resume->close();
								appOsInterface()->openURL(localResumeFile);
							}
							delete resume;
						}
						break;
						case 1: //online file and local file are the same
							viewResume(row);
						break;
						default: //some error
							appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_ERROR, filename + contents);
					}
				}
			});
			appOnlineServices()->getFile(requestid, key, value, "resume", _userId(row), resume(row));
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
					const int requestid{appUtils()->generateUniqueId("sendRequestToCoaches"_L1)};
					auto conn = std::make_shared<QMetaObject::Connection>();
					*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid,i]
										(const int request_id, const int ret_code, const QString &ret_string) {
						if (request_id == requestid)
						{
							disconnect(*conn);
							if (ret_code == 0)
							{
								const QString &coach_id{m_availableCoaches->data(i, USER_COL_ID)};
								if (QFile::copy(profileFile_template.arg(m_onlineCoachesDir, coach_id), profileFile_template.arg(m_dirForRequestedCoaches, coach_id)))
								{
									appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, tr("Coach contacting") +
										record_separator + tr("Online coach contacted ") + m_availableCoaches->data(i, USER_COL_NAME));
								}
							}
							else
								appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_ERROR, ret_string);
						}
					});
					appOnlineServices()->sendRequestToCoach(requestid, key, value, m_availableCoaches->data(i, USER_COL_ID));
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
			const int requestid{appUtils()->generateUniqueId("getOnlineCoachesList"_L1)};
			auto conn = std::make_shared<QMetaObject::Connection>();
			*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid,get_list_only]
								(const int request_id, const int ret_code, const QString &ret_string) {
				if (request_id == requestid)
				{
					disconnect(*conn);
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
						if (m_availableCoaches->sanitize(coaches, USER_COL_ID))
							emit availableCoachesChanged();

						//First pass
						for (qsizetype i{coaches.count()-1}; i >= 0; --i)
						{
							if (findUserById(coaches.at(i)) != -1)
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
				}
			});
			appOnlineServices()->getOnlineCoachesList(requestid, key, value);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		appKeyChain()->readKey(_userId(0));
	}
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

void DBUserModel::slot_keepNoLongerAvailableUser(bool keep)
{
	mb_keepUnavailableUser = keep;
	QMutexLocker locker(m_mutex);
	m_condition->wakeOne(); // Wake up one waiting thread
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
	else
	{
		if (!mb_userRegistered && !mb_onlineCheckInInProgress)
		{
			mb_onlineCheckInInProgress = true;
			registerUserOnline();
		}
		return mb_userRegistered == true;
	}
}

void DBUserModel::registerUserOnline()
{
	if (mb_mainUserConfigured)
	{
		connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this] (const QString &key, const QString &value) {
			const int requestid{appUtils()->generateUniqueId("registerUserOnline"_L1)};
			auto conn = std::make_shared<QMetaObject::Connection>();
			*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid,key,value]
								(const int request_id, const int ret_code, const QString &ret_string) {
				if (request_id == requestid)
				{
					disconnect(*conn);
					mb_onlineCheckInInProgress = false;
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
						{
							auto conn2 = std::make_shared<QMetaObject::Connection>();
							*conn2 = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn2,requestid]
														(const int request_id, const int ret_code, const QString &ret_string) {
								if (request_id == requestid)
								{
									disconnect(*conn2);
									if (ret_code == 0)
									{
										mb_userRegistered = true;
										emit mainUserOnlineCheckInChanged();
									}
									appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, tpNetworkTitle + record_separator + tr("User information updated"));
								}
							});
							appOnlineServices()->registerUser(requestid, key, value);
						}
						break;
						default:
							appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_ERROR, ret_string);
							mb_userRegistered = false;
						break;
					}
				}
			});
			appOnlineServices()->checkUser(requestid, key, value);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		appKeyChain()->readKey(_userId(0));
	}
}

inline QString DBUserModel::generateUniqueUserId() const
{
	return QString::number(QDateTime::currentMSecsSinceEpoch());
}

QString DBUserModel::resume(const uint row) const
{
	if (row < m_modeldata.count() && !_userId(row).isEmpty())
	{
		if (row != m_tempRow)
		{
			const QDir &localFilesDir{row == 0 ? m_appDataPath : isCoach(row) ? m_dirForCurrentCoaches : m_dirForCurrentClients};
			const QFileInfoList &files{localFilesDir.entryInfoList(QDir::Files|QDir::NoDotAndDotDot|QDir::NoSymLinks)};
			for (const auto &it: files)
			{
				if (it.fileName().contains("_resume."_L1))
					return it.filePath();
			}
		}
		else
		{
			if (m_tempRowUserInfo)
				return m_tempRowUserInfo->associatedFile(m_tempRowUserInfo->currentRow(), ASSOCIATED_FILES_RESUME);
		}
	}
	return QString {};
}

void DBUserModel::viewResume(const uint row)
{
	const QString &localResumeFile{getResumeFile(m_dirForCurrentCoaches)};
	if (QFile::exists(localResumeFile))
		appOsInterface()->openURL(localResumeFile);
}

//Only applicable to the main user that is a coach
void DBUserModel::checkIfCoachRegisteredOnline()
{
	connect(this, &DBUserModel::coachesListReceived, this, [this] (const QStringList &coaches_list) {
		mb_coachRegistered = coaches_list.contains(_userId(0));
		emit coachOnlineStatus(mb_coachRegistered == true);
	}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	getOnlineCoachesList(true);
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
		const int requestid{appUtils()->generateUniqueId(QLatin1StringView{netID.toLatin1()})};
		auto conn = std::make_shared<QMetaObject::Connection>();
		*conn = connect(appOnlineServices(), &TPOnlineServices::fileReceived, this, [this,conn,requestid,netID,save_as_filename]
							(const int request_id, const int ret_code, const QString &filename, const QByteArray &contents) {
			if (request_id == requestid)
			{
				disconnect(*conn);
				switch (ret_code)
				{
					case 0: //file downloaded
					{
						static_cast<void>(QFile::remove(save_as_filename));
						QFile *profile{new QFile{save_as_filename, this}};
						if (profile->open(QIODeviceBase::NewOnly|QIODeviceBase::Text))
						{
							profile->write(contents);
							profile->close();
							emit userProfileAcquired(netID, true);
						}
						else
							emit userProfileAcquired(netID, false);
						delete profile;
					}
					break;
					case 1: //online file and local file are the same
						emit userProfileAcquired(netID, true);
					break;
					default: //some error
						appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_ERROR, filename + contents);
				}
			}
		});
		appOnlineServices()->getFile(requestid, key, value, userProfileFileName, netID, save_as_filename);
	}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	appKeyChain()->readKey(_userId(0));
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
				const int requestid{appUtils()->generateUniqueId("sendProfileToServer"_L1)};
				auto conn = std::make_shared<QMetaObject::Connection>();
				*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid,profile]
									(const int request_id, const int ret_code, const QString &ret_string) {
					if (request_id == requestid)
					{
						disconnect(*conn);
						profile->close();
						delete profile;
					}
				});
				appOnlineServices()->sendFile(requestid, key, value, profile);
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
				const int requestid{appUtils()->generateUniqueId("sendUserInfoToServer"_L1)};
				auto conn = std::make_shared<QMetaObject::Connection>();
				*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid,userdata]
										(const int request_id, const int ret_code, const QString &ret_string) {
					if (request_id == requestid)
					{
						disconnect(*conn);
						userdata->close();
						delete userdata;
						if (ret_code == 0)
							appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, tpNetworkTitle + record_separator + tr("Online user information updated"));
						else
							appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_ERROR, tpNetworkTitle + record_separator + ret_string);
					}
				});
				appOnlineServices()->updateOnlineUserInfo(requestid, key, value, userdata);
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
			appKeyChain()->readKey(_userId(0));
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
		connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,avatar_file] (const QString &key, const QString &value) {
			const int requestid{appUtils()->generateUniqueId("sendAvatarToServer"_L1)};
			auto conn = std::make_shared<QMetaObject::Connection>();
			*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid,avatar_file]
									(const int request_id, const int ret_code, const QString &ret_string) {
				if (request_id == requestid)
				{
					disconnect(*conn);
					avatar_file->close();
					delete avatar_file;
				}
			});
			appOnlineServices()->sendFile(requestid, key, value, avatar_file);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		appKeyChain()->readKey(_userId(0));
	}
	else
		delete avatar_file;
}

void DBUserModel::downloadAvatarFromServer(const uint row)
{
	connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this,row] (const QString &key, const QString &value) {
		const int requestid{appUtils()->generateUniqueId(QLatin1StringView{_userId(row).toLatin1()})};
		auto conn = std::make_shared<QMetaObject::Connection>();
		*conn = connect(appOnlineServices(), &TPOnlineServices::fileReceived, this, [this,conn,requestid,row]
							(const int request_id, const int ret_code, const QString &filename, const QByteArray &contents) {
			if (request_id == requestid)
			{
				disconnect(*conn);
				switch (ret_code)
				{
					case 0: //file downloaded
					{
						QString destDir;
						if (row == 0 )
							destDir = m_appDataPath;
						else if (row == m_tempRow && m_tempRowUserInfo)
							destDir = m_tempRowUserInfo->sourcePath();
						else
							destDir = m_appDataPath + _userId(row) + '/';

						const QString &imagefile{destDir + filename};
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
						if (row == m_tempRow && m_tempRowUserInfo)
							m_tempRowUserInfo->setAvatar(m_tempRowUserInfo->currentRow(), imagefile, true);
						setAvatar(row, imagefile, false, false);
					}
					break;
					case 1: //online file and local file are the same
					break;
					default: //some error
						appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_ERROR, filename + contents);
				}
			}
		});
		appOnlineServices()->getFile(requestid, key, value, _userId(row) + "_avatar", _userId(row), avatar(row));
	}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	appKeyChain()->readKey(_userId(row));
}

void DBUserModel::startServerPolling()
{
	if (!onlineCheckIn())
	{
		connect(this, &DBUserModel::mainUserOnlineCheckInChanged, this, [this] () {
			startServerPolling();
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		return;
	}

	if (!m_mainTimer)
	{
		m_mainTimer = new QTimer{this};
		m_mainTimer->setInterval(POLLING_INTERVAL);
		m_mainTimer->callOnTimeout([this] () { pollServer(); });
		m_mainTimer->start();

		if (isCoach(0))
		{
			QDir requestsDir{m_dirForClientsRequests};
			if (!requestsDir.exists())
				requestsDir.mkpath(m_dirForClientsRequests);
			QDir clientsDir{m_dirForCurrentClients};
			if (!clientsDir.exists())
				clientsDir.mkpath(m_dirForCurrentClients);
		}
		if (isClient(0))
		{
			QDir requests_dir{m_dirForRequestedCoaches};
			if (!requests_dir.exists())
				requests_dir.mkpath(m_dirForRequestedCoaches);
			QDir coaches_dir{m_dirForCurrentCoaches};
			if (!coaches_dir.exists())
				coaches_dir.mkpath(m_dirForCurrentCoaches);
		}
	}
	pollServer();
}

void DBUserModel::pollServer()
{
	connect(appKeyChain(), &TPKeyChain::keyRestored, this, [this] (const QString &key, const QString &value) {
		m_password = value;
		if (isCoach(0))
		{
			if (!mb_coachRegistered)
			{
				//poll immediatelly after receiving confirmation the man user is  a registerd coach
				connect(this, &DBUserModel::coachOnlineStatus, this, [this] (bool registered) {
					if (registered)
					{
						pollClientsRequests();
						pollCurrentClients();
					}
				}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
				checkIfCoachRegisteredOnline();
			}
			else
			{
				if (mb_coachRegistered == true)
				{
					pollClientsRequests();
					pollCurrentClients();
				}
			}
		}
		if (isClient(0))
		{
			pollCoachesAnswers();
			pollCurrentCoaches();
		}
		//TODO
		//checkMessages();
		//checkWorkouts();
		//checkNewMesos();
	}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	appKeyChain()->readKey(_userId(0));
}

void DBUserModel::pollClientsRequests()
{
	const int requestid{appUtils()->generateUniqueId("pollClientsRequests"_L1)};
	auto conn = std::make_shared<QMetaObject::Connection>();
	*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid]
							(const int request_id, const int ret_code, const QString &ret_string) {
		if (request_id == requestid)
		{
			disconnect(*conn);
			if (ret_code == 0)
			{
				QStringList requests_list{std::move(ret_string.split(' ', Qt::SkipEmptyParts))};
				if (!m_pendingClientRequests)
					m_pendingClientRequests = new OnlineUserInfo{this};
				if (m_pendingClientRequests->sanitize(requests_list, USER_COL_ID))
					emit pendingClientsRequestsChanged();

				//First pass
				for (qsizetype i{requests_list.count()-1}; i >= 0; --i)
				{
					if (findUserById(requests_list.at(i)) != -1)
					{
						appOnlineServices()->removeClientRequest(0, _userId(0), m_password, requests_list.at(i));
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
					const QString &client_profile{m_localProfileFile.arg(m_dirForClientsRequests, requests_list.at(x))};
					getUserOnlineProfile(requests_list.at(x), client_profile);
				}
			}
		}
	});
	appOnlineServices()->checkClientsRequests(requestid, _userId(0), m_password);
}

void DBUserModel::addPendingClient(const QString &user_id)
{
	if (!m_pendingClientRequests->containsUser(user_id))
	{
		const QString &client_profile{profileFile_template.arg(m_dirForClientsRequests, user_id)};
		if (m_pendingClientRequests->dataFromFileSource(client_profile, user_id))
		{
			//Indicate that the online user is a client. acceptUser will look for this info in order to setup the new user and the main user's field:
			//add the newly accpted user as a coach(USER_COL_COACHES) or as a client(USER_COL_CLIENTS)
			m_pendingClientRequests->setData(m_pendingClientRequests->count()-1, USER_COL_COACHES, STR_ZERO);
			emit pendingClientsRequestsChanged();
		}
	}
}

void DBUserModel::pollCoachesAnswers()
{
	const int requestid{appUtils()->generateUniqueId("pollCoachesAnswers"_L1)};
	auto conn = std::make_shared<QMetaObject::Connection>();
	*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid]
							(const int request_id, const int ret_code, const QString &ret_string) {
		if (request_id == requestid)
		{
			disconnect(*conn);
			if (ret_code == 0)
			{
				QStringList answers_list{std::move(ret_string.split(' ', Qt::SkipEmptyParts))};
				if (!m_pendingCoachesResponses)
					m_pendingCoachesResponses = new OnlineUserInfo{this};
				if (m_pendingCoachesResponses->sanitize(answers_list, USER_COL_ID))
					emit pendingCoachesResponsesChanged();

				//First pass
				for (qsizetype i{answers_list.count()-1}; i >= 0; --i)
				{
					QString coach_id{answers_list.at(i)};
					if (coach_id.endsWith("AOK"_L1))
					{
						coach_id.chop(3);
						if (findUserById(coach_id) != -1)
						{
							appOnlineServices()->removeCoachAnwers(requestid, _userId(0), m_password, coach_id);
							answers_list.remove(i);
						}
					}
					else //if (coach_id.endsWith("NAY"_L1))
						answers_list.remove(i);
				}

				//Second pass
				qsizetype n_connections{answers_list.count()};
				auto conn = std::make_shared<QMetaObject::Connection>();
				*conn = connect(this, &DBUserModel::userProfileAcquired, this, [this,conn,answers_list,n_connections]
																			(const QString &userid, const bool success) mutable {
					if (--n_connections == 0)
						disconnect(*conn);
					if (success)
						addCoachAnswer(userid);
				});
				for (qsizetype x{0}; x < answers_list.count(); ++x)
				{
					QString coach_id{std::move(answers_list.at(x))};
					coach_id.chop(3);
					const QString &coach_profile{m_localProfileFile.arg(m_dirForRequestedCoaches, coach_id)};
					getUserOnlineProfile(coach_id, coach_profile);
				}
			}
		}
	});
	appOnlineServices()->checkCoachesAnswers(requestid, _userId(0), m_password);
}

void DBUserModel::addCoachAnswer(const QString &user_id)
{
	if (!m_pendingCoachesResponses->containsUser(user_id))
	{
		const QString &coach_profile{profileFile_template.arg(m_dirForRequestedCoaches, user_id)};
		if (m_pendingCoachesResponses->dataFromFileSource(coach_profile, user_id))
		{
			//Indicate that the online user is a coach. acceptUser will look for this info in order to setup the new user and the main user's field:
			//add the newly accepted user as a coach(USER_COL_COACHES) or as a client(USER_COL_CLIENTS)
			m_pendingCoachesResponses->setData(m_pendingCoachesResponses->count()-1, USER_COL_COACHES, STR_ONE);
			emit pendingCoachesResponsesChanged();
		}
	}
}

void DBUserModel::addAvailableCoach(const QString &user_id)
{
	if (!m_availableCoaches->containsUser(user_id))
	{
		const QString &coach_profile{profileFile_template.arg(m_onlineCoachesDir, user_id)};
		if (m_availableCoaches->dataFromFileSource(coach_profile, user_id))
		{
			//Indicate that the online user is a coach. acceptUser will look for this info in order to setup the new user and the main user's field:
			//add the newly accepted user as a coach(USER_COL_COACHES) or as a client(USER_COL_CLIENTS)
			m_availableCoaches->setData(m_availableCoaches->count()-1, USER_COL_COACHES, STR_ONE);
			emit availableCoachesChanged();
		}
	}
}

void DBUserModel::pollCurrentClients()
{
	const int requestid{appUtils()->generateUniqueId("pollCurrentClients"_L1)};
	auto conn = std::make_shared<QMetaObject::Connection>();
	*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid]
								(const int request_id, const int ret_code, const QString &ret_string) {
		if (request_id == requestid)
		{
			disconnect(*conn);
			if (ret_code == 0)
			{
				QStringList clients_list{std::move(ret_string.split(' ', Qt::SkipEmptyParts))};
				m_mutex = nullptr;
				m_condition = nullptr;
				bool connected{false};
				for (qsizetype i{m_modeldata.count()-1}; i >= 1 ; --i)
				{
					if (!clients_list.contains(_userId(i)))
					{
						m_mutex = new QMutex{};
						m_condition = new QWaitCondition{};
						if (!connected)
						{
							connect(appMainWindow(), SIGNAL(keepNoLongerAvailableUser(bool)), this, SLOT(slot_keepNoLongerAvailableUser(bool)));
							connected = true;
						}
						QMetaObject::invokeMethod(appMainWindow(), "showUserNoLongerAvailable", Q_ARG(QString, _userId(i) + tr(" - unavailable")),
							Q_ARG(QString, tr("The user is no longer available as your client. If you need to know more about this, contact them to "
							"find out the reason. Remove the user from your list of clients?")));
						// Lock mutex and wait
						QMutexLocker locker(m_mutex);
						m_condition->wait(m_mutex); // Blocks here until woken the signal comming from QML
						if (!mb_keepUnavailableUser)
							removeUser(i);
					}
				}
				if (m_mutex != nullptr)
				{
					delete m_mutex;
					delete m_condition;
					disconnect(appMainWindow(), SIGNAL(keepNoLongerAvailableUser(bool)), this, SLOT(slot_keepNoLongerAvailableUser(bool)));
				}
			}
		}
	});
	appOnlineServices()->checkCurrentClients(requestid, _userId(0), m_password);
}

void DBUserModel::pollCurrentCoaches()
{
	const int requestid{appUtils()->generateUniqueId("pollCurrentCoaches"_L1)};
	auto conn = std::make_shared<QMetaObject::Connection>();
	*conn = connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,conn,requestid]
							(const int request_id, const int ret_code, const QString &ret_string) {
		if (request_id == requestid)
		{
			disconnect(*conn);
			if (ret_code == 0)
			{
				QStringList coaches_list{std::move(ret_string.split(' ', Qt::SkipEmptyParts))};
				m_mutex = nullptr;
				m_condition = nullptr;
				bool connected{false};
				for (qsizetype i{m_modeldata.count()-1}; i >= 1 ; --i)
				{
					if (!coaches_list.contains(_userId(i)))
					{
						m_mutex = new QMutex{};
						m_condition = new QWaitCondition{};
						if (!connected)
						{
							connect(appMainWindow(), SIGNAL(keepNoLongerAvailableUser(bool)), this, SLOT(slot_keepNoLongerAvailableUser(bool)));
							connected = true;
						}
						QMetaObject::invokeMethod(appMainWindow(), "showUserNoLongerAvailable", Q_ARG(QString, _userId(i) + tr(" - unavailable")),
							Q_ARG(QString, tr("The user is no longer available as your coach. If you need to know more about this, contact them to "
							"find out the reason. Remove the user from your list of coaches?")));
						// Lock mutex and wait
						QMutexLocker locker(m_mutex);
						m_condition->wait(m_mutex); // Blocks here until woken the signal comming from QML
						if (!mb_keepUnavailableUser)
							removeUser(i);
					}
				}
				if (m_mutex != nullptr)
				{
					delete m_mutex;
					delete m_condition;
					disconnect(appMainWindow(), SIGNAL(keepNoLongerAvailableUser(bool)), this, SLOT(slot_keepNoLongerAvailableUser(bool)));
				}
			}
		}
	});
	appOnlineServices()->checkCurrentCoaches(requestid, _userId(0), m_password);
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
