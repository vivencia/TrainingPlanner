// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef PROVIDERBACKEND_H
#define PROVIDERBACKEND_H

#include <QObject>
#include <QtPositioning/qgeocoordinate.h>

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

class ProviderBackend : public QObject
{

Q_OBJECT

public:
	explicit inline ProviderBackend(QObject* parent = nullptr) : QObject{parent} {}

	virtual void requestWeatherInfo(const QString& city) = 0;
	virtual void requestWeatherInfo(const QGeoCoordinate& coordinate) = 0;
	virtual void requestWeatherInfoFromNet(const QGeoCoordinate& coordinate) {}

signals:
	// The first element in weatherDetails represents current weather.
	// Next are the weather forecast, including the current day.
	// The LocationInfo object should contain valid coordinate only when it was
	// initially used to request the weather. If the city name was used, an
	// empty coordinate is expected to be transferred.
	void weatherInformation(const st_LocationInfo& location, const QList<st_WeatherInfo>& weatherDetails);
	void errorOccurred();
};

#endif // PROVIDERBACKEND_H
