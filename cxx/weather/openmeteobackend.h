// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef OPENMETEOBACKEND_H
#define OPENMETEOBACKEND_H

#include "providerbackend.h"

class QNetworkAccessManager;
class QNetworkReply;

class OpenMeteoBackend : public ProviderBackend
{
Q_OBJECT

public:
	explicit OpenMeteoBackend(QObject *parent = nullptr);

	void requestWeatherInfo(const QString& city) override;
	void requestWeatherInfo(const QGeoCoordinate& coordinate) override;

private slots:
	void handleWeatherForecastReply(QNetworkReply* reply, const st_LocationInfo& location);

private:
	void generateWeatherRequest(const QString& city, const QGeoCoordinate& coordinate);

	QNetworkAccessManager* m_networkManager;
};

#endif // OPENMETEOBACKEND_H
