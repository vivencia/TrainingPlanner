#include "qmlexercisesdatabaseinterface.h"

#include "dbexerciseslistmodel.h"
#include "pageslistmodel.h"
#include "qmlitemmanager.h"
#include "qmlworkoutinterface.h"
#include "tputils.h"

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
		const QString &exportFileName{appSettings()->localAppFilesDir() % tr("TrainingPlanner Exercises List") % TPUtils::TP_FILE_EXTENSION};
		exportFileMessageId = appExercisesList()->exportToFile(exportFileName);
		if (exportFileMessageId >= 0) {
#ifdef Q_OS_ANDROID
			if (bShare) {
				appOsInterface()->shareFile(exportFileName);
				exportFileMessageId = TP_RET_CODE_SHARE_OK;
			}
			else
#endif
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
		appItemManager()->chooseFileToImport();
	else
		appUtils()->viewOrOpenFile(filename, TPUtils::FT_TP_EXERCISES);
}

void QmlExercisesDatabaseInterface::getExercisesPage(QmlWorkoutInterface *connect_page)
{
	if (!m_exercisesComponent) {
		m_exercisesProperties["exercisesManager"_L1] = std::move(QVariant::fromValue(this));
		m_exercisesProperties["chooseButtonEnabled"_L1] = std::move(connect_page != nullptr);
		m_exercisesComponent = new QQmlComponent{appQmlEngine(), "TpQml.Pages"_L1, "ExercisesListPage", QQmlComponent::Asynchronous};
		connect(m_exercisesComponent, &QQmlComponent::statusChanged, this, [this,connect_page] (QQmlComponent::Status status) {
			getExercisesPage(connect_page);
		});
	}
	else {
		if (!m_exercisesPage) {
			switch (m_exercisesComponent->status()) {
			case QQmlComponent::Ready:
				m_exercisesComponent->disconnect();
				createExercisesPage(connect_page);
				break;
#ifndef QT_NO_DEBUG
			case QQmlComponent::Loading:
				break;
			case QQmlComponent::Null:
			case QQmlComponent::Error:
				qDebug() << m_exercisesComponent->errorString();
				break;
#else
			default: break;
#endif
			}
		}
		else {
			m_exercisesPage->setProperty("chooseButtonEnabled", connect_page != nullptr);
			appExercisesList()->clearSelectedEntries();
			if (connect_page) {
				disconnect(m_exercisesPage, SIGNAL(exerciseChosen()), nullptr, nullptr);
				connect(m_exercisesPage, SIGNAL(exerciseChosen()), connect_page, SLOT(newExerciseChosen()));
			}
			appPagesListModel()->openPage(m_exercisesPage);
		}
	}
}

void QmlExercisesDatabaseInterface::createExercisesPage(QmlWorkoutInterface *connect_page)
{
	m_exercisesPage = static_cast<QQuickItem*>(m_exercisesComponent->createWithInitialProperties(m_exercisesProperties,
																								  appQmlEngine()->rootContext()));
#ifndef QT_NO_DEBUG
	if (!m_exercisesPage) {
		m_exercisesComponent->errorString();
		return;
	}
#endif
	appQmlEngine()->setObjectOwnership(m_exercisesPage, QQmlEngine::CppOwnership);
	m_exercisesPage->setParentItem(appItemManager()->AppPagesVisualParent());
	appExercisesList()->clearSelectedEntries();
	appPagesListModel()->openPage(m_exercisesPage, std::move(tr("Exercises Database")));
	if (connect_page)
		connect(m_exercisesPage, SIGNAL(exerciseChosen()), connect_page, SLOT(newExerciseFromExercisesList()));
}
