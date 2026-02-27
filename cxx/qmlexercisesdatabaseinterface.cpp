#include "qmlexercisesdatabaseinterface.h"

#include "dbexerciseslistmodel.h"
#include "pageslistmodel.h"
#include "qmlitemmanager.h"
#include "qmlworkoutinterface.h"
#include "osinterface.h"

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>

void QmlExercisesDatabaseInterface::saveExercise()
{

}

const uint QmlExercisesDatabaseInterface::removeExercise(const uint row)
{
	appExercisesList()->removeExercise(row);
	return row > 0 ? row - 1 : 0;
}

void QmlExercisesDatabaseInterface::exportExercises(const bool bShare)
{
	int exportFileMessageId{0};
	if (appExercisesList()->collectExportData()) {
		const QString &exportFileName{appSettings()->localAppFilesDir() + tr("TrainingPlanner Exercises List") + ".txt"_L1};
		exportFileMessageId = appExercisesList()->exportToFile(exportFileName);
		if (exportFileMessageId >= 0) {
			if (bShare) {
				appOsInterface()->shareFile(exportFileName);
				exportFileMessageId = TP_RET_CODE_SHARE_OK;
			}
			else
				QMetaObject::invokeMethod(appMainWindow(), "chooseFolderToSave", Q_ARG(QString, exportFileName));
		}
		appItemManager()->displayMessageOnAppWindow(exportFileMessageId, exportFileName);
	}
	else
		exportFileMessageId = TP_RET_CODE_NOTHING_TO_EXPORT;
	appItemManager()->displayMessageOnAppWindow(exportFileMessageId);
}

void QmlExercisesDatabaseInterface::importExercises(const QString &filename)
{
	if (filename.isEmpty())
		QMetaObject::invokeMethod(appMainWindow(), "chooseFileToImport");
	else
		appUtils()->viewOrOpenFile(filename, TPUtils::FT_TP_EXERCISES);
}

void QmlExercisesDatabaseInterface::getExercisesPage(QmlWorkoutInterface *connectPage)
{
	if (!m_exercisesComponent)
		createExercisesPage(connectPage);
	else
	{
		m_exercisesPage->setProperty("bChooseButtonEnabled", connectPage != nullptr);
		appExercisesList()->clearSelectedEntries();
		if (connectPage)
		{
			disconnect(m_exercisesPage, SIGNAL(exerciseChosen()), nullptr, nullptr);
			connect(m_exercisesPage, SIGNAL(exerciseChosen()), connectPage, SLOT(newExerciseChosen()));
		}
		appPagesListModel()->openPage(m_exercisesPage);
	}
}

void QmlExercisesDatabaseInterface::createExercisesPage(QmlWorkoutInterface *connectPage)
{
	m_exercisesProperties.insert("bChooseButtonEnabled"_L1, connectPage != nullptr);
	m_exercisesProperties.insert("exercisesManager"_L1, QVariant::fromValue(this));
	m_exercisesComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/qml/Pages/ExercisesListPage.qml"_L1}, QQmlComponent::Asynchronous};
	switch (m_exercisesComponent->status()) {
		case QQmlComponent::Ready:
			createExercisesPage_part2(connectPage);
		break;
		case QQmlComponent::Loading:
			connect(m_exercisesComponent, &QQmlComponent::statusChanged, this, [this,connectPage] (QQmlComponent::Status status) {
				createExercisesPage(connectPage);
			}, Qt::SingleShotConnection);
		break;
		case QQmlComponent::Null:
		case QQmlComponent::Error:
			#ifndef QT_NO_DEBUG
			qDebug() << m_exercisesComponent->errorString();
			#endif
		break;
	}
}

void QmlExercisesDatabaseInterface::createExercisesPage_part2(QmlWorkoutInterface *connectPage)
{
	m_exercisesPage = static_cast<QQuickItem*>(m_exercisesComponent->createWithInitialProperties(m_exercisesProperties, appQmlEngine()->rootContext()));
#ifndef QT_NO_DEBUG
	if (!m_exercisesPage) {
		m_exercisesComponent->errorString();
		return;
	}
#endif
	appQmlEngine()->setObjectOwnership(m_exercisesPage, QQmlEngine::CppOwnership);
	m_exercisesPage->setParentItem(appMainWindow()->contentItem());
	appExercisesList()->clearSelectedEntries();
	appPagesListModel()->openPage(m_exercisesPage, std::move(tr("Exercises Database")));
	if (connectPage)
		connect(m_exercisesPage, SIGNAL(exerciseChosen()), connectPage, SLOT(newExerciseFromExercisesList()));
}
