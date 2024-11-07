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
	inline QString sunrise(const uint day) const { return formatTime(m_weatherData.at(day).value(key_sunrise)) + "hs"_L1; }
	inline QString sunset(const uint day) const { return formatTime(m_weatherData.at(day).value(key_sunset))  + "hs"_L1; }
	inline QString temperature(const uint day) const { return formatTemp(m_weatherData.at(day).value(key_temp)); }
	inline QString feel_temperature(const uint day) const { return formatTemp(m_weatherData.at(day).value(key_tempfeel)); }
	inline QString min_temperature(const uint day) const { return formatTemp(m_weatherData.at(day).value(key_min)); }
	inline QString max_temperature(const uint day) const { return formatTemp(m_weatherData.at(day).value(key_max)); }
	inline QString pressure(const uint day) const { return m_weatherData.at(day).value(key_pressure) + "hPa"_L1; }
	inline QString humidity(const uint day) const { return m_weatherData.at(day).value(key_humidity) + "%"_L1; }
	inline QString uv_index(const uint day) const { return m_weatherData.at(day).value(key_uv); }
	inline QString wind_speed(const uint day) const { return formatSpeed(m_weatherData.at(day).value(key_wind)); }
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
		return QString::number(qRound(strTemp.toDouble() - kZeroKelvin)) + u"°C"_s;
	}

	inline QString formatDate(const QString& strDate) const
	{
		const QDateTime& dt{QDateTime::fromSecsSinceEpoch(strDate.toUInt())};
		return appUtils()->appLocale()->toString(dt, "ddd d/M"_L1);
	}

	inline QString formatTime(const QString& strDate) const
	{
		const QDateTime& dt{QDateTime::fromSecsSinceEpoch(strDate.toUInt())};
		return appUtils()->appLocale()->toString(dt.toLocalTime(), "HH:mm:ss"_L1);
	}

	inline QString formatSpeed(const QString& strSpeed) const
	{
		return QString::number(qRound(strSpeed.toDouble() * 3.6)) + "km/h"_L1;
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
							if (word == "dt"_L1)
							{
								if (m_weatherData.count() < 5)
								{
									QMap<QString,QString> data;
									m_weatherData.append(std::move(data));
								}
								else
								{
									m_bParsedOK = true;
									return;
								}
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
	if (code == "01d"_L1 || code == "01n"_L1)
		return "sunny"_L1;
	else if (code == "02d"_L1 || code == "02n"_L1)
		return "sunny-very-few-clouds"_L1;
	else if (code == "03d"_L1 || code == "03n"_L1)
		return "few-clouds"_L1;
	else if (code == "04d"_L1 || code == "04n"_L1)
		return "overcast"_L1;
	else if (code == "09d"_L1 || code == "09n"_L1 || code == "10d"_L1 || code == "10n"_L1)
		return "showers"_L1;
	else if (code == "11d"_L1 || code == "11n"_L1)
		return "thundershower"_L1;
	else if (code == "13d"_L1 || code == "13n"_L1)
		return "snow"_L1;
	else if (code == "50d"_L1 || code == "50n"_L1)
		return "fog"_L1;

	return "sunny"_L1; // default choice
}

static QStringList parseOpenWeatherReverseGeocodingReply(const QByteArray& replyData)
{
	QStringList parsedData;
	if (!replyData.isEmpty())
	{
		bool bCanInsert(true);
		QString word;
		QByteArray::const_iterator itr(replyData.constBegin());
		const QByteArray::const_iterator itr_end(replyData.constEnd());
		do {
			if (QChar::isLetterOrNumber(*itr))
				word.append(*itr);
			else
			{
				switch (*itr)
				{
					case ' ':
						word.append(*itr);
					break;
					case ':':
						bCanInsert = (word == appSettings()->appLocale().left(2));
						word.clear();
					break;
					case ',':
						if (bCanInsert)
						{
							parsedData.append(std::move(word));
							if (parsedData.count() == 2)
								return parsedData;
						}
						word.clear();
					default:
						continue;
				}
			}
		} while (++itr != itr_end);
	}
	return parsedData;
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
				word.append(*itr);
			else
			{
				switch (*itr)
				{
					case '-':
					case '.':
					case ' ':
						word.append(*itr);
					break;
					case ':':
						key = std::move(word.simplified());
						word.clear();
					break;
					case ',':
						parsedData.insert(key, word.simplified());
						word.clear();
					break;
					default: continue;
				}
			}
		} while (++itr != itr_end);
	}
	return parsedData;
}

OpenWeatherMapBackend::OpenWeatherMapBackend(QObject *parent)
	: QObject{parent},
	  m_networkManager{new QNetworkAccessManager(this)}
{}

void OpenWeatherMapBackend::requestWeatherInfo(const QString& city)
{
	m_locationName = city;
	QUrlQuery query;
	QUrl url{"https://api.api-ninjas.com/v1/geocoding"_L1};
	query.addQueryItem("city"_L1, city);
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
				requestWeatherInfoFromNet(coordinate);
			}
			else
				ERROR_MESSAGE("Could not parse network reply:  ", reply->readAll())
		}
		else
			ERROR_MESSAGE("Network reply:  ", reply->errorString())
	});
}

void OpenWeatherMapBackend::requestWeatherInfo(const QGeoCoordinate& coordinate)
{
	QUrlQuery query;
	QUrl url{"http://api.openweathermap.org/geo/1.0/reverse"_L1};
	query.addQueryItem("lat"_L1, QString::number(coordinate.latitude()));
	query.addQueryItem("lon"_L1, QString::number(coordinate.longitude()));
	query.addQueryItem("limit"_L1, STR_ONE);
	query.addQueryItem("appid"_L1, "31d07fed3c1e19a6465c04a40c71e9a0"_L1);
	url.setQuery(query);

	QNetworkRequest net_request{url};
	QNetworkReply* reply{m_networkManager->get(net_request)};
	connect(reply, &QNetworkReply::finished, this, [this,reply,coordinate] () {
		DEFINE_SOURCE_LOCATION
		if (!reply->error())
		{
			const QStringList& parsedData(parseOpenWeatherReverseGeocodingReply(reply->readAll()));
			if (!parsedData.isEmpty())
			{
				m_locationName = parsedData.count() == 2 ? parsedData.at(1) : parsedData.at(0);
				requestWeatherInfoFromNet(coordinate);
			}
			else
				ERROR_MESSAGE("Could not parse network reply:  ", reply->readAll())
		}
		else
			ERROR_MESSAGE("Network reply:  ", reply->errorString())
	});
}

void OpenWeatherMapBackend::requestWeatherInfoFromNet(const QGeoCoordinate& coordinate)
{
	QUrlQuery query;
	QUrl url{"http://api.openweathermap.org/data/3.0/onecall"_L1};
	query.addQueryItem("lat"_L1, QString::number(coordinate.latitude()));
	query.addQueryItem("lon"_L1, QString::number(coordinate.longitude()));
	query.addQueryItem("appid"_L1, "31d07fed3c1e19a6465c04a40c71e9a0"_L1);
	query.addQueryItem("exclude"_L1, "minutely,hourly,alerts"_L1);
	query.addQueryItem("lang"_L1, appSettings()->appLocale().left(2));
	url.setQuery(query);

	QNetworkReply* reply{m_networkManager->get(QNetworkRequest{url})};
	connect(reply, &QNetworkReply::finished, this, [this, reply, coordinate]() { handleWeatherInfoResquestReply(reply, coordinate); });
}

void OpenWeatherMapBackend::requestWeatherInfo(const QString& city, const QGeoCoordinate& coordinate)
{
	m_locationName = city;
	requestWeatherInfoFromNet(coordinate);
}

void OpenWeatherMapBackend::handleWeatherInfoResquestReply(QNetworkReply* reply, const QGeoCoordinate& coordinate)
{
	if (!reply)
		return;

	bool parsed(false);
	if (!reply->error())
	{
		const parseOpenWeatherAPIResponse netData{reply->readAll()};
		parsed = netData.parsedOK();
		if (parsed)
		{
			st_LocationInfo currentLocation;
			currentLocation.m_name = m_locationName;
			currentLocation.m_coordinate = coordinate;

			QList<st_WeatherInfo> weatherDetails;
			for (uint i(0); i < netData.forecastDays(); ++i)
			{
				st_WeatherInfo weatherInfo;
				weatherInfo.m_coordinates = '(' + std::move(QString::number(coordinate.latitude())) + ',' + std::move(QString::number(coordinate.longitude())) + ')';
				weatherInfo.m_dayOfWeek = std::move(netData.date(i));
				weatherInfo.m_weatherIconId = std::move(netData.weather_icon(i));
				weatherInfo.m_weatherDescription = std::move(netData.weather_description(i));
				weatherInfo.m_humidity = std::move(netData.humidity(i));
				weatherInfo.m_pressure = std::move(netData.pressure(i));
				weatherInfo.m_wind = std::move(netData.wind_speed(i));
				weatherInfo.m_uvi = std::move(netData.uv_index(i));
				weatherInfo.m_sunrise = std::move(netData.sunrise(i));
				weatherInfo.m_sunset = std::move(netData.sunset(i));

				switch (i)
				{
					default:
						weatherInfo.m_temp_max = std::move(netData.max_temperature(i));
						weatherInfo.m_temp_min = std::move(netData.min_temperature(i));
					break;
					case 0:
						weatherInfo.m_temperature = std::move(netData.temperature(0));
						weatherInfo.m_temperature_feel = std::move(netData.feel_temperature(0));
						weatherInfo.m_provider_name = std::move("www.openweathermap.org"_L1);
					break;
				}
				weatherDetails.append(weatherInfo);
			}
			emit weatherInformation(currentLocation, weatherDetails);
		}
	}
	if (!parsed)
	{
		DEFINE_SOURCE_LOCATION
		if (reply->error())
			ERROR_MESSAGE("Reply error.", reply->errorString())
		else
			ERROR_MESSAGE("Failed to parse current weather JSON.", reply->readAll());
	}
	reply->deleteLater();
}
