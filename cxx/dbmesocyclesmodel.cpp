#include "dbmesocyclesmodel.h"
#include "dbmesocalendarmodel.h"
#include "tpglobals.h"
#include "dbmesosplitmodel.h"
#include "tpsettings.h"

#include <QSettings>
#include <utility>

DBMesocyclesModel* DBMesocyclesModel::app_meso_model(nullptr);

DBMesocyclesModel::DBMesocyclesModel(QObject* parent)
	: TPListModel{parent}, m_userModel(nullptr), m_mostRecentOwnMesoIdx(-1)
{
	if (!app_meso_model)
		app_meso_model = this;

	setObjectName(DBMesocyclesObjectName);
	m_tableId = MESOCYCLES_TABLE_ID;
	m_fieldCount = MESOCYCLES_TOTAL_COLS;
	m_exportName = std::move(tr("Training Plan"));

	//m_roleNames[mesoNameRole] = std::move("mesoName"); //TODO TODO
	m_roleNames[mesoNameRole] = std::move("mesoName");
	m_roleNames[mesoStartDateRole] = std::move("mesoStartDate");
	m_roleNames[mesoEndDateRole] = std::move("mesoEndDate");
	m_roleNames[mesoSplitRole] = std::move("mesoSplit");
	m_roleNames[mesoCoachRole] = std::move("mesoCoach");
	m_roleNames[mesoClientRole] = std::move("mesoClient");

	mColumnNames.reserve(MESOCYCLES_TOTAL_COLS);
	mColumnNames.append(QString()); //MESOCYCLES_COL_ID
	mColumnNames.append(std::move(tr("Plan's name: ")));
	mColumnNames.append(std::move(tr("Start date: ")));
	mColumnNames.append(std::move(tr("End date: ")));
	mColumnNames.append(std::move(tr("Plan's considerations: ")));
	mColumnNames.append(std::move(tr("Number of weeks: ")));
	mColumnNames.append(std::move(tr("Weekly Training Division: ")));
	mColumnNames.append(QString()); //MESOCYCLES_COL_COACH
	mColumnNames.append(QString()); //MESOCYCLES_COL_CLIENT
	mColumnNames.append(QString()); //MESOCYCLES_COL_FILE
	mColumnNames.append(std::move(tr("Type: ")));
	mColumnNames.append(std::move(tr("Mesocycle-style plan: ")));

	m_splitModel = new DBMesoSplitModel(this, false, -1);
}

DBMesocyclesModel::~DBMesocyclesModel()
{
	delete m_splitModel;
	for (uint i(0); i < m_calendarModelList.count(); ++i)
		delete m_calendarModelList[i];
}

void DBMesocyclesModel::setUserModel(DBUserModel* usermodel)
{
	m_userModel = usermodel;
	updateColumnLabels();
}

const uint DBMesocyclesModel::newMesocycle(const QStringList& infolist)
{
	appendList(infolist);
	m_splitModel->appendList(QStringList() << STR_MINUS_ONE << STR_MINUS_ONE << QString() << QString() <<
		QString() << QString() << QString() << QString());

	const uint meso_idx(count()-1);
	m_splitModel->setMesoIdx(meso_idx);
	m_calendarModelList.append(new DBMesoCalendarModel(this, meso_idx));
	m_newMesoCalendarChanged.append(false);
	if (isOwnMeso(meso_idx))
	{
		m_mostRecentOwnMesoIdx = meso_idx;
		emit mostRecentOwnMesoChanged(m_mostRecentOwnMesoIdx);
	}
	setCurrentMesoIdx(meso_idx);
	uchar newMesoRequiredFields(0);
	setBit(newMesoRequiredFields, MESOCYCLES_COL_NAME);
	setBit(newMesoRequiredFields, MESOCYCLES_COL_STARTDATE);
	setBit(newMesoRequiredFields, MESOCYCLES_COL_ENDDATE);
	setBit(newMesoRequiredFields, MESOCYCLES_COL_SPLIT);
	m_isNewMeso.append(newMesoRequiredFields);
	return meso_idx;
}

void DBMesocyclesModel::delMesocycle(const uint meso_idx)
{
	delete m_calendarModelList.at(meso_idx);
	m_calendarModelList.remove(meso_idx);
	m_isNewMeso.remove(meso_idx);

	m_splitModel->removeRow(meso_idx);
	removeRow(meso_idx);

	for (uint i(meso_idx); i < count(); ++i)
	{
		m_splitModel->setMesoIdx(i);
		m_calendarModelList[i]->setMesoIdx(i);
		emit mesoIdxChanged(i-1, i);
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
	m_currentMesoIdx = appSettings()->lastViewedMesoIdx();
}

void DBMesocyclesModel::setModified(const uint meso_idx, const uint field)
{
	if (isNewMeso(meso_idx))
	{
		unSetBit(m_isNewMeso[meso_idx], field);
		if (!isNewMeso(meso_idx))
			emit isNewMesoChanged(meso_idx);
	}
	emit mesoChanged(meso_idx, field);
}

QString DBMesocyclesModel::muscularGroup(const uint meso_idx, const QChar& splitLetter) const
{
	return m_splitModel->splitX(meso_idx, appUtils()->splitLetterToMesoSplitIndex(splitLetter));
}

QString DBMesocyclesModel::splitLetter(const uint meso_idx, const uint day_of_week) const
{
	return day_of_week <= 6 ? split(meso_idx).at(day_of_week) : QString();
}

void DBMesocyclesModel::setId(const uint meso_idx, const QString& new_id)
{
	m_modeldata[meso_idx][MESOCYCLES_COL_ID] = new_id;
	m_splitModel->m_modeldata[meso_idx][MESOSPLIT_COL_MESOID] = new_id;
}

void DBMesocyclesModel::setStartDate(const uint meso_idx, const QDate& new_date)
{
	m_modeldata[meso_idx][MESOCYCLES_COL_STARTDATE] = QString::number(new_date.toJulianDay());
	setModified(meso_idx, MESOCYCLES_COL_STARTDATE);
	if (!isNewMeso(meso_idx))
		emit mesoCalendarFieldsChanged(meso_idx);
	else
		m_newMesoCalendarChanged[meso_idx] = true;
}

void DBMesocyclesModel::setEndDate(const uint meso_idx, const QDate& new_date)
{
	m_modeldata[meso_idx][MESOCYCLES_COL_ENDDATE] = QString::number(new_date.toJulianDay());
	setModified(meso_idx, MESOCYCLES_COL_ENDDATE);
	if (!isNewMeso(meso_idx))
		emit mesoCalendarFieldsChanged(meso_idx);
	else
		m_newMesoCalendarChanged[meso_idx] = true;
}

void DBMesocyclesModel::setSplit(const uint meso_idx, const QString& new_split)
{
	m_modeldata[meso_idx][MESOCYCLES_COL_SPLIT] = new_split;
	setModified(meso_idx, MESOCYCLES_COL_SPLIT);
	if (isNewMeso(meso_idx))
		emit mesoCalendarFieldsChanged(meso_idx);
	else
		m_newMesoCalendarChanged[meso_idx] = true;
}

void DBMesocyclesModel::setOwnMeso(const uint meso_idx, const bool bOwnMeso)
{
	if (isOwnMeso(meso_idx) != bOwnMeso)
	{
		setClient(meso_idx, bOwnMeso ? m_userModel->userName(0) : m_userModel->getCurrentUserName(false));
		const int cur_ownmeso(m_mostRecentOwnMesoIdx);
		findNextOwnMeso();
		if (cur_ownmeso != m_mostRecentOwnMesoIdx)
			emit mostRecentOwnMesoChanged(m_mostRecentOwnMesoIdx);
		emit isOwnMesoChanged(meso_idx);
	}
}

void DBMesocyclesModel::setMuscularGroup(const uint meso_idx, const QChar& splitLetter, const QString& newSplitValue, const uint initiator_id)
{
	const uint splitField(appUtils()->splitLetterToMesoSplitIndex(splitLetter));
	if (splitField < SIMPLE_MESOSPLIT_TOTAL_COLS)
	{
		m_splitModel->m_modeldata[meso_idx][splitField] = newSplitValue;
		setModified(meso_idx, MESOCYCLES_COL_MUSCULARGROUP);
		emit muscularGroupChanged(meso_idx, initiator_id, splitField, splitLetter);
	}
}

void DBMesocyclesModel::setCurrentMesoIdx(const uint meso_idx)
{
	if (meso_idx != m_currentMesoIdx)
	{
		m_currentMesoIdx = meso_idx;
		appSettings()->setLastViewedMesoIdx(meso_idx);
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

QVariant DBMesocyclesModel::data(const QModelIndex &index, int role) const
{
	const int row(index.row());
	if( row >= 0 && row < m_modeldata.count() )
	{
		switch(role)
		{
			case mesoNameRole:
				if (m_userModel->userName(0) == client(row))
					return QVariant(u"<b>**  "_qs + name(row) + u"  **</b>"_qs);
				else
					return QVariant(u"<b>"_qs + name(row) + u"</b>"_qs);
			case mesoStartDateRole:
				return QVariant(mColumnNames[MESOCYCLES_COL_STARTDATE] + u"<b>"_qs +
					appUtils()->formatDate(startDate(row)) + u"</b>"_qs);
			case mesoEndDateRole:
				return QVariant(mColumnNames[MESOCYCLES_COL_ENDDATE] + u"<b>"_qs +
					appUtils()->formatDate(endDate(row)) + u"</b>"_qs);
			case mesoSplitRole:
				return QVariant(mColumnNames[MESOCYCLES_COL_SPLIT] + u"<b>"_qs + split(row) + u"</b>"_qs);
			case mesoCoachRole:
				if (!coach(row).isEmpty())
					return QVariant(mColumnNames[MESOCYCLES_COL_COACH] + u"<b>"_qs + coach(row) + u"</b>"_qs);
			case mesoClientRole:
				if (!client(row).isEmpty())
					return QVariant(mColumnNames[MESOCYCLES_COL_CLIENT] + u"<b>"_qs + client(row) + u"</b>"_qs);

		}
	}
	return QString();
}

bool DBMesocyclesModel::isDateWithinMeso(const int meso_idx, const QDate& date) const
{
	if (meso_idx >= 0 && count() > 0)
	{
		if (date >= startDate(meso_idx))
		{
			if (date <= endDate(meso_idx))
				return true;
		}
	}
	return false;
}

int DBMesocyclesModel::getPreviousMesoId(const int current_mesoid) const
{
	for(uint x(1); x < count(); ++x)
	{
		if (_id(x) == current_mesoid)
			return _id(x-1);
	}
	return -1;
}

QDate DBMesocyclesModel::getPreviousMesoEndDate(const int current_mesoid) const
{
	for(uint x(1); x < count(); ++x)
	{
		if (_id(x) == current_mesoid)
			return endDate(x-1);
	}
	// 1 or 0 meso records = no previous meso. The first meso can start anywhere in 2024
	return QDate(2024, 1, 1);
}

QDate DBMesocyclesModel::getNextMesoStartDate(const int mesoid) const
{
	for(uint x(0); x < count() - 1; ++x)
	{
		if (_id(x) == mesoid)
			return startDate(x+1);
	}
	 //This is the most current meso. The cut off date for it is undetermined. So we set a value that is 6 months away
	return QDate::currentDate().addMonths(6);
}

QDate DBMesocyclesModel::getLastMesoEndDate() const
{
	if (count() > 0)
		return endDate(count()-1);
	return QDate::currentDate();
}

//Called when importing from a text file
bool DBMesocyclesModel::isDifferent(const TPListModel* const model)
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

int DBMesocyclesModel::exportToFile(const QString& filename, const bool, const bool) const
{
	int res(this->TPListModel::exportToFile(filename, true, false));
	if (res >= 0)
	{
		m_splitModel->setExportRow(m_mesoIdx);
		res = m_splitModel->TPListModel::exportToFile(filename, false, true);
	}
	return res;
}

int DBMesocyclesModel::importFromFile(const QString& filename)
{
	QFile* inFile{new QFile(filename)};
	if (!inFile->open(QIODeviceBase::ReadOnly|QIODeviceBase::Text))
	{
		delete inFile;
		return  APPWINDOW_MSG_OPEN_FAILED;
	}

	QStringList modeldata(MESOCYCLES_TOTAL_COLS);
	modeldata[0] = STR_MINUS_ONE;
	QStringList splitmodeldata(SIMPLE_MESOSPLIT_TOTAL_COLS);
	splitmodeldata[MESOSPLIT_COL_ID] = STR_MINUS_ONE;
	splitmodeldata[MESOSPLIT_COL_MESOID] = STR_MINUS_ONE;

	uint col(2);
	QString value;
	char buf[512];
	qint64 lineLength(0);
	while ((lineLength = inFile->readLine(buf, sizeof(buf))) != -1)
	{
		if (strstr(buf, STR_END_EXPORT.toLatin1().constData()) == NULL)
		{
			if (lineLength > 10)
			{
				if (strstr(buf, "##") != NULL)
				{
					if (col < MESOCYCLES_TOTAL_COLS)
					{
						if (col != MESOCYCLES_COL_FILE)
						{
							value = buf;
							value.remove(0, value.indexOf(':') + 2);
							if (isFieldFormatSpecial(col))
								modeldata[col] = formatFieldToImport(col, value, buf);
							else
								modeldata[col] = value;
						}
					}
					else
					{
						if (col == SIMPLE_MESOSPLIT_TOTAL_COLS)
							break;
						value = buf;
						const int splitidx(static_cast<int>(value.at(value.indexOf(':')-1).cell()) - static_cast<int>('A') + 2);
						if (splitidx >= 2 && splitidx <= 7)
							splitmodeldata[splitidx] = value.remove(0, value.indexOf(':') + 2);
					}
					++col;
				}
			}
		}
		else
			break;
	}
	m_modeldata.append(modeldata);
	m_splitModel->m_modeldata.append(splitmodeldata);
	inFile->close();
	delete inFile;
	return col >= MESOCYCLES_COL_SPLIT ? APPWINDOW_MSG_READ_FROM_FILE_OK : APPWINDOW_MSG_UNKNOWN_FILE_FORMAT;
}

bool DBMesocyclesModel::updateFromModel(const uint meso_idx, TPListModel* model)
{
	setImportMode(true);
	for (uint i(MESOCYCLES_COL_ID); i < MESOCYCLES_TOTAL_COLS; ++i)
		m_modeldata[meso_idx][i] = std::move(model->m_modeldata.at(0).at(i));
	const DBMesoSplitModel* const splitModel(static_cast<const DBMesoSplitModel* const>(const_cast<TPListModel*>(model)));
	for (uint i(MESOSPLIT_COL_ID); i < SIMPLE_MESOSPLIT_TOTAL_COLS; ++i)
		m_splitModel->m_modeldata[meso_idx][i] = splitModel->m_modeldata.at(0).at(i);
	return true;
}

QString DBMesocyclesModel::formatFieldToExport(const uint field, const QString& fieldValue) const
{
	switch (field)
	{
		case MESOCYCLES_COL_STARTDATE:
		case MESOCYCLES_COL_ENDDATE:
			return appUtils()->formatDate(QDate::fromJulianDay(fieldValue.toInt()));
		case MESOCYCLES_COL_COACH:
		case MESOCYCLES_COL_CLIENT:
			return fieldValue;
		case MESOCYCLES_COL_REALMESO:
			return fieldValue == STR_ONE ? tr("Yes") : tr("No");
	}
	return QString(); //never reached
}

QString DBMesocyclesModel::formatFieldToImport(const uint field, const QString& fieldValue, const QString& fieldName) const
{
	switch (field)
	{
		case MESOCYCLES_COL_STARTDATE:
		case MESOCYCLES_COL_ENDDATE:
			return QString::number(appUtils()->getDateFromStrDate(fieldValue).toJulianDay());
		case MESOCYCLES_COL_COACH:
			return fieldName.contains(tr("Coach")) ? fieldValue : QString();
		case MESOCYCLES_COL_CLIENT:
			return fieldName.contains(tr("Client")) ? fieldValue : QString();
		case MESOCYCLES_COL_REALMESO:
			return fieldValue == tr("Yes") ? STR_ONE : STR_ZERO;
	}
	return QString(); //never reached
}
