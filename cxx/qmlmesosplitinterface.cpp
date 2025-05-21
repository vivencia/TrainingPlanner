#include "qmlmesosplitinterface.h"

#include "dbexerciseslistmodel.h"
#include "dbexercisesmodel.h"
#include "dbinterface.h"
#include "dbmesocyclesmodel.h"
#include "qmlitemmanager.h"
#include "tputils.h"

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>

QmlMesoSplitInterface::~QmlMesoSplitInterface()
{
	if (m_plannerComponent)
	{
		for (const auto split_page : m_splitPages)
		{
			emit removePageFromMainMenu(split_page);
			delete split_page;
		}
		delete m_splitComponent;
		delete m_plannerPage;
		delete m_plannerComponent;
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
					QMap<QChar,DBExercisesModel*> allSplits(std::move(data.value<QMap<QChar,DBExercisesModel*>>()));
					QMap<QChar,DBExercisesModel*>::const_iterator mapModel(allSplits.constBegin());
					const QMap<QChar,DBExercisesModel*>::const_iterator mapEnd(allSplits.constEnd());
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

void QmlMesoSplitInterface::moveRow(const uint from, const uint to, DBExercisesModel* splitModel)
{
	splitModel->moveExercise(from, to);

	emit splitModel->splitChanged(0, 100); //Save the data
}

void QmlMesoSplitInterface::removeRow()
{
	const int cur_row{currentSplitModel()->currentRow()};
	QMetaObject::invokeMethod(m_plannerPage, "showDeleteDialog", Q_ARG(int, cur_row),
															Q_ARG(QString, currentSplitModel()->exerciseName(cur_row)));
}

void QmlMesoSplitInterface::swapMesoPlans()
{
	DBExercisesModel* tempSplit{m_splitModels.value(currentSplitLetter().at(0))};
	m_splitModels[currentSplitLetter().at(0)] = m_splitModels.value(currentSwappableLetter().at(0));
	appDBInterface()->saveMesoSplitComplete(currentSplitModel());
	m_splitModels[currentSwappableLetter().at(0)] = tempSplit;
	appDBInterface()->saveMesoSplitComplete(m_splitModels.value(currentSplitLetter().at(0)));
}

void QmlMesoSplitInterface::loadSplitFromPreviousMeso()
{
	if (m_prevMesoId >= 0)
		appDBInterface()->loadSplitFromPreviousMeso(m_prevMesoId, currentSplitModel());
}

void QmlMesoSplitInterface::simpleExercisesList(DBExercisesModel* splitModel, const bool show, const bool multi_sel, const uint exercise_idx)
{
	m_simpleExercisesListRequester = splitModel;
	m_simpleExercisesListExerciseIdx = exercise_idx;
	if (show)
	{
		if (appExercisesList()->count() == 0)
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

static void muscularGroupSimplified(QString &muscularGroup)
{
	muscularGroup = muscularGroup.replace(',', ' ').simplified();
	const QStringList &words(muscularGroup.split(' '));

	if (words.count() > 0)
	{
		QStringList::const_iterator itr(words.begin());
		const QStringList::const_iterator &itr_end(words.end());
		muscularGroup.clear();

		do
		{
			if((*itr).length() < 3)
				continue;
			if (!muscularGroup.isEmpty())
				muscularGroup.append(' ');
			muscularGroup.append((*itr).toLower());
			if (muscularGroup.endsWith('s', Qt::CaseInsensitive))
				muscularGroup.chop(1);
			muscularGroup.remove('.');
			muscularGroup.remove('(');
			muscularGroup.remove(')');
		} while (++itr != itr_end);
	}
}

QString QmlMesoSplitInterface::findSwappableModel() const
{
	QString muscularGroup1{std::move(appMesoModel()->muscularGroup(mesoIdx(), currentSplitLetter().at(0)))};
	if (!muscularGroup1.isEmpty())
	{
		muscularGroupSimplified(muscularGroup1);
		QString muscularGroup2;
		const QString &mesoSplit{appMesoModel()->split(mesoIdx())};
		QString::const_iterator itr{mesoSplit.constBegin()};
		const QString::const_iterator &itr_end{mesoSplit.constEnd()};

		do {
			if ((*itr) == QChar('R'))
				continue;
			else if ((*itr) == currentSplitLetter().at(0))
				continue;

			muscularGroup2 = appMesoModel()->muscularGroup(mesoIdx(), *itr);
			if (!muscularGroup2.isEmpty())
			{
				muscularGroupSimplified(muscularGroup2);
				if (appUtils()->stringsAreSimiliar(muscularGroup1, muscularGroup2))
					return static_cast<QString>(*itr);
			}
		} while (++itr != itr_end);
	}
	return QString{};
}

QQuickItem* QmlMesoSplitInterface::setCurrentPage(const int index)
{
	m_currentSplitPage = m_splitPages.value(QChar(static_cast<int>('A') + index));
	m_currentSplitLetter = std::move(m_splitPages.key(m_currentSplitPage));
	m_bHasExercises = currentSplitModel()->count() > 1;
	m_currentSwappableLetter = std::move(findSwappableModel());
	emit currentPageChanged();
	return m_currentSplitPage;
}

DBExercisesModel* QmlMesoSplitInterface::currentSplitModel() const
{
	return appMesoModel()->splitModel(m_mesoIdx, m_currentSplitLetter);
}

void QmlMesoSplitInterface::exerciseSelected()
{
	const int row{m_simpleExercisesListRequester->currentRow()};
	const uint nsets{m_simpleExercisesListRequester->setsNumber(row)};

	QString exerciseName, nReps, nWeight;
	const bool b_is_composite(appExercisesList()->selectedEntriesCount() > 1);

	exerciseName = std::move(appExercisesList()->selectedEntriesValue_fast(0, EXERCISES_LIST_COL_MAINNAME) + " - "_L1 +
					appExercisesList()->selectedEntriesValue_fast(0, 2));

	if (!b_is_composite)
	{
		nReps = std::move(appUtils()->makeCompositeValue(appExercisesList()->selectedEntriesValue_fast(0, EXERCISES_LIST_COL_REPSNUMBER),
							nsets, set_separator));
		nWeight = std::move(appUtils()->makeCompositeValue(appExercisesList()->selectedEntriesValue_fast(0, EXERCISES_LIST_COL_WEIGHT),
							nsets, set_separator));
	}
	else
	{
		nReps = std::move(appUtils()->makeDoubleCompositeValue(appExercisesList()->selectedEntriesValue_fast(0, EXERCISES_LIST_COL_REPSNUMBER),
							nsets, 2, set_separator, comp_exercise_separator));
		nWeight = std::move(appUtils()->makeDoubleCompositeValue(appExercisesList()->selectedEntriesValue_fast(0, EXERCISES_LIST_COL_WEIGHT),
							nsets, 2, set_separator, comp_exercise_separator));
		appUtils()->setCompositeValue(1, appExercisesList()->selectedEntriesValue_fast(1, EXERCISES_LIST_COL_MAINNAME) + " - "_L1 +
						appExercisesList()->selectedEntriesValue_fast(1, 2), exerciseName, comp_exercise_separator);
		appUtils()->setCompositeValue(1, appUtils()->makeCompositeValue(
										appExercisesList()->selectedEntriesValue_fast(1, EXERCISES_LIST_COL_REPSNUMBER), nsets, set_separator),
										nReps, comp_exercise_separator);
		appUtils()->setCompositeValue(1, appUtils()->makeCompositeValue(
										appExercisesList()->selectedEntriesValue_fast(1, EXERCISES_LIST_COL_WEIGHT), nsets, set_separator),
										nWeight, comp_exercise_separator);
	}

	switch (m_simpleExercisesListExerciseIdx)
	{
		case 0:
		{
			m_simpleExercisesListRequester->setExerciseName(row, exerciseName);
			const uint set_number{m_simpleExercisesListRequester->workingSet(row)};
			if (!m_simpleExercisesListRequester->isFieldUserModified(row, MESOSPLIT_COL_SETTYPE))
			{
				const int cur_set_type{m_simpleExercisesListRequester->setType(row, set_number)};
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
	m_currentSplitLetter = "N"_L1;
	m_currentSwappableLetter = "N"_L1;
	m_bHasExercises = false;

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
	QMetaObject::invokeMethod(m_plannerPage, "createNavButtons");
	emit plannerPageCreated();
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

	DBExercisesModel *splitmodel(m_splitModels.value(splitletter));

	m_splitProperties["splitModel"_L1] = std::move(QVariant::fromValue(splitmodel));
	m_splitProperties["splitManager"_L1] = std::move(QVariant::fromValue(this));

	QQuickItem *item{static_cast<QQuickItem*>(m_splitComponent->createWithInitialProperties(m_splitProperties, appQmlEngine()->rootContext()))};
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
	connect(splitmodel, &DBExercisesModel::splitChanged, this, [this,splitmodel] (const uint row, const uint) {
		if (row == 100)
			appDBInterface()->deleteMesoSplitTable(splitmodel);
		else
		{
			appDBInterface()->saveMesoSplitComplete(splitmodel);
			appMesoModel()->checkIfCanExport(m_mesoIdx);
		}
	});
}

void QmlMesoSplitInterface::setSplitPageProperties(QQuickItem* splitPage, const DBExercisesModel* const splitModel)
{
	m_prevMesoId = appMesoModel()->getPreviousMesoId(appMesoModel()->client(m_mesoIdx), appMesoModel()->_id(m_mesoIdx));
	if (m_prevMesoId >= 0)
	{
		if (appDBInterface()->mesoHasPlan(m_prevMesoId, splitModel->splitLetter()))
			m_prevMesoName = appMesoModel()->name(appMesoModel()->idxFromId(m_prevMesoId));
		else
			m_prevMesoId = -1; //Nothing from previous meso to import
	}
}
