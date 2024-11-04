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
	void handleWeatherInfoResquestReply(QNetworkReply* reply, const QGeoCoordinate& coordinate);

private:
	void requestWeatherInfoFromNet(const QGeoCoordinate& coordinate);

	QNetworkAccessManager* m_networkManager;
	QString m_locationName;
};

#endif // OPENWEATHERMAPBACKEND_H
