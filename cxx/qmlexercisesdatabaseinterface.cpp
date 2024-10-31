#include "qmlexercisesdatabaseinterface.h"

#include "dbexercisesmodel.h"
#include "dbinterface.h"
#include "qmlitemmanager.h"
#include "osinterface.h"
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

const uint QmlExercisesDatabaseInterface::removeExercise(const uint row)
{
	appDBInterface()->removeExercise(appExercisesModel()->actualIndex(row));
	appExercisesModel()->removeExercise(row);
	return row > 0 ? row - 1 : 0;
}

QString QmlExercisesDatabaseInterface::exerciseNameLabel() const { return appExercisesModel()->columnLabel(EXERCISES_COL_MAINNAME); }
QString QmlExercisesDatabaseInterface::exerciseSubNameLabel() const { return appExercisesModel()->columnLabel(EXERCISES_COL_SUBNAME); }
QString QmlExercisesDatabaseInterface::muscularGroupLabel() const { return appExercisesModel()->columnLabel(EXERCISES_COL_MUSCULARGROUP); }
QString QmlExercisesDatabaseInterface::setsNumberLabel() const { return appExercisesModel()->columnLabel(EXERCISES_COL_SETSNUMBER); }
QString QmlExercisesDatabaseInterface::repsNumberLabel() const { return appExercisesModel()->columnLabel(EXERCISES_COL_REPSNUMBER); }
QString QmlExercisesDatabaseInterface::weightLabel() const { return appExercisesModel()->columnLabel(EXERCISES_COL_WEIGHT); }
QString QmlExercisesDatabaseInterface::mediaLabel() const { return appExercisesModel()->columnLabel(EXERCISES_COL_MEDIAPATH); }

void QmlExercisesDatabaseInterface::exportExercises(const bool bShare)
{
	int exportFileMessageId(0);
	if (appExercisesModel()->collectExportData())
	{
		const QString& exportFileName{appOsInterface()->appDataFilesPath() + tr("TrainingPlanner Exercises List.txt")};
		exportFileMessageId = appExercisesModel()->exportToFile(exportFileName);
		if (exportFileMessageId >= 0)
		{
			if (bShare)
			{
				appOsInterface()->shareFile(exportFileName);
				exportFileMessageId = APPWINDOW_MSG_SHARE_OK;
			}
			else
				QMetaObject::invokeMethod(m_mainWindow, "chooseFolderToSave", Q_ARG(QString, exportFileName));
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
		QMetaObject::invokeMethod(m_mainWindow, "chooseFileToImport");
	else
		appItemManager()->openRequestedFile(filename, IFC_EXERCISES);
}

void QmlExercisesDatabaseInterface::getExercisesPage(const bool bChooseButtonEnabled, QQuickItem* connectPage)
{
	if (!m_exercisesComponent)
	{
		if (appExercisesModel()->count() == 0)
			appDBInterface()->getAllExercises();
		createExercisesPage(bChooseButtonEnabled, connectPage);
	}
	else
	{
		m_exercisesPage->setProperty("bChooseButtonEnabled", bChooseButtonEnabled);
		appExercisesModel()->clearSelectedEntries();
		if (connectPage)
		{
			disconnect(m_exercisesPage, SIGNAL(exerciseChosen()), nullptr, nullptr);
			connect(m_exercisesPage, SIGNAL(exerciseChosen()), connectPage, SLOT(gotExercise()));
		}
		QMetaObject::invokeMethod(m_mainWindow, "pushOntoStack", Q_ARG(QQuickItem*, m_exercisesPage));
	}
}

void QmlExercisesDatabaseInterface::createExercisesPage(const bool bChooseButtonEnabled, QQuickItem* connectPage)
{
	m_exercisesComponent = new QQmlComponent{m_qmlEngine, QUrl{u"qrc:/qml/Pages/ExercisesPage.qml"_s}, QQmlComponent::Asynchronous};
	m_exercisesProperties.insert(u"bChooseButtonEnabled"_s, bChooseButtonEnabled);
	m_exercisesProperties.insert(u"exercisesManager"_s, QVariant::fromValue(this));

	if (m_exercisesComponent->status() != QQmlComponent::Ready)
	{
		connect(m_exercisesComponent, &QQmlComponent::statusChanged, this, [this,connectPage] (QQmlComponent::Status) {
			return createExercisesPage_part2(connectPage);
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
	}
	else
		createExercisesPage_part2(connectPage);
}

void QmlExercisesDatabaseInterface::createExercisesPage_part2(QQuickItem* connectPage)
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
		m_exercisesPage = static_cast<QQuickItem*>(m_exercisesComponent->createWithInitialProperties(m_exercisesProperties, m_qmlEngine->rootContext()));
		m_qmlEngine->setObjectOwnership(m_exercisesPage, QQmlEngine::CppOwnership);
		m_exercisesPage->setParentItem(m_mainWindow->contentItem());
		appExercisesModel()->clearSelectedEntries();
		QMetaObject::invokeMethod(m_mainWindow, "pushOntoStack", Q_ARG(QQuickItem*, m_exercisesPage));
		if (connectPage)
			connect(m_exercisesPage, SIGNAL(exerciseChosen()), connectPage, SLOT(gotExercise()));

		connect(appExercisesModel(), &DBExercisesModel::exerciseChanged, this, [this] (const uint index) {
			appDBInterface()->saveExercises();
		});

		connect(appTr(), &TranslationClass::applicationLanguageChanged, this, [this] () {
			appExercisesModel()->fillColumnNames();
		emit labelsChanged();
	});
	}
}
