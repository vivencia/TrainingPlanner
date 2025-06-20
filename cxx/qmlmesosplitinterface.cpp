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
		appItemManager()->addMainMenuShortCut(tr("Exercises Planner: ") + appMesoModel()->name(m_mesoIdx), m_plannerPage);
}

void QmlMesoSplitInterface::addExercise()
{
	currentSplitModel()->setWorkingExercise(currentSplitModel()->addExercise(true));
}

void QmlMesoSplitInterface::removeExercise()
{
	const uint exercise_number{currentSplitModel()->workingExercise()};
	QMetaObject::invokeMethod(m_plannerPage, "showDeleteDialog", Q_ARG(QString,
						currentSplitModel()->exerciseName(exercise_number, currentSplitModel()->workingSubExercise(exercise_number))));
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
	if (m_hasPreviousPlan.value(m_currentSplitLetter))
	{
		currentSplitModel()->setMesoId(m_prevMesoId);
		auto conn = std::make_shared<QMetaObject::Connection>();
		*conn = connect(appDBInterface(), &DBInterface::databaseReady, this, [this,conn] (const uint db_id) mutable {
			if (db_id == MESOSPLIT_TABLE_ID)
			{
				disconnect(*conn);
				currentSplitModel()->setMesoId(appMesoModel()->id(m_mesoIdx));
				appDBInterface()->saveMesoSplit(currentSplitModel());
				appMesoModel()->checkIfCanExport(m_mesoIdx);
			}
		});
		appDBInterface()->getMesoSplit(currentSplitModel());
	}
}

void QmlMesoSplitInterface::simpleExercisesList(const bool show, const bool multi_sel)
{
	if (show)
	{
		if (appExercisesList()->count() == 0)
			appDBInterface()->getAllExercises();
		connect(m_plannerPage, SIGNAL(exerciseSelectedFromSimpleExercisesList()), currentSplitModel(), SLOT(newExerciseFromExercisesList()));
		connect(m_plannerPage, SIGNAL(simpleExercisesListClosed()), this, SLOT(hideSimpleExercisesList()));
		QMetaObject::invokeMethod(m_plannerPage, "showSimpleExercisesList", Q_ARG(bool, multi_sel));
	}
	else
	{
		disconnect(m_plannerPage, SIGNAL(exerciseSelectedFromSimpleExercisesList()), currentSplitModel(), SLOT(newExerciseFromExercisesList()));
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

	for (const auto split_model : std::as_const(m_splitModels))
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
	m_currentSplitPage = m_splitPages.value(QChar(static_cast<int>('A') + index));
	m_currentSplitLetter = std::move(m_splitPages.key(m_currentSplitPage));
	m_currentSwappableLetter = std::move(findSwappableModel());
	emit currentPageChanged();
	return m_currentSplitPage;
}

bool QmlMesoSplitInterface::hasExercises() const
{
	return currentSplitModel() ? currentSplitModel()->exerciseCount() > 0 : false;
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
#ifndef QT_NO_DEBUG
			else if (status == QQmlComponent::Error)
			{
				qDebug() << m_plannerComponent->errorString();
				return;
			}
#endif
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
	appQmlEngine()->setObjectOwnership(m_plannerPage, QQmlEngine::CppOwnership);
	m_plannerPage->setParentItem(appMainWindow()->findChild<QQuickItem*>("appStackView"));
	QMetaObject::invokeMethod(m_plannerPage, "createNavButtons");
	emit plannerPageCreated();
	appItemManager()->addMainMenuShortCut(tr("Exercises Planner: ") + appMesoModel()->name(m_mesoIdx), m_plannerPage, [this] () {
		cleanUp();
	});
}

void QmlMesoSplitInterface::createMesoSplitPages()
{
	if (m_splitComponent == nullptr)
		m_splitComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/qml/Pages/MesoSplitPlanner.qml"_L1}, QQmlComponent::Asynchronous};

	if (m_splitComponent->status() == QQmlComponent::Ready)
		createMesoSplitPages_part2();
	else
	{
		connect(m_splitComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status status) {
			if (status == QQmlComponent::Ready)
				createMesoSplitPages_part2();
#ifndef QT_NO_DEBUG
			else if (m_splitComponent->status() == QQmlComponent::Error)
			{
				qDebug() << m_splitComponent->errorString();
				return;
			}
	#endif
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	}
}

void QmlMesoSplitInterface::createMesoSplitPages_part2()
{
	m_prevMesoId = "-2"_L1;
	m_splitProperties["splitManager"_L1] = std::move(QVariant::fromValue(this));
	m_splitProperties["height"_L1] = m_plannerPage->property("splitPageHeight").toInt();

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
		QMetaObject::invokeMethod(m_plannerPage, "insertSplitPage", Q_ARG(QQuickItem*, item), Q_ARG(int, static_cast<int>(idx++)));

		connect(appMesoModel(), &DBMesocyclesModel::muscularGroupChanged, this, [this] (const uint meso_idx, const int splitIndex, const QChar& splitLetter) {
			if (meso_idx == m_mesoIdx)
				updateMuscularGroup(splitLetter);
		});
		connect(split_model, &DBExercisesModel::exerciseModified, this, [this,split_model] (const uint exercise_number, const uint exercise_idx, const uint set_number, const uint field) {
			if (field != EXERCISE_IGNORE_NOTIFY_IDX)
			{
				appDBInterface()->saveMesoSplit(split_model);
				//appMesoModel()->checkIfCanExport(m_mesoIdx);
			}
		});
	}
}

void QmlMesoSplitInterface::setSplitPageProperties(const QChar &split_letter)
{
	int prev_mesoid{m_prevMesoId.toInt()};
	if (prev_mesoid == -1)
		m_hasPreviousPlan[split_letter] = false;
	else
	{
		if (prev_mesoid == -2)
		{
			prev_mesoid = appMesoModel()->getPreviousMesoId(appMesoModel()->client(m_mesoIdx), appMesoModel()->_id(m_mesoIdx));
			m_prevMesoId = QString::number(prev_mesoid);
		}

		if (prev_mesoid >= 0)
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
