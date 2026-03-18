// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#pragma once

#include <QApplication>
#include <QGeoCoordinate>
#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;
class QUrlQuery;

using namespace Qt::Literals::StringLiterals;

struct st_WeatherInfo
{
	QString m_coordinates{std::move("(0,0)"_L1)};
	QString m_dayOfWeek{std::move("??"_L1)};
	QString m_weatherIconId{std::move("error.png"_L1)};
	QString m_weatherDescription;
	QString m_temperature{std::move(u"??°C"_s)};
	QString m_temperature_feel;
	QString m_temp_min{std::move("??"_L1)};
	QString m_temp_max{std::move("??"_L1)};
	QString m_humidity;
	QString m_pressure;
	QString m_wind;
	QString m_uvi;
	QString m_sunrise;
	QString m_sunset;
	QString m_provider_name;
};

struct st_LocationInfo
{
	QString m_name;
	QString m_state;
	QString m_country;
	QString m_strCoordinate;
	QGeoCoordinate m_coordinate;
};

class OpenWeatherMapBackend : public QObject
{

Q_OBJECT

public:
	explicit OpenWeatherMapBackend(QObject *parent = nullptr);
	~OpenWeatherMapBackend() = default;

	void getCityFromCoordinates(const QGeoCoordinate &coordinate);
	void searchForCities(const QString &search_term);
	void requestWeatherInfoFromNet(const QGeoCoordinate &coordinate);
	void requestWeatherInfo(const QString &city, const QGeoCoordinate &coordinate);

signals:
	void receivedCityFromCoordinates(const QString &city, const QGeoCoordinate &coordinate);
	void receivedCitiesFromSearch(const QList<st_LocationInfo> *foundLocations);
	void weatherInformation(const st_LocationInfo &location, const QList<st_WeatherInfo> &weatherDetails);

private slots:
	void handleWeatherInfoRequestReply(QNetworkReply *reply, const QGeoCoordinate &coordinate);

private:
	QNetworkAccessManager *m_networkManager;
	QString m_locationName;
	QList<st_LocationInfo> m_foundLocations;
	QStringList m_citiesFromCoords;

	void parseOpenWeatherGeocodingReply(const QByteArray &replyData);
	void parseOpenWeatherReverseGeocodingReply(const QByteArray &replyData);
};
