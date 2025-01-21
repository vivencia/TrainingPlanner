#ifndef TPGLOBALS_H
#define TPGLOBALS_H

#include <QString>

using namespace Qt::Literals::StringLiterals;

static const QString& TP_APP_VERSION("v20250106-A"_L1);

#ifndef QT_NO_DEBUG
#include <QDebug>
#include <source_location>
#define SOURCE_LOCATION {std::source_location::current()}
#define DEFINE_SOURCE_LOCATION const std::source_location& location SOURCE_LOCATION;;
#define PRINT_SOURCE_LOCATION qDebug().noquote() << location.file_name() << "::"_L1 << location.function_name() << "::"_L1 << location.line();
#define SUCCESS_MESSAGE(message1, message2) { qDebug() << ' '; qDebug() << "******SUCCESS******"_L1; qDebug() << message << message2; \
			qDebug() << "******SUCCESS******"_L1; qDebug() << ' '; }
#define SUCCESS_MESSAGE_WITH_STATEMENT(statement) { qDebug() << ' '; qDebug() << "******SUCCESS******"_L1; \
			statement; qDebug() << "******SUCCESS******"_L1; qDebug() << ' '; }
#define ERROR_MESSAGE(message1, message2) { qDebug() << ' '; qDebug() << "******ERROR******"_L1; \
			PRINT_SOURCE_LOCATION; qDebug() << message1 << message2; qDebug() << "******ERROR******"_L1; qDebug() << ' '; }
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
static const QString& comp_exercise_fancy_separator(" + "_L1);

static const uint APP_TABLES_NUMBER(6);
static const uint EXERCISES_TABLE_ID(0x0001);
static const uint MESOCYCLES_TABLE_ID(0x0002);
static const uint MESOSPLIT_TABLE_ID(0x0003);
static const uint MESOCALENDAR_TABLE_ID(0x0004);
static const uint TRAININGDAY_TABLE_ID(0x0005);
static const uint USER_TABLE_ID(0x0006);

static const QString& DBExercisesObjectName("Exercises"_L1);
static const QString& DBMesocyclesObjectName("Mesocycles"_L1);
static const QString& DBMesoSplitObjectName("MesocyclesSplits"_L1);
static const QString& DBMesoCalendarObjectName("MesoCalendar"_L1);
static const QString& DBTrainingDayObjectName("TrainingDay"_L1);
static const QString& DBUserObjectName("UserProfile"_L1);

static const QString& STR_MINUS_ONE("-1"_L1);
static const QString& STR_ZERO("0"_L1);
static const QString& STR_ONE("1"_L1);
static const QString& STR_END_EXPORT("####\n\n"_L1);

enum {
	IFC_USER = 1,
	IFC_MESO = 2,
	IFC_MESOSPLIT = 3,
		IFC_MESOSPLIT_A = 4,
		IFC_MESOSPLIT_B = 5,
		IFC_MESOSPLIT_C = 6,
		IFC_MESOSPLIT_D = 7,
		IFC_MESOSPLIT_E = 8,
		IFC_MESOSPLIT_F = 9,
	IFC_EXERCISES = 10,
	IFC_TDAY = 11,
	IFC_ANY = 12 //This value is needed for QmlItemManager but this .h file is not included in qmlitemmanager.h. Remember: change here, change there
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
#define APPWINDOW_MSG_IMPORT_FAILED_SAME_DATA -9
#define APPWINDOW_MSG_IMPORT_CANCELED -10
#define APPWINDOW_MSG_IMPORT_FAILED -11
#define APPWINDOW_MSG_OPEN_CREATE_FILE_FAILED -12
#define APPWINDOW_MSG_WRONG_IMPORT_FILE_TYPE -13
#define APPWINDOW_MSG_UNKNOWN_ERROR -100
#define APPWINDOW_MSG_CUSTOM_MESSAGE 1000
#define APPWINDOW_MSG_CUSTOM_WARNING 1001
#define APPWINDOW_MSG_CUSTOM_ERROR 1002

#define SET_TYPE_REGULAR 0
#define SET_TYPE_PYRAMID 1
#define SET_TYPE_DROP 2
#define SET_TYPE_CLUSTER 3
#define SET_TYPE_GIANT 4
#define SET_TYPE_MYOREPS 5
#define SET_TYPE_REVERSE_PYRAMID 6

template <typename T>
inline void setBit(T& __restrict var, const unsigned char bit)
{
	if ((bit - 1) >= 0)
		var |= (2 << (bit - 1));
	else
		var |= 1;
}

template <typename T>
inline void unSetBit(T& __restrict var, const unsigned char bit)
{
	if ((bit - 1) >= 0)
		var &= ~(2 << (bit - 1));
	else
		var &= ~1;
}

template <typename T>
inline bool isBitSet(const T& __restrict var, const unsigned char bit)
{
	if ((bit - 1) >= 0)
		return static_cast<bool>(var & (2 << (bit - 1)));
	else
		return static_cast<bool>(var & 1);
}
#endif // TPGLOBALS_H
