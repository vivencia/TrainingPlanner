// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef OPENWEATHERMAPBACKEND_H
#define OPENWEATHERMAPBACKEND_H

#include "providerbackend.h"

class QNetworkAccessManager;
class QNetworkReply;
class QUrlQuery;

class OpenWeatherMapBackend : public ProviderBackend
{

Q_OBJECT

public:
	explicit OpenWeatherMapBackend(QObject* parent = nullptr);
	~OpenWeatherMapBackend() = default;

	void requestWeatherInfo(const QString& city) override;
	void requestWeatherInfo(const QGeoCoordinate& coordinate) override;

private slots:
	void handleCurrentWeatherReply(QNetworkReply* reply, const QGeoCoordinate& coordinate);
	void handleWeatherForecastReply(QNetworkReply* reply, const st_LocationInfo& location, const st_WeatherInfo& currentWeather);

private:
	void requestCurrentWeather(QUrlQuery& query, const QGeoCoordinate& coordinate);
	void requestWeatherForecast(const st_LocationInfo& location, const st_WeatherInfo& currentWeather);

	QNetworkAccessManager* m_networkManager;
	const QString m_appId;
};

#endif // OPENWEATHERMAPBACKEND_H
