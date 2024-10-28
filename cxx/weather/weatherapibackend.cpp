// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "weatherapibackend.h"
#include "../tpglobals.h"

#include <QtCore/qjsonarray.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qjsonvalue.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qurlquery.h>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qnetworkreply.h>
#include <QtPositioning/qgeocoordinate.h>

static QString niceTemperatureString(double t)
{
	return QString::number(qRound(t)) + QChar(0xB0);
}

/*
	Converts weather code to a string that will be used to show the icon.
	The possible strings are based on the icon names. The icon name is built up
	as follows:
		weather-[mystring].svg
	where [mystring] is the value returned by this method.
*/
static QString weatherCodeToString(int code)
{
	switch (code)
	{
		case 1000: return u"sunny"_s;
		case 1003: return u"sunny-very-few-clouds"_s;
		case 1006: return u"few-clouds"_s;
		case 1009: return u"overcast"_s;
		case 1030:
		case 1135:
		case 1147: return u"fog"_s;
		case 1063:
		case 1072:
		case 1150:
		case 1153:
		case 1168:
		case 1171:
		case 1180:
		case 1183:
		case 1186:
		case 1189:
		case 1198: return u"showers-scattered"_s;
		case 1066:
		case 1069:
		case 1114:
		case 1117:
		case 1210:
		case 1213:
		case 1216:
		case 1219:
		case 1222:
		case 1225:
		case 1237:
		case 1255:
		case 1258:
		case 1261:
		case 1264:
		case 1279:
		case 1282: return u"snow"_s;
		case 1087: return u"storm"_s;
		case 1192:
		case 1195:
		case 1201:
		case 1240:
		case 1243:
		case 1246: return u"showers"_s;
		case 1204:
		case 1207:
		case 1249:
		case 1252: return u"sleet"_s;
		case 1273:
		case 1276: return u"thundershower"_s;
		default: return u"sunny"_s;
	}
}

static void parseWeatherDescription(const QJsonObject& object, st_WeatherInfo &info)
{
	const QJsonObject& conditionObject{object.value(u"condition").toObject()};
	info.m_weatherDescription = std::move(conditionObject.value(u"text").toString());
	info.m_weatherIconId = weatherCodeToString(conditionObject.value(u"code").toInt());
}

WeatherApiBackend::WeatherApiBackend(QObject *parent)
	: ProviderBackend{parent},
	  m_networkManager{new QNetworkAccessManager(this)}, m_apiKey{u"8edde160c63c4be6b77101828211208"_s}
{}

void WeatherApiBackend::requestWeatherInfo(const QString& city)
{
	generateWeatherRequest(city, QGeoCoordinate());
}

void WeatherApiBackend::requestWeatherInfo(const QGeoCoordinate& coordinate)
{
	const QString& coordinateStr{u"%1,%2"_s.arg(coordinate.latitude()).arg(coordinate.longitude())};
	generateWeatherRequest(coordinateStr, coordinate);
}

void WeatherApiBackend::handleWeatherForecastReply(QNetworkReply* reply, const QGeoCoordinate& coordinate)
{
	if (!reply)
	{
		emit errorOccurred();
		return;
	}
	bool parsed = false;
	if (!reply->error())
	{
		QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
		const QJsonObject documentObject = document.object();
		const QJsonObject locationObject = documentObject.value(u"location").toObject();

		// extract location name
		st_LocationInfo location;
		location.m_name = std::move(locationObject.value(u"name").toString());
		if (coordinate.isValid())
			location.m_coordinate = coordinate;

		// extract current weather
		st_WeatherInfo currentWeather;
		const QJsonObject& currentWeatherObject{documentObject.value(u"current").toObject()};
		const QJsonValue& temperature{currentWeatherObject.value(u"temp_c")};
		if (temperature.isDouble())
			currentWeather.m_temperature = niceTemperatureString(temperature.toDouble());

		parseWeatherDescription(currentWeatherObject, currentWeather);

		parsed = !location.m_name.isEmpty() && !currentWeather.m_temperature.isEmpty();

		if (parsed)
		{
			QList<st_WeatherInfo> weatherDetails;
			weatherDetails << currentWeather;
			const QJsonObject& forecastObject{documentObject.value(u"forecast").toObject()};
			const QJsonArray& forecastDays{forecastObject.value(u"forecastday").toArray()};
			for (qsizetype i(0); i < forecastDays.size(); ++i) {
				const QJsonObject& dayInfo{forecastDays.at(i).toObject()};
				st_WeatherInfo info;

				const QDateTime& dt{QDateTime::fromSecsSinceEpoch(dayInfo.value(u"date_epoch").toInteger())};
				info.m_dayOfWeek = std::move(dt.toString(u"ddd"));

				const QJsonObject& dayObject = dayInfo.value(u"day").toObject();
				const QJsonValue& minTemp{dayObject.value(u"mintemp_c")};
				const QJsonValue& maxTemp{dayObject.value(u"maxtemp_c")};
				if (minTemp.isDouble() && maxTemp.isDouble())
					info.m_temperature = niceTemperatureString(minTemp.toDouble()) + QChar('/') + niceTemperatureString(maxTemp.toDouble());

				parseWeatherDescription(dayObject, info);

				if (!info.m_temperature.isEmpty() && !info.m_weatherIconId.isEmpty())
					weatherDetails.push_back(info);
			}
			emit weatherInformation(location, weatherDetails);
		}
	}
	if (!parsed)
	{
		emit errorOccurred();
		DEFINE_SOURCE_LOCATION
		if (reply->error())
			ERROR_MESSAGE("Reply error.", reply->errorString())
		else
			ERROR_MESSAGE("Failed to parse current weather JSON.", QString());
	}

	reply->deleteLater();
}

void WeatherApiBackend::generateWeatherRequest(const QString& locationString, const QGeoCoordinate& coordinate)
{
	QUrl url{u"https://api.weatherapi.com/v1/forecast.json"_s};

	QUrlQuery query;
	query.addQueryItem(u"key"_s, m_apiKey);
	query.addQueryItem(u"q"_s, locationString);
	query.addQueryItem(u"days"_s, u"4"_s);
	query.addQueryItem(u"aqi"_s, u"no"_s);
	query.addQueryItem(u"alerts"_s, u"no"_s);
	url.setQuery(query);

	QNetworkReply* reply{m_networkManager->get(QNetworkRequest{url})};
	connect(reply, &QNetworkReply::finished, this, [this, reply, coordinate]() { handleWeatherForecastReply(reply, coordinate); });
}
