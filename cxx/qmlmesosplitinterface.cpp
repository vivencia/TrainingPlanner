#include "qmlmesosplitinterface.h"

#include "dbexercisesmodel.h"
#include "dbworkoutsorsplitstable.h"
#include "dbmesocyclesmodel.h"
#include "pageslistmodel.h"
#include "qmlitemmanager.h"
#include "thread_manager.h"
#include "tputils.h"

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>

void QmlMesoSplitInterface::cleanUp()
{
	if (m_plannerComponent)
	{
		delete m_plannerPage;
		delete m_plannerComponent;
		delete m_splitComponent;
		qDeleteAll(m_splitPages);
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

void QmlMesoSplitInterface::swapMesoPlans()
{
	DBSplitModel* tempSplit{currentSplitModel()};
	m_mesoModel->splitModelsForMeso(m_mesoIdx).insert(currentSplitLetter(),
										m_mesoModel->splitModelsForMeso(m_mesoIdx).value(currentSwappableLetter()));
	m_mesoModel->splitModelsForMeso(m_mesoIdx).insert(currentSwappableLetter(), tempSplit);
}

void QmlMesoSplitInterface::loadSplitFromPreviousMeso()
{
	if (m_hasPreviousPlan.value(m_currentSplitLetter))
	{
		DBSplitModel *split_model{new DBSplitModel{m_mesoModel, currentSplitModel()->database(),
																					m_mesoIdx, m_currentSplitLetter, true}};
		connect(split_model, &DBSplitModel::exerciseCountChanged, this, [this,split_model] () {
			if (split_model->exerciseCount() > 0)
			{
				connect(currentSplitModel(), &DBSplitModel::exerciseCountChanged, this, [this,split_model] () {
					split_model->setMesoIdx(m_mesoIdx);
					delete currentSplitModel();
					m_mesoModel->splitModelsForMeso(m_mesoIdx).insert(m_currentSplitLetter, split_model);
				});
				currentSplitModel()->clearExercises(true);
			}
		});
	}
}

void QmlMesoSplitInterface::exportMesoSplit(const bool bShare)
{
	const QString &suggestedName{m_mesoModel->name(m_mesoIdx) % tr(" - Exercises Plan - Split ") % currentSplitLetter() % ".txt"_L1};
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
	m_currentSplitLetter = m_currentSplitPage ? std::move(m_splitPages.key(m_currentSplitPage)) : 'A';
	m_mesoModel->splitModel(m_mesoIdx, m_currentSplitLetter)->plugDBModelInterfaceIntoDatabase();
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

void QmlMesoSplitInterface::createPlannerPage()
{
	m_plannerComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/qml/Pages/ExercisesPlanner.qml"_L1}, QQmlComponent::Asynchronous};
	m_plannerProperties["splitManager"_L1] = QVariant::fromValue(this);

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
		#ifndef QT_NO_DEBUG
		case QQmlComponent::Null:
		case QQmlComponent::Error:
			qDebug() << m_plannerComponent->errorString();
		break;
		#endif
	}
}

void QmlMesoSplitInterface::createPlannerPage_part2()
{
	m_plannerPage = static_cast<QQuickItem*>(m_plannerComponent->createWithInitialProperties(
															m_plannerProperties, appQmlEngine()->rootContext()));
	#ifndef QT_NO_DEBUG
	if (!m_plannerPage)
	{
		qDebug() << m_plannerComponent->errorString();
		return;
	}
	#endif
	appQmlEngine()->setObjectOwnership(m_plannerPage, QQmlEngine::CppOwnership);
	m_plannerPage->setParentItem(appMainWindow()->findChild<QQuickItem*>("appStackView"));
	m_swipeView = m_plannerPage->findChild<QQuickItem*>("swipeView");
	emit plannerPageCreated();

	appPagesListModel()->openPage(m_plannerPage,
			std::move(tr("Exercises Planner: ") + m_mesoModel->name(m_mesoIdx)), [this] () { cleanUp(); });
	connect(m_mesoModel, &DBMesocyclesModel::mesoChanged, this, [this] (const uint meso_idx, const uint field) {
		if (meso_idx == m_mesoIdx)
		{
			if (field == MESO_FIELD_SPLIT)
				syncSplitPagesWithMesoSplit();
		}
	});
	connect(appItemManager(), &QmlItemManager::selectedExerciseFromSimpleExercisesList, [this] (QQuickItem *parentPage) {
		if (parentPage == m_plannerPage)
			currentSplitModel()->newExerciseFromExercisesList();
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
		#ifndef QT_NO_DEBUG
		case QQmlComponent::Null:
		case QQmlComponent::Error:
			qDebug() << m_splitComponent->errorString();
		break;
		#endif
	}
}

void QmlMesoSplitInterface::createMesoSplitPages_part2()
{
	m_splitProperties["pageManager"_L1] = QVariant::fromValue(this);
	m_splitProperties["height"_L1] = m_plannerPage->property("splitPageHeight").toInt();
	const QString &split_letters{m_mesoModel->usedSplits(m_mesoIdx)};
	for (const auto &split_letter : split_letters)
		addPage(split_letter);
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

	for (const auto &split_letter : split_letters)
	{
		if (!m_splitPages.value(split_letter))
			addPage(split_letter);
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

void QmlMesoSplitInterface::addPage(const QChar &split_letter)
{
	DBSplitModel *split_model{m_mesoModel->splitModel(m_mesoIdx, split_letter)};
	m_splitProperties["exercisesModel"_L1] = QVariant::fromValue(split_model);
	setSplitPageProperties(split_model);

	QQuickItem *item{static_cast<QQuickItem*>(m_splitComponent->createWithInitialProperties(m_splitProperties, appQmlEngine()->rootContext()))};
	#ifndef QT_NO_DEBUG
	if (!item)
	{
		qDebug() << m_splitComponent->errorString();
		return;
	}
	#endif
	m_splitPages.insert(split_letter, item);
	appQmlEngine()->setObjectOwnership(item, QQmlEngine::CppOwnership);
	item->setParentItem(m_swipeView);
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
	return QChar{};
}
