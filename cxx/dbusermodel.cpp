#include "dbusermodel.h"

DBUserModel::DBUserModel(QObject *parent)
	: TPListModel(parent), mb_empty(false), m_userRow(0), m_searchRow(-1)
{
	m_tableId = EXERCISES_TABLE_ID;
	setObjectName(DBExercisesObjectName);

	mColumnNames.reserve(USER_TOTAL_COLS);
	mColumnNames.append(QString());
	mColumnNames.append(tr("Name: "));
	mColumnNames.append(tr("Birthday: "));
	mColumnNames.append(tr("Sex: "));
	mColumnNames.append(tr("Phone: "));
	mColumnNames.append(tr("E-mail: "));
	mColumnNames.append(tr("Social Media: "));
	mColumnNames.append(tr("Role: "));
	mColumnNames.append(tr("Goal: "));
	mColumnNames.append(QString());
	mColumnNames.append(tr("Coach: "));
}

/*switch (userModel->mainUserAppUseMode())
	{
		case COACH_OPT_SINGLE_USER: return;
		case COACH_OPT_SINGLE_COACH: bShowUsers = true; bShowCoaches = false; break;
		case COACH_OPT_SINGLE_USER_WITH_COACH: break;
		case COACH_OPT_COACH_USER_WITH_COACHES: bShowUsers = true; break;
	}
	*/
void DBUserModel::addUser(const bool coach)
{
	uint use_mode(1);
	uint cur_coach(0);
	uint cur_client(0);
	if (!m_modeldata.isEmpty())
	{
		switch (m_modeldata.at(0).at(USER_COL_APP_USE_MODE).toInt())
		{
			case 1:
			case 3:
				if (!coach)
					return;
				use_mode = 2;
				cur_coach = 1;
			break;

			case 2:
				if (coach)
					return;
				use_mode = 0;
				cur_client = 1;
			break;

			case 4:
				if (coach)
				{
					use_mode = 2;
					cur_coach = 1;
				}
				else
				{
					use_mode = 0;
					cur_client = 1;
				}
			break;
		}
	}
	appendList(QStringList() << u"-1"_qs << QString() << u"2451545"_qs << QString() << QString() <<
		QString() << QString() << QString() << QString() << QString() << u"image://tpimageprovider/0"_qs <<
		QString::number(use_mode) << QString::number(cur_coach) << QString::number(cur_client));
	m_userRow = m_modeldata.count() - 1;
}

void DBUserModel::removeUser(const int row)
{
	if (row >= 1 && row < m_modeldata.count())
	{
		removeRow(row);
		if (row <= m_userRow)
			m_userRow--;
	}
}

int DBUserModel::loadUserInfo(const QString& name)
{
	for (uint i(0); i < m_modeldata.count(); ++i)
	{
		if (m_modeldata.at(i).at(USER_COL_NAME) == name)
			return i;
	}
	return -1;
}

int DBUserModel::findFirstUser(const bool bCoach)
{
	int searchRow(1);
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

void DBUserModel::exportToText(QFile* outFile, const bool bFancy) const
{
	QString strHeader;
	if (bFancy)
		strHeader = u"##"_qs + objectName() + u"\n\n"_qs;
	else
		strHeader = u"##0x0"_qs + QString::number(m_tableId) + u"\n"_qs;

	outFile->write(strHeader.toUtf8().constData());
	outFile->write(exportExtraInfo().toUtf8().constData());
	if (bFancy)
		outFile->write("\n\n", 2);
	else
		outFile->write("\n", 1);

	QList<QStringList>::const_iterator itr(m_modeldata.constBegin());
	const QList<QStringList>::const_iterator itr_end(m_modeldata.constEnd());

	while (itr != itr_end)
	{
		for (uint i(0); i < (*itr).count(); ++i)
		{
			if (bFancy)
			{
				if (i < mColumnNames.count())
				{
					if (!mColumnNames.at(i).isEmpty())
					{
						outFile->write(mColumnNames.at(i).toUtf8().constData());
						outFile->write((*itr).at(i).toUtf8().constData());
						outFile->write("\n", 1);
					}
				}
			}
			else
			{
				outFile->write((*itr).at(i).toUtf8().constData());
					outFile->write(QByteArray(1, record_separator.toLatin1()), 1);
			}
		}
		if (bFancy)
			outFile->write("\n", 1);
		else
			outFile->write(QByteArray(1, record_separator2.toLatin1()), 1);
		++itr;
	}

	if (bFancy)
		outFile->write(tr("##End##\n").toUtf8().constData());
	else
		outFile->write("##end##");
}

bool DBUserModel::importFromFancyText(QFile* inFile, QString& inData)
{
	char buf[256];
	QStringList modeldata;
	uint col(1);
	QString value;

	//Because a DBMesocyclesModel does not have an extra info to export nor import, inFile is already at the
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
