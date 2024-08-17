#include "dbmesocyclesmodel.h"
#include "dbusermodel.h"
#include "runcommands.h"

DBMesocyclesModel::DBMesocyclesModel(QObject* parent, DBUserModel* userModel)
	: TPListModel(parent), m_userModel(userModel)
{
	m_tableId = MESOCYCLES_TABLE_ID;
	setObjectName(DBMesocyclesObjectName);

	m_roleNames[mesoNameRole] = "mesoName";
	m_roleNames[mesoStartDateRole] = "mesoStartDate";
	m_roleNames[mesoEndDateRole] = "mesoEndDate";
	m_roleNames[mesoNoteRole] = "mesoNote";
	m_roleNames[mesoWeeksRole] = "mesoWeeks";
	m_roleNames[mesoSplitRole] = "mesoSplit";
	m_roleNames[realMesoRole] = "realMeso";

	mColumnNames.reserve(MESOCYCLES_TOTAL_COLS);
	mColumnNames.append(QString());
	mColumnNames.append(tr("Mesocycle's name: "));
	mColumnNames.append(tr("Start date: "));
	mColumnNames.append(tr("End date: "));
	mColumnNames.append(tr("Mesocycle's considerations: "));
	mColumnNames.append(tr("Number of weeks: "));
	mColumnNames.append(tr("Weekly Training Division: "));

	QString strCoach;
	QString strClient;
	switch (m_userModel->appUseMode(0))
	{
		case APP_USE_MODE_SINGLE_USER: break;
		case APP_USE_MODE_SINGLE_COACH:
			strClient = tr("Client: ");
		break;
		case APP_USE_MODE_SINGLE_USER_WITH_COACH:
			strCoach = tr("Coach/Trainer: ");
		break;
		case APP_USE_MODE_COACH_USER_WITH_COACHES:
			strClient = tr("Client: ");
			strCoach = tr("Coach/Trainer: ");
		break;
	}
	mColumnNames.append(strCoach);
	mColumnNames.append(strClient);
	mColumnNames.append(QString());
	mColumnNames.append(tr("Type: "));
}

void DBMesocyclesModel::exportToText(QFile *outFile, const bool bFancy) const
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

	QString value;
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
						if (!isFieldFormatSpecial(i))
							value = (*itr).at(i);
						else
							value = formatFieldToExport((*itr).at(i));
						outFile->write(value.replace(subrecord_separator, '|').toUtf8().constData());
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

bool DBMesocyclesModel::importFromFancyText(QFile* inFile, QString& inData)
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
		modeldata.append(value); //meso name
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
				modeldata.append(modeldata.at(MESOCYCLES_COL_ENDDATE).toInt() != 0 ? u"1"_qs : u"0"_qs); //MESOCYCLES_COL_REALMESO
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
				if (isFieldFormatSpecial(col))
					modeldata.append(formatFieldToImport(value));
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

QString DBMesocyclesModel::formatFieldToExport(const QString &fieldValue) const
{
	return runCmd()->formatDate(QDate::fromJulianDay(fieldValue.toInt()));
}

QString DBMesocyclesModel::formatFieldToImport(const QString &fieldValue) const
{
	return QString::number(runCmd()->getDateFromStrDate(fieldValue).toJulianDay());
}

QVariant DBMesocyclesModel::data(const QModelIndex &index, int role) const
{
	const int row(index.row());
	if( row >= 0 && row < m_modeldata.count() )
	{
		switch(role) {
			case mesoStartDateRole:
			case mesoEndDateRole:
				return QDate::fromJulianDay(static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole)).toLongLong());
			case mesoNameRole:
			case mesoNoteRole:
			case mesoWeeksRole:
			case mesoSplitRole:
				return static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole));
			case realMesoRole:
				return static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole)) == QStringLiteral("1");
		}
	}
	return QString();
}
uint DBMesocyclesModel::getTotalSplits(const uint row) const
{
	uint nSplits(0);
	if (row < m_modeldata.count())
	{
		if (!m_modeldata.isEmpty())
		{
			const QString mesoSplit(m_modeldata.at(row).at(MESOCYCLES_COL_SPLIT));
			QString::const_iterator itr(mesoSplit.constBegin());
			const QString::const_iterator itr_end(mesoSplit.constEnd());
			QString mesoLetters;

			do {
				if (static_cast<QChar>(*itr) == QChar('R'))
					continue;
				if (mesoLetters.contains(static_cast<QChar>(*itr)))
					continue;
				mesoLetters.append(static_cast<QChar>(*itr));
				nSplits++;
			} while (++itr != itr_end);
		}
	}
	return nSplits;
}

int DBMesocyclesModel::getMesoIdx(const int mesoId) const
{
	const QString strMesoId(QString::number(mesoId));
	for(int x(0); x < count(); ++x)
	{
		if (getFast(x, MESOCYCLES_COL_ID) == strMesoId)
			return x;
	}
	return -1;
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

bool DBMesocyclesModel::isDateWithinCurrentMeso(const QDate& date) const
{
	if (count() > 0)
	{
		if (date >= getDateFast(currentRow(), MESOCYCLES_COL_STARTDATE))
		{
			if (date <= getDateFast(currentRow(), MESOCYCLES_COL_ENDDATE))
				return true;
		}
	}
	return false;
}
//Called when importing from a text file
bool DBMesocyclesModel::isDifferent(const DBMesocyclesModel* model)
{
	if (model->count() > 0)
	{
		if (count() == 0)
			return true;
	}
	else
		return false; //model is not usefull

	bool bEqual(true);
	for (uint n(0); n < count(); ++n)
	{
		for (uint i(1); i < model->m_modeldata.at(0).count(); ++i)
		{
			if (m_modeldata.at(n).at(i) != model->m_modeldata.at(0).at(i))
			{
				bEqual = false;
				break;
			}
		}
		if (bEqual)
			return false;
		bEqual = true;
	}
	return true;
}
