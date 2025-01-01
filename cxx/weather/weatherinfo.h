#ifndef WEATHERINFO_H
#define WEATHERINFO_H

#include "openweathermapbackend.h"

#include <QGeoPositionInfo>
#include <QGeoPositionInfoSource>
#include <QObject>
#include <qqml.h>
#include <QQmlListProperty>
#include <QString>

class WeatherData : public QObject
{

Q_OBJECT

Q_PROPERTY(QString coordinates READ coordinates NOTIFY dataChanged)
Q_PROPERTY(QString dayOfWeek READ dayOfWeek NOTIFY dataChanged)
Q_PROPERTY(QString weatherIcon READ weatherIcon NOTIFY dataChanged)
Q_PROPERTY(QString weatherDescription READ weatherDescription NOTIFY dataChanged)
Q_PROPERTY(QString temperature READ temperature NOTIFY dataChanged)
Q_PROPERTY(QString extraInfo READ extraInfo NOTIFY dataChanged)
Q_PROPERTY(QString provider READ provider NOTIFY dataChanged)
Q_PROPERTY(QString minMaxTemperatures READ minMaxTemperatures NOTIFY dataChanged)

QML_ANONYMOUS

public:
	explicit inline WeatherData(QObject* parent = nullptr) : QObject{parent} {}

	inline QString coordinates() const { return m_coordinates; }
	inline QString dayOfWeek() const { return m_dayOfWeek; }
	inline QString weatherIcon() const { return m_icon; }
	inline QString weatherDescription() const { return m_description; }
	inline QString temperature() const { return m_temperature; }
	inline QString extraInfo() const { return m_extra_info; }
	inline QString provider() const { return m_provider; }
	inline QString minMaxTemperatures() const { return m_minmax; }

	void setWeatherInfo(const st_WeatherInfo& w_info, const st_WeatherInfo* w_currentdayforecast = nullptr);

signals:
	void dataChanged();

private:
	QString m_coordinates;
	QString m_dayOfWeek;
	QString m_icon;
	QString m_description;
	QString m_temperature;
	QString m_minmax;
	QString m_extra_info;
	QString m_provider;
};

class WeatherInfoPrivate;

class WeatherInfo : public QObject
{

Q_OBJECT

Q_PROPERTY(bool canUseGps READ canUseGps CONSTANT)
Q_PROPERTY(QString city READ city WRITE setCity NOTIFY cityChanged)
Q_PROPERTY(QString gpsCity READ gpsCity WRITE setGpsCity NOTIFY gpsCityChanged)
Q_PROPERTY(QStringList locationList READ locationList NOTIFY locationListChanged)
Q_PROPERTY(WeatherData* weather READ weather NOTIFY weatherChanged)
Q_PROPERTY(QQmlListProperty<WeatherData> forecast READ forecast NOTIFY weatherChanged)

public:
	explicit WeatherInfo(QObject* parent = nullptr);
	~WeatherInfo();

	bool canUseGps() const;
	QString city() const;
	void setCity(const QString& value);
	QString gpsCity() const;
	void setGpsCity(const QString& value);
	QStringList locationList() const { return m_locationList; }
	WeatherData* weather() const;
	QQmlListProperty<WeatherData> forecast() const;

#ifdef Q_OS_ANDROID
	Q_INVOKABLE void requestWeatherForGpsCity();
#endif
	Q_INVOKABLE void requestWeatherForSavedCity(const uint index);
	Q_INVOKABLE void refreshWeather();
	Q_INVOKABLE void searchForCities(const QString& place);
	Q_INVOKABLE void locationSelected(const uint index);

private slots:
#ifdef Q_OS_ANDROID
	void positionUpdated(const QGeoPositionInfo& gpsPos);
	void positionError(QGeoPositionInfoSource::Error e);
	void gotGPSLocation(const QString& city, const QGeoCoordinate& coord);
#endif
	void handleWeatherData(const st_LocationInfo& location, const QList<st_WeatherInfo>& weatherDetails);
	void buildLocationsList(QList<st_LocationInfo>* foundLocations);

signals:
	void cityChanged();
	void gpsCityChanged();
	void weatherChanged();
	void locationListChanged();

private:
	bool applyWeatherData(const QString& city, const QList<st_WeatherInfo>& weatherDetails);

	WeatherInfoPrivate* d;
	QMap<QString, st_LocationInfo> m_usedLocations;
	QList<st_LocationInfo>* m_foundLocations;
	st_LocationInfo m_gpsLocation;
	QStringList m_locationList;
};

#endif // WEATHERINFO_H
