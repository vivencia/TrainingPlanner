#include "dbusermodel.h"
#include "runcommands.h"

DBUserModel::DBUserModel(QObject *parent)
	: TPListModel(parent), mb_empty(false), m_searchRow(-1)
{
	m_tableId = EXERCISES_TABLE_ID;
	setObjectName(DBUserObjectName);

	mColumnNames.reserve(USER_TOTAL_COLS);
	mColumnNames.append(QString());
	mColumnNames.append(tr("Name: "));
	mColumnNames.append(tr("Birthday: "));
	mColumnNames.append(tr("Sex: "));
	mColumnNames.append(tr("Phone: "));
	mColumnNames.append(u"E-mail: "_qs);
	mColumnNames.append(tr("Social Media: "));
	mColumnNames.append(QString());
	mColumnNames.append(tr("Role: "));
	mColumnNames.append(tr("Goal: "));
	mColumnNames.append(u"Avatar: "_qs);
	mColumnNames.append(QString());
	mColumnNames.append(QString());
	mColumnNames.append(QString());
}

int DBUserModel::addUser(const bool bCoach)
{
	uint use_mode(1);
	uint cur_coach(0);
	uint cur_client(0);
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
				cur_coach = 1;
			break;

			case APP_USE_MODE_SINGLE_COACH:
				if (bCoach)
					return -1;
				use_mode = APP_USE_MODE_SINGLE_USER;
				cur_client = 1;
			break;

			case APP_USE_MODE_COACH_USER_WITH_COACHES:
				if (bCoach)
				{
					use_mode = APP_USE_MODE_SINGLE_COACH;
					cur_coach = 1;
				}
				else
				{
					use_mode = APP_USE_MODE_SINGLE_USER;
					cur_client = 1;
				}
			break;
		}
	}
	appendList(QStringList() << u"-1"_qs << QString() << u"2451545"_qs << QString() << QString() <<
		QString() << QString() << QString() << QString() << QString() << u"image://tpimageprovider/0"_qs <<
		QString::number(use_mode) << QString::number(cur_coach) << QString::number(cur_client));
	if (m_modeldata.count() > 1)
		emit userAdded(use_mode);
	return m_modeldata.count() - 1;
}

uint DBUserModel::removeUser(const int row, const bool bCoach)
{
	if (row >= 1 && row < m_modeldata.count())
	{
		removeRow(row);
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
		if (m_modeldata.at(searchRow).at(USER_COL_APP_USE_MODE) == (bCoach ? u"2"_qs : u"0"_qs))
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
		if (m_modeldata.at(searchRow).at(USER_COL_APP_USE_MODE) == (bCoach ? u"2"_qs : u"0"_qs))
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

	int searchRow(m_searchRow);
	for (; searchRow >= 0; --searchRow)
	{
		if (m_modeldata.at(searchRow).at(USER_COL_APP_USE_MODE) == (bCoach ? u"2"_qs : u"0"_qs))
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
		if (m_modeldata.at(searchRow).at(USER_COL_APP_USE_MODE) == (bCoach ? u"2"_qs : u"0"_qs))
		{
			m_searchRow = searchRow;
			break;
		}
	}
	return m_searchRow;
}

QString DBUserModel::getCurrentUserName(const bool bCoach) const
{
	const int row(m_modeldata.at(0).at(bCoach ? USER_COL_CURRENT_COACH : USER_COL_CURRENT_COACH).toInt());
	return row > 0 ? m_modeldata.at(row).at(USER_COL_NAME) : QString();
}

const int DBUserModel::getRowByCoachName(const QString& coachname) const
{
	for (uint i(1); i < m_modeldata.count(); ++i)
	{
		if (m_modeldata.at(i).at(USER_COL_NAME) == coachname)
			if (m_modeldata.at(i).at(USER_COL_APP_USE_MODE).toUInt() == APP_USE_MODE_SINGLE_COACH)
				return i;
	}
	return -1;
}

QStringList DBUserModel::getCoaches() const
{
	QStringList coaches;
	for (uint i(1); i < m_modeldata.count(); ++i)
	{
		if (m_modeldata.at(i).at(USER_COL_APP_USE_MODE).toUInt() == APP_USE_MODE_SINGLE_COACH)
			coaches.append(m_modeldata.at(i).at(USER_COL_NAME));
	}
	return coaches;
}

QStringList DBUserModel::getClients() const
{
	QStringList clients;
	for (uint i(1); i < m_modeldata.count(); ++i)
	{
		if (m_modeldata.at(i).at(USER_COL_APP_USE_MODE).toUInt() == APP_USE_MODE_CLIENTS)
			clients.append(m_modeldata.at(i).at(USER_COL_NAME));
	}
	return clients;
}

void DBUserModel::setUserName(const int row, const QString& new_name)
{
	if (row >= 0 && row < m_modeldata.count())
	{
		m_modeldata[row][USER_COL_NAME] = new_name;
		setModified(true);
	}
}

void DBUserModel::setBirthDate(const int row, const QDate& new_date)
{
	if (row >= 0 && row < m_modeldata.count())
	{
		m_modeldata[row][USER_COL_BIRTHDAY] = QString::number(new_date.toJulianDay());
		setModified(true);
	}
}

void DBUserModel::setSex(const int row, const QString& new_sex)
{
	if (row >= 0 && row < m_modeldata.count())
	{
		m_modeldata[row][USER_COL_SEX] = new_sex;
		setModified(true);
	}
}

void DBUserModel::setPhone(const int row, const QString& new_phone)
{
	if (row >= 0 && row < m_modeldata.count())
	{
		m_modeldata[row][USER_COL_PHONE] = new_phone;
		setModified(true);
	}
}

void DBUserModel::setEmail(const int row, const QString& new_email)
{
	if (row >= 0 && row < m_modeldata.count())
	{
		m_modeldata[row][USER_COL_EMAIL] = new_email;
		setModified(true);
	}
}

void DBUserModel::setSocialMedia(const int row, const QString& new_social)
{
	if (row >= 0 && row < m_modeldata.count())
	{
		m_modeldata[row][USER_COL_SOCIALMEDIA] = new_social;
		setModified(true);
	}
}

void DBUserModel::setUserRole(const int row, const QString& new_role)
{
	if (row >= 0 && row < m_modeldata.count())
	{
		m_modeldata[row][USER_COL_USERROLE] = new_role;
		setModified(true);
	}
}

void DBUserModel::setCoachRole(const int row, const QString& new_role)
{
	if (row >= 0 && row < m_modeldata.count())
	{
		m_modeldata[row][USER_COL_COACHROLE] = new_role;
		setModified(true);
	}
}

void DBUserModel::setGoal(const int row, const QString& new_goal)
{
	if (row >= 0 && row < m_modeldata.count())
	{
		m_modeldata[row][USER_COL_GOAL] = new_goal;
		setModified(true);
	}
}

void DBUserModel::setAvatar(const int row, const QString& new_avatar)
{
	if (row >= 0 && row < m_modeldata.count())
	{
		m_modeldata[row][USER_COL_AVATAR] = new_avatar;
		setModified(true);
	}
}

void DBUserModel::setAppUseMode(const int row, const int new_use_opt)
{
	if (row >= 0 && row < m_modeldata.count())
	{
		m_modeldata[row][USER_COL_APP_USE_MODE] = QString::number(new_use_opt);
		emit appUseModeChanged(row);
		setModified(true);
	}
}

void DBUserModel::setCurrentCoach(const int row, const int new_current_coach)
{
	if (row >= 0 && row < m_modeldata.count())
	{
		m_modeldata[row][USER_COL_CURRENT_COACH] = QString::number(new_current_coach);
		setModified(true);
	}
}

void DBUserModel::setCurrentUser(const int row, const int new_current_user)
{
	if (row >= 0 && row < m_modeldata.count())
	{
		m_modeldata[row][USER_COL_CURRENT_USER] = QString::number(new_current_user);
		setModified(true);
	}
}

QString DBUserModel::formatFieldToExport(const uint field, const QString& fieldValue) const
{
	if (field == USER_COL_BIRTHDAY)
		return runCmd()->formatDate(QDate::fromJulianDay(fieldValue.toInt()));
	else
	{
		if (fieldValue.contains(u"tpimageprovider"_qs))
			return fieldValue.right(fieldValue.length()-24);
		else

		{
			if (m_modeldata.at(m_exportRows.at(0)).at(USER_COL_SEX) == tr("Male"))
				return u"Avatar-5"_qs;
			else
				return u"Avatar-0"_qs;
		}
	}
}

QString DBUserModel::formatFieldToImport(const uint field, const QString& fieldValue) const
{
	if (field == USER_COL_BIRTHDAY)
		return QString::number(runCmd()->getDateFromStrDate(fieldValue).toJulianDay());
	else
		return u"image://tpimageprovider/" + fieldValue.right(fieldValue.length()-7);
}

bool DBUserModel::updateFromModel(const TPListModel* model)
{
	if (model->count() > 0)
	{
		QList<QStringList>::const_iterator lst_itr(model->m_modeldata.constBegin());
		const QList<QStringList>::const_iterator lst_itrend(model->m_modeldata.constEnd());
		uint lastIndex(m_modeldata.count());
		do {
			m_modifiedIndices.append(lastIndex++);
			appendList((*lst_itr));
		} while (++lst_itr != lst_itrend);
		return true;
	}
	return false;
}

bool DBUserModel::importFromText(QFile* inFile, QString& inData)
{
	char buf[256];
	QStringList modeldata;
	uint col(1);
	QString value;

	//Because a DBUserModel does not have an extra info to export nor import, inFile is already at the
	//first relevant information of the meso, its name
	inData.chop(1);
	int sep_idx(inData.indexOf(':'));
	if (sep_idx != -1)
	{
		value = inData.right(inData.length() - sep_idx - 2);
		modeldata.append(u"-1"_qs); //id
		modeldata.append(value); //name
		col++;
	}
	else
		return false;

	while (inFile->readLine(buf, sizeof(buf)) != -1) {
		inData = buf;
		inData.chop(1);
		if (inData.isEmpty())
		{
			if (!modeldata.isEmpty())
			{
				appendList(modeldata);
				modeldata.clear();
				col = 1;
			}
		}
		else
		{
			sep_idx = inData.indexOf(':');
			if (sep_idx != -1)
			{
				value = inData.right(inData.length() - sep_idx - 2);
				modeldata.append(value);
				col++;
			}
			else
			{
				if (inData.contains(u"##"_qs))
					break;
			}
		}
	}
	return count() > 0;
}
