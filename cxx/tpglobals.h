#pragma once

#include <QString>

using namespace Qt::Literals::StringLiterals;

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
