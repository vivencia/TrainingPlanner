#include "dbusermodel.h"

#include "qmlitemmanager.h"
#include "tpglobals.h"
#include "tputils.h"
#include "translationclass.h"
#include "online_services/tponlineservices.h"

#include <utility>

DBUserModel* DBUserModel::_appUserModel(nullptr);

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
	appendList_fast(std::move(QStringList{} << STR_MINUS_ONE << QString{} << std::move("2451545"_L1) << STR_ZERO << QString{} <<
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
			const uint app_use_mode{m_modeldata.at(i).at(USER_COL_APP_USE_MODE).toUInt()};
			if (app_use_mode == APP_USE_MODE_SINGLE_COACH || app_use_mode == APP_USE_MODE_COACH_USER_WITH_COACH)
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
		const uint app_use_mode{m_modeldata.at(i).at(USER_COL_APP_USE_MODE).toUInt()};
		if (app_use_mode == APP_USE_MODE_SINGLE_COACH || app_use_mode == APP_USE_MODE_COACH_USER_WITH_COACH)
			coaches.append(m_modeldata.at(i).at(USER_COL_NAME));
	}
	return coaches;
}

QStringList DBUserModel::getClients() const
{
	QStringList clients;
	for (uint i{0}; i < m_modeldata.count(); ++i)
	{
		const uint app_use_mode{m_modeldata.at(i).at(USER_COL_APP_USE_MODE).toUInt()};
		if (app_use_mode == APP_USE_MODE_CLIENTS || app_use_mode == APP_USE_MODE_SINGLE_USER_WITH_COACH)
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

static QString getNetworkUserName(const QString &userName, const uint app_use_mode)
{
	QString net_name{std::move(appUtils()->stripDiacriticsFromString(userName))};
	net_name = std::move(net_name.toLower());
	static_cast<void>(net_name.replace(' ', '_'));

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

static QString makeUserPassword(const QString &userName)
{
	QString password{userName};
	static_cast<void>(password.replace(' ', '_'));
	return password;
}

void DBUserModel::setUserName(const int row, const QString &new_name, const int prev_use_mode, const int ret_code, const QString &networkReply)
{
	const QString &net_name{getNetworkUserName(new_name, appUserModel()->appUseMode(row))}; //Used for easier identification on the server side
	const QString &password{makeUserPassword(new_name)};

	if (ret_code == -1000)
	{
		appOnlineServices()->checkUser(net_name, password);
		connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,row,new_name,prev_use_mode] (const int ret_code, const QString& ret_string) {
			if (_userName(row).isEmpty()) //User not registered in the local database
				appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, tr("Online registration") + record_separator + ret_string);
			setUserName(row, new_name, prev_use_mode, ret_code, ret_string);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	}
	else
	{
		switch (ret_code)
		{
			case 6: //User does not exist in the online database
				connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this,row,new_name] (const int ret_code, const QString& ret_string) {
					appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, tr("Online registration") + record_separator + ret_string);
				}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
				if (appUserModel()->_userName(row).isEmpty())
					appOnlineServices()->registerUser(net_name, password);
				else
					appOnlineServices()->alterUser(getNetworkUserName(appUserModel()->_userName(row), prev_use_mode != -1 ? prev_use_mode :
						appUserModel()->appUseMode(row)), net_name, password);
			case 0: //User already and correctly registered on the server. But, for whatwever reason, might not be on the local database. So, skip the break statement
				if (new_name != _userName(row))
				{
					m_modeldata[row][USER_COL_NAME] = new_name;
					emit userModified(row, USER_COL_NAME);
					if (m_modeldata.count() > 1 && m_modeldata.at(row).at(USER_COL_ID) == STR_MINUS_ONE)
						emit userAddedOrRemoved(row, true);
				}
			break;
			default:
				appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_ERROR, networkReply);
		}
		emit userNameOK(row, ret_code == 0);
	}
}

void DBUserModel::setCoachPublicStatus(const uint row, const bool bPublic)
{
	const QString &user_name{userName(row)};
	appOnlineServices()->addOrRemoveCoach(user_name, makeUserPassword(user_name), bPublic);
	connect(appOnlineServices(), &TPOnlineServices::networkRequestProcessed, this, [this] (const int ret_code, const QString& ret_string) {
		appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_CUSTOM_MESSAGE, tr("Coach registration") + record_separator + ret_string);
	}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
}

int DBUserModel::importFromFile(const QString &filename)
{
	QFile *inFile{new QFile{filename}};
	if (!inFile->open(QIODeviceBase::ReadOnly|QIODeviceBase::Text))
	{
		delete inFile;
		return APPWINDOW_MSG_OPEN_FAILED;
	}

	char buf[128];
	qint64 lineLength(0);
	uint col(USER_COL_NAME);
	QString value;
	bool bFoundModelInfo(false);
	const QString tableIdStr("0x000"_L1 + QString::number(USER_TABLE_ID));
	QStringList modeldata(USER_TOTAL_COLS);
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
		m_modeldata.append(std::move(modeldata));
	return col >= USER_COL_APP_USE_MODE ? APPWINDOW_MSG_READ_FROM_FILE_OK : APPWINDOW_MSG_UNKNOWN_FILE_FORMAT;
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
