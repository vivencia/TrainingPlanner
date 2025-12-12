// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "weatherinfo.h"

#include "openweathermapbackend.h"
#include "../tpsettings.h"
#include "../tputils.h"

#include <QCoreApplication>
#include <QGeoCircle>
#include <QGeoCoordinate>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QUrlQuery>

#if QT_CONFIG(permissions)
#include <QPermissions>
#endif

using namespace Qt::Literals::StringLiterals;

void WeatherData::setWeatherInfo(const st_WeatherInfo &w_info, const st_WeatherInfo *w_currentdayforecast)
{
	m_dayOfWeek = std::move(w_info.m_dayOfWeek);
	m_coordinates = std::move(w_info.m_coordinates);
	m_temperature = std::move(w_info.m_temperature + tr(" (Feels: ") + w_info.m_temperature_feel + ')');
	if (w_currentdayforecast)
		m_temperature += '\n' + std::move(w_currentdayforecast->m_temp_min + '/' + w_currentdayforecast->m_temp_max);
	m_icon = std::move(w_info.m_weatherIconId);
	m_description = std::move(tr("Weather now(") + std::move(appUtils()->formatTime(
									QTime::currentTime(), TPUtils::TF_FANCY)) + ")\n"_L1 + w_info.m_weatherDescription);
	m_extra_info = std::move(
					tr("Humidity: ") + w_info.m_humidity + '\t' + tr("Pressure: ") + w_info.m_pressure + '\n' +
					tr("Wind speed: " ) + w_info.m_wind   + ' ' + tr("UV Index: ") + w_info.m_uvi    + '\n' +
					tr("Sun rise: ") + w_info.m_sunrise  + '\n' + tr("Sun set: ") + w_info.m_sunset);
	m_provider = std::move(w_info.m_provider_name);
	m_minmax = std::move(w_info.m_temp_min + '/' + w_info.m_temp_max);
	emit dataChanged();
}

class WeatherInfoPrivate
{

public:
	QGeoPositionInfoSource *src{nullptr};
	QGeoCoordinate coord;
	QString city;
	QString gpsMessage;
	WeatherData now;
	WeatherData nextThreeDays[3];
	QQmlListProperty<WeatherData> *fcProp{nullptr};
	bool canUseGPS{false};
	OpenWeatherMapBackend *m_currentBackend{nullptr};
};

static void forecastAppend(QQmlListProperty<WeatherData> *prop, WeatherData *val)
{
	Q_UNUSED(val);
	Q_UNUSED(prop);
}

static WeatherData *forecastAt(QQmlListProperty<WeatherData> *prop, qsizetype index)
{
	WeatherInfoPrivate *d{static_cast<WeatherInfoPrivate*>(prop->data)};
	return &d->nextThreeDays[index];
}

static qsizetype forecastCount(QQmlListProperty<WeatherData> *prop)
{
	return 3;
}

static void forecastClear(QQmlListProperty<WeatherData> *prop)
{
	Q_UNUSED(prop);
}

WeatherInfo::WeatherInfo(QObject *parent)
	: QObject{parent}, d{new WeatherInfoPrivate}
{
	d->fcProp = new QQmlListProperty<WeatherData>{this, d, forecastAppend, forecastCount, forecastAt, forecastClear};
	d->m_currentBackend = new OpenWeatherMapBackend{this};
	connect(d->m_currentBackend, &OpenWeatherMapBackend::receivedCitiesFromSearch, this, &WeatherInfo::buildLocationsList);
	connect(d->m_currentBackend, &OpenWeatherMapBackend::weatherInformation, this, &WeatherInfo::handleWeatherData);
	requestWeatherForSavedCity(0);

#ifdef Q_OS_ANDROID
	connect(d->m_currentBackend, &OpenWeatherMapBackend::receivedCityFromCoordinates, this, &WeatherInfo::gotGPSLocation);

	d->src = QGeoPositionInfoSource::createDefaultSource(this);
	if (d->src)
	{
		QLocationPermission permission;
		permission.setAccuracy(QLocationPermission::Precise);
		permission.setAvailability(QLocationPermission::WhenInUse);

		switch (qApp->checkPermission(permission))
		{
			case Qt::PermissionStatus::Undetermined:
				qApp->requestPermission(permission, [this] (const QPermission &permission) {
					if (permission.status() == Qt::PermissionStatus::Granted)
						setCanUseGps(true);
					else
						positionError(QGeoPositionInfoSource::AccessError);
				});
			break;
			case Qt::PermissionStatus::Denied:
				positionError(QGeoPositionInfoSource::AccessError);
			break;
			case Qt::PermissionStatus::Granted:
				setCanUseGps(true);
			break;
		}

		if (d->canUseGPS)
		{
			setGpsMessage(tr("Resquesting GPS info..."));
			connect(d->src, &QGeoPositionInfoSource::positionUpdated, this, &WeatherInfo::positionUpdated);
			connect(d->src, &QGeoPositionInfoSource::errorOccurred, this, &WeatherInfo::positionError);
			d->src->setUpdateInterval(5000);
			d->src->startUpdates();
			return;
		}
	}
#else
	setGpsMessage(tr("Cannot use GPS on this device"));
#endif
}

WeatherInfo::~WeatherInfo()
{
	if (d->src)
		d->src->stopUpdates();
	if (d->fcProp)
		delete d->fcProp;
	delete d;
}

#ifdef Q_OS_ANDROID
void WeatherInfo::positionUpdated(const QGeoPositionInfo &gpsPos)
{
	d->m_currentBackend->getCityFromCoordinates(gpsPos.coordinate());
}

void WeatherInfo::positionError(QGeoPositionInfoSource::Error e)
{
	QString str_error;
	bool can_usegps{false};
	switch (e)
	{
		case QGeoPositionInfoSource::AccessError:
			str_error = std::move(tr("The connection setup to the remote positioning backend failed because "
									 "the application lacked the required privileges"));
		break;
		case QGeoPositionInfoSource::ClosedError:
			str_error = std::move(tr("Location(GPS) service is disabled. Enable it(turn it on) to use satelite location on this device"));
		break;
		case QGeoPositionInfoSource::UnknownSourceError:
			str_error = std::move(tr("An unidentified error occurred."));
		break;
		case QGeoPositionInfoSource::NoError:
			can_usegps = true;
		break;
		case QGeoPositionInfoSource::UpdateTimeoutError:
			str_error = std::move(tr("If requestUpdate() was called, this error indicates that the current "
				"position could not be retrieved within the specified timeout. If startUpdates() was called, "
				"this error indicates that this QGeoPositionInfoSource subclass determined that it will not be "
				"able to provide further regular updates. In the latter case the error would not be emitted again "
				"until after the regular updates resume"));
	}
	setCanUseGps(can_usegps);
	if (!can_usegps)
		setGpsMessage(str_error);
}

void WeatherInfo::gotGPSLocation(const QString &city, const QGeoCoordinate &coord)
{
	m_gpsLocation.m_name = city;
	m_gpsLocation.m_coordinate.setLatitude(coord.latitude());
	m_gpsLocation.m_coordinate.setLongitude(coord.longitude());
	setGpsMessage(tr("Using GPS location:\n") + city);
	addLocationToConfig(city, coord);
	d->src->setUpdateInterval(60000);
}
#endif

void WeatherInfo::handleWeatherData(const st_LocationInfo &location, const QList<st_WeatherInfo> &weatherDetails)
{
	if (!weatherDetails.isEmpty())
	{
		const st_WeatherInfo &w_info{weatherDetails.first()};
		d->now.setWeatherInfo(w_info, &(weatherDetails.at(1)));
		for(uint i{0}; i < 3; ++i)
		{
			if (weatherDetails.count() > i+2)
				d->nextThreeDays[i].setWeatherInfo(weatherDetails.at(i+2));
		}
		setCity(location.m_name);
		d->coord = location.m_coordinate;
		addLocationToConfig(location.m_name, location.m_coordinate);
		emit weatherChanged();
	}
	else
		d->now.setWeatherInfo(st_WeatherInfo{});
}

inline void WeatherInfo::addLocationToConfig(const QString &location, const QGeoCoordinate &coord)
{
	appSettings()->addWeatherLocation(location, QString::number(coord.latitude()), QString::number(coord.longitude()));
}

void WeatherInfo::buildLocationsList(const QList<st_LocationInfo> *foundLocations)
{
	m_locationList.clear();
	m_foundLocations = const_cast<QList<st_LocationInfo>*>(foundLocations);
	for (const auto &location : std::as_const(*foundLocations))
		m_locationList.append(location.m_name + " ("_L1 + location.m_state + ' ' +  location.m_country + ')');
	emit locationListChanged();
}

WeatherData *WeatherInfo::weather() const
{
	return &(d->now);
}

QQmlListProperty<WeatherData> WeatherInfo::forecast() const
{
	return *(d->fcProp);
}

bool WeatherInfo::canUseGps() const
{
	return d->canUseGPS;
}

void WeatherInfo::setCanUseGps(const bool can_usegps)
{
	if (can_usegps != d->canUseGPS)
	{
		d->canUseGPS = can_usegps;
		emit canUseGpsChanged();
	}
}

QString WeatherInfo::city() const
{
	return d->city;
}

void WeatherInfo::setCity(const QString &value)
{
	if (value != d->city)
	{
		d->city = value;
		emit cityChanged();
	}
}

QString WeatherInfo::gpsMessage() const
{
	return d->gpsMessage;
}

void WeatherInfo::setGpsMessage(const QString &message)
{
	if (message != d->gpsMessage)
	{
		d->gpsMessage = message;
		emit gpsMessageChanged();
	}
}

#ifdef Q_OS_ANDROID
void WeatherInfo::requestWeatherForGpsCity()
{
	d->m_currentBackend->requestWeatherInfo(m_gpsLocation.m_name, m_gpsLocation.m_coordinate);
}
#endif

void WeatherInfo::requestWeatherForSavedCity(const uint index)
{
	const QString &weather_location{appSettings()->weatherLocationName(index)};
	if (!weather_location.isEmpty())
	{
		QGeoCoordinate coord;
		const std::pair<QString,QString> &coords{appSettings()->weatherLocationCoordinates(index)};
		coord.setLatitude(coords.first.toDouble());
		coord.setLongitude(coords.second.toDouble());
		d->m_currentBackend->requestWeatherInfo(weather_location, coord);
	}
}

void WeatherInfo::refreshWeather()
{
	d->m_currentBackend->requestWeatherInfo(d->city, d->coord);
}

void WeatherInfo::searchForCities(const QString &place)
{
	m_locationList.clear();
	emit locationListChanged();
	if (place.length() >= 5)
		d->m_currentBackend->searchForCities(place);
}

void WeatherInfo::locationSelected(const uint index)
{
	d->m_currentBackend->requestWeatherInfo(m_foundLocations->at(index).m_name, m_foundLocations->at(index).m_coordinate);
	m_locationList.clear();
	emit locationListChanged();
}
