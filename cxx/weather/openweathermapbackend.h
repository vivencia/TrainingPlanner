// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef OPENWEATHERMAPBACKEND_H
#define OPENWEATHERMAPBACKEND_H

#include <QGeoCoordinate>
#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;
class QUrlQuery;

struct st_WeatherInfo
{
	QString m_coordinates;
	QString m_dayOfWeek;
	QString m_weatherIconId;
	QString m_weatherDescription;
	QString m_temperature;
	QString m_temperature_feel;
	QString m_temp_max;
	QString m_temp_min;
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
	explicit OpenWeatherMapBackend(QObject* parent = nullptr);
	~OpenWeatherMapBackend() = default;

	void requestWeatherInfo(const QString& city);
	void requestWeatherInfo(const QGeoCoordinate& coordinate);
	void requestWeatherInfoFromNet(const QGeoCoordinate& coordinate);
	void requestWeatherInfo(const QString& city, const QGeoCoordinate& coordinate);

signals:
	void weatherInformation(const st_LocationInfo& location, const QList<st_WeatherInfo>& weatherDetails);

private slots:
	void handleWeatherInfoResquestReply(QNetworkReply* reply, const QGeoCoordinate& coordinate);

private:
	QNetworkAccessManager* m_networkManager;
	QString m_locationName;
};

#endif // OPENWEATHERMAPBACKEND_H
