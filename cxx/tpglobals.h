#ifndef TPGLOBALS_H
#define TPGLOBALS_H

#include <QString>
#ifdef DEBUG
#include <QDebug>
#endif

static const QString& TP_APP_VERSION(u"v20240818-B"_qs);

#ifdef DEBUG
#define MSG_OUT(message) qDebug() << message;
#else
#define MSG_OUT(message)
#endif

static const QLatin1Char record_separator(29);
static const QLatin1Char record_separator2(30);
static const QLatin1Char subrecord_separator(31);
static const QLatin1Char fancy_record_separator2(';');

static const uint APP_TABLES_NUMBER(6);
static const uint EXERCISES_TABLE_ID(0x0001);
static const uint MESOCYCLES_TABLE_ID(0x0002);
static const uint MESOSPLIT_TABLE_ID(0x0003);
static const uint MESOCALENDAR_TABLE_ID(0x0004);
static const uint TRAININGDAY_TABLE_ID(0x0005);
static const uint USER_TABLE_ID(0x0006);

static const QString& DBExercisesObjectName(u"Exercises"_qs);
static const QString& DBMesocyclesObjectName(u"Mesocycles"_qs);
static const QString& DBMesoSplitObjectName(u"MesocyclesSplits"_qs);
static const QString& DBMesoCalendarObjectName(u"MesoCalendar"_qs);
static const QString& DBTrainingDayObjectName(u"TrainingDay"_qs);
static const QString& DBUserObjectName(u"UserProfile"_qs);

static const QString& STR_MINUS_ONE(u"-1"_qs);
static const QString& STR_ZERO(u"0"_qs);
static const QString& STR_ONE(u"1"_qs);
static const QString& STR_END_EXPORT(u"####\n\n"_qs);

enum {
	IFC_USER = 0x01,
	IFC_MESO = 0x02,
	IFC_MESOSPLIT = 0x04,
	IFC_EXERCISES = 0x08,
	IFC_TDAY = 0x10,
	IFC_ANY= 0x1F
} typedef importFileContents;

#define APPWINDOW_MSG_IMPORT_OK 3
#define APPWINDOW_MSG_EXPORT_OK 2
#define APPWINDOW_MSG_SHARE_OK 1
#define APPWINDOW_MSG_READ_FROM_FILE_OK 0
#define APPWINDOW_MSG_OPEN_FAILED -1
#define APPWINDOW_MSG_UNKNOWN_FILE_FORMAT -2
#define APPWINDOW_MSG_CORRUPT_FILE -3
#define APPWINDOW_MSG_NOTHING_TODO -4
#define APPWINDOW_MSG_NO_MESO -5
#define APPWINDOW_MSG_NOTHING_TO_EXPORT -6
#define APPWINDOW_MSG_SHARE_FAILED -7
#define APPWINDOW_MSG_EXPORT_FAILED -8
#define APPWINDOW_MSG_IMPORT_FAILED -9
#define APPWINDOW_MSG_OPEN_CREATE_FILE_FAILED -10
#define APPWINDOW_MSG_WRONG_IMPORT_FILE_TYPE -11
#define APPWINDOW_MSG_UNKNOWN_ERROR -100

#ifdef Q_OS_ANDROID
	#define FONT_POINT_SIZE 15
	#define FONT_POINT_SIZE_LISTS 11
	#define FONT_POINT_SIZE_TEXT 13
	#define FONT_POINT_SIZE_TITLE 20
#else
	#define FONT_POINT_SIZE 12
	#define FONT_POINT_SIZE_LISTS 8
	#define FONT_POINT_SIZE_TEXT 10
	#define FONT_POINT_SIZE_TITLE 18
#endif

#endif // TPGLOBALS_H
