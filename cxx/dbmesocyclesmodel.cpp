#include "dbmesocyclesmodel.h"
#include "runcommands.h"

DBMesocyclesModel::DBMesocyclesModel(QObject *parent)
	: TPListModel(parent)
{
	m_tableId = MESOCYCLES_TABLE_ID;
	setObjectName(DBMesocyclesObjectName);

	// Set names to the role name hash container (QHash<int, QByteArray>)
	m_roleNames[mesoIdRole] = "mesoId";
	m_roleNames[mesoNameRole] = "mesoName";
	m_roleNames[mesoStartDateRole] = "mesoStartDate";
	m_roleNames[mesoEndDateRole] = "mesoEndDate";
	m_roleNames[mesoNoteRole] = "mesoNote";
	m_roleNames[mesoWeeksRole] = "mesoWeeks";
	m_roleNames[mesoSplitRole] = "mesoSplit";
	m_roleNames[mesoDrugsRole] = "mesoDrugs";
	m_roleNames[realMesoRole] = "realMeso";

	mColumnNames.reserve(MESOCYCLES_COL_DRUGS+1);
	mColumnNames.append(QString());
	mColumnNames.append(tr("Mesocycle's name: "));
	mColumnNames.append(tr("Start date: "));
	mColumnNames.append(tr("End date: "));
	mColumnNames.append(tr("Mesocycle's considerations: "));
	mColumnNames.append(tr("Number of weeks: "));
	mColumnNames.append(tr("Weekly Training Division: "));
	mColumnNames.append(tr("Drug Protocol: "));
}

void DBMesocyclesModel::updateFromModel(TPListModel* model)
{
	if (model->count() > 0)
	{
		updateList(model->m_modeldata.at(0), count() - 1);
		setCurrentRow(count() - 1);
	}
}

/*const QString DBMesocyclesModel::exportExtraInfo() const
{
	QString extraInfo;
	for (uint i(0); i < 6; ++i)
		extraInfo.append(tr("Split%1: ").arg(QChar(static_cast<char>('A' + i))) + m_extraInfo.at(i) + u"\n"_qs);
	extraInfo.chop(2);
	return extraInfo;
}*/

bool DBMesocyclesModel::importExtraInfo(const QString& extraInfo)
{
	QString::const_iterator itr(extraInfo.constBegin());
	const QString::const_iterator itr_end(extraInfo.constEnd());
	int chr_colon(0);
	int chr_sep(0);
	uint chr_pos(0);

	while (itr != itr_end)
	{
		switch((*itr).toLatin1())
		{
			case ':':
				chr_colon = chr_pos + 2;
			break;
			case '\n':
				chr_sep = chr_pos;
				m_extraInfo.append(extraInfo.mid(chr_colon, chr_sep - chr_colon));
			break;
		}
		++chr_pos;
		++itr;
	}
	return !m_extraInfo.isEmpty();
}

bool DBMesocyclesModel::importFromFancyText(QFile* inFile)
{
	char buf[256];
	QString inData;
	QStringList modeldata;
	int sep_idx(-1);
	uint col(0);
	QString value;

	while (inFile->readLine(buf, sizeof(buf)) != -1) {
		inData = buf;
		inData.chop(1);
		if (inData.isEmpty())
		{
			if (!modeldata.isEmpty())
			{
				appendList(modeldata);
				modeldata.clear();
				col = 0;
			}
		}
		else
		{
			sep_idx = inData.indexOf(':');
			if (sep_idx != -1)
			{
				value = inData.right(inData.length() - sep_idx - 2);
				//The subtraction is because the ID column is not exported making all columns in the exported text MESOCYCLES_COL_* - 1
				if (col == MESOCYCLES_COL_STARTDATE-1 ||col == MESOCYCLES_COL_ENDDATE-1)
					modeldata.append(QString::number(runCmd()->getDateFromStrDate(value).toJulianDay()));
				else
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

QString DBMesocyclesModel::formatField(const QString &fieldValue) const
{
	return runCmd()->formatDate(QDate::fromJulianDay(fieldValue.toInt()));
}

uint DBMesocyclesModel::getTotalSplits(const uint row) const
{
	const QString mesoSplit(m_modeldata.at(row).at(MESOCYCLES_COL_SPLIT));
	QString::const_iterator itr(mesoSplit.constBegin());
	const QString::const_iterator itr_end(mesoSplit.constEnd());
	QString mesoLetters;
	uint nSplits(0);

	do {
		if (static_cast<QChar>(*itr) == QChar('R'))
			continue;
		if (mesoLetters.contains(static_cast<QChar>(*itr)))
			continue;
		mesoLetters.append(static_cast<QChar>(*itr));
		nSplits++;
	} while (++itr != itr_end);
	return nSplits;
}

/*void DBMesocyclesModel::setSplitInfo(const QString& splitA, const QString& splitB, const QString& splitC,
									const QString& splitD, const QString& splitE, const QString& splitF)
{
	m_extraInfo.clear();
	m_extraInfo.append(QStringList() << splitA << splitB << splitC << splitD << splitE << splitF);
}*/

QVariant DBMesocyclesModel::data(const QModelIndex &index, int role) const
{
	const int row(index.row());
	if( row >= 0 && row < m_modeldata.count() )
	{
		switch(role) {
			case mesoIdRole:
				return static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole)).toUInt();
			case mesoStartDateRole:
			case mesoEndDateRole:
				return QDate::fromJulianDay(static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole)).toLongLong());
			case mesoNameRole:
			case mesoNoteRole:
			case mesoWeeksRole:
			case mesoSplitRole:
			case mesoDrugsRole:
				return static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole));
			case realMesoRole:
				return static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole)) == QStringLiteral("1");
			case Qt::DisplayRole:
				return m_modeldata.at(row).at(index.column());
		}
	}
	return QVariant();
}

bool DBMesocyclesModel::setData(const QModelIndex &index, const QVariant& value, int role)
{
	const int row(index.row());
	if( row >= 0 && row < m_modeldata.count() )
	{
		switch(role) {
			case mesoIdRole:
			case mesoNameRole:
			case mesoStartDateRole:
			case mesoEndDateRole:
			case mesoNoteRole:
			case mesoWeeksRole:
			case mesoSplitRole:
			case mesoDrugsRole:
			case realMesoRole:
				m_modeldata[row][role-Qt::UserRole] = value.toString();
				setModified(true);
				emit dataChanged(index, index, QList<int>() << role);
				return true;
		}
	}
	return false;
}

QString DBMesocyclesModel::getMesoInfo(const int mesoid, const uint field) const
{
	for(uint x(0); x < count(); ++x)
	{
		if (static_cast<QString>(m_modeldata.at(x).at(MESOCYCLES_COL_ID)).toInt() == mesoid)
			return m_modeldata.at(x).at(field);
	}
	return QString();
}

int DBMesocyclesModel::getPreviousMesoId(const int current_mesoid) const
{
	for(uint x(1); x < count(); ++x)
	{
		if (static_cast<QString>(m_modeldata.at(x).at(MESOCYCLES_COL_ID)).toInt() == current_mesoid)
			return static_cast<QString>(m_modeldata.at(x-1).at(MESOCYCLES_COL_ID)).toInt();
	}
	return -1;
}

QDate DBMesocyclesModel::getPreviousMesoEndDate(const int current_mesoid) const
{
	for(uint x(1); x < count(); ++x)
	{
		if (static_cast<QString>(m_modeldata.at(x).at(MESOCYCLES_COL_ID)).toInt() == current_mesoid)
			return QDate::fromJulianDay(static_cast<QString>(m_modeldata.at(x-1).at(MESOCYCLES_COL_ENDDATE)).toLongLong());
	}
	// 1 or 0 meso records = no previous meso. The first meso can start anywhere in 2024
	return QDate(2024, 1, 1);
}

QDate DBMesocyclesModel::getNextMesoStartDate(const int mesoid) const
{
	for(uint x(0); x < count() - 1; ++x)
	{
		if (static_cast<QString>(m_modeldata.at(x).at(MESOCYCLES_COL_ID)).toInt() == mesoid)
			return QDate::fromJulianDay(static_cast<QString>(m_modeldata.at(x+1).at(MESOCYCLES_COL_STARTDATE)).toLongLong());
	}
	 //This is the most current meso. The cut off date for it is undetermined. So we set a value that is 6 months away
	return QDate::currentDate().addMonths(6);
}

QDate DBMesocyclesModel::getLastMesoEndDate() const
{
	if (count() > 0)
		return QDate::fromJulianDay(static_cast<QString>(m_modeldata.last().at(MESOCYCLES_COL_ENDDATE)).toLongLong());
	return QDate::currentDate();
}

int DBMesocyclesModel::mesoThatHasDate(const QDateTime& datetime) const
{
	const QDate date(datetime.date());
	uint mesoIdx(0);
	for(; mesoIdx < count(); ++mesoIdx)
	{
		if (date >= getDateFast(mesoIdx, MESOCYCLES_COL_STARTDATE))
		{
			if (date <= getDateFast(mesoIdx, MESOCYCLES_COL_ENDDATE))
				return mesoIdx;
		}
	}
	return -5; //cannot return -1 because it will be the currentRow of mesocyclesModel when it is empty and we might get matches we do not want
}
