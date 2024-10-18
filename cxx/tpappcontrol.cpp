#include "tpappcontrol.h"
#include "tpglobals.h"
#include "tputils.h"
#include "dbinterface.h"
#include "dbusermodel.h"
#include "dbmesocyclesmodel.h"
#include "dbexercisesmodel.h"
#include "dbmesosplitmodel.h"
#include "dbtrainingdaymodel.h"
#include "qmlitemmanager.h"
#include "osinterface.h"

#include <QGuiApplication>
#include <QSettings>

TPAppControl* TPAppControl::app_control(nullptr);

void TPAppControl::init(QQmlApplicationEngine* qml_engine)
{
	appDBInterface()->init();
	createItemManager();
	QmlManager()->configureQmlEngine(qml_engine);

#ifdef Q_OS_ANDROID
	appOsInterface()->appStartUpNotifications();
#endif
}

void TPAppControl::getMesocyclePage(const uint meso_idx)
{
	m_itemManager.at(meso_idx)->getMesocyclePage();
}

uint TPAppControl::createNewMesocycle(const bool bCreatePage)
{
	QDate startDate, endDate, minimumStartDate;
	if (appMesoModel()->count() == 0)
	{
		minimumStartDate.setDate(2023, 1, 2); //first monday of that year
		startDate = QDate::currentDate();
		endDate = appUtils()->createFutureDate(startDate, 0, 2, 0);
	}
	else
	{
		if (appMesoModel()->isRealMeso(appMesoModel()->count() - 1))
			minimumStartDate = appUtils()->getMesoStartDate(appMesoModel()->getLastMesoEndDate());
		else
			minimumStartDate = QDate::currentDate();
		startDate = minimumStartDate;
		endDate = appUtils()->createFutureDate(minimumStartDate, 0, 2, 0);
	}

	const uint meso_idx = appMesoModel()->newMesocycle(QStringList() << STR_MINUS_ONE << tr("New Plan") << QString::number(startDate.toJulianDay()) <<
		QString::number(endDate.toJulianDay()) << QString() << QString::number(appUtils()->calculateNumberOfWeeks(startDate, endDate)) <<
		u"ABCDERR"_qs << appUserModel()->currentCoachName(0) << appUserModel()->currentUserName(0) << QString() << QString() << STR_ONE);

	QmlItemManager* itemMngr{new QmlItemManager{meso_idx}};
	m_itemManager.append(itemMngr);

	if (bCreatePage)
		itemMngr->createMesocyclePage(minimumStartDate, appUtils()->createFutureDate(startDate,0,6,0), startDate);

	return meso_idx;
}

void TPAppControl::removeMesocycle(const uint meso_idx)
{
	appDBInterface()->removeMesocycle(meso_idx);
	appMesoModel()->delMesocycle(meso_idx);
	QmlItemManager* itemMngr{m_itemManager.at(meso_idx)};
	itemMngr->disconnect();
	delete itemMngr;
	m_itemManager.remove(meso_idx);
}

void TPAppControl::exportMeso(const uint meso_idx, const bool bShare, const bool bCoachInfo)
{
	m_itemManager.at(meso_idx)->exportMeso(bShare, bCoachInfo);
}

void TPAppControl::getExercisesPlannerPage(const uint meso_idx)
{
	m_itemManager.at(meso_idx)->getExercisesPlannerPage();
}

void TPAppControl::getMesoCalendarPage(const uint meso_idx)
{
	m_itemManager.at(meso_idx)->getMesoCalendarPage();
}

void TPAppControl::getTrainingDayPage(const uint meso_idx, const QDate& date)
{
	m_itemManager.at(meso_idx)->getTrainingDayPage(date);
}

void TPAppControl::openRequestedFile(const QString& filename, const int wanted_content)
{
	QFile* inFile{new QFile(filename)};
	if (!inFile->open(QIODeviceBase::ReadOnly|QIODeviceBase::Text))
	{
		delete inFile;
		return;
	}

	uint fileContents(0);
	qint64 lineLength(0);
	char buf[128];
	QString inData;

	while ((lineLength = inFile->readLine(buf, sizeof(buf))) != -1)
	{
		if (lineLength > 10)
		{
			if (strstr(buf, "##") != NULL)
			{
				inData = buf;
				if (inData.startsWith(u"##"_qs))
				{
					if (inData.indexOf(DBUserObjectName) != -1)
						fileContents |= IFC_USER;
					if (inData.indexOf(DBMesoSplitObjectName) != -1)
						fileContents |= IFC_MESOSPLIT;
					else if (inData.indexOf(DBMesocyclesObjectName) != -1)
						fileContents |= IFC_MESO;
					else if (inData.indexOf(DBTrainingDayObjectName) != -1)
						fileContents |= IFC_TDAY;
					else if (inData.indexOf(DBExercisesObjectName) != -1)
						fileContents |= IFC_EXERCISES;
				}
			}
		}
	}
	if (fileContents != 0)
	{
		QmlItemManager* itemMngr(nullptr);
		if (fileContents & IFC_MESO & wanted_content)
		{
			const uint tempmeso_idx{createNewMesocycle(false)};
			itemMngr = m_itemManager.at(tempmeso_idx);
		}
		else
		{
			if (fileContents & IFC_MESO & wanted_content || fileContents & IFC_TDAY & wanted_content)
				itemMngr = m_itemManager.at(appMesoModel()->mostRecentOwnMesoIdx());
			else if (fileContents & IFC_EXERCISES & wanted_content)
				itemMngr = QmlManager();
		}
		if (itemMngr)
			itemMngr->displayImportDialogMessage(fileContents, filename);
		else
			QmlManager()->displayMessageOnAppWindow(APPWINDOW_MSG_WRONG_IMPORT_FILE_TYPE);
	}
}

void TPAppControl::importFromFile(const QString& filename, const int wanted_content)
{
	int importFileMessageId(0);
	if (wanted_content & IFC_MESO)
	{
		if (wanted_content & IFC_USER)
		{
			DBUserModel* usermodel{new DBUserModel};
			usermodel->deleteLater();
			importFileMessageId = usermodel->importFromFile(filename);
			if (importFileMessageId >= 0)
				incorporateImportedData(usermodel);
		}

		DBMesocyclesModel* mesomodel{new DBMesocyclesModel};
		mesomodel->deleteLater();
		importFileMessageId = mesomodel->importFromFile(filename);
		if (importFileMessageId >= 0)
			incorporateImportedData(mesomodel);

		if (wanted_content & IFC_MESOSPLIT)
		{
			DBMesoSplitModel* splitModel{new DBMesoSplitModel};
			splitModel->deleteLater();
			importFileMessageId = splitModel->importFromFile(filename);
			if (importFileMessageId >= 0)
				incorporateImportedData(splitModel);
		}
	}
	else
	{
		if (wanted_content & IFC_MESOSPLIT)
		{
			DBMesoSplitModel* splitModel{new DBMesoSplitModel};
			splitModel->deleteLater();
			importFileMessageId = splitModel->importFromFile(filename);
			if (importFileMessageId >= 0)
				incorporateImportedData(splitModel);
		}
		else if (wanted_content & IFC_TDAY)
		{
			DBTrainingDayModel* tDayModel{new DBTrainingDayModel};
			tDayModel->deleteLater();
			importFileMessageId = tDayModel->importFromFile(filename);
			if (importFileMessageId >= 0)
				incorporateImportedData(tDayModel);
		}
		else if (wanted_content & IFC_EXERCISES)
		{
			DBExercisesModel* exercisesModel{new DBExercisesModel};
			exercisesModel->deleteLater();
			importFileMessageId = exercisesModel->importFromFile(filename);
			if (importFileMessageId >= 0)
				incorporateImportedData(exercisesModel);
		}
	}
	QmlManager()->displayMessageOnAppWindow(importFileMessageId);
}

void TPAppControl::incorporateImportedData(const TPListModel* const model)
{
	switch (model->tableID())
	{
		case EXERCISES_TABLE_ID:
			appExercisesModel()->updateFromModel(model);
			//appDBInterface()->saveExercise();
		break;
		case USER_TABLE_ID:
			static_cast<void>(appUserModel()->updateFromModel(model));
			appDBInterface()->saveUser(appUserModel()->count()-1);
		break;
		case MESOCYCLES_TABLE_ID:
			if (appMesoModel()->isDifferent(model))
			{
				const uint meso_idx = createNewMesocycle(false);
				appMesoModel()->updateFromModel(meso_idx, model);
				appDBInterface()->saveMesocycle(meso_idx);
			}
		break;
		case MESOSPLIT_TABLE_ID:
		{
			DBMesoSplitModel* newSplitModel{static_cast<DBMesoSplitModel*>(const_cast<TPListModel*>(model))};
			DBMesoSplitModel* splitModel{m_itemManager.at(appMesoModel()->mostRecentOwnMesoIdx())->getSplitModel(newSplitModel->splitLetter().at(0))};
			splitModel->updateFromModel(newSplitModel);
			appDBInterface()->saveMesoSplitComplete(splitModel);
		}
		break;
		case TRAININGDAY_TABLE_ID:
		{
			DBTrainingDayModel* newTDayModel{static_cast<DBTrainingDayModel*>(const_cast<TPListModel*>(model))};
			DBTrainingDayModel* tDayModel{m_itemManager.at(appMesoModel()->mostRecentOwnMesoIdx())->gettDayModel(QDate::currentDate())};
			if (tDayModel->exerciseCount() == 0)
			{
				tDayModel->updateFromModel(newTDayModel);
				appDBInterface()->saveTrainingDay(tDayModel);
			}
			else
				; //Offer option to import into another day
		}
		break;
	}
}

void TPAppControl::createItemManager()
{
	const uint n_mesos(appMesoModel()->count());
	m_itemManager.reserve(n_mesos);
	for (uint i(0); i < n_mesos; ++i)
		m_itemManager.append(new QmlItemManager{i});
}
