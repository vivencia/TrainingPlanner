#include "dbmesocyclesmodel.h"
#include "dbmesocalendarmodel.h"
#include "dbmesosplitmodel.h"
#include "tputils.h"

#include <QSettings>

DBMesocyclesModel::DBMesocyclesModel(QObject* parent, DBUserModel* userModel)
	: TPListModel(parent), m_userModel(userModel), m_mostRecentOwnMesoIdx(-1)
{
	m_tableId = MESOCYCLES_TABLE_ID;
	setObjectName(DBMesocyclesObjectName);

	m_roleNames[mesoNameRole] = "mesoName";
	m_roleNames[mesoStartDateRole] = "mesoStartDate";
	m_roleNames[mesoEndDateRole] = "mesoEndDate";
	m_roleNames[mesoSplitRole] = "mesoSplit";
	m_roleNames[mesoCoachRole] = "mesoCoach";
	m_roleNames[mesoClientRole] = "mesoClient";

	mColumnNames.reserve(MESOCYCLES_TOTAL_COLS);
	mColumnNames.append(QString());
	mColumnNames.append(tr("Plan's name: "));
	mColumnNames.append(tr("Start date: "));
	mColumnNames.append(tr("End date: "));
	mColumnNames.append(tr("Plan's considerations: "));
	mColumnNames.append(tr("Number of weeks: "));
	mColumnNames.append(tr("Weekly Training Division: "));
	mColumnNames.append(QString()); //Coach
	mColumnNames.append(QString()); //Client
	mColumnNames.append(QString());
	mColumnNames.append(tr("Type: "));

	updateColumnLabels();

	m_splitModel = new DBMesoSplitModel(this, false, -1);
}

DBMesocyclesModel::~DBMesocyclesModel()
{
	delete m_splitModel;
	for (uint i(0); i < m_calendarModelList.count(); ++i)
		delete m_calendarModelList[i];
}

const uint DBMesocyclesModel::newMesocycle(const QStringList& infolist)
{
	appendList(infolist);
	m_splitModel->appendList(QStringList() << STR_MINUS_ONE << STR_MINUS_ONE << QString() << QString() <<
		QString() << QString() << QString() << QString());

	const uint meso_idx(count()-1);
	m_calendarModelList.insert(meso_idx, new DBMesoCalendarModel(this, meso_idx));
	m_splitModel->setMesoIdx(meso_idx);
	getTotalSplits(meso_idx);
	if (isOwnMeso(meso_idx))
	{
		m_mostRecentOwnMesoIdx = meso_idx;
		emit mostRecentOwnMesoChanged(m_mostRecentOwnMesoIdx);
	}
	setCurrentMesoIdx(meso_idx);
	return meso_idx;
}

void DBMesocyclesModel::delMesocycle(const uint meso_idx)
{
	delete m_calendarModelList.value(meso_idx);
	m_calendarModelList.remove(meso_idx);
	m_totalSplits.remove(meso_idx);

	m_splitModel->removeFromList(meso_idx);
	removeFromList(meso_idx);

	for (uint i(meso_idx); i < count(); ++i)
	{
		m_splitModel->setMesoIdx(i);
		m_calendarModelList[i]->setMesoIdx(i);
	}

	if (m_mostRecentOwnMesoIdx > meso_idx)
		--m_mostRecentOwnMesoIdx;
	else if (meso_idx == m_mostRecentOwnMesoIdx)
	{
		m_mostRecentOwnMesoIdx = -1;
		findNextOwnMeso();
	}
	emit mostRecentOwnMesoChanged(m_mostRecentOwnMesoIdx);
}

void DBMesocyclesModel::finishedLoadingFromDatabase()
{
	setReady(true);
	m_currentMesoIdx = appSettings()->value("lastViewedMesoIdx").toInt();
}

void DBMesocyclesModel::setCurrentMesoIdx(const uint meso_idx)
{
	if (meso_idx != m_currentMesoIdx)
	{
		m_currentMesoIdx = meso_idx;
		appSettings()->setValue("lastViewedMesoIdx", meso_idx);
		appSettings()->sync();
		emit currentMesoIdxChanged();
	}
}

void DBMesocyclesModel::findNextOwnMeso()
{
	for (int i(count() -1); i >= 0; --i)
	{
		if (isOwnMeso(i))
		{
			m_mostRecentOwnMesoIdx = i;
			break;
		}
	}
}

bool DBMesocyclesModel::setMesoStartDate(const uint meso_idx, const QDate& new_date)
{
	const QString strJulianDate(QString::number(new_date.toJulianDay()));
	if (strJulianDate != getFast(meso_idx, MESOCYCLES_COL_STARTDATE))
	{
		setFast(meso_idx, MESOCYCLES_COL_STARTDATE, strJulianDate);
		emit mesoCalendarFieldsChanged(meso_idx);
		return true;
	}
	return false;
}

bool DBMesocyclesModel::setMesoEndDate(const uint meso_idx, const QDate& new_date)
{
	const QString strJulianDate(QString::number(new_date.toJulianDay()));
	if (strJulianDate != getFast(meso_idx, MESOCYCLES_COL_ENDDATE))
	{
		setFast(meso_idx, MESOCYCLES_COL_ENDDATE, strJulianDate);
		emit mesoCalendarFieldsChanged(meso_idx);
		return true;
	}
	return false;
}

bool DBMesocyclesModel::setMesoSplit(const uint meso_idx, const QString& new_split)
{
	if (new_split != getFast(meso_idx, MESOCYCLES_COL_SPLIT))
	{
		setFast(meso_idx, MESOCYCLES_COL_SPLIT, new_split);
		emit mesoCalendarFieldsChanged(meso_idx);
		getTotalSplits(meso_idx);
		return true;
	}
	return false;
}

QString DBMesocyclesModel::getSplitLetter(const uint meso_idx, const uint day_of_week) const
{
	if (day_of_week >= 0 && day_of_week <= 6)
		return getFast(meso_idx, MESOCYCLES_COL_SPLIT).at(day_of_week);
	return QString();
}

QString DBMesocyclesModel::getMuscularGroup(const uint meso_idx, const QString& splitLetter) const
{
	return splitLetter != u"R"_qs ?
		m_splitModel->getFast(meso_idx, static_cast<int>(splitLetter.at(0).cell()) - static_cast<int>('A') + 2) :
		tr("Rest day");
}

void DBMesocyclesModel::setMuscularGroup(const uint meso_idx, const QString& splitLetter, const QString& newSplitValue)
{
	const uint splitField(static_cast<int>(splitLetter.at(0).cell()) - static_cast<int>('A') + 2);
	if (splitField < 6)
	{
		m_splitModel->setFast(meso_idx, splitField, newSplitValue);
		emit modifiedChanged();
		emit muscularGroupChanged(splitField, splitLetter);
	}
}

void DBMesocyclesModel::setOwnMeso(const int meso_idx, const bool bOwnMeso)
{
	if (set(meso_idx, MESOCYCLES_COL_CLIENT, bOwnMeso ? m_userModel->userName(0) : m_userModel->getCurrentUserName(false)))
	{
		emit isOwnMesoChanged(meso_idx);
		const int cur_ownmeso(m_mostRecentOwnMesoIdx);
		findNextOwnMeso();
		if (cur_ownmeso != m_mostRecentOwnMesoIdx)
			emit mostRecentOwnMesoChanged(m_mostRecentOwnMesoIdx);
	}
}

QVariant DBMesocyclesModel::data(const QModelIndex &index, int role) const
{
	const int row(index.row());
	if( row >= 0 && row < m_modeldata.count() )
	{
		switch(role)
		{
			case mesoNameRole:
				if (m_userModel->userName(0) == m_modeldata.at(row).at(MESOCYCLES_COL_CLIENT))
					return QVariant(u"<b>**  "_qs + m_modeldata.at(row).at(MESOCYCLES_COL_NAME) + u"  **</b>"_qs);
				else
					return QVariant(u"<b>"_qs + m_modeldata.at(row).at(MESOCYCLES_COL_NAME) + u"</b>"_qs);
			case mesoStartDateRole:
				return QVariant(mColumnNames[MESOCYCLES_COL_STARTDATE] + u"<b>"_qs +
					appUtils()->formatDate(getDateFast(row, MESOCYCLES_COL_STARTDATE)) + u"</b>"_qs);
			case mesoEndDateRole:
				return QVariant(mColumnNames[MESOCYCLES_COL_ENDDATE] + u"<b>"_qs +
					appUtils()->formatDate(getDateFast(row, MESOCYCLES_COL_ENDDATE)) + u"</b>"_qs);
			case mesoSplitRole:
				return QVariant(mColumnNames[MESOCYCLES_COL_SPLIT] + u"<b>"_qs + m_modeldata.at(row).at(MESOCYCLES_COL_SPLIT) + u"</b>"_qs);
			case mesoCoachRole:
				if (!m_modeldata.at(row).at(MESOCYCLES_COL_COACH).isEmpty())
					return QVariant(mColumnNames[MESOCYCLES_COL_COACH] + u"<b>"_qs + m_modeldata.at(row).at(MESOCYCLES_COL_COACH) + u"</b>"_qs);
			case mesoClientRole:
				if (!m_modeldata.at(row).at(MESOCYCLES_COL_CLIENT).isEmpty())
					return QVariant(mColumnNames[MESOCYCLES_COL_CLIENT] + u"<b>"_qs + m_modeldata.at(row).at(MESOCYCLES_COL_CLIENT) + u"</b>"_qs);

		}
	}
	return QString();
}

void DBMesocyclesModel::getTotalSplits(const uint meso_idx)
{
	uint nSplits(0);
	if (meso_idx < m_modeldata.count())
	{
		if (!m_modeldata.isEmpty())
		{
			const QString mesoSplit(m_modeldata.at(meso_idx).at(MESOCYCLES_COL_SPLIT));
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
	m_totalSplits.insert(meso_idx, nSplits);
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
		return QDate::fromJulianDay(static_cast<QString>(m_modeldata.constLast().at(MESOCYCLES_COL_ENDDATE)).toLongLong());
	return QDate::currentDate();
}

bool DBMesocyclesModel::isDateWithinMeso(const uint meso_idx, const QDate& date) const
{
	if (count() > 0)
	{
		if (date >= getDateFast(meso_idx, MESOCYCLES_COL_STARTDATE))
		{
			if (date <= getDateFast(meso_idx, MESOCYCLES_COL_ENDDATE))
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

void DBMesocyclesModel::updateColumnLabels()
{
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
		case APP_USE_MODE_COACH_USER_WITH_COACH:
			strClient = tr("Client: ");
			strCoach = tr("Coach/Trainer: ");
		break;
	}
	mColumnNames[MESOCYCLES_COL_COACH] = strCoach;
	mColumnNames[MESOCYCLES_COL_CLIENT] = strClient;
}

bool DBMesocyclesModel::importFromText(QFile* inFile, QString& inData)
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
		modeldata.append(STR_MINUS_ONE); //id
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
				modeldata.append(modeldata.at(MESOCYCLES_COL_ENDDATE).toInt() != 0 ? STR_ONE : STR_ZERO); //MESOCYCLES_COL_REALMESO
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

QString DBMesocyclesModel::formatFieldToExport(const uint field, const QString &fieldValue) const
{
	if (field == MESOCYCLES_COL_STARTDATE || field == MESOCYCLES_COL_ENDDATE)
		return appUtils()->formatDate(QDate::fromJulianDay(fieldValue.toInt()));
	else
		return QString();
}

QString DBMesocyclesModel::formatFieldToImport(const QString &fieldValue) const
{
	return QString::number(appUtils()->getDateFromStrDate(fieldValue).toJulianDay());
}
