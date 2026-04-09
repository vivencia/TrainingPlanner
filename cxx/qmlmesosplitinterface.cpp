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
	if (m_plannerComponent) {
		delete m_plannerPage;
		delete m_plannerComponent;
		delete m_splitComponent;
		qDeleteAll(m_splitPages);
	}
}

void QmlMesoSplitInterface::getExercisesPlannerPage()
{
	if (!m_plannerComponent) {
		m_plannerComponent = new QQmlComponent{appQmlEngine(), "TpQml.Pages"_L1, "ExercisesPlanner"_L1, QQmlComponent::Asynchronous};
		m_plannerProperties["splitManager"_L1] = std::move(QVariant::fromValue(this));
		connect(m_plannerComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status status) { getExercisesPlannerPage(); });
	}
	else {
		if (!m_plannerPage) {
			switch (m_plannerComponent->status()) {
			case QQmlComponent::Ready:
				m_plannerComponent->disconnect();
				createPlannerPage();
				break;
#ifndef QT_NO_DEBUG
			case QQmlComponent::Loading:
				break;

			case QQmlComponent::Null:
			case QQmlComponent::Error:
				qDebug() << m_plannerComponent->errorString();
				break;
#else
			default: break;
#endif
			}
		}
		else
			appPagesListModel()->openPage(m_plannerPage);
	}
}

void QmlMesoSplitInterface::swapMesoPlans()
{
	DBSplitModel *tempSplit{currentSplitModel()};
	m_mesoModel->splitModelsForMeso(m_mesoIdx).insert(currentSplitLetter(),
														m_mesoModel->splitModelsForMeso(m_mesoIdx).value(currentSwappableLetter()));
	m_mesoModel->splitModelsForMeso(m_mesoIdx).insert(currentSwappableLetter(), tempSplit);
}

void QmlMesoSplitInterface::loadSplitFromPreviousMeso()
{
	if (m_hasPreviousPlan.value(m_currentSplitLetter)) {
		DBSplitModel *split_model{new DBSplitModel{m_mesoModel, currentSplitModel()->database(), m_mesoIdx, m_currentSplitLetter, true}};
		connect(split_model, &DBSplitModel::exerciseCountChanged, this, [this,split_model] () {
			if (split_model->exerciseCount() > 0) {
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

static void muscularGroupSimplified(QString &muscularGroup)
{
	muscularGroup = std::move(muscularGroup.replace(',', ' ').simplified());
	const QStringList &words(muscularGroup.split(' '));
	muscularGroup.clear();
	for (const auto &word : words) {
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

DBExercisesModel *QmlMesoSplitInterface::currentSplitModel() const
{
	return m_mesoModel->splitModel(m_mesoIdx, m_currentSplitLetter);
}

DBExercisesModel *QmlMesoSplitInterface::splitModel(const int index) const
{
	QChar split_letter{static_cast<char>(static_cast<int>('A') + index)};
	return m_mesoModel->splitModel(m_mesoIdx, split_letter);
}

bool QmlMesoSplitInterface::haveExercises() const
{
	return currentSplitModel() ? currentSplitModel()->exerciseCount() > 0 : false;
}

void QmlMesoSplitInterface::createPlannerPage()
{
	m_plannerPage = static_cast<QQuickItem*>(m_plannerComponent->createWithInitialProperties(m_plannerProperties, appQmlEngine()->rootContext()));
	#ifndef QT_NO_DEBUG
	if (!m_plannerPage) {
		qDebug() << m_plannerComponent->errorString();
		return;
	}
	#endif
	appQmlEngine()->setObjectOwnership(m_plannerPage, QQmlEngine::CppOwnership);
	m_plannerPage->setParentItem(appItemManager()->AppPagesVisualParent());
	m_swipeView = qobject_cast<QQuickItem*>(m_plannerPage->findChild<QQuickItem*>("swipeView"));
	loadMesoSplitComponent();

	appPagesListModel()->openPage(m_plannerPage, std::move(tr("Exercises Planner: ") % m_mesoModel->name(m_mesoIdx)), [this] () { cleanUp(); });
	connect(m_mesoModel, &DBMesocyclesModel::mesoChanged, this, [this] (const uint meso_idx, const uint field) {
		if (meso_idx == m_mesoIdx) {
			if (field == DBMesocyclesModel::MESO_FIELD_SPLIT)
				syncSplitPagesWithMesoSplit();
		}
	});

	connect(appItemManager(), &QmlItemManager::selectedExerciseFromSimpleExercisesList, this, [this] (QQuickItem *parentPage) {
		if (parentPage == m_plannerPage)
			currentSplitModel()->newExerciseFromExercisesList();
	});
}

void QmlMesoSplitInterface::loadMesoSplitComponent()
{
	if (!m_splitComponent) {
		m_splitComponent = new QQmlComponent{appQmlEngine(), "TpQml.Exercises"_L1, "WorkoutOrSplitExercisesList"_L1, QQmlComponent::Asynchronous};
		m_splitProperties["splitPageManager"_L1] = std::move(QVariant::fromValue(this));
		m_splitProperties["workoutPageManager"_L1] = std::move(QVariant::fromValue(nullptr));
		m_splitProperties["height"_L1] = std::move(m_plannerPage->property("splitPageHeight").toInt());
		connect(m_splitComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status status) { loadMesoSplitComponent(); });
	}
	else {
		switch (m_splitComponent->status()) {
		case QQmlComponent::Ready:
			{
				const QString &split_letters{m_mesoModel->usedSplits(m_mesoIdx)};
				for (const auto &split_letter : split_letters)
					addPage(split_letter);
			}
			break;
#ifndef QT_NO_DEBUG
		case QQmlComponent::Loading:
		break;
		case QQmlComponent::Null:
		case QQmlComponent::Error:
			qDebug() << m_splitComponent->errorString();
		break;
#else
		default: break;
#endif
		}
	}
}

void QmlMesoSplitInterface::addPage(const QChar &split_letter)
{
	DBSplitModel *split_model{m_mesoModel->splitModel(m_mesoIdx, split_letter)};
	m_splitProperties["exercisesModel"_L1] = std::move(QVariant::fromValue(split_model));
	setSplitPageProperties(split_model);

	QQuickItem *item{static_cast<QQuickItem*>(m_splitComponent->createWithInitialProperties(m_splitProperties, appQmlEngine()->rootContext()))};
#ifndef QT_NO_DEBUG
	if (!item) {
		qDebug() << m_splitComponent->errorString();
		return;
	}
#endif
	appQmlEngine()->setObjectOwnership(item, QQmlEngine::CppOwnership);
	m_splitPages.insert(split_letter, item);
	QMetaObject::invokeMethod(m_plannerPage, "addPage", Q_ARG(QQuickItem*, item));
}

void QmlMesoSplitInterface::setSplitPageProperties(DBSplitModel *split_model)
{
	auto prev_mesoid{m_mesoModel->getPreviousMesoId(m_mesoModel->client(m_mesoIdx), m_mesoModel->_id(m_mesoIdx))};
	if (prev_mesoid >= 0) {
		m_prevMesoId = QString::number(prev_mesoid);
		auto conn{std::make_shared<QMetaObject::Connection>()};
		*conn = connect(split_model->database(), &TPDatabaseTable::actionFinished, this, [this,conn,split_model]
								(const ThreadManager::StandardOps op, const QVariant &return_value1, const QVariant &return_value2) {
			if (op == ThreadManager::CustomOperation) {
				disconnect(*conn);
				const bool has_prevplan{return_value2.toBool()};
				m_hasPreviousPlan.insert(split_model->splitLetter(), has_prevplan);
				if (has_prevplan)
					m_prevMesoName = m_mesoModel->name(m_mesoModel->idxFromId(m_prevMesoId));
			}
		});
		auto x = [this,split_model] () -> std::pair<QVariant,QVariant> {
			return split_model->database()->mesoHasAllSplitPlans(m_mesoModel->id(m_mesoIdx), m_mesoModel->usedSplits(m_mesoIdx));
		};
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

	for (const auto &split_letter : split_letters) {
		if (!m_splitPages.value(split_letter))
			addPage(split_letter);
	}
}

void QmlMesoSplitInterface::removePage(const QChar &split_letter)
{
	QQuickItem *split_page{m_splitPages.value(split_letter)};
	if (split_page) {
		delete split_page;
		m_mesoModel->removeSplit(m_mesoIdx, split_letter);
	}
}

QChar QmlMesoSplitInterface::findSwappableModel() const
{
	QString muscularGroup1{std::move(currentSplitModel()->muscularGroup())};
	muscularGroupSimplified(muscularGroup1);
	const QString &split_letters{m_mesoModel->usedSplits(m_mesoIdx)};
	for (const auto &split_letter : split_letters) {
		if (split_letter != m_currentSplitLetter) {
			QString muscularGroup2{std::move(m_mesoModel->muscularGroup(mesoIdx(), split_letter))};
			muscularGroupSimplified(muscularGroup2);
			if (appUtils()->similarityBetweenStrings(muscularGroup1, muscularGroup2) >= 0.8)
				return split_letter;
		}
	}
	return QChar{};
}
