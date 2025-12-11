#include "qmlmesosplitinterface.h"

#include "dbexercisesmodel.h"
#include "dbworkoutsorsplitstable.h"
#include "dbmesocyclesmodel.h"
#include "pageslistmodel.h"
#include "qmlitemmanager.h"
#include "thread_manager.h"
#include "tpsettings.h"
#include "tputils.h"

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>

void QmlMesoSplitInterface::cleanUp()
{
	if (m_plannerComponent)
	{
		qDeleteAll(m_splitPages);
		delete m_splitComponent;
		delete m_plannerPage;
		delete m_plannerComponent;
	}
}

void QmlMesoSplitInterface::getExercisesPlannerPage()
{
	if (!m_plannerPage)
	{
		connect(this, &QmlMesoSplitInterface::plannerPageCreated, this,
								&QmlMesoSplitInterface::createMesoSplitPages, Qt::SingleShotConnection);
		createPlannerPage();
	}
	else
		appPagesListModel()->openPage(m_plannerPage);
}

void QmlMesoSplitInterface::addExercise()
{
	currentSplitModel()->setWorkingExercise(currentSplitModel()->addExercise(true));
}

void QmlMesoSplitInterface::removeExercise(const int exercise_number)
{
	if (exercise_number >= 0)
	{
		if (appSettings()->alwaysAskConfirmation())
			QMetaObject::invokeMethod(m_plannerPage, "showDeleteDialog", Q_ARG(QString,
						currentSplitModel()->exerciseName(exercise_number, currentSplitModel()->workingSubExercise(exercise_number))));
		else
			currentSplitModel()->delExercise(exercise_number);
	}
	else
		currentSplitModel()->delExercise(currentSplitModel()->workingExercise());
}

void QmlMesoSplitInterface::swapMesoPlans()
{
	DBSplitModel* tempSplit{currentSplitModel()};
	m_mesoModel->splitModelsForMeso(m_mesoIdx)[currentSplitLetter()] =
							m_mesoModel->splitModelsForMeso(m_mesoIdx).value(currentSwappableLetter());
	m_mesoModel->splitModelsForMeso(m_mesoIdx)[currentSwappableLetter()] = tempSplit;
}

void QmlMesoSplitInterface::loadSplitFromPreviousMeso()
{
	if (m_hasPreviousPlan.value(m_currentSplitLetter))
	{
		DBSplitModel *split_model{new DBSplitModel{m_mesoModel, currentSplitModel()->database(), m_mesoIdx, m_currentSplitLetter}};
		connect(split_model, &DBSplitModel::exerciseCountChanged, this, [this,split_model] () {
			m_mesoModel->splitModelsForMeso(m_mesoIdx)[m_currentSplitLetter] = split_model;
			delete split_model;
		});
	}
}

void QmlMesoSplitInterface::simpleExercisesList(const bool show)
{
	if (show)
		appItemManager()->showSimpleExercisesList(m_plannerPage, currentSplitModel()->muscularGroup());
	else
		appItemManager()->hideSimpleExercisesList(m_plannerPage);
}

void QmlMesoSplitInterface::exportMesoSplit(const bool bShare)
{
	const QString &suggestedName{m_mesoModel->name(m_mesoIdx) +
				tr(" - Exercises Plan - Split ") + currentSplitLetter() + ".txt"_L1};
	const QString &exportFileName{appItemManager()->setExportFileName(suggestedName)};
	appItemManager()->continueExport(currentSplitModel()->exportToFormattedFile(exportFileName), bShare);
}

void QmlMesoSplitInterface::importMesoSplit(const QString &filename)
{
	if (filename.isEmpty())
	{
		m_mesoModel->setImportIdx(m_mesoIdx);
		QMetaObject::invokeMethod(appMainWindow(), "chooseFileToImport", Q_ARG(int, IFC_MESOSPLIT));
	}
	else
		appItemManager()->openRequestedFile(filename, IFC_MESOSPLIT);
}

static void muscularGroupSimplified(QString &muscularGroup)
{
	muscularGroup = muscularGroup.replace(',', ' ').simplified();
	const QStringList &words(muscularGroup.split(' '));
	muscularGroup.clear();
	for (const auto &word : words)
	{
		if(word.length() < 3)
			continue;
		if (!muscularGroup.isEmpty())
			muscularGroup.append(' ');
		muscularGroup.append(word.toLower());
		if (muscularGroup.endsWith('s', Qt::CaseInsensitive))
			muscularGroup.chop(1);
		muscularGroup.remove('.');
		muscularGroup.remove('(');
		muscularGroup.remove(')');
	}
}

QQuickItem* QmlMesoSplitInterface::setCurrentPage(const int index)
{
	m_currentSplitPage = m_splitPages.value(QChar(static_cast<char>(static_cast<int>('A') + index)));
	m_currentSplitLetter = std::move(m_splitPages.key(m_currentSplitPage));
	m_currentSwappableLetter = std::move(findSwappableModel());
	emit currentPageChanged();
	return m_currentSplitPage;
}

DBSplitModel *QmlMesoSplitInterface::currentSplitModel() const
{
	return m_mesoModel->splitModel(m_mesoIdx, m_currentSplitLetter);
}

bool QmlMesoSplitInterface::haveExercises() const
{
	return currentSplitModel() ? currentSplitModel()->exerciseCount() > 0 : false;
}

void QmlMesoSplitInterface::changeExerciseName()
{
	currentSplitModel()->newExerciseFromExercisesList();
}

void QmlMesoSplitInterface::createPlannerPage()
{
	m_plannerComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/qml/Pages/ExercisesPlanner.qml"_L1}, QQmlComponent::Asynchronous};
	m_plannerProperties["splitManager"_L1] = QVariant::fromValue(this);
	m_currentSplitLetter = 'N';
	m_currentSwappableLetter = 'N';

	switch (m_plannerComponent->status())
	{
		case QQmlComponent::Ready:
			createPlannerPage_part2();
		break;
		case QQmlComponent::Loading:
			connect(m_plannerComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status status) {
				createPlannerPage_part2();
			}, Qt::SingleShotConnection);
		break;
		case QQmlComponent::Null:
		case QQmlComponent::Error:
			#ifndef QT_NO_DEBUG
			qDebug() << m_plannerComponent->errorString();
			#endif
		break;
	}
}

void QmlMesoSplitInterface::createPlannerPage_part2()
{
	m_plannerPage = static_cast<QQuickItem*>(m_plannerComponent->createWithInitialProperties(
															m_plannerProperties, appQmlEngine()->rootContext()));
	appQmlEngine()->setObjectOwnership(m_plannerPage, QQmlEngine::CppOwnership);
	m_plannerPage->setParentItem(appMainWindow()->findChild<QQuickItem*>("appStackView"));
	emit plannerPageCreated();

	appPagesListModel()->openPage(m_plannerPage,
			std::move(tr("Exercises Planner: ") + m_mesoModel->name(m_mesoIdx)), [this] () { cleanUp(); });
	connect(m_plannerPage, SIGNAL(exerciseSelectedFromSimpleExercisesList()), this, SLOT(changeExerciseName()));
	connect(m_mesoModel, &DBMesocyclesModel::mesoChanged, this, [this] (const uint meso_idx, const uint field) {
		if (meso_idx == m_mesoIdx)
		{
			if (field == MESOCYCLES_COL_SPLIT)
				syncSplitPagesWithMesoSplit();
		}
	});
}

void QmlMesoSplitInterface::createMesoSplitPages()
{
	if (m_splitComponent == nullptr)
		m_splitComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/qml/ExercisesAndSets/WorkoutOrSplitExercisesList.qml"_L1}, QQmlComponent::Asynchronous};

	switch (m_splitComponent->status())
	{
		case QQmlComponent::Ready:
			createMesoSplitPages_part2();
		break;
		case QQmlComponent::Loading:
			connect(m_splitComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status status) {
				createMesoSplitPages_part2();
			}, Qt::SingleShotConnection);
		break;
		case QQmlComponent::Null:
		case QQmlComponent::Error:
			#ifndef QT_NO_DEBUG
			qDebug() << m_splitComponent->errorString();
			#endif
		break;
	}
}

void QmlMesoSplitInterface::createMesoSplitPages_part2()
{
	m_splitProperties["pageManager"_L1] = std::move(QVariant::fromValue(this));
	m_splitProperties["height"_L1] = m_plannerPage->property("splitPageHeight").toInt();

	uint index{0};
	const QString &split_letters{m_mesoModel->usedSplits(m_mesoIdx)};
	for (const auto &split_letter : split_letters)
		addPage(split_letter, index++);
}

void QmlMesoSplitInterface::setSplitPageProperties(DBSplitModel *split_model)
{
	auto prev_mesoid{m_mesoModel->getPreviousMesoId(m_mesoModel->client(m_mesoIdx), m_mesoModel->_id(m_mesoIdx))};
	if (prev_mesoid >= 0)
	{
		m_prevMesoId = QString::number(prev_mesoid);

		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(split_model->database(), &TPDatabaseTable::actionFinished, this, [this,conn,split_model]
					(const ThreadManager::StandardOps op, const QVariant &return_value1, const QVariant &return_value2)
		{
			if (op == ThreadManager::CustomOperation)
			{
				disconnect(*conn);
				const bool has_prevplan{return_value2.toBool()};
				m_hasPreviousPlan.insert(split_model->splitLetter(), has_prevplan);
				if (has_prevplan)
					m_prevMesoName = m_mesoModel->name(m_mesoModel->idxFromId(m_prevMesoId));
			}
		});
		auto x = [this,split_model] () -> std::pair<QVariant,QVariant> {
							return split_model->database()->mesoHasAllSplitPlans(m_mesoModel->id(m_mesoIdx), m_mesoModel->usedSplits(m_mesoIdx)); };
		split_model->database()->setCustQueryFunction(x);
		appThreadManager()->runAction(split_model->database(), ThreadManager::CustomOperation);
	}
}

void QmlMesoSplitInterface::syncSplitPagesWithMesoSplit()
{
	const QString &split_letters{m_mesoModel->usedSplits(m_mesoIdx)};
	int current_splits{0};
	const QMap<QChar,QQuickItem*>::const_iterator itr_begin{m_splitPages.constBegin()};
	QMap<QChar,QQuickItem*>::const_iterator itr{m_splitPages.constEnd()};
	do {
		--itr;
		if (!split_letters.contains(itr.key()))
			removePage(itr.key());
		if (itr == itr_begin)
			break;
	} while (true);

	uint index{0};
	for (const auto &split_letter : split_letters)
	{
		if (!m_splitPages.value(split_letter))
			addPage(split_letter, index);
		index++;
	}
}

void QmlMesoSplitInterface::removePage(const QChar &split_letter)
{
	QQuickItem *split_page{m_splitPages.value(split_letter)};
	if (split_page)
	{
		delete split_page;
		m_mesoModel->removeSplit(m_mesoIdx, split_letter);
	}
}

void QmlMesoSplitInterface::addPage(const QChar &split_letter, const uint index)
{
	DBSplitModel *split_model{m_mesoModel->splitModel(m_mesoIdx, split_letter)};
	m_splitProperties["exercisesModel"_L1] = std::move(QVariant::fromValue(split_model));
	setSplitPageProperties(split_model);

	QQuickItem *item{static_cast<QQuickItem*>(m_splitComponent->createWithInitialProperties(m_splitProperties, appQmlEngine()->rootContext()))};
	appQmlEngine()->setObjectOwnership(item, QQmlEngine::CppOwnership);
	item->setParentItem(m_plannerPage);

	m_splitPages.insert(split_letter, item);
	QMetaObject::invokeMethod(m_plannerPage, "insertSplitPage", Q_ARG(QQuickItem*, item), Q_ARG(int, static_cast<int>(index)));
}

QChar QmlMesoSplitInterface::findSwappableModel() const
{
	QString muscularGroup1{std::move(currentSplitModel()->muscularGroup())};
	muscularGroupSimplified(muscularGroup1);
	const QString &split_letters{m_mesoModel->usedSplits(m_mesoIdx)};
	for (const auto &split_letter : split_letters)
	{
		if (split_letter != m_currentSplitLetter)
		{
			QString muscularGroup2{std::move(m_mesoModel->muscularGroup(mesoIdx(), split_letter))};
			muscularGroupSimplified(muscularGroup2);
			if (appUtils()->similarityBetweenStrings(muscularGroup1, muscularGroup2) >= 0.8)
				return split_letter;
		}
	}
	return 'N';
}
