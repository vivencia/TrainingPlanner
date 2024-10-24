#include "dbusermodel.h"
#include "tpglobals.h"
#include "tputils.h"

#include <utility>

DBUserModel* DBUserModel::_appUserModel(nullptr);

DBUserModel::DBUserModel(QObject *parent)
	: TPListModel{parent}, mb_empty(false), m_searchRow(-1)
{
	_appUserModel = this;

	setObjectName(DBUserObjectName);
	m_tableId = EXERCISES_TABLE_ID;
	m_exportName = std::move(tr("Coach information"));

	mColumnNames.reserve(USER_TOTAL_COLS);
	mColumnNames.append(QString());
	mColumnNames.append(std::move(tr("Name: ")));
	mColumnNames.append(std::move(tr("Birthday: ")));
	mColumnNames.append(std::move(tr("Sex: ")));
	mColumnNames.append(std::move(tr("Phone: ")));
	mColumnNames.append(std::move(u"E-mail: "_s));
	mColumnNames.append(std::move(tr("Social Media: ")));
	mColumnNames.append(std::move(tr("Your are: ")));
	mColumnNames.append(std::move(tr("Professional job: ")));
	mColumnNames.append(std::move(tr("Goal: ")));
	mColumnNames.append(std::move(u"Avatar: "_s));
	mColumnNames.append(QString());
	mColumnNames.append(QString());
	mColumnNames.append(QString());
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
	appendList(QStringList() << STR_MINUS_ONE << QString() << std::move(u"2451545"_s) << STR_ZERO << QString() <<
		QString() << QString() << QString() << QString() << QString() << std::move(u"image://tpimageprovider/m5"_s) <<
		QString::number(use_mode) << QString::number(cur_coach) << QString::number(cur_client));
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
		if (m_modeldata.at(searchRow).at(USER_COL_APP_USE_MODE) == (bCoach ? u"2"_s : u"0"_s))
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
		if (m_modeldata.at(searchRow).at(USER_COL_APP_USE_MODE) == (bCoach ? u"2"_s : u"0"_s))
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
		if (m_modeldata.at(searchRow).at(USER_COL_APP_USE_MODE) == (bCoach ? u"2"_s : u"0"_s))
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
		if (m_modeldata.at(searchRow).at(USER_COL_APP_USE_MODE) == (bCoach ? u"2"_s : u"0"_s))
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

const int DBUserModel::getRowByName(const QString& username) const
{
	for (uint i(1); i < m_modeldata.count(); ++i)
	{
		if (m_modeldata.at(i).at(USER_COL_NAME) == username)
			return i;
	}
	return -1;
}

QStringList DBUserModel::getCoaches() const
{
	QStringList coaches;
	if (appUseMode(0) >= APP_USE_MODE_SINGLE_USER_WITH_COACH)
		coaches.append(m_modeldata.at(0).at(USER_COL_NAME));
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

int DBUserModel::importFromFile(const QString& filename)
{
	QFile* inFile{new QFile(filename)};
	if (!inFile->open(QIODeviceBase::ReadOnly|QIODeviceBase::Text))
	{
		delete inFile;
		return APPWINDOW_MSG_OPEN_FAILED;
	}

	char buf[128];
	qint64 lineLength(0);
	uint col(1);
	QString value;

	QStringList modeldata(USER_TOTAL_COLS);
	modeldata[0] = STR_MINUS_ONE;

	while ((lineLength = inFile->readLine(buf, sizeof(buf))) != -1)
	{
		if (strstr(buf, STR_END_EXPORT.toLatin1().constData()) == NULL)
		{
			if (lineLength > 10)
			{
				if (strstr(buf, "##") != NULL)
				{
					if (col < USER_COL_APP_USE_MODE)
					{
						if (col != USER_COL_USERROLE)
						{
							value = buf;
							if (!isFieldFormatSpecial(col))
								modeldata[col] = value.remove(0, value.indexOf(':') + 2);
							else
								modeldata[col] = formatFieldToImport(col, value.remove(0, value.indexOf(':') + 2));
						}
						++col;
					}
					else if (col == USER_COL_APP_USE_MODE)
						modeldata[USER_COL_APP_USE_MODE] = QString::number(APP_USE_MODE_SINGLE_COACH);
				}
			}
		}
		else
			break;
	}
	m_modeldata.append(modeldata);
	inFile->close();
	delete inFile;
	return modeldata.count() > 1 ? APPWINDOW_MSG_READ_FROM_FILE_OK : APPWINDOW_MSG_UNKNOWN_FILE_FORMAT;
}

bool DBUserModel::updateFromModel(const TPListModel* const model)
{
	appendList(model->m_modeldata.at(0));
	return true;
}

QString DBUserModel::formatFieldToExport(const uint field, const QString& fieldValue) const
{
	switch (field)
	{
		case USER_COL_BIRTHDAY:
			return appUtils()->formatDate(QDate::fromJulianDay(fieldValue.toInt()));
		case USER_COL_SEX:
			return fieldValue == STR_ZERO ? tr("Male") : tr("Female");
		case USER_COL_SOCIALMEDIA:
		{
			QString strSocial{fieldValue};
			return strSocial.replace(record_separator, fancy_record_separator1);
		}
		case USER_COL_AVATAR:
			if (fieldValue.contains(u"tpimageprovider"_s))
				return fieldValue.last(fieldValue.length()-24);
			else
				return m_modeldata.at(m_exportRows.at(0)).at(USER_COL_SEX) == STR_ZERO ? u"Avatar-m5"_s : u"Avatar-f0"_s;
		default: return QString();
	}
}

QString DBUserModel::formatFieldToImport(const uint field, const QString& fieldValue) const
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
			return u"image://tpimageprovider/"_s + fieldValue.last(fieldValue.length()-7);
		default: return QString();
	}
}
