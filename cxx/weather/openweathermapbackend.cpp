// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "openweathermapbackend.h"
#include "../tpglobals.h"
#include "../tpsettings.h"
#include "../tputils.h"

#include <QtCore/qjsonarray.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qjsonvalue.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qurlquery.h>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qnetworkreply.h>
#include <QtPositioning/qgeocoordinate.h>

using namespace Qt::Literals::StringLiterals;

static constexpr auto kZeroKelvin = 273.15;

static const QString& key_today{"current"_L1};
static const QString& key_forecast{"daily"_L1};
static const QString& key_date{"dt"_L1};
static const QString& key_sunrise{"sunrise"_L1};
static const QString& key_sunset{"sunset"_L1};
static const QString& key_temp{"temp"_L1};
static const QString& key_tempfeel{"feels_like"_L1};
static const QString& key_min{"min"_L1};
static const QString& key_max{"max"_L1};
static const QString& key_pressure{"pressure"_L1};
static const QString& key_humidity{"humidity"_L1};
static const QString& key_uv{"uvi"_L1};
static const QString& key_wind{"wind_speed"_L1};
static const QString& key_description{"description"_L1};
static const QString& key_icon{"icon"_L1};

static const QString& keys_current{"dt sunrise sunset temp feels_like pressure humidity uvi wind_speed description icon "_L1};
static const QString& keys_daily{"dt sunrise sunset min max pressure humidity uvi wind_speed description icon "_L1};
static const QString& weather_field{"weather"_L1};

class parseOpenWeatherAPIResponse
{

public:
	explicit parseOpenWeatherAPIResponse(const QByteArray& net_response);
	inline const bool parsedOK() const { return m_bParsedOK; }
	inline const uint forecastDays() const { return m_weatherData.count(); }

	inline QString date(const uint day) const { return formatDate(m_weatherData.at(day).value(key_date)); }
	inline QString sunrise(const uint day) const { return formatTime(m_weatherData.at(day).value(key_sunrise)) + u"h"_s; }
	inline QString sunset(const uint day) const { return formatTime(m_weatherData.at(day).value(key_sunset))  + u"h"_s; }
	inline QString temperature(const uint day) const { return formatTemp(m_weatherData.at(day).value(key_temp)); }
	inline QString feel_temperature(const uint day) const { return formatTemp(m_weatherData.at(day).value(key_tempfeel)); }
	inline QString min_temperature(const uint day) const { return formatTemp(m_weatherData.at(day).value(key_min)); }
	inline QString max_temperature(const uint day) const { return formatTemp(m_weatherData.at(day).value(key_max)); }
	inline QString pressure(const uint day) const { return m_weatherData.at(day).value(key_pressure) + u"hPa"_s; }
	inline QString humidity(const uint day) const { return m_weatherData.at(day).value(key_humidity) + u"%"_s; }
	inline QString uv_index(const uint day) const { return u"UV "_s + formatUVI(m_weatherData.at(day).value(key_uv)); }
	inline QString wind_speed(const uint day) const { return m_weatherData.at(day).value(key_date) + u"km/h"_s; }
	inline QString weather_description(const uint day) const { return m_weatherData.at(day).value(key_description); }
	inline QString weather_icon(const uint day) const { return weatherIconCodeToString(m_weatherData.at(day).value(key_icon)); }

private:
	bool m_bParsedOK;
	QList<QMap<QString,QString>> m_weatherData;
	const QString* m_usedKeys;

	inline const bool isCurrentKey(const QString& word)
	{
		if (word == key_today)
		{
			m_usedKeys = &keys_current;
			return true;
		}
		return false;
	}

	inline const bool isDailyKey(const QString& word)
	{
		if (m_usedKeys == &keys_daily)
			return true; //once daily starts, it continues until the end
		if (word == key_forecast)
		{
			m_usedKeys = &keys_daily;
			return true;
		}
		return false;
	}

	inline const bool isKey(const QString& word) const
	{
		return m_usedKeys ? m_usedKeys->contains(word + ' ') : false;
	}

	inline const bool isWeatherSubField(const QString& word) const
	{
		return weather_field == word;
	}

	inline QString formatTemp(const QString& strTemp) const
	{
		return QString::number(qRound(strTemp.toDouble() - kZeroKelvin)) + u"°C"_s; //QChar(0xB0);
	}

	inline QString formatDate(const QString& strDate) const
	{
		const QDateTime& dt{QDateTime::fromSecsSinceEpoch(strDate.toUInt())};
		return appUtils()->appLocale()->toString(dt, u"ddd d/M");
	}

	inline QString formatTime(const QString& strDate) const
	{
		const QDateTime& dt{QDateTime::fromSecsSinceEpoch(strDate.toUInt())};
		return appUtils()->appLocale()->toString(dt, u"HH:mm:ss");
	}

	inline QString formatUVI(const QString& strUVI) const
	{
		QString ret{strUVI.right(2)};
		if (ret.at(0) == '0')
			ret.remove(0, 1);
		return ret;
	}

	QString weatherIconCodeToString(const QString& iconcode) const;
};

parseOpenWeatherAPIResponse::parseOpenWeatherAPIResponse(const QByteArray& net_response)
	: m_usedKeys(nullptr)
{
	if (!net_response.isEmpty())
	{
		QString word, key;
		bool have_key(false), inside_field(false);
		QByteArray::const_iterator itr(net_response.constBegin());
		const QByteArray::const_iterator itr_end(net_response.constEnd());
		do {
			if (QChar::isLetterOrNumber(*itr))
				word.append(*itr);
			else
			{
				switch (*itr)
				{
					case '{':
						inside_field = isCurrentKey(word) || isDailyKey(word) || isWeatherSubField(word);
						word.clear();
					break;
					case ':':
						if (inside_field && isKey(word))
						{
							if (word == u"dt"_s)
							{
								if (m_weatherData.count() < 5)
								{
									QMap<QString,QString> data;
									m_weatherData.append(std::move(data));
								}
								else
									return;
							}
							key = std::move(word);
							have_key = true;
							word.clear();
						}
					break;

					case '-':
					case '.':
					case ' ':
					case '_':
						word.append(*itr);
					break;
					case ',':
						if (have_key)
						{
							QMap<QString,QString>& parsedData = m_weatherData.last();
							parsedData.insert(std::move(key), std::move(word));
							have_key = false;
						}
						word.clear();
					break;

					default: continue;
				}
			}
		} while (++itr != itr_end);
	}
	m_bParsedOK = m_weatherData.count() > 0;
}

/*
	Converts weather code to a string that will be used to show the icon.
	The possible strings are based on the icon names. The icon name is built up
	as follows: weather-[mystring].svg
	where [mystring] is the value returned by this method.
*/
QString parseOpenWeatherAPIResponse::weatherIconCodeToString(const QString& code) const
{
	if (code == u"01d"_s || code == u"01n"_s)
		return u"sunny"_s;
	else if (code == u"02d"_s || code == u"02n"_s)
		return u"sunny-very-few-clouds"_s;
	else if (code == u"03d"_s || code == u"03n"_s)
		return u"few-clouds"_s;
	else if (code == u"04d"_s || code == u"04n"_s)
		return u"overcast"_s;
	else if (code == u"09d"_s || code == u"09n"_s || code == u"10d"_s || code == u"10n"_s)
		return u"showers"_s;
	else if (code == u"11d"_s || code == u"11n"_s)
		return u"thundershower"_s;
	else if (code == u"13d"_s || code == u"13n"_s)
		return u"snow"_s;
	else if (code == u"50d"_s || code == u"50n"_s)
		return u"fog"_s;

	return u"sunny"_s; // default choice
}

static QMap<QString,QString> parseApiNinjasReply(const QByteArray& replyData)
{
	QMap<QString,QString> parsedData;
	if (!replyData.isEmpty())
	{
		QString word, key;
		QByteArray::const_iterator itr(replyData.constBegin());
		const QByteArray::const_iterator itr_end(replyData.constEnd());
		do {
			if (QChar::isLetterOrNumber(*itr))
			{
				word.append(*itr);
			}
			else
			{
				if (*itr == '-' || *itr == '.')
					word.append(*itr);
				else if (*itr == ':')
				{
					key = std::move(word);
					word.clear();
				}
				else if (*itr == ',')
				{
					parsedData.insert(key, word);
					word.clear();
				}
			}
		} while (++itr != itr_end);
	}
	return parsedData;
}

static void parseWeatherDescription(const QJsonObject& object, st_WeatherInfo& info)
{
	const QJsonArray& weatherArray{object.value(u"weather").toArray()};
	if (!weatherArray.isEmpty())
	{
		const QJsonObject& obj{weatherArray.first().toObject()};
		info.m_weatherDescription = std::move(obj.value(u"description").toString());
		//info.m_weatherIconId = std::move(weatherCodeToString(obj.value(u"icon").toString()));
	}
	else
	{
		DEFINE_SOURCE_LOCATION
		ERROR_MESSAGE("An empty weather array is returned.", QString())
	}
}

OpenWeatherMapBackend::OpenWeatherMapBackend(QObject *parent)
	: ProviderBackend{parent},
	  m_networkManager{new QNetworkAccessManager(this)}
{}

void OpenWeatherMapBackend::requestWeatherInfo(const QString& city)
{
	QUrlQuery query;
	QUrl url{u"https://api.api-ninjas.com/v1/geocoding"_s};
	query.addQueryItem(u"city"_s, city);
	url.setQuery(query);
	QNetworkRequest net_request{url};
	net_request.setRawHeader("X-Api-Key", "fVD0CWxVLhaz7cnhK21PCw==aOqhLKStgURO363U");
	QNetworkReply* reply{m_networkManager->get(net_request)};
	connect(reply, &QNetworkReply::finished, this, [this,reply] () {
		DEFINE_SOURCE_LOCATION
		if (!reply->error())
		{
			const QMap<QString,QString>& parsedData(parseApiNinjasReply(reply->readAll()));
			if (!parsedData.isEmpty())
			{
				const QGeoCoordinate coordinate(parsedData.value("latitude").toDouble(), parsedData.value("longitude").toDouble());
				requestCurrentWeather(coordinate);
			}
			else
			{
				emit errorOccurred();
				ERROR_MESSAGE("Could not parse network reply:  ", reply->readAll())
			}
		}
		else
		{
			emit errorOccurred();
			ERROR_MESSAGE("Network reply:  ", reply->errorString())
		}
	});
}

void OpenWeatherMapBackend::requestWeatherInfo(const QGeoCoordinate& coordinate)
{	
	requestCurrentWeather(coordinate);
}

void OpenWeatherMapBackend::handleCurrentWeatherReply(QNetworkReply* reply, const QGeoCoordinate& coordinate)
{
	if (!reply)
	{
		emit errorOccurred();
		return;
	}

	bool parsed(false);
	if (!reply->error())
	{
		// extract info about current weather
		qDebug() << reply->readAll();
		const QJsonDocument& document{QJsonDocument::fromJson(reply->readAll())};
		const QJsonObject& documentObject{document.object()};

		st_LocationInfo currentLocation;
		currentLocation.m_name = std::move(documentObject.value(u"name").toString());
		if (coordinate.isValid())
			currentLocation.m_coordinate = coordinate;

		st_WeatherInfo currentWeather;
		parseWeatherDescription(documentObject, currentWeather);

		const QJsonObject& currentObject{documentObject.value(u"current").toObject()};
		const QJsonValue& tempValue{currentObject.value(u"temp")};
		//if (tempValue.isDouble())
		//	currentWeather.m_temperature = niceTemperatureString(tempValue.toDouble());
		currentWeather.m_coordinates = '(' + QString::number(coordinate.latitude()) + ',' + QString::number(coordinate.longitude()) + ')';

		parsed = !currentLocation.m_name.isEmpty() && !currentWeather.m_temperature.isEmpty();

		if (parsed)
		{
			const QJsonValue& tempFeelValue{currentObject.value(u"feels_like")};
			currentWeather.m_temperature_feel = tempFeelValue.toString();
			const QJsonValue& humidityValue{currentObject.value(u"humidity")};
			currentWeather.m_humidity = humidityValue.toString();
			const QJsonValue& pressureValue{currentObject.value(u"pressure")};
			currentWeather.m_pressure = pressureValue.toString();
			const QJsonValue& windValue{currentObject.value(u"wind_speed")};
			currentWeather.m_wind = windValue.toString();

			QList<st_WeatherInfo> weatherDetails;
			weatherDetails << currentWeather; // current weather will be the first in the list

			const QJsonArray& daysList{documentObject.value(u"daily").toArray()};
			for (qsizetype i(0); i < 4; ++i) // include current day as well
			{
				const QJsonObject& dayObject{daysList.at(i).toObject()};
				st_WeatherInfo info;

				const QDateTime& dt{QDateTime::fromSecsSinceEpoch(dayObject.value(u"dt").toInteger())};
				info.m_dayOfWeek = appUtils()->appLocale()->toString(dt, u"ddd");

				const QJsonObject& tempObject{dayObject.value(u"temp").toObject()};
				const QJsonValue& minTemp{tempObject.value(u"min")};
				const QJsonValue& maxTemp{tempObject.value(u"max")};
				//if (minTemp.isDouble() && maxTemp.isDouble())
				//	info.m_temperature = std::move(niceTemperatureString(minTemp.toDouble()) + QChar('/') + niceTemperatureString(maxTemp.toDouble()));
				//else
				{
					DEFINE_SOURCE_LOCATION
					ERROR_MESSAGE("Failed to parse min or max temperature.", QString())
				}
				parseWeatherDescription(dayObject, info);

				if (!info.m_temperature.isEmpty() && !info.m_weatherIconId.isEmpty())
					weatherDetails.push_back(info);
			}
			emit weatherInformation(currentLocation, weatherDetails);
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

void OpenWeatherMapBackend::handleWeatherForecastReply(QNetworkReply* reply, const st_LocationInfo& location, const st_WeatherInfo& currentWeather)
{
	if (!reply)
	{
		emit errorOccurred();
		return;
	}
	if (!reply->error())
	{
		const QJsonDocument& document{QJsonDocument::fromJson(reply->readAll())};
		const QJsonObject& documentObject{document.object()};

		QList<st_WeatherInfo> weatherDetails;
		// current weather will be the first in the list
		weatherDetails << currentWeather;

		const QJsonArray& daysList{documentObject.value(u"list").toArray()};
		// include current day as well
		for (qsizetype i(0); i < daysList.size(); ++i) {
			const QJsonObject& dayObject{daysList.at(i).toObject()};
			st_WeatherInfo info;

			const QDateTime& dt{QDateTime::fromSecsSinceEpoch(dayObject.value(u"dt").toInteger())};
			info.m_dayOfWeek = dt.toString(u"ddd");

			const QJsonObject& tempObject{dayObject.value(u"temp").toObject()};
			const QJsonValue& minTemp{tempObject.value(u"min")};
			const QJsonValue& maxTemp{tempObject.value(u"max")};
			//if (minTemp.isDouble() && maxTemp.isDouble())
			//	info.m_temperature = std::move(niceTemperatureString(minTemp.toDouble()) + QChar('/') + niceTemperatureString(maxTemp.toDouble()));
			//else
			{
				DEFINE_SOURCE_LOCATION
				ERROR_MESSAGE("Failed to parse min or max temperature.", QString())
			}

			parseWeatherDescription(dayObject, info);

			if (!info.m_temperature.isEmpty() && !info.m_weatherIconId.isEmpty())
				weatherDetails.push_back(info);
		}
		emit weatherInformation(location, weatherDetails);
	}
	else
	{
		emit errorOccurred();
		DEFINE_SOURCE_LOCATION
		ERROR_MESSAGE(reply->errorString(), QString())
	}
	reply->deleteLater();
}

void OpenWeatherMapBackend::requestCurrentWeather(const QGeoCoordinate coordinate)
{
	QUrlQuery query;
	QUrl url{u"http://api.openweathermap.org/data/3.0/onecall"_s};
	query.addQueryItem(u"lat"_s, QString::number(coordinate.latitude()));
	query.addQueryItem(u"lon"_s, QString::number(coordinate.longitude()));
	query.addQueryItem(u"appid"_s, u"31d07fed3c1e19a6465c04a40c71e9a0"_s);
	query.addQueryItem(u"exclude"_s, u"minutely,hourly,alerts"_s);
	query.addQueryItem(u"lang"_s, appSettings()->appLocale().left(2));
	url.setQuery(query);

	QNetworkReply* reply{m_networkManager->get(QNetworkRequest{url})};
	connect(reply, &QNetworkReply::finished, this, [this, reply, coordinate]() { handleCurrentWeatherReply(reply, coordinate); });
}

void OpenWeatherMapBackend::requestWeatherForecast(const st_LocationInfo &location, const st_WeatherInfo &currentWeather)
{
	QUrl url{u"http://api.openweathermap.org/data/3.0/forecast/daily"_s};
	QUrlQuery query;
	query.addQueryItem(u"mode"_s, u"json"_s);
//	query.addQueryItem(u"APPID"_s, m_appId);
	query.addQueryItem(u"q"_s, location.m_name);
	query.addQueryItem(u"cnt"_s, u"4"_s);
	query.addQueryItem(u"lang"_s, appSettings()->appLocale().left(2));
	url.setQuery(query);

	QNetworkReply* reply{m_networkManager->get(QNetworkRequest{url})};
	connect(reply, &QNetworkReply::finished, this, [this, reply, location, currentWeather]() {
		handleWeatherForecastReply(reply, location, currentWeather);
	});
}
