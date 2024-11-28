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

void WeatherData::setWeatherInfo(const st_WeatherInfo& w_info, const st_WeatherInfo* w_currentdayforecast)
{
	m_dayOfWeek = std::move(w_info.m_dayOfWeek);
	m_coordinates = std::move(w_info.m_coordinates);
	m_temperature = std::move(w_info.m_temperature + tr(" (Feels: ") + w_info.m_temperature_feel + ')');
	if (w_currentdayforecast)
		m_temperature += '\n' + std::move(w_currentdayforecast->m_temp_min + '/' + w_currentdayforecast->m_temp_max);
	m_icon = std::move(w_info.m_weatherIconId);
	m_description = std::move(tr("Weather now(") + std::move(appUtils()->currentFormattedTimeString()) + ")\n"_L1 + w_info.m_weatherDescription);
	m_extra_info = std::move(
					tr("Humidity: ") + w_info.m_humidity + '\t' + tr("Pressure: ") + w_info.m_pressure + '\n' +
					tr("Wind speed: " ) + w_info.m_wind   + ' ' + tr("UV Index: ") + w_info.m_uvi    + '\n' +
					tr("Sun rise: ") + w_info.m_sunrise  + '\t' + tr("Sun set: ") + w_info.m_sunset);
	m_provider = std::move(w_info.m_provider_name);
	m_minmax = std::move(w_info.m_temp_min + '/' + w_info.m_temp_max);
	emit dataChanged();
}

class WeatherInfoPrivate
{

public:
	QGeoPositionInfoSource* src = nullptr;
	QGeoCoordinate coord;
	QString city;
	QString gpsCity;
	WeatherData now;
	WeatherData nextThreeDays[3];
	QQmlListProperty<WeatherData>* fcProp = nullptr;
	bool canUseGPS = false;
	OpenWeatherMapBackend* m_currentBackend = nullptr;
};

static void forecastAppend(QQmlListProperty<WeatherData>* prop, WeatherData *val)
{
	Q_UNUSED(val);
	Q_UNUSED(prop);
}

static WeatherData *forecastAt(QQmlListProperty<WeatherData>* prop, qsizetype index)
{
	WeatherInfoPrivate* d = static_cast<WeatherInfoPrivate*>(prop->data);
	return &d->nextThreeDays[index];
}

static qsizetype forecastCount(QQmlListProperty<WeatherData>* prop)
{
	return 3;
}

static void forecastClear(QQmlListProperty<WeatherData>* prop)
{
	Q_UNUSED(prop);
}

WeatherInfo::WeatherInfo(QObject* parent)
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
				qApp->requestPermission(permission, [this] (const QPermission& permission) {
					if (permission.status() == Qt::PermissionStatus::Granted)
						d->canUseGPS = true;
					else
						positionError(QGeoPositionInfoSource::AccessError);
				});
			break;
			case Qt::PermissionStatus::Denied:
				d->canUseGPS = false;
				qWarning("Location permission is denied");
				positionError(QGeoPositionInfoSource::AccessError);
			break;
			case Qt::PermissionStatus::Granted:
				d->canUseGPS = true;
			break;
		}

		if (d->canUseGPS)
		{
			setGpsCity(tr("Resquesting GPS info..."));
			connect(d->src, &QGeoPositionInfoSource::positionUpdated, this, &WeatherInfo::positionUpdated);
			connect(d->src, &QGeoPositionInfoSource::errorOccurred, this, &WeatherInfo::positionError);
			d->src->setUpdateInterval(5000);
			d->src->startUpdates();
			return;
		}
	}
#else
	setGpsCity(tr("Cannot use GPS on this device"));
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
void WeatherInfo::positionUpdated(const QGeoPositionInfo& gpsPos)
{
	d->m_currentBackend->getCityFromCoordinates(gpsPos.coordinate());
}

//Not working
void WeatherInfo::positionError(QGeoPositionInfoSource::Error e)
{
	Q_UNUSED(e);
	setGpsCity(tr("No GPS signal"));
}

void WeatherInfo::gotGPSLocation(const QString& city, const QGeoCoordinate& coord)
{
	m_gpsLocation.m_name = city;
	m_gpsLocation.m_coordinate.setLatitude(coord.latitude());
	m_gpsLocation.m_coordinate.setLongitude(coord.longitude());
	setGpsCity(city);
	d->src->setUpdateInterval(60000);
}
#endif

void WeatherInfo::handleWeatherData(const st_LocationInfo& location, const QList<st_WeatherInfo>& weatherDetails)
{
	if (!weatherDetails.isEmpty())
	{
		const st_WeatherInfo& w_info(weatherDetails.first());
		d->now.setWeatherInfo(w_info, &(weatherDetails.at(1)));
		for(uint i(0); i < 3; ++i)
		{
			if (weatherDetails.count() > i+2)
				d->nextThreeDays[i].setWeatherInfo(weatherDetails.at(i+2));
		}

		setCity(location.m_name);

		const QString* coordinates{&(weatherDetails.first().m_coordinates)};
		const int comma_idx = coordinates->indexOf(',');
		const QString latitude{std::move(coordinates->sliced(1, comma_idx-1))};
		const QString longitude{std::move(coordinates->sliced(comma_idx+1,coordinates->length()-comma_idx-2))};
		appSettings()->addWeatherCity(location.m_name, latitude, longitude);
		emit weatherChanged();

		d->coord.setLatitude(latitude.toDouble());
		d->coord.setLongitude(longitude.toDouble());
	}
	d->now.setWeatherInfo(st_WeatherInfo{});
}

void WeatherInfo::buildLocationsList(QList<st_LocationInfo>* foundLocations)
{
	m_locationList.clear();
	m_foundLocations = foundLocations;
	for (uint i(0); i < foundLocations->count(); ++i)
	{
		const st_LocationInfo* l_info(&(foundLocations->at(i)));
		m_locationList.append(l_info->m_name + " ("_L1 + l_info->m_state + ' ' +  l_info->m_country + ')');
	}
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

QString WeatherInfo::city() const
{
	return d->city;
}

void WeatherInfo::setCity(const QString& value)
{
	if (value != d->city)
	{
		d->city = value;
		emit cityChanged();
	}
}

QString WeatherInfo::gpsCity() const
{
	return d->gpsCity;
}

void WeatherInfo::setGpsCity(const QString& value)
{
	if (value != d->gpsCity)
	{
		d->gpsCity = value;
		emit gpsCityChanged();
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
	d->m_currentBackend->requestWeatherInfo(appSettings()->weatherCity(index), appSettings()->weatherCityCoordinates(index));
}

void WeatherInfo::refreshWeather()
{
	d->m_currentBackend->requestWeatherInfo(d->city, d->coord);
}

void WeatherInfo::searchForCities(const QString& place)
{
	m_locationList.clear();
	emit locationListChanged();
	if (place.length() >= 5)
		d->m_currentBackend->searchForCities(place);
}

void WeatherInfo::locationSelected(const uint index)
{
	d->m_currentBackend->requestWeatherInfo(m_foundLocations->at(index).m_name, m_foundLocations->at(index).m_coordinate);
	m_usedLocations.insert(m_locationList.at(index), m_foundLocations->at(index));
	m_locationList.clear();
	emit locationListChanged();
}
