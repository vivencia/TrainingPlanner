/****************************************************************************
** Generated QML type registration code
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <QtQml/qqml.h>
#include <QtQml/qqmlmoduleregistration.h>

#include <dbexercisesmodel.h>
#include <dbmesocalendarmodel.h>
#include <dbmesocyclesmodel.h>
#include <dbmesosplitmodel.h>
#include <dbtrainingdaymodel.h>
#include <graph.h>
#include <tplistmodel.h>


#if !defined(QT_STATIC)
#define Q_QMLTYPE_EXPORT Q_DECL_EXPORT
#else
#define Q_QMLTYPE_EXPORT
#endif
Q_QMLTYPE_EXPORT void qml_register_types_Graph()
{
    qmlRegisterTypesAndRevisions<DBExercisesModel>("Graph", 1);
    qmlRegisterAnonymousType<QAbstractItemModel, 254>("Graph", 1);
    qmlRegisterTypesAndRevisions<DBMesoCalendarModel>("Graph", 1);
    qmlRegisterTypesAndRevisions<DBMesoSplitModel>("Graph", 1);
    qmlRegisterTypesAndRevisions<DBMesocyclesModel>("Graph", 1);
    qmlRegisterTypesAndRevisions<DBTrainingDayModel>("Graph", 1);
    qmlRegisterTypesAndRevisions<Graph>("Graph", 1);
    qmlRegisterAnonymousType<QQuickItem, 254>("Graph", 1);
    qmlRegisterTypesAndRevisions<TPListModel>("Graph", 1);
    qmlRegisterModule("Graph", 1, 0);
}

static const QQmlModuleRegistration graphRegistration("Graph", qml_register_types_Graph);
