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
		connect(this, &QmlMesoSplitInterface::plannerPageCreated, this,
								&QmlMesoSplitInterface::createMesoSplitPages, Qt::SingleShotConnection);
		createPlannerPage();
	}
	else
		emit addPageToMainMenu(tr("Exercises Planner: ") + appMesoModel()->name(m_mesoIdx), m_plannerPage);
}

void QmlMesoSplitInterface::addExercise()
{
	currentSplitModel()->setWorkingExercise(currentSplitModel()->addExercise(true));
}

void QmlMesoSplitInterface::moveExercise(const uint from, const uint to)
{
	currentSplitModel()->moveExercise(from, to);
}

void QmlMesoSplitInterface::removeExercise()
{
	QMetaObject::invokeMethod(m_plannerPage, "showDeleteDialog",
							  Q_ARG(QString, currentSplitModel()->exerciseName(currentSplitModel()->workingExercise())));
}

void QmlMesoSplitInterface::swapMesoPlans()
{
	DBExercisesModel* tempSplit{currentSplitModel()};
	m_splitModels[currentSplitLetter()] = m_splitModels.value(currentSwappableLetter());
	appDBInterface()->saveMesoSplit(currentSplitModel());
	m_splitModels[currentSwappableLetter()] = tempSplit;
	appDBInterface()->saveMesoSplit(m_splitModels.value(currentSplitLetter()));
}

//TODO
void QmlMesoSplitInterface::loadSplitFromPreviousMeso()
{
	if (m_prevMesoId >= 0)
		appDBInterface()->getMesoSplit(currentSplitModel());
}

void QmlMesoSplitInterface::simpleExercisesList(const bool show, const bool multi_sel)
{
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

void QmlMesoSplitInterface::exportMesoSplit(const bool bShare)
{
	const QString &suggestedName{appMesoModel()->name(m_mesoIdx) +
				tr(" - Exercises Plan - Split ") + currentSplitLetter() + ".txt"_L1};
	const QString &exportFileName{appItemManager()->setExportFileName(suggestedName)};
	appItemManager()->continueExport(currentSplitModel()->exportToFormattedFile(exportFileName), bShare);
}

void QmlMesoSplitInterface::exportAllMesoSplits(const bool bShare)
{
	const QString &suggestedName{appMesoModel()->name(m_mesoIdx) + std::move(tr(" - Exercises Plan.txt"))};
	const QString &exportFileName{appItemManager()->setExportFileName(suggestedName)};
	QFile *out_file{appUtils()->openFile(exportFileName, QIODeviceBase::WriteOnly|QIODeviceBase::Truncate|QIODeviceBase::Text)};
	if (!out_file)
		appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_OPEN_CREATE_FILE_FAILED, exportFileName);

	for (const auto split_model : m_splitModels)
	{
		if (split_model->exportToFormattedFile(exportFileName, out_file) != APPWINDOW_MSG_EXPORT_OK)
		{
			appItemManager()->displayMessageOnAppWindow(APPWINDOW_MSG_EXPORT_FAILED, exportFileName);
			return;
		}
	}
	appItemManager()->continueExport(APPWINDOW_MSG_EXPORT_OK, bShare);
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

QQuickItem* QmlMesoSplitInterface::setCurrentPage(const int index)
{
	m_currentSplitPage = m_splitPages.value(QChar(static_cast<int>('A') + index));
	m_currentSplitLetter = std::move(m_splitPages.key(m_currentSplitPage));
	m_currentSwappableLetter = std::move(findSwappableModel());
	emit currentPageChanged();
	return m_currentSplitPage;
}

bool QmlMesoSplitInterface::hasExercises() const
{
	return currentSplitModel()->exerciseCount() > 0;
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
	simpleExercisesList(false);
}

void QmlMesoSplitInterface::createPlannerPage()
{
	m_plannerComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/qml/Pages/ExercisesPlanner.qml"_L1}, QQmlComponent::Asynchronous};
	m_plannerProperties["splitManager"_L1] = QVariant::fromValue(this);
	m_currentSplitLetter = 'N';
	m_currentSwappableLetter = 'N';

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
	m_plannerPage = static_cast<QQuickItem*>(m_plannerComponent->createWithInitialProperties(
															m_plannerProperties, appQmlEngine()->rootContext()));
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
		for (const auto &error : m_plannerComponent->errors())
			qDebug() << error.description();
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

void QmlMesoSplitInterface::createMesoSplitPages()
{
	if (m_splitComponent == nullptr)
		m_splitComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/qml/Pages/MesoSplitPlanner.qml"_L1}, QQmlComponent::Asynchronous};

	if (m_splitComponent->status() == QQmlComponent::Ready)
		createMesoSplitPages_part2();
	else
	{
		connect(m_splitComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status) {
			createMesoSplitPages_part2();
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	}
}

void QmlMesoSplitInterface::createMesoSplitPages_part2()
{
	#ifndef QT_NO_DEBUG
	if (m_splitComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_splitComponent->errorString();
		for (const auto &error : m_plannerComponent->errors())
			qDebug() << error.description();
		return;
	}
	#endif

	m_prevMesoId = -2;
	m_splitProperties["splitManager"_L1] = std::move(QVariant::fromValue(this));

	uint idx{0};
	const QString &split_letters{appMesoModel()->usedSplits(m_mesoIdx)};
	for (const auto &split_letter : split_letters)
	{
		setSplitPageProperties(split_letter);
		DBExercisesModel *split_model{appMesoModel()->splitModel(m_mesoIdx, split_letter)};
		m_splitModels[split_letter] = split_model;
		m_splitProperties["splitModel"_L1] = std::move(QVariant::fromValue(split_model));

		QQuickItem *item{static_cast<QQuickItem*>(m_splitComponent->createWithInitialProperties(m_splitProperties, appQmlEngine()->rootContext()))};
		appQmlEngine()->setObjectOwnership(item, QQmlEngine::CppOwnership);
		item->setParentItem(m_plannerPage);

		m_splitPages.insert(split_letter, item);
		QMetaObject::invokeMethod(m_plannerPage, "insertSplitPage", Q_ARG(QQuickItem*, item), Q_ARG(uint, idx));

		connect(appMesoModel(), &DBMesocyclesModel::muscularGroupChanged, this, [this] (const uint meso_idx, const int splitIndex, const QChar& splitLetter) {
			if (meso_idx == m_mesoIdx)
				updateMuscularGroup(splitLetter);
		});
		connect(split_model, &DBExercisesModel::exerciseModified, this, [this,split_model] (const uint exercise_number, const uint exercise_idx, const uint set_number, const uint field) {
			if (field != EXERCISE_IGNORE_NOTIFY_IDX)
			{
				appDBInterface()->saveMesoSplit(split_model);
				appMesoModel()->checkIfCanExport(m_mesoIdx);
			}
		});
	}
}

void QmlMesoSplitInterface::setSplitPageProperties(const QChar &split_letter)
{
	if (m_prevMesoId == -1)
		m_hasPreviousPlan[split_letter] = false;
	else
	{
		if (m_prevMesoId == -2)
			m_prevMesoId = appMesoModel()->getPreviousMesoId(appMesoModel()->client(m_mesoIdx), appMesoModel()->_id(m_mesoIdx));

		if (m_prevMesoId >= 0)
		{
			if (appDBInterface()->mesoHasSplitPlan(m_prevMesoId, split_letter))
			{
				m_prevMesoName = appMesoModel()->name(appMesoModel()->idxFromId(m_prevMesoId));
				m_hasPreviousPlan[split_letter] = true;
				return;
			}
		}
		m_hasPreviousPlan[split_letter] = false;
	}
}

//Updates MesoSplitPlanner page with the changes originating in MesocyclePage.qml
void QmlMesoSplitInterface::updateMuscularGroup(const QChar &split_letter)
{
	QQuickItem* splitPage(m_splitPages.value(split_letter));
	if (splitPage)
		QMetaObject::invokeMethod(splitPage, "updateTxtGroups", Q_ARG(QString, appMesoModel()->muscularGroup(m_mesoIdx, split_letter)));
}

QChar QmlMesoSplitInterface::findSwappableModel() const
{
	QString muscularGroup1{std::move(appMesoModel()->muscularGroup(mesoIdx(), currentSplitLetter()))};
	if (!muscularGroup1.isEmpty())
	{
		muscularGroupSimplified(muscularGroup1);
		QString muscularGroup2;
		const QString &split_letters{appMesoModel()->usedSplits(m_mesoIdx)};
		for (const auto &split_letter : split_letters)
		{
			muscularGroup2 = appMesoModel()->muscularGroup(mesoIdx(), split_letter);
			if (!muscularGroup2.isEmpty())
			{
				muscularGroupSimplified(muscularGroup2);
				if (appUtils()->stringsAreSimiliar(muscularGroup1, muscularGroup2))
					return split_letter;
			}
		}
	}
	return 'N';
}
