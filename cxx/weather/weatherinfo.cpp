// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "weatherinfo.h"
#include "openmeteobackend.h"
#include "openweathermapbackend.h"
#include "weatherapibackend.h"
#include "../tpsettings.h"
#include "../tputils.h"

#include <QtPositioning/qgeocircle.h>
#include <QtPositioning/qgeocoordinate.h>
#if QT_CONFIG(permissions)
#include <QtCore/qpermissions.h>
#endif
#include <QtCore/qcoreapplication.h>
#include <QtCore/qtimer.h>

using namespace Qt::Literals::StringLiterals;

void WeatherData::setWeatherInfo(const st_WeatherInfo& w_info)
{
	m_dayOfWeek = std::move(w_info.m_dayOfWeek);
	m_coordinates = std::move(w_info.m_coordinates);
	m_temperature = std::move(w_info.m_temperature + tr(" (Feels: ") + w_info.m_temperature_feel + ')');
	m_icon = std::move(w_info.m_weatherIconId);
	m_description = std::move(tr("Weather now(") + std::move(appUtils()->getCurrentTimeString()) + ")\n" + w_info.m_weatherDescription);
	m_extra_info = std::move(
					tr("Humidity: ") + w_info.m_humidity + '\t' + tr("Pressure: ") + w_info.m_pressure + '\n' +
					tr("Wind speed: " ) + w_info.m_wind   + ' ' + tr("UV Index: ") + w_info.m_uvi    + '\n' +
					tr("Sun rise: ") + w_info.m_sunrise  + '\t' + tr("Sun set: ") + w_info.m_sunset);
	m_provider = std::move(w_info.m_provider_name);
	m_minmax = std::move(w_info.m_temp_min + '/' + w_info.m_temp_max);
	emit dataChanged();
}

/*
	The class is used as a cache for the weather information.
	It contains a map to cache weather for cities.
	The gps location is cached separately.

	For the coordiante search we do not compare the coordinate directly, but
	check if it's within a circle of 3 km radius (we assume that the weather
	does not really change within that radius).

	The cache returns a pair with empty location and weather data if no data
	is found, or if the data is outdated.
*/
class WeatherDataCache
{
public:
	WeatherDataCache() = default;

	using WeatherDataPair = QPair<QString, QList<st_WeatherInfo>>;

	WeatherDataPair getWeatherData(const QString& name) const;
	WeatherDataPair getWeatherData(const QGeoCoordinate& coordinate) const;

	void addCacheElement(const st_LocationInfo& location, const QList<st_WeatherInfo>& info);

	static bool isCacheResultValid(const WeatherDataPair& result);

private:
	struct CacheItem
	{
		qint64 m_cacheTime;
		QList<st_WeatherInfo> m_weatherData;
	};

	QMap<QString, CacheItem> m_cityCache;

	QGeoCoordinate m_gpsLocation;
	QString m_gpsName;
	CacheItem m_gpsData;

	static const qint64 kCacheTimeoutInterval = 3600; // 1 hour
	static const int kCircleRadius = 3000; // 3 km
};

WeatherDataCache::WeatherDataPair WeatherDataCache::getWeatherData(const QString& name) const
{
	if (m_cityCache.contains(name))
	{
		const qint64 currentTime{QDateTime::currentSecsSinceEpoch()};
		const auto &item{m_cityCache.value(name)};
		if (currentTime - item.m_cacheTime < kCacheTimeoutInterval)
			return qMakePair(name, item.m_weatherData);
	}
	return qMakePair(QString(), QList<st_WeatherInfo>());
}

WeatherDataCache::WeatherDataPair
WeatherDataCache::getWeatherData(const QGeoCoordinate& coordinate) const
{
	if (m_gpsLocation.isValid() && !m_gpsName.isEmpty())
	{
		const QGeoCircle area{m_gpsLocation, kCircleRadius};
		if (area.contains(coordinate))
		{
			const qint64 currentTime{QDateTime::currentSecsSinceEpoch()};
			if (currentTime - m_gpsData.m_cacheTime < kCacheTimeoutInterval)
				return qMakePair(m_gpsName, m_gpsData.m_weatherData);
		}
	}
	return qMakePair(QString(), QList<st_WeatherInfo>());
}

void WeatherDataCache::addCacheElement(const st_LocationInfo& location, const QList<st_WeatherInfo>& info)
{
	// It it expected that we have valid QGeoCoordinate only when the weather
	// is received based on coordinates.
	const qint64 currentTime{QDateTime::currentSecsSinceEpoch()};
	if (location.m_coordinate.isValid())
	{
		m_gpsLocation = location.m_coordinate;
		m_gpsName = location.m_name;
		m_gpsData = { currentTime, info };
	}
	else
		m_cityCache[location.m_name] = { currentTime, info };
}

bool WeatherDataCache::isCacheResultValid(const WeatherDataCache::WeatherDataPair& result)
{
	return !result.first.isEmpty() && !result.second.isEmpty();
}

class WeatherInfoPrivate
{

public:
	QGeoPositionInfoSource* src = nullptr;
	QGeoCoordinate coord;
	QString city;
	WeatherData now;
	WeatherData nextThreeDays[3];
	QQmlListProperty<WeatherData>* fcProp = nullptr;
	bool ready = false;
	bool useGps = false;
	bool canUseGPS = false;
#ifdef Q_OS_ANDROID
	QString gpsCity;
#endif
	WeatherDataCache m_dataCache;
	ProviderBackend* m_currentBackend = nullptr;
	QList<ProviderBackend*> m_supportedBackends;
	qsizetype m_currentBackendIndex = 0;
};

static void forecastAppend(QQmlListProperty<WeatherData> *prop, WeatherData *val)
{
	Q_UNUSED(val);
	Q_UNUSED(prop);
}

static WeatherData *forecastAt(QQmlListProperty<WeatherData>* prop, qsizetype index)
{
	WeatherInfoPrivate *d = static_cast<WeatherInfoPrivate*>(prop->data);
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
#ifdef Q_OS_ANDROID
	, gpsWaitTimer(nullptr)
#endif
{
	d->fcProp = new QQmlListProperty<WeatherData>{this, d, forecastAppend,
														   forecastCount,
														   forecastAt,
														   forecastClear};

	d->m_supportedBackends.push_back(new OpenWeatherMapBackend{this});
	d->m_supportedBackends.push_back(new WeatherApiBackend{this});
	d->m_supportedBackends.push_back(new OpenMeteoBackend{this});
	registerBackend(0);

#ifdef Q_OS_ANDROID
	d->src = QGeoPositionInfoSource::createDefaultSource(this);

	if (d->src)
	{

#if QT_CONFIG(permissions)
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
				d->src->startUpdates();
			break;
		}

		if (d->canUseGPS)
		{
			d->src->startUpdates();
			connect(d->src, &QGeoPositionInfoSource::positionUpdated, this, &WeatherInfo::positionUpdated);
			connect(d->src, &QGeoPositionInfoSource::errorOccurred, this, &WeatherInfo::positionError);
			setUseGps(true);
		}
#else
		d->src->startUpdates();
#endif
	}
	else
	{
		d->canUseGPS = false;
		setCity(appSettings()->weatherCity(0));
	}
#else
	setCity(appSettings()->weatherCity(0));
#endif
}

WeatherInfo::~WeatherInfo()
{
#ifdef Q_OS_ANDROID
	if (gpsWaitTimer)
		delete gpsWaitTimer;
#endif
	if (d->src)
		d->src->stopUpdates();
	if (d->fcProp)
		delete d->fcProp;
	delete d;
}

void WeatherInfo::positionUpdated(QGeoPositionInfo gpsPos)
{
	d->coord = gpsPos.coordinate();
	if (!d->useGps)
		return;
	requestWeatherByCoordinates();
}

void WeatherInfo::positionError(QGeoPositionInfoSource::Error e)
{
	Q_UNUSED(e);
	qWarning() << "Position source error. Falling back to simulation mode.";

	if (d->useGps)
		setCity(appSettings()->weatherCity(0));
}

void WeatherInfo::refreshWeather()
{
	if (d->city.isEmpty())
		return;
	requestWeatherByCity();
}

void WeatherInfo::handleWeatherData(const st_LocationInfo& location, const QList<st_WeatherInfo>& weatherDetails)
{
	if (applyWeatherData(location.m_name, weatherDetails))
		d->m_dataCache.addCacheElement(location, weatherDetails);
}

void WeatherInfo::switchToNextBackend()
{
	deregisterCurrentBackend();
	registerBackend(d->m_currentBackendIndex + 1);
	if (d->m_currentBackend)
	{
		// repeat the query
		if (d->useGps)
			requestWeatherByCoordinates();
		else
			requestWeatherByCity();
	}
	else
	{
		qWarning("The application has iterated through all of the weather backends, "
				 "and none of them seems to respond now. Please wait until any of the "
				 "backends becomes available again.");
	}
}

bool WeatherInfo::applyWeatherData(const QString& city, const QList<st_WeatherInfo>& weatherDetails)
{
	// Check that we didn't get outdated weather data. The city should match, if only we do not use GPS.
	if (city != d->city && !d->useGps)
		return false;

	if (city != d->city && d->useGps)
	{
		setCity(city);
#ifdef Q_OS_ANDROID
		setGpsCity(city);
#endif
	}

	if (!weatherDetails.isEmpty())
	{
		const st_WeatherInfo& w_info(weatherDetails.first());
		d->now.setWeatherInfo(w_info);
		for(uint i(0); i < 3; ++i)
			d->nextThreeDays[i].setWeatherInfo(weatherDetails.at(i+2));
	}

	if (!d->ready)
	{
		d->ready = true;
		emit readyChanged();
	}
	appSettings()->setCurrentWeatherCity(city);
	emit weatherChanged();
	return true;
}

void WeatherInfo::requestWeatherByCoordinates()
{
	d->ready = false;
	const auto cacheResult = d->m_dataCache.getWeatherData(d->coord);
	if (WeatherDataCache::isCacheResultValid(cacheResult))
		applyWeatherData(cacheResult.first, cacheResult.second);
	else if (d->m_currentBackend)
		d->m_currentBackend->requestWeatherInfo(d->coord);
}

void WeatherInfo::requestWeatherByCity()
{
	d->ready = false;
	const auto cacheResult = d->m_dataCache.getWeatherData(d->city);
	if (WeatherDataCache::isCacheResultValid(cacheResult))
		applyWeatherData(cacheResult.first, cacheResult.second);
	else if (d->m_currentBackend)
		d->m_currentBackend->requestWeatherInfo(d->city);
}

void WeatherInfo::registerBackend(qsizetype index)
{
	if (index >= 0 && index < d->m_supportedBackends.size())
	{
		d->m_currentBackend = d->m_supportedBackends.at(index);
		d->m_currentBackendIndex = index;
		connect(d->m_currentBackend, &ProviderBackend::weatherInformation, this, &WeatherInfo::handleWeatherData);
		connect(d->m_currentBackend, &ProviderBackend::errorOccurred, this, &WeatherInfo::switchToNextBackend);
	}
}

void WeatherInfo::deregisterCurrentBackend()
{
	if (d->m_currentBackend)
	{
		disconnect(d->m_currentBackend, &ProviderBackend::weatherInformation, this, &WeatherInfo::handleWeatherData);
		disconnect(d->m_currentBackend, &ProviderBackend::errorOccurred, this, &WeatherInfo::switchToNextBackend);
		d->m_currentBackend = nullptr;
	}
}

bool WeatherInfo::hasValidCity() const
{
	return (!(d->city.isEmpty()) && d->city.size() > 1 && d->city != "");
}

bool WeatherInfo::hasValidWeather() const
{
	return hasValidCity() && (!(d->now.weatherIcon().isEmpty()) &&
							  (d->now.weatherIcon().size() > 1) &&
							  d->now.weatherIcon() != "");
}

WeatherData *WeatherInfo::weather() const
{
	return &(d->now);
}

QQmlListProperty<WeatherData> WeatherInfo::forecast() const
{
	return *(d->fcProp);
}

bool WeatherInfo::ready() const
{
	return d->ready;
}

QString WeatherInfo::loadMessage() const
{
	if (!d->ready)
	{
		if (d->canUseGPS)
		{
			if (!d->coord.isValid())
				return tr("Waiting for GPS signal...");
		}
		return tr("Loading weather data from the internet...");
	}
	return QString();
}

bool WeatherInfo::hasSource() const
{
	return (d->src != NULL);
}

bool WeatherInfo::useGps() const
{
	return d->useGps;
}

bool WeatherInfo::canUseGps() const
{
	return d->canUseGPS;
}

void WeatherInfo::setUseGps(const bool value)
{
	if (value != d->useGps)
	{
		d->useGps = value;
		if (value)
		{	
			// if we already have a valid GPS position, do not wait until it updates, but query the city immediately
			if (d->coord.isValid())
			{
				setCity(QString(), true);
				requestWeatherByCoordinates();
			}
			else
			{
#ifdef Q_OS_ANDROID
				if (!gpsWaitTimer)
					gpsWaitTimer = new QTimer{this};
				else
				{
					static uint attempts(0);
					qDebug() << "GPS Attempt: " << attempts;
					switch (attempts++)
					{
						case 0:
							connect(gpsWaitTimer, &QTimer::timeout, this, [this] () { setUseGps(true); });
							gpsWaitTimer->setInterval(10000);
							gpsWaitTimer->start();
							return;
						break;
						case 1: return;
						case 2: //After 20 seconds, revert back to city mode
							disconnect(gpsWaitTimer, &QTimer::timeout, nullptr, nullptr);
							d->canUseGPS = false;
							setCity(appSettings()->weatherCity(0));
							attempts = 0; //can be attempted again any time later
						break;
					}
				}
#endif
			requestWeatherByCity();
			}
		}
		emit useGpsChanged();
	}
}

QString WeatherInfo::city() const
{
	return d->city;
}

void WeatherInfo::setCity(const QString& value, const bool changeCityOnly)
{
	if (value != d->city)
	{
		d->city = value;
		appSettings()->setCurrentWeatherCity(value);
		emit cityChanged();
		if (!changeCityOnly)
		{
			if (!value.isEmpty())
			{
				setUseGps(false);
				requestWeatherByCity();
			}
		}
	}
}

#ifdef Q_OS_ANDROID
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
#endif

