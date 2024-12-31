#include "qmlmesosplitinterface.h"

#include "dbexercisesmodel.h"
#include "dbinterface.h"
#include "dbmesocyclesmodel.h"
#include "dbmesosplitmodel.h"
#include "qmlitemmanager.h"
#include "osinterface.h"
#include "tputils.h"

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>

QmlMesoSplitInterface::~QmlMesoSplitInterface()
{
	if (m_plannerComponent)
	{
		QMapIterator<QChar,QQuickItem*> t(m_splitPages);
		t.toFront();
		while (t.hasNext()) {
			t.next();
			emit removePageFromMainMenu(t.value());
			delete t.value();
		}
		delete m_splitComponent;

		QMapIterator<QChar,DBMesoSplitModel*> z(m_splitModels);
		z.toFront();
		while (z.hasNext()) {
			z.next();
			delete z.value();
		}

		delete m_plannerPage;
		delete m_plannerComponent;
	}
}

void QmlMesoSplitInterface::setMesoIdx(const uint new_meso_idx)
{
	m_mesoIdx = new_meso_idx;
	QMap<QChar,DBMesoSplitModel*>::const_iterator itr(m_splitModels.constBegin());
	const QMap<QChar,DBMesoSplitModel*>::const_iterator& itr_end(m_splitModels.constEnd());
	while (itr != itr_end)
	{
		(*itr)->setMesoIdx(new_meso_idx);
		++itr;
	}
}

void QmlMesoSplitInterface::getExercisesPlannerPage()
{
	if (!m_plannerComponent)
	{
		connect(appDBInterface(), &DBInterface::databaseReadyWithData, this, [this] (const uint table_id, QVariant data) {
			if (table_id == MESOSPLIT_TABLE_ID)
			{
				connect(this, &QmlMesoSplitInterface::plannerPageCreated, this, [this,data] () {
					QMap<QChar,DBMesoSplitModel*> allSplits(std::move(data.value<QMap<QChar,DBMesoSplitModel*>>()));
					QMap<QChar,DBMesoSplitModel*>::const_iterator mapModel(allSplits.constBegin());
					const QMap<QChar,DBMesoSplitModel*>::const_iterator mapEnd(allSplits.constEnd());
					QChar splitletter;
					const QString& mesoSplit{appMesoModel()->split(m_mesoIdx)};
					do {
						splitletter = (*mapModel)->_splitLetter();
						if (mesoSplit.contains(splitletter))
						{
							m_splitModels.insert(splitletter, std::move(*mapModel));
							createMesoSplitPage(splitletter);
						}
					} while (++mapModel != mapEnd);
				}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
				createPlannerPage();
			}
		});
		appDBInterface()->loadAllSplits(m_mesoIdx);
	}
	else
		emit addPageToMainMenu(tr("Exercises Planner: ") + appMesoModel()->name(m_mesoIdx), m_plannerPage);
}

void QmlMesoSplitInterface::changeMuscularGroup(const QString& new_musculargroup, DBMesoSplitModel* splitModel)
{
	splitModel->setMuscularGroup(new_musculargroup);
	appMesoModel()->setMuscularGroup(m_mesoIdx, splitModel->_splitLetter(), new_musculargroup, false);
	setSplitPageProperties(m_splitPages.value(splitModel->_splitLetter()), splitModel);
}

void QmlMesoSplitInterface::moveRow(const uint from, const uint to, DBMesoSplitModel* splitModel)
{
	splitModel->moveRow(from, to);
	emit splitModel->modelChanged(); //Update the view on QML
	emit splitModel->splitChanged(0, 100); //Save the data
}

void QmlMesoSplitInterface::swapMesoPlans(const QString& splitLetter1, const QString& splitLetter2)
{
	m_splitPages.value(splitLetter1.at(0))->setProperty("splitLetter", splitLetter2);
	m_splitPages.value(splitLetter2.at(0))->setProperty("splitLetter", splitLetter1);
	DBMesoSplitModel* tempSplit(m_splitModels.value(splitLetter1.at(0)));
	m_splitModels[splitLetter1.at(0)] = m_splitModels.value(splitLetter2.at(0));
	appDBInterface()->saveMesoSplitComplete(m_splitModels.value(splitLetter1.at(0)));
	m_splitModels[splitLetter2.at(0)] = tempSplit;
	appDBInterface()->saveMesoSplitComplete(m_splitModels.value(splitLetter1.at(0)));
}

void QmlMesoSplitInterface::loadSplitFromPreviousMeso(DBMesoSplitModel* splitModel)
{
	const int prevMesoId(getSplitPage(splitModel->_splitLetter())->property("prevMesoId").toInt());
	if (prevMesoId >= 0)
		appDBInterface()->loadSplitFromPreviousMeso(prevMesoId, splitModel);
}

void QmlMesoSplitInterface::simpleExercisesList(DBMesoSplitModel* splitModel, const bool show, const bool multi_sel, const uint exercise_idx)
{
	m_simpleExercisesListRequester = splitModel;
	m_simpleExercisesListExerciseIdx = exercise_idx;
	if (show)
	{
		if (appExercisesModel()->count() == 0)
			appDBInterface()->getAllExercises();
		connect(m_plannerPage, SIGNAL(exerciseSelectedFromSimpleExercisesList()), this, SLOT(exerciseSelected()));
		connect(m_plannerPage, SIGNAL(simpleExercisesListClosed()), this, SLOT(hideSimpleExercisesList()));
		QMetaObject::invokeMethod(m_plannerPage, "showSimpleExercisesList", Q_ARG(bool, multi_sel));
	}
	else
	{
		disconnect(m_plannerPage, SIGNAL(exerciseSelectedFromSimpleExercisesList()), this, SLOT(exerciseSelected()));
		disconnect(m_plannerPage, SIGNAL(simpleExercisesListClosed()), this, SLOT(hideSimpleExercisesList()));
		QMetaObject::invokeMethod(m_plannerPage, "hideSimpleExercisesList");
	}
}

void QmlMesoSplitInterface::exportMesoSplit(const bool bShare, const QString& splitLetter)
{
	int exportFileMessageId(0);
	QString mesoSplit, suggestedName;
	if (splitLetter == "X"_L1)
	{
		mesoSplit = appMesoModel()->split(m_mesoIdx);
		suggestedName = appMesoModel()->name(m_mesoIdx) + std::move(tr(" - Exercises Plan.txt"));
	}
	else
	{
		mesoSplit = splitLetter;
		suggestedName = appMesoModel()->name(m_mesoIdx) + std::move(tr(" - Exercises Plan - Split ")) + splitLetter + ".txt"_L1;
	}
	const QString& exportFileName{appItemManager()->setExportFileName(suggestedName)};

	QString mesoLetters;
	QString::const_iterator itr(mesoSplit.constBegin());
	const QString::const_iterator& itr_end(mesoSplit.constEnd());
	do {
		if (*itr == QChar('R'))
			continue;
		if (mesoLetters.contains(*itr))
			continue;
		mesoLetters.append(*itr);
		exportFileMessageId = m_splitModels.value(*itr)->exportToFile(exportFileName);
	} while (++itr != itr_end);
	appItemManager()->continueExport(exportFileMessageId, bShare);
}

void QmlMesoSplitInterface::importMesoSplit(const QString& filename)
{
	if (filename.isEmpty())
	{
		appMesoModel()->setImportIdx(m_mesoIdx);
		QMetaObject::invokeMethod(appMainWindow(), "chooseFileToImport", Q_ARG(int, IFC_MESOSPLIT));
	}
	else
		appItemManager()->openRequestedFile(filename, IFC_MESOSPLIT);
}

void QmlMesoSplitInterface::exerciseSelected()
{
	const uint row(m_simpleExercisesListRequester->currentRow());
	const uint nsets(m_simpleExercisesListRequester->setsNumber(row));

	QString exerciseName, nReps, nWeight;
	const bool b_is_composite(appExercisesModel()->selectedEntriesCount() > 1);

	exerciseName = appExercisesModel()->selectedEntriesValue_fast(0, EXERCISES_COL_MAINNAME) + " - "_L1 +
					appExercisesModel()->selectedEntriesValue_fast(0, 2);

	if (!b_is_composite)
	{
		nReps = std::move(appUtils()->makeCompositeValue(appExercisesModel()->selectedEntriesValue_fast(0, EXERCISES_COL_REPSNUMBER),
							nsets, set_separator));
		nWeight = std::move(appUtils()->makeCompositeValue(appExercisesModel()->selectedEntriesValue_fast(0, EXERCISES_COL_WEIGHT),
							nsets, set_separator));
	}
	else
	{
		nReps = std::move(appUtils()->makeDoubleCompositeValue(appExercisesModel()->selectedEntriesValue_fast(0, EXERCISES_COL_REPSNUMBER),
							nsets, 2, set_separator, comp_exercise_separator));
		nWeight = std::move(appUtils()->makeDoubleCompositeValue(appExercisesModel()->selectedEntriesValue_fast(0, EXERCISES_COL_WEIGHT),
							nsets, 2, set_separator, comp_exercise_separator));
		appUtils()->setCompositeValue(1, appExercisesModel()->selectedEntriesValue_fast(1, EXERCISES_COL_MAINNAME) + " - "_L1 +
						appExercisesModel()->selectedEntriesValue_fast(1, 2), exerciseName, comp_exercise_separator);
		appUtils()->setCompositeValue(1, appUtils()->makeCompositeValue(
										appExercisesModel()->selectedEntriesValue_fast(1, EXERCISES_COL_REPSNUMBER), nsets, set_separator),
										nReps, comp_exercise_separator);
		appUtils()->setCompositeValue(1, appUtils()->makeCompositeValue(
										appExercisesModel()->selectedEntriesValue_fast(1, EXERCISES_COL_WEIGHT), nsets, set_separator),
										nWeight, comp_exercise_separator);
	}

	switch (m_simpleExercisesListExerciseIdx)
	{
		case 0:
		{
			m_simpleExercisesListRequester->setExerciseName(row, exerciseName);
			const uint set_number(m_simpleExercisesListRequester->workingSet(row));
			if (!m_simpleExercisesListRequester->isFieldUserModified(row, MESOSPLIT_COL_SETTYPE))
			{
				const int cur_set_type(m_simpleExercisesListRequester->setType(row, set_number));
				if (b_is_composite && cur_set_type != SET_TYPE_GIANT)
					m_simpleExercisesListRequester->setSetType(row, set_number, SET_TYPE_GIANT, false);
				else if (!b_is_composite && cur_set_type == SET_TYPE_GIANT)
					m_simpleExercisesListRequester->setSetType(row, set_number, SET_TYPE_REGULAR, false);
			}
			if (!nReps.isEmpty())
			{
				if (!m_simpleExercisesListRequester->isFieldUserModified(row, MESOSPLIT_COL_REPSNUMBER))
					m_simpleExercisesListRequester->_setSetsReps(row, nReps);
			}
			if (!nWeight.isEmpty())
			{
				if (!m_simpleExercisesListRequester->isFieldUserModified(row, MESOSPLIT_COL_WEIGHT))
					m_simpleExercisesListRequester->_setSetsWeights(row, nWeight);
			}
		}
		break;
		case 1:
			m_simpleExercisesListRequester->setExerciseName1(row, exerciseName);
		break;
		case 2:
			m_simpleExercisesListRequester->setExerciseName2(row, exerciseName);
	}
}

void QmlMesoSplitInterface::hideSimpleExercisesList()
{
	simpleExercisesList(nullptr, false);
}

void QmlMesoSplitInterface::createPlannerPage()
{
	m_plannerComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/qml/Pages/ExercisesPlanner.qml"_L1}, QQmlComponent::Asynchronous};
	m_plannerProperties["splitManager"_L1] = QVariant::fromValue(this);
	if (m_plannerComponent->status() != QQmlComponent::Ready)
		connect(m_plannerComponent, &QQmlComponent::statusChanged, this, [this](QQmlComponent::Status status) {
			if (status == QQmlComponent::Ready)
				QmlMesoSplitInterface::createPlannerPage_part2();
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	else
		createPlannerPage_part2();
}

void QmlMesoSplitInterface::createPlannerPage_part2()
{
	m_plannerPage = static_cast<QQuickItem*>(m_plannerComponent->createWithInitialProperties(m_plannerProperties, appQmlEngine()->rootContext()));
	if (m_plannerComponent->status() != QQmlComponent::Ready)
	{
		connect(m_plannerComponent, &QQmlComponent::statusChanged, this, [this](QQmlComponent::Status status) {
			if (status == QQmlComponent::Ready)
				QmlMesoSplitInterface::createPlannerPage_part2();
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		return;
	}

	#ifndef QT_NO_DEBUG
	if (m_plannerComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_plannerComponent->errorString();
		for (uint i(0); i < m_plannerComponent->errors().count(); ++i)
			qDebug() << m_plannerComponent->errors().at(i).description();
		return;
	}
	#endif
	appQmlEngine()->setObjectOwnership(m_plannerPage, QQmlEngine::CppOwnership);
	m_plannerPage->setParentItem(appMainWindow()->findChild<QQuickItem*>("appStackView"));
	emit plannerPageCreated();
	QMetaObject::invokeMethod(m_plannerPage, "createNavButtons");

	connect(this, &QmlMesoSplitInterface::addPageToMainMenu, appItemManager(), &QmlItemManager::addMainMenuShortCut);
	connect(this, &QmlMesoSplitInterface::removePageFromMainMenu, appItemManager(), &QmlItemManager::removeMainMenuShortCut);
	emit addPageToMainMenu(tr("Exercises Planner: ") + appMesoModel()->name(m_mesoIdx), m_plannerPage);
}

void QmlMesoSplitInterface::createMesoSplitPage(const QChar& splitletter)
{
	if (m_splitComponent == nullptr)
		m_splitComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/qml/Pages/MesoSplitPlanner.qml"_L1}, QQmlComponent::Asynchronous};

	if (m_splitComponent->status() == QQmlComponent::Ready)
		createMesoSplitPage_part2(splitletter);
	else
	{
		connect(m_splitComponent, &QQmlComponent::statusChanged, this, [this,splitletter] (QQmlComponent::Status) {
			createMesoSplitPage_part2(splitletter);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	}
}

void QmlMesoSplitInterface::createMesoSplitPage_part2(const QChar& splitletter)
{
	#ifndef QT_NO_DEBUG
	if (m_splitComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_splitComponent->errorString();
		for (uint i(0); i < m_splitComponent->errors().count(); ++i)
			qDebug() << m_splitComponent->errors().at(i).description();
		return;
	}
	#endif

	DBMesoSplitModel* splitmodel(m_splitModels.value(splitletter));

	m_splitProperties["splitModel"_L1] = QVariant::fromValue(splitmodel);
	m_splitProperties["parentItem"_L1] = QVariant::fromValue(m_plannerPage);
	m_splitProperties["splitManager"_L1] = QVariant::fromValue(this);

	QQuickItem* item (static_cast<QQuickItem*>(m_splitComponent->createWithInitialProperties(m_splitProperties, appQmlEngine()->rootContext())));
	appQmlEngine()->setObjectOwnership(item, QQmlEngine::CppOwnership);
	item->setParentItem(m_plannerPage);

	if (splitmodel->count() > 0)
		splitmodel->setCurrentRow(0);
	setSplitPageProperties(item, splitmodel);

	m_splitPages.insert(splitmodel->splitLetter().at(0), item);
	QMetaObject::invokeMethod(m_plannerPage, "insertSplitPage", Q_ARG(QQuickItem*, item),
								Q_ARG(int, appUtils()->splitLetterToIndex(splitmodel->splitLetter())));

	connect(appMesoModel(), &DBMesocyclesModel::muscularGroupChanged, this, [this] (const uint meso_idx, const int splitIndex, const QChar& splitLetter) {
		if (meso_idx == m_mesoIdx)
		{
			if (splitIndex < m_splitModels.count())
				updateMuscularGroup(m_splitModels.value(splitLetter));
		}
	});
	connect(splitmodel, &DBMesoSplitModel::splitChanged, this, [this,splitmodel] (const uint row, const uint) {
		if (row == 100)
			appDBInterface()->deleteMesoSplitTable(splitmodel);
		else
			appDBInterface()->saveMesoSplitComplete(splitmodel);
	});
}

void QmlMesoSplitInterface::setSplitPageProperties(QQuickItem* splitPage, const DBMesoSplitModel* const splitModel)
{
	int prevMesoId{appMesoModel()->getPreviousMesoId(appMesoModel()->client(m_mesoIdx), appMesoModel()->_id(m_mesoIdx))};
	if (prevMesoId >= 0)
	{
		if (appDBInterface()->mesoHasPlan(prevMesoId, splitModel->splitLetter()))
			splitPage->setProperty("prevMesoName", appMesoModel()->name(prevMesoId));
		else
			prevMesoId = -1; //Nothing from previous meso to import
	}
	const QString& swappableLetter(splitModel->findSwappableModel());
	splitPage->setProperty("prevMesoId", prevMesoId);
	splitPage->setProperty("swappableLetter", swappableLetter);
	splitPage->setProperty("bCanSwapPlan", !swappableLetter.isEmpty());
}

//Updates MesoSplitPlanner(and its corresponding models) with the changes originating in MesocyclePage.qml
void QmlMesoSplitInterface::updateMuscularGroup(DBMesoSplitModel* splitModel)
{
	const QString& musculargroup{appMesoModel()->muscularGroup(m_mesoIdx, splitModel->_splitLetter())};
	splitModel->setMuscularGroup(musculargroup);
	QQuickItem* splitPage(getSplitPage(splitModel->_splitLetter()));
	if (splitPage)
	{
		setSplitPageProperties(splitPage, splitModel);
		QMetaObject::invokeMethod(splitPage, "updateTxtGroups", Q_ARG(QString, musculargroup));
	}
}
