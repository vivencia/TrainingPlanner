#pragma once

#include <QString>

using namespace Qt::Literals::StringLiterals;

static const QString &TP_APP_VERSION("v202504 Alpha 1"_L1);

#ifndef QT_NO_DEBUG
#include <QDebug>
#include <source_location>
#define SOURCE_LOCATION {std::source_location::current()}
#define DEFINE_SOURCE_LOCATION const std::source_location &location SOURCE_LOCATION;;
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

constexpr QLatin1Char record_separator(28);
constexpr QLatin1Char exercises_separator(29);
constexpr QLatin1Char comp_exercise_separator(30);
constexpr QLatin1Char set_separator(31);
constexpr QLatin1Char fancy_record_separator1('|');
constexpr QLatin1Char fancy_record_separator2(';');
constexpr QLatin1StringView comp_exercise_fancy_separator(" + "_L1);

static const QString &STR_MINUS_ONE("-1"_L1);
static const QString &STR_ZERO("0"_L1);
static const QString &STR_ONE("1"_L1);
static const QString &STR_END_EXPORT("####\n\n"_L1);

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
#define APPWINDOW_MSG_DEFERRED_ACTION 2000

static int deferredActionId()
{
	static uint da_id{APPWINDOW_MSG_DEFERRED_ACTION};
	return da_id++;
}

template <typename T>
inline void setBit(T &__restrict var, const unsigned char bit)
{
	if ((bit - 1) >= 0)
		var |= (2 << (bit - 1));
	else
		var |= 1;
}

template <typename T>
inline void unSetBit(T &__restrict var, const unsigned char bit)
{
	if ((bit - 1) >= 0)
		var &= ~(2 << (bit - 1));
	else
		var &= ~1;
}

template <typename T>
inline bool isBitSet(const T &__restrict var, const unsigned char bit)
{
	if ((bit - 1) >= 0)
		return static_cast<bool>(var & (2 << (bit - 1)));
	else
		return static_cast<bool>(var & 1);
}
