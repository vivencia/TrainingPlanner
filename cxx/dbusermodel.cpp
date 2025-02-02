#include "dbusermodel.h"

#include "osinterface.h"
#include "qmlitemmanager.h"
#include "tpglobals.h"
#include "tpimage.h"
#include "tputils.h"
#include "translationclass.h"
#include "online_services/tponlineservices.h"

#include <QFile>
#include <QStandardPaths>

#include <utility>

DBUserModel* DBUserModel::_appUserModel(nullptr);

static const QLatin1StringView userprofileFileName{"profile.txt"_L1};
static const QString &appDataPath{QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/Files/"_L1};

DBUserModel::DBUserModel(QObject *parent, const bool bMainUserModel)
	: TPListModel{parent}, mb_empty(false), m_searchRow(-1)
{
	setObjectName(DBUserObjectName);
	m_tableId = USER_TABLE_ID;

	if (bMainUserModel)
	{
		_appUserModel = this;
		m_exportName = std::move(tr("Coach information"));

		mColumnNames.reserve(USER_TOTAL_COLS);
		mColumnNames.append(QString{});
		mColumnNames.append(std::move(tr("Name: ")));
		mColumnNames.append(std::move(tr("Birthday: ")));
		mColumnNames.append(std::move(tr("Sex: ")));
		mColumnNames.append(std::move(tr("Phone: ")));
		mColumnNames.append(std::move("E-mail: "_L1));
		mColumnNames.append(std::move(tr("Social Media: ")));
		mColumnNames.append(std::move(tr("Your are: ")));
		mColumnNames.append(std::move(tr("Professional job: ")));
		mColumnNames.append(std::move(tr("Goal: ")));
		mColumnNames.append(std::move("Avatar: "_L1));
		mColumnNames.append(QString{});
		mColumnNames.append(QString{});
		mColumnNames.append(QString{});

		connect(appTr(), &TranslationClass::applicationLanguageChanged, this, [this] () {
			mColumnNames[USER_COL_NAME] = std::move(tr("Name: "));
			mColumnNames[USER_COL_BIRTHDAY] = std::move(tr("Birthday: "));
			mColumnNames[USER_COL_SEX] = std::move(tr("Sex: "));
			mColumnNames[USER_COL_PHONE] = std::move(tr("Phone: "));
			mColumnNames[USER_COL_SOCIALMEDIA] = std::move(tr("Social Media: "));
			mColumnNames[USER_COL_USERROLE] = std::move(tr("Your are: "));
			mColumnNames[USER_COL_COACHROLE] = std::move(tr("Professional job: "));
			mColumnNames[USER_COL_GOAL] = std::move(tr("Goal: "));
			emit labelsChanged();
		});
	}
}

int DBUserModel::addUser(const bool bCoach)
{
	uint use_mode(1);
	int cur_coach(-1);
	int cur_client(-1);
	if (!m_modeldata.isEmpty())
	{
		switch (m_modeldata.at(0).at(USER_COL_APP_USE_MODE).toInt())
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
	appendList_fast(std::move(QStringList{} << STR_MINUS_ONE << QString{} << std::move("2424151"_L1) << "-1"_L1 << QString{} <<
		QString{} << QString{} << QString{} << QString{} << QString{} << std::move("image://tpimageprovider/m5"_L1) <<
		QString::number(use_mode) << QString::number(cur_coach) << QString::number(cur_client)));
	return m_modeldata.count() - 1;
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

void DBUserModel::setUserName(const int row, const QString &new_name, const int prev_use_mode, const QDate &prev_birthdate,
								int ret_code, const QString &networkReply)
{
	if (!appOsInterface()->tpServerOK())
	{
		_setUserName(row, new_name);
		return;
	}

	const QString &net_name{getNetworkUserName(new_name, appUseMode(row), birthDate(row))}; //How the user is identified on the server side
	const QString &password{makeUserPassword(new_name)};

	if (ret_code == -1000)
	{
		appOnlineServices()->checkUser(net_name, password);
		connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,row,new_name,prev_use_mode,prev_birthdate] (const int ret_code, const QString& ret_string) {
			if (_userName(row).isEmpty()) //User not registered in the local database
				appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, tr("Online registration") + record_separator + ret_string);
			setUserName(row, new_name, prev_use_mode, prev_birthdate, ret_code, ret_string);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	}
	else
	{
		switch (ret_code)
		{
			case 6: //User does not exist in the online database
				connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,row,new_name] (const int ret_code, const QString& ret_string) {
					if (ret_code == 0)
					{
						appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, tr("Online registration") + record_separator + ret_string);
						_setUserName(row, new_name);
					}
					else
						appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_ERROR, ret_string);
					emit userNameOK(row, ret_code == 0);
				}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
				if (appUserModel()->_userName(row).isEmpty())
					appOnlineServices()->registerUser(net_name, password);
				else
					appOnlineServices()->alterUser(getNetworkUserName(appUserModel()->_userName(row), prev_use_mode != -1 ? prev_use_mode :
						appUseMode(row), prev_birthdate.isValid() ? prev_birthdate : birthDate(row)), net_name, password);
				return;
			case 0: //User already and correctly registered on the server. But, for whatwever reason, might not be on the local database. So, skip the break statement
				_setUserName(row, new_name);
			break;
			default:
				appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_ERROR, networkReply);
		}
		emit userNameOK(row, ret_code == 0);
	}
}

void DBUserModel::setAvatar(const int row, const QString &new_avatar)
{
	TPImage img{nullptr};
	img.setSource(new_avatar);
	QString avatar_str{std::move(appDataPath + "avatar.png"_L1)};
	img.saveToDisk(avatar_str);
	m_modeldata[row][USER_COL_AVATAR] = std::move(avatar_str);
	emit userModified(row, USER_COL_AVATAR);

	if (row == 0 && appOsInterface()->tpServerOK())
	{
		QFile *avatar_file{new QFile{avatar(0), this}};
		if (avatar_file->open(QIODeviceBase::ReadOnly))
		{
			appOnlineServices()->sendFile(networkUserName(0), networkUserPassword(0), avatar_file);
			avatar_file->close();
		}
		delete avatar_file;
	}
}

void DBUserModel::setCoachPublicStatus(const uint row, const bool bPublic)
{
	if (!appOsInterface()->tpServerOK())
		return;

	if (row == 0 && isCoach(0)) //Only applicable to the main user that is a coach
	{
		const QString &net_name{getNetworkUserName(_userName(0), appUseMode(0), birthDate(0))}; //How the user is identified on the server side
		const QString &password{makeUserPassword(_userName(0))};
		appOnlineServices()->addOrRemoveCoach(net_name, password, bPublic);
		connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,bPublic] (const int ret_code, const QString& ret_string) {
			appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, tr("Coach registration") + record_separator + ret_string);
			if (ret_code == 0 && bPublic)
				mb_coachRegistered = true;
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	}
}

void DBUserModel::isCoachAlreadyRegisteredOnline(const uint row)
{
	if (!appOsInterface()->tpServerOK())
		return;

	if (row == 0 && isCoach(0)) //Only applicable to the main user that is a coach
	{
		if (!mb_coachRegistered)
		{
			connect(this, &DBUserModel::coachesListReceived, this, [this] (const QStringList &coaches_list) {
				const QString &net_name{getNetworkUserName(_userName(0), appUseMode(0), birthDate(0))};
				mb_coachRegistered = coaches_list.contains(net_name);
				emit coachOnlineStatus(mb_coachRegistered == true);
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
			getOnlineCoachesList();
		}
	}
}

void DBUserModel::uploadResume(const uint row, const QString &resumeFileName)
{
	if (!appOsInterface()->tpServerOK())
		return;

	if (row == 0 && isCoach(0)) //Only applicable to the main user that is a coach
	{
		const QString &resumeFileName_ok{appUtils()->getCorrectPath(resumeFileName)};
		QFileInfo fi{resumeFileName_ok};
		if (fi.isReadable())
		{
			const qsizetype idx{resumeFileName_ok.lastIndexOf('.')};
			const QString &extension{idx > 0 ? resumeFileName_ok.last(resumeFileName_ok.length() - idx) : QString{}};
			const QString &localResumeFileName{appDataPath + "resume"_L1 + extension};
			if (QFile::copy(resumeFileName_ok, localResumeFileName))
			{
				QFile *resume{new QFile{localResumeFileName, this}};
				if (resume->open(QIODeviceBase::ReadOnly))
				{
					const QString &net_name{getNetworkUserName(_userName(0), appUseMode(row), birthDate(row))}; //How the user is identified on the server side
					const QString &password{makeUserPassword(_userName(0))};
					connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,resume] (const int ret_code, const QString& ret_string) {
						appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, tr("Résumé uploading") + record_separator + ret_string);
						resume->close();
						resume->remove();
						delete resume;
					}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
					appOnlineServices()->sendFile(net_name, password, resume);
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
	if (!appOsInterface()->tpServerOK())
		return;

	if (coach_index < m_onlineUserInfo.count())
	{
		const QString &net_name{getNetworkUserName(_userName(0), appUseMode(0), birthDate(0))}; //How the user is identified on the server side
		const QString &password{makeUserPassword(_userName(0))};
		connect(appOnlineServices(), &TPOnlineServices::binaryFileReceived, this, [this]
						(const int ret_code, const QString &filename, const QByteArray &contents) {
			if (ret_code == 0)
			{
				const QString &localResumeFileName{appDataPath + filename};
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
		appOnlineServices()->getBinFile(net_name, password, "resume"_L1, m_onlineUserInfo.at(coach_index).at(USER_COL_NET_NAME));
	}
}

void DBUserModel::mainUserConfigurationFinished()
{
	emit mainUserConfigurationFinishedSignal();
	if (!appOsInterface()->tpServerOK())
		return;

	const QString &localProfile{appDataPath + userprofileFileName};
	if (exportToFile(localProfile, true, true) == APPWINDOW_MSG_EXPORT_OK)
	{
		QFile *profile{new QFile{localProfile, this}};
		if (profile->open(QIODeviceBase::ReadOnly))
		{
			appOnlineServices()->sendFile(networkUserName(0), networkUserPassword(0), profile);
			profile->close();
		}
		delete profile;
	}
}

void DBUserModel::sendRequestToCoaches(const QList<bool>& selectedCoaches)
{
	if (!appOsInterface()->tpServerOK())
		return;

	for (uint i{0}; i < selectedCoaches.count(); ++i)
	{
		if (selectedCoaches.at(i))
		{
			const QString &net_name{getNetworkUserName(_userName(0), appUseMode(0), birthDate(0))}; //How the user is identified on the server side
			const QString &password{makeUserPassword(_userName(0))};
			connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,net_name] (const int ret_code, const QString& ret_string) {
				if (ret_code == 0)
					appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, tr("Coach contacting") + record_separator + ret_string);
				else
					appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_ERROR, ret_string);
			}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
			appOnlineServices()->sendRequestToCoach(net_name, password, m_onlineUserInfo.at(i).at(USER_COL_NET_NAME));
		}
	}
}

void DBUserModel::getOnlineCoachesList()
{
	const QString &net_name{getNetworkUserName(_userName(0), appUseMode(0), birthDate(0))}; //How the user is identified on the server side
	const QString &password{makeUserPassword(_userName(0))};
	connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,net_name] (const int ret_code, const QString& ret_string) {
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
	appOnlineServices()->getCoachesList(net_name, password);
}

void DBUserModel::getUserOnlineProfile(const QString& netName, uint n_max_profiles)
{
	if (!appOsInterface()->tpServerOK())
		return;

	const QString &net_name{getNetworkUserName(_userName(0), appUseMode(0), birthDate(0))}; //How the user is identified on the server side
	const QString &password{makeUserPassword(_userName(0))};

	connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,netName,n_max_profiles] (const int ret_code, const QString& ret_string) {
		if (ret_code != 1)
		{
			const QString &temp_profile_filename{appDataPath + "temp_profile.txt"_L1};
			QFile *temp_profile{new QFile{temp_profile_filename, this}};
			if (temp_profile->open(QIODeviceBase::WriteOnly|QIODeviceBase::Truncate|QIODeviceBase::Text))
			{
				temp_profile->write(ret_string.toUtf8().constData());
				temp_profile->close();
				if (_importFromFile(temp_profile_filename, m_onlineUserInfo) == APPWINDOW_MSG_READ_FROM_FILE_OK)
					m_onlineUserInfo.last()[USER_COL_NET_NAME] = netName;
				static_cast<void>(temp_profile->remove());
			}
			delete temp_profile;
			if (n_max_profiles == m_onlineUserInfo.count())
				emit userProfileAcquired();
		}
	}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	appOnlineServices()->getFile(net_name, password, userprofileFileName, netName);
}

bool DBUserModel::updateFromModel(TPListModel *model)
{
	if (count() == 1 && m_modeldata.at(0).at(USER_COL_ID) == "-1"_L1)
		m_modeldata.clear();
	appendList(std::move(model->m_modeldata[0]));
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
		case USER_COL_AVATAR:
			if (fieldValue.contains("tpimageprovider"_L1))
				return fieldValue.last(fieldValue.length()-24);
			else
				return m_modeldata.at(m_exportRows.at(0)).at(USER_COL_SEX) == STR_ZERO ? std::move("Avatar-m5"_L1) : std::move("Avatar-f0"_L1);
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
		case USER_COL_AVATAR:
			return "image://tpimageprovider/"_L1 + fieldValue;
		default: return QString{};
	}
}

QString DBUserModel::getNetworkUserName(const QString &userName, const uint app_use_mode, const QDate &birthdate) const
{
	QString net_name{std::move(appUtils()->stripDiacriticsFromString(userName))};
	net_name = std::move(net_name.toLower());
	static_cast<void>(net_name.replace(' ', '_'));
	net_name.append('_' + appUtils()->formatDate(birthdate, TPUtils::DF_CATALOG));
	switch (app_use_mode)
	{
		case APP_USE_MODE_CLIENTS:
		case APP_USE_MODE_SINGLE_USER:
			static_cast<void>(net_name.prepend(std::move("u_"_L1)));
		break;
		case APP_USE_MODE_SINGLE_USER_WITH_COACH:
		case APP_USE_MODE_COACH_USER_WITH_COACH:
			static_cast<void>(net_name.prepend(std::move("uc_"_L1)));
		break;
		case APP_USE_MODE_SINGLE_COACH:
			static_cast<void>(net_name.prepend(std::move("c_"_L1)));
		break;
	}
	return net_name;
}

QString DBUserModel::makeUserPassword(const QString &userName) const
{
	QString password{userName};
	static_cast<void>(password.replace(' ', '_'));
	return password;
}

void DBUserModel::_setUserName(const uint row, const QString &new_name)
{
	if (new_name != _userName(row))
	{
		m_modeldata[row][USER_COL_NAME] = new_name;
		emit userModified(row, USER_COL_NAME);
		if (m_modeldata.count() > 1 && m_modeldata.at(row).at(USER_COL_ID) == STR_MINUS_ONE)
			emit userAddedOrRemoved(row, true);
	}
}

int DBUserModel::_importFromFile(const QString &filename, QList<QStringList>& targetModel)
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
						if (col != USER_COL_USERROLE)
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
