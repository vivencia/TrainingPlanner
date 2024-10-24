#ifndef TPGLOBALS_H
#define TPGLOBALS_H

#include <QString>

using namespace Qt::Literals::StringLiterals;

static const QString& TP_APP_VERSION(u"v20241005-A"_s);

#ifndef QT_NO_DEBUG
#include <QDebug>
#include <source_location>
#define SOURCE_LOCATION {std::source_location::current()}
#define DEFINE_SOURCE_LOCATION const std::source_location& location SOURCE_LOCATION;;
#define PRINT_SOURCE_LOCATION qDebug().noquote() << location.file_name() << u"::"_s << location.function_name() << u"::"_s << location.line();
#define SUCCESS_MESSAGE(message1, message2) { qDebug() << ' '; qDebug() << u"******SUCCESS******"_s; qDebug() << message << message2; \
			qDebug() << u"******SUCCESS******"_s; qDebug() << ' '; }
#define SUCCESS_MESSAGE_WITH_STATEMENT(statement) { qDebug() << ' '; qDebug() << u"******SUCCESS******"_s; \
			statement; qDebug() << u"******SUCCESS******"_s; qDebug() << ' '; }
#define ERROR_MESSAGE(message1, message2) { qDebug() << ' '; qDebug() << u"******ERROR******"_s; \
			PRINT_SOURCE_LOCATION; qDebug() << message1 << message2; qDebug() << u"******ERROR******"_s; qDebug() << ' '; }
#define LOG_MESSAGE(message) qDebug() << message;
#else
#define SOURCE_LOCATION
#define DEFINE_SOURCE_LOCATION
#define PRINT_SOURCE_LOCATION
#define SUCCESS_MESSAGE(message1, message2)
#define SUCCESS_MESSAGE_WITH_STATEMENT(statement)
#define ERROR_MESSAGE(message1, message2)
#define LOG_MESSAGE(message)
#endif

static const QLatin1Char record_separator(28);
static const QLatin1Char exercises_separator(29);
static const QLatin1Char comp_exercise_separator(30);
static const QLatin1Char set_separator(31);
static const QLatin1Char fancy_record_separator1('|');
static const QLatin1Char fancy_record_separator2(';');
static const QString& comp_exercise_fancy_separator(u" + "_s);

static const uint APP_TABLES_NUMBER(6);
static const uint EXERCISES_TABLE_ID(0x0001);
static const uint MESOCYCLES_TABLE_ID(0x0002);
static const uint MESOSPLIT_TABLE_ID(0x0003);
static const uint MESOCALENDAR_TABLE_ID(0x0004);
static const uint TRAININGDAY_TABLE_ID(0x0005);
static const uint USER_TABLE_ID(0x0006);

static const QString& DBExercisesObjectName(u"Exercises"_s);
static const QString& DBMesocyclesObjectName(u"Mesocycles"_s);
static const QString& DBMesoSplitObjectName(u"MesocyclesSplits"_s);
static const QString& DBMesoCalendarObjectName(u"MesoCalendar"_s);
static const QString& DBTrainingDayObjectName(u"TrainingDay"_s);
static const QString& DBUserObjectName(u"UserProfile"_s);

static const QString& STR_MINUS_ONE(u"-1"_s);
static const QString& STR_ZERO(u"0"_s);
static const QString& STR_ONE(u"1"_s);
static const QString& STR_END_EXPORT(u"####\n\n"_s);

#ifdef Q_OS_ANDROID
static const QString& FONT_POINT_SIZE(u"18"_s);
static const QString& FONT_POINT_SIZE_LISTS(u"14"_s);
static const QString& FONT_POINT_SIZE_TEXT(u"16"_s);
static const QString& FONT_POINT_SIZE_TITLE(u"23"_s);
#else
static const QString& FONT_POINT_SIZE(u"12"_s);
static const QString& FONT_POINT_SIZE_LISTS(u"8"_s);
static const QString& FONT_POINT_SIZE_TEXT(u"10"_s);
static const QString& FONT_POINT_SIZE_TITLE(u"18"_s);
#endif

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

#define SET_TYPE_REGULAR 0
#define SET_TYPE_PYRAMID 1
#define SET_TYPE_DROP 2
#define SET_TYPE_CLUSTER 3
#define SET_TYPE_GIANT 4
#define SET_TYPE_MYOREPS 5
#define SET_TYPE_REVERSE_PYRAMID 6

template <typename T>
inline void setBit (T& __restrict var, const unsigned char bit)
{
	if ((bit - 1) >= 0)
		var |= (2 << (bit - 1));
	else
		var |= 1;
}

template <typename T>
inline void unSetBit (T& __restrict var, const unsigned char bit)
{
	if ((bit - 1) >= 0)
		var &= ~(2 << (bit - 1));
	else
		var &= ~1;
}
#endif // TPGLOBALS_H
