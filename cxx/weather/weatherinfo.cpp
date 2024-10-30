// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "weatherinfo.h"
#include "openmeteobackend.h"
#include "openweathermapbackend.h"
#include "weatherapibackend.h"
#include "../tpsettings.h"

#include <QtPositioning/qgeocircle.h>
#include <QtPositioning/qgeocoordinate.h>
#if QT_CONFIG(permissions)
#include <QtCore/qpermissions.h>
#endif
#include <QtCore/qcoreapplication.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qtimer.h>

using namespace Qt::Literals::StringLiterals;

WeatherData::WeatherData(QObject* parent) :
		QObject(parent)
{
}

WeatherData::WeatherData(const WeatherData& other) :
		QObject(nullptr),
		m_dayOfWeek(other.m_dayOfWeek),
		m_weather(other.m_weather),
		m_weatherDescription(other.m_weatherDescription),
		m_temperature(other.m_temperature)
{
}

WeatherData::WeatherData(const st_WeatherInfo& other)
	: QObject(nullptr),
	  m_dayOfWeek(other.m_dayOfWeek),
	  m_weather(other.m_weatherIconId),
	  m_weatherDescription(other.m_weatherDescription),
	  m_temperature(other.m_temperature)
{
}

QString WeatherData::coordinates() const
{
	return m_coordinates;
}

QString WeatherData::dayOfWeek() const
{
	return m_dayOfWeek;
}

QString WeatherData::weatherIcon() const
{
	return m_weather;
}

QString WeatherData::weatherDescription() const
{
	return m_weatherDescription;
}

QString WeatherData::temperature() const
{
	return m_temperature;
}

void WeatherData::setCoordinates(const QString& value)
{
	m_coordinates = value;
	emit dataChanged();
}

void WeatherData::setDayOfWeek(const QString& value)
{
	m_dayOfWeek = value;
	emit dataChanged();
}

void WeatherData::setWeatherIcon(const QString& value)
{
	m_weather = value;
	emit dataChanged();
}

void WeatherData::setWeatherDescription(const QString& value)
{
	m_weatherDescription = value;
	emit dataChanged();
}

void WeatherData::setTemperature(const QString& value)
{
	m_temperature = value;
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
	QList<WeatherData*> forecast;
	QQmlListProperty<WeatherData>* fcProp = nullptr;
	bool ready = false;
#ifdef Q_OS_ANDROID
	bool useGps = true;
	bool canUseGPS = true;
	QString gpsCity;
#else
	bool useGps = false;
	bool canUseGPS = false;
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
	return d->forecast.at(index);
}

static qsizetype forecastCount(QQmlListProperty<WeatherData>* prop)
{
	WeatherInfoPrivate* d = static_cast<WeatherInfoPrivate*>(prop->data);
	return d->forecast.size();
}

static void forecastClear(QQmlListProperty<WeatherData>* prop)
{
	static_cast<WeatherInfoPrivate*>(prop->data)->forecast.clear();
}

WeatherInfo::WeatherInfo(QObject* parent)
	: QObject{parent}, d{new WeatherInfoPrivate}
{
	d->fcProp = new QQmlListProperty<WeatherData>{this, d, forecastAppend,
														   forecastCount,
														   forecastAt,
														   forecastClear};

	d->m_supportedBackends.push_back(new OpenWeatherMapBackend{this});
	d->m_supportedBackends.push_back(new WeatherApiBackend{this});
	d->m_supportedBackends.push_back(new OpenMeteoBackend{this});
	registerBackend(0);

	d->src = QGeoPositionInfoSource::createDefaultSource(this);

	if (d->src)
	{
		d->useGps = true;
		connect(d->src, &QGeoPositionInfoSource::positionUpdated, this, &WeatherInfo::positionUpdated);
		connect(d->src, &QGeoPositionInfoSource::errorOccurred, this, &WeatherInfo::positionError);

#if QT_CONFIG(permissions)
		QLocationPermission permission;
		permission.setAccuracy(QLocationPermission::Precise);
		permission.setAvailability(QLocationPermission::WhenInUse);

		switch (qApp->checkPermission(permission))
		{
			case Qt::PermissionStatus::Undetermined:
				qApp->requestPermission(permission, [this] (const QPermission& permission) {
					if (permission.status() == Qt::PermissionStatus::Granted)
						d->src->startUpdates();
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
				d->src->startUpdates();
			break;
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

	QTimer* refreshTimer{new QTimer{this}};
	connect(refreshTimer, &QTimer::timeout, this, &WeatherInfo::refreshWeather);
	using namespace std::chrono;
	refreshTimer->start(60s);
}

WeatherInfo::~WeatherInfo()
{
	if (d->src)
		d->src->stopUpdates();
	if (d->fcProp)
		delete d->fcProp;
	qDeleteAll(d->forecast);
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

	// activate simulation mode
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

	// delete previous forecast
	qDeleteAll(d->forecast);
	d->forecast.clear();

	// The first item in the list represents current weather.
	if (!weatherDetails.isEmpty())
	{
		const st_WeatherInfo& w_info(weatherDetails.first());
		d->now.setCoordinates(w_info.m_coordinates);
		d->now.setTemperature(w_info.m_temperature + tr("(Feels: ") + w_info.m_temperature_feel + ')');
		d->now.setWeatherIcon(w_info.m_weatherIconId);
		d->now.setWeatherDescription(w_info.m_weatherDescription + '\n' +
										w_info.m_humidity + u"% / "_s +
										w_info.m_pressure + u"Pa / "_s +
										w_info.m_wind + u"km/h"_s);
	}

	// The other items represent weather forecast. The amount of items depends
	// on the provider backend.
	for (qsizetype i(1); i < weatherDetails.size(); ++i)
	{
		WeatherData* forecastEntry{new WeatherData(weatherDetails.at(i))};
		d->forecast.append(forecastEntry);
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
	const auto cacheResult = d->m_dataCache.getWeatherData(d->coord);
	if (WeatherDataCache::isCacheResultValid(cacheResult))
		applyWeatherData(cacheResult.first, cacheResult.second);
	else if (d->m_currentBackend)
		d->m_currentBackend->requestWeatherInfo(d->coord);
}

void WeatherInfo::requestWeatherByCity()
{
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
			setCity(QString());
			emit cityChanged();
			emit weatherChanged();
			// if we already have a valid GPS position, do not wait until it updates, but query the city immediately
			if (d->coord.isValid())
				requestWeatherByCoordinates();
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

