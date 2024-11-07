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

void WeatherData::setWeatherInfo(const st_WeatherInfo& w_info)
{
	m_dayOfWeek = std::move(w_info.m_dayOfWeek);
	m_coordinates = std::move(w_info.m_coordinates);
	m_temperature = std::move(w_info.m_temperature + tr(" (Feels: ") + w_info.m_temperature_feel + ')');
	m_icon = std::move(w_info.m_weatherIconId);
	m_description = std::move(tr("Weather now(") + std::move(appUtils()->currentFormattedTimeString()) + ")\n" + w_info.m_weatherDescription);
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
	// It it expected that we have valid QGeoCoordinate only when the weather is received based on coordinates.
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
	bool useGps = false;
	bool canUseGPS = false;
#ifdef Q_OS_ANDROID
	QString gpsCity;
#endif
	WeatherDataCache m_dataCache;
	OpenWeatherMapBackend* m_currentBackend = nullptr;
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
	: QObject{parent}, d{new WeatherInfoPrivate}, m_netAccessManager(nullptr)
#ifdef Q_OS_ANDROID
	,gpsWaitTimer(nullptr), m_gpsOnAttempts(0), m_gpsPosError(0)
#endif
{
	d->fcProp = new QQmlListProperty<WeatherData>{this, d, forecastAppend,
														   forecastCount,
														   forecastAt,
														   forecastClear};

	d->m_currentBackend = new OpenWeatherMapBackend{this};
	connect(d->m_currentBackend, &OpenWeatherMapBackend::weatherInformation, this, &WeatherInfo::handleWeatherData);

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
			if (appSettings()->useGPS())
			{
				qWarning() << "--------------"  << " Can use GPS startUpdates()" << "------------";
				setUseGps(true);
				return;
			}
			else
				setUseGps(false);
		}
#else
		d->src->startUpdates();
#endif
	}
#else
	requestWeatherFor(appSettings()->weatherCity(0), appSettings()->weatherCityCoordinates(0));
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

#ifdef Q_OS_ANDROID
void WeatherInfo::positionUpdated(const QGeoPositionInfo& gpsPos)
{
	qWarning() << "--------------"  << "positionUpdated()" << "------------";
	if (d->useGps)
	{
		qWarning() << "--------------"  << "positionUpdated() - distance to last position:   " << d->coord.distanceTo(gpsPos.coordinate()) << "------------";
		m_gpsPosError = 0;
		d->src->setUpdateInterval(1200000); //update every 20 minutes
		d->src->startUpdates();
		const QGeoCoordinate& newCoord = gpsPos.coordinate();
		if (d->coord.isValid() && newCoord.isValid())
		{
			if (d->coord.distanceTo(newCoord) < 5000)
				return;
		}
		qWarning() << "--------------"  << "positionUpdated() - calling requestWeatherByCoordinates()" << "------------";
		d->coord = std::move(newCoord);
		requestWeatherByCoordinates();
	}
	else //not using GPS for the moment, no need to keep track of its signal
	{
		d->src->stopUpdates();
		d->src->disconnect();
	}
}

//Not working
void WeatherInfo::positionError(QGeoPositionInfoSource::Error e)
{
	Q_UNUSED(e);
	if (d->useGps)
	{
		if (m_gpsPosError == 0)
		{
			d->src->stopUpdates();
			d->src->setUpdateInterval(2000); //update every 2 seconds until positionUpdated gets called
			d->src->startUpdates();
			++m_gpsPosError;
		}
		else if (m_gpsPosError++ >= 5)
			setUseGps(false);
	}
	else //not using GPS for the moment, no need to keep track of its signal
	{
		d->src->stopUpdates();
		d->src->disconnect();
	}
}
#endif

void WeatherInfo::handleWeatherData(const st_LocationInfo& location, const QList<st_WeatherInfo>& weatherDetails)
{
	if (applyWeatherData(location.m_name, weatherDetails))
		d->m_dataCache.addCacheElement(location, weatherDetails);
}

bool WeatherInfo::applyWeatherData(const QString& city, const QList<st_WeatherInfo>& weatherDetails)
{
	setCity(city);
#ifdef Q_OS_ANDROID
	setGpsCity(city);
#endif

	if (!weatherDetails.isEmpty())
	{
		const st_WeatherInfo& w_info(weatherDetails.first());
		d->now.setWeatherInfo(w_info);
		for(uint i(0); i < 3; ++i)
		{
			if (weatherDetails.count() > i+2)
				d->nextThreeDays[i].setWeatherInfo(weatherDetails.at(i+2));
		}
	}

	const QString* coordinates{&(weatherDetails.first().m_coordinates)};
	const int comma_idx = coordinates->indexOf(',');
	const QString latitude{std::move(coordinates->sliced(1, comma_idx-1))};
	const QString longitude{std::move(coordinates->sliced(comma_idx+1,coordinates->length()-comma_idx-2))};
	appSettings()->addWeatherCity(city, latitude, longitude);
	emit weatherChanged();

	d->coord.setLatitude(latitude.toDouble());
	d->coord.setLongitude(longitude.toDouble());
	return true;
}

void WeatherInfo::requestWeatherByCoordinates()
{
	const auto cacheResult = d->m_dataCache.getWeatherData(d->coord);
	if (WeatherDataCache::isCacheResultValid(cacheResult))
	{
		qWarning() << "--------------"  << " requestWeatherByCoordinates() using cache" << "------------";
		applyWeatherData(cacheResult.first, cacheResult.second);
	}
	else if (d->m_currentBackend)
	{
		qWarning() << "--------------"  << " requestWeatherByCoordinates() request weather for coordinates:  " << d->coord << "------------";
		d->m_currentBackend->requestWeatherInfo(d->coord);
	}
}

void WeatherInfo::requestWeatherByCity()
{
	const auto cacheResult = d->m_dataCache.getWeatherData(d->city);
	if (WeatherDataCache::isCacheResultValid(cacheResult))
	{
		qWarning() << "--------------"  << " requestWeatherByCity() using cache" << "------------";
		applyWeatherData(cacheResult.first, cacheResult.second);
	}
	else if (d->m_currentBackend)
	{
		qWarning() << "--------------"  << " requestWeatherByCity() request weather from backend for city:  " << d->city << "------------";
		d->m_currentBackend->requestWeatherInfo(d->city);
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
	if (!d->canUseGPS)
		return;
#ifdef Q_OS_ANDROID
	if (d->useGps != value)
	{
		d->useGps = value;
		emit useGpsChanged();
		appSettings()->setUseGPS(value);
		if (value)
		{
			setGpsCity(tr("Resquesting GPS info..."));
			connect(d->src, &QGeoPositionInfoSource::positionUpdated, this, &WeatherInfo::positionUpdated);
			//connect(d->src, &QGeoPositionInfoSource::errorOccurred, this, &WeatherInfo::positionError);
			d->src->requestUpdate();
		}
		else
		{
			d->src->stopUpdates();
			d->src->disconnect();
		}
	}

	if (value)
	{
		// if we already have a valid GPS position, do not wait until it updates, but query the city immediately
		if (d->coord.isValid())
		{
			qWarning() << "--------------"  << " setUseGPS(true) -> d->coord.isValid()" << "------------";
			if (gpsWaitTimer && gpsWaitTimer->isActive())
			{
				gpsWaitTimer->stop();
				disconnect(gpsWaitTimer, &QTimer::timeout, nullptr, nullptr);
				m_gpsOnAttempts = 0;
			}
			setCity(QString(), true);
			requestWeatherByCoordinates();
		}
		else
		{
			qWarning() << "--------------"  << " setUseGPS(true) -> !d->coord.isValid()" << "------------";
			if (!gpsWaitTimer)
				gpsWaitTimer = new QTimer{this};

			qWarning() << "--------------"  << " setUseGPS(true) ->GPS Attempts: " << m_gpsOnAttempts;
			switch (m_gpsOnAttempts++)
			{
				case 0:
					qWarning() << "--------------"  << " setUseGPS(true) -> starting timer: " << m_gpsOnAttempts;
					connect(gpsWaitTimer, &QTimer::timeout, this, [this] () { setUseGps(true); });
					gpsWaitTimer->setInterval(5000);
					gpsWaitTimer->start();
					return;
				break;
				case 4: //After 20 seconds, revert back to city mode
					qWarning() << "--------------"  << " setUseGPS(true) -> GPS Failed" << "------------";
					disconnect(gpsWaitTimer, &QTimer::timeout, nullptr, nullptr);
					d->canUseGPS = false;
					setCity(appSettings()->weatherCity(0));
					m_gpsOnAttempts = 0; //can be attempted again any time later
				break;
				default: return;
			}
		}
	}
#else
	Q_UNUSED(value);
#endif
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

void WeatherInfo::refreshWeather()
{
	requestWeatherFor(d->city, d->coord);
}

void WeatherInfo::placeLookUp(const QString& place)
{
	m_locationList.clear();
	m_foundLocations.clear();
	emit locationListChanged();

	if (place.length() >= 5)
	{
		QUrlQuery query;
		QUrl url{u"http://api.openweathermap.org/geo/1.0/direct"_s};
		query.addQueryItem(u"q"_s, place);
		query.addQueryItem(u"appid"_s, u"31d07fed3c1e19a6465c04a40c71e9a0"_s);
		url.setQuery(query);
		if (!m_netAccessManager)
			m_netAccessManager = new QNetworkAccessManager{this};
		QNetworkRequest net_request{url};
		QNetworkReply* reply{m_netAccessManager->get(net_request)};
		connect(reply, &QNetworkReply::finished, this, [this,reply] () {
			buildLocationList(reply->readAll());
		});
	}
}

void WeatherInfo::locationSelected(const uint index)
{
	d->m_currentBackend->requestWeatherInfo(m_foundLocations.at(index).m_name, m_foundLocations.at(index).m_coordinate);
	m_usedLocations.insert(m_locationList.at(index), std::move(m_foundLocations.at(index)));
	m_locationList.clear();
	m_foundLocations.clear();
	emit locationListChanged();
}

void WeatherInfo::requestWeatherFor(const QString& city, const QGeoCoordinate& coord)
{
	if (!city.isEmpty())
		d->m_currentBackend->requestWeatherInfo(city, coord);
}

void WeatherInfo::buildLocationList(const QByteArray& net_data)
{
	if (net_data.length() > 50)
	{
		parseReply(net_data);
		if (m_foundLocations.count() > 0)
		{
			m_locationList.clear();
			for (uint i(0); i < m_foundLocations.count(); ++i)
			{
				const st_LocationInfo* l_info(&(m_foundLocations.at(i)));
				m_locationList.append(l_info->m_name + u" ("_s + l_info->m_state + ' ' +  l_info->m_country + ')');
			}
			emit locationListChanged();
		}
	}
}

void WeatherInfo::parseReply(const QByteArray& replyData)
{
	st_LocationInfo tempData;
	QString word, *strInfo(nullptr);
	uint pos(0), word_start(0), word_end(0);
	bool ignore_untill_lext_bracket(false);

	const QString data{replyData};
	QString::const_iterator itr(data.constBegin());
	const QString::const_iterator itr_end(data.constEnd());
	do {
		if (ignore_untill_lext_bracket)
		{
			if (*itr != '}')
				continue;
		}
		if (((*itr).isLetterOrNumber()))
		{
			if (word_start == 0)
				word_start = pos;
			else
				word_end = pos;
		}
		else {
			switch ((*itr).cell())
			{
				case '-':
					if ((*(itr-1)).cell() == ':')
						word_start = pos;
				break;
				case ':':
					word = data.sliced(word_start, word_end-word_start+1);
					if (word.contains("local_"))
						ignore_untill_lext_bracket = true;
					else
					{
						if (word.contains("name"_L1))
							strInfo = &(tempData.m_name);
						else if (word.contains("lat"_L1))
						{
							tempData.m_coordinate.setLatitude(0);
							strInfo = &(tempData.m_strCoordinate);
						}
						else if (word.contains("lon"_L1))
							strInfo = &(tempData.m_strCoordinate);
						else if (word.contains("cou"_L1))
							strInfo = &(tempData.m_country);
						else
							strInfo = &(tempData.m_state);
					}
					word_start = word_end = 0;
				break;
				case ',':
					if (strInfo)
					{
						*strInfo = data.sliced(word_start, word_end-word_start+1);
						bool ok(false);
						strInfo->left(2).toInt(&ok);
						if (ok)
						{
							if (tempData.m_coordinate.latitude() == 0)
								tempData.m_coordinate.setLatitude(strInfo->toDouble());
							else
								tempData.m_coordinate.setLongitude(strInfo->toDouble());
						}
						strInfo = nullptr;
						word_start = word_end = 0;
					}
				break;
				case '}':
					if (ignore_untill_lext_bracket)
						ignore_untill_lext_bracket = false;
					else
					{
						*strInfo = data.sliced(word_start, word_end-word_start+1);
						strInfo = nullptr;
						m_foundLocations.append(tempData);
						word_start = word_end = 0;
					}
				break;
				default: continue;
			}
		}
	} while (++pos && (++itr != itr_end));
}
