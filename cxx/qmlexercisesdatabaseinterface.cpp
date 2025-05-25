#include "qmlexercisesdatabaseinterface.h"

#include "dbexerciseslistmodel.h"
#include "dbinterface.h"
#include "qmlitemmanager.h"
#include "qmlworkoutinterface.h"
#include "osinterface.h"
#include "tputils.h"
#include "translationclass.h"

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>

QmlExercisesDatabaseInterface::~QmlExercisesDatabaseInterface()
{
	delete m_exercisesPage;
	delete m_exercisesComponent;
}

void QmlExercisesDatabaseInterface::saveExercise()
{
	appDBInterface()->saveExercises();
}

const uint QmlExercisesDatabaseInterface::removeExercise(const uint row)
{
	appDBInterface()->removeExercise(appExercisesList()->actualIndex(row));
	appExercisesList()->removeExercise(row);
	return row > 0 ? row - 1 : 0;
}

QString QmlExercisesDatabaseInterface::exerciseNameLabel() const { return appExercisesList()->columnLabel(EXERCISES_LIST_COL_MAINNAME); }
QString QmlExercisesDatabaseInterface::exerciseSubNameLabel() const { return appExercisesList()->columnLabel(EXERCISES_LIST_COL_SUBNAME); }
QString QmlExercisesDatabaseInterface::muscularGroupLabel() const { return appExercisesList()->columnLabel(EXERCISES_LIST_COL_MUSCULARGROUP); }
QString QmlExercisesDatabaseInterface::setsNumberLabel() const { return appExercisesList()->columnLabel(EXERCISES_LIST_COL_SETSNUMBER); }
QString QmlExercisesDatabaseInterface::repsNumberLabel() const { return appExercisesList()->columnLabel(EXERCISES_LIST_COL_REPSNUMBER); }
QString QmlExercisesDatabaseInterface::weightLabel() const { return appExercisesList()->columnLabel(EXERCISES_LIST_COL_WEIGHT); }
QString QmlExercisesDatabaseInterface::mediaLabel() const { return appExercisesList()->columnLabel(EXERCISES_LIST_COL_MEDIAPATH); }

void QmlExercisesDatabaseInterface::exportExercises(const bool bShare)
{
	int exportFileMessageId{0};
	if (appExercisesList()->collectExportData())
	{
		const QString& exportFileName{appUtils()->localAppFilesDir() + tr("TrainingPlanner Exercises List") + ".txt"_L1};
		exportFileMessageId = appExercisesList()->exportToFile(exportFileName);
		if (exportFileMessageId >= 0)
		{
			if (bShare)
			{
				appOsInterface()->shareFile(exportFileName);
				exportFileMessageId = APPWINDOW_MSG_SHARE_OK;
			}
			else
				QMetaObject::invokeMethod(appMainWindow(), "chooseFolderToSave", Q_ARG(QString, exportFileName));
		}
		emit displayMessageOnAppWindow(exportFileMessageId, exportFileName);
	}
	else
		exportFileMessageId = APPWINDOW_MSG_NOTHING_TO_EXPORT;
	emit displayMessageOnAppWindow(exportFileMessageId);
}

void QmlExercisesDatabaseInterface::importExercises(const QString& filename)
{
	if (filename.isEmpty())
		QMetaObject::invokeMethod(appMainWindow(), "chooseFileToImport", Q_ARG(int, IFC_EXERCISES));
	else
		appItemManager()->openRequestedFile(filename, IFC_EXERCISES);
}

void QmlExercisesDatabaseInterface::getExercisesPage(QmlWorkoutInterface* connectPage)
{
	if (!m_exercisesComponent)
	{
		if (appExercisesList()->count() == 0)
			appDBInterface()->getAllExercises();
		createExercisesPage(connectPage);
	}
	else
	{
		m_exercisesPage->setProperty("bChooseButtonEnabled", connectPage != nullptr);
		appExercisesList()->clearSelectedEntries();
		if (connectPage)
		{
			disconnect(m_exercisesPage, SIGNAL(exerciseChosen()), nullptr, nullptr);
			connect(m_exercisesPage, SIGNAL(exerciseChosen()), connectPage, SLOT(createExerciseObject()));
		}
		QMetaObject::invokeMethod(appMainWindow(), "pushOntoStack", Q_ARG(QQuickItem*, m_exercisesPage));
	}
}

void QmlExercisesDatabaseInterface::createExercisesPage(QmlWorkoutInterface* connectPage)
{
	m_exercisesComponent = new QQmlComponent{appQmlEngine(), QUrl{"qrc:/qml/Pages/ExercisesPage.qml"_L1}, QQmlComponent::Asynchronous};
	m_exercisesProperties.insert("bChooseButtonEnabled"_L1, connectPage != nullptr);
	m_exercisesProperties.insert("exercisesManager"_L1, QVariant::fromValue(this));

	if (m_exercisesComponent->status() != QQmlComponent::Ready)
	{
		connect(m_exercisesComponent, &QQmlComponent::statusChanged, this, [this,connectPage] (QQmlComponent::Status) {
			return createExercisesPage_part2(connectPage);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	}
	else
		createExercisesPage_part2(connectPage);
}

void QmlExercisesDatabaseInterface::createExercisesPage_part2(QmlWorkoutInterface *connectPage)
{
	#ifndef QT_NO_DEBUG
	if (m_exercisesComponent->status() == QQmlComponent::Error)
	{
		qDebug() << m_exercisesComponent->errorString();
		for (uint i(0); i < m_exercisesComponent->errors().count(); ++i)
			qDebug() << m_exercisesComponent->errors().at(i).description();
		return;
	}
	#endif
	if (m_exercisesComponent->status() == QQmlComponent::Ready)
	{
		m_exercisesPage = static_cast<QQuickItem*>(m_exercisesComponent->createWithInitialProperties(m_exercisesProperties, appQmlEngine()->rootContext()));
		appQmlEngine()->setObjectOwnership(m_exercisesPage, QQmlEngine::CppOwnership);
		m_exercisesPage->setParentItem(appMainWindow()->contentItem());
		appExercisesList()->clearSelectedEntries();
		QMetaObject::invokeMethod(appMainWindow(), "pushOntoStack", Q_ARG(QQuickItem*, m_exercisesPage));
		if (connectPage)
			connect(m_exercisesPage, SIGNAL(exerciseChosen()), connectPage, SLOT(createExerciseObject()));

		connect(appTr(), &TranslationClass::applicationLanguageChanged, this, [this] () {
			appExercisesList()->fillColumnNames();
			emit labelsChanged();
		});
	}
}
