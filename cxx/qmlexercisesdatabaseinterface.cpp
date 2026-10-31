#include "qmlexercisesdatabaseinterface.h"

#include "dbexerciseslistmodel.h"
#include "pageslistmodel.h"
#include "qmlitemmanager.h"
#include "qmlworkoutinterface.h"

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>

void QmlExercisesDatabaseInterface::getExercisesPage(QmlWorkoutInterface *connect_page)
{
	if (!m_exercisesComponent) {
		m_exercisesComponent = new QQmlComponent{appQmlEngine(), "TpQml.Pages"_L1, "ExercisesListPage", QQmlComponent::Asynchronous};
		connect(m_exercisesComponent, &QQmlComponent::statusChanged, this, [this,connect_page] (QQmlComponent::Status status) {
			getExercisesPage(connect_page);
		});
	} else {
		if (!m_exercisesPage) {
			switch (m_exercisesComponent->status()) {
			case QQmlComponent::Ready:
				m_exercisesComponent->disconnect();
				m_exercisesPage = static_cast<QQuickItem*>(m_exercisesComponent->createWithInitialProperties(
								{{"exercisesManager"_L1, QVariant::fromValue(this)}}, appQmlEngine()->rootContext()));
#ifndef QT_NO_DEBUG
				if (!m_exercisesPage) {
					m_exercisesComponent->errorString();
					return;
				}
#endif
				appQmlEngine()->setObjectOwnership(m_exercisesPage, QQmlEngine::CppOwnership);
				m_exercisesPage->setParentItem(appItemManager()->appPagesVisualParent());
				getExercisesPage(connect_page);
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
		} else {
			m_exercisesPage->setProperty("chooseButtonEnabled", connect_page != nullptr);
			appExercisesList()->clearSelection();
			if (connect_page) {
				disconnect(m_exercisesPage, SIGNAL(exerciseChosen()), nullptr, nullptr);
				connect(m_exercisesPage, SIGNAL(exerciseChosen()), connect_page, SLOT(newExerciseChosen()));
			}
			appPagesListModel()->openPage(m_exercisesPage, std::move(tr("Exercises Database")));
		}
	}
}
