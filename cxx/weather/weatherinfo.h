#ifndef WEATHERINFO_H
#define WEATHERINFO_H

#include "providerbackend.h"

#include <QObject>
#include <QString>
#include <QtPositioning/qgeopositioninfo.h>
#include <QtPositioning/qgeopositioninfosource.h>
#include <QtQml/qqml.h>
#include <QtQml/qqmllist.h>

class WeatherData : public QObject
{

Q_OBJECT

Q_PROPERTY(QString coordinates READ coordinates WRITE setCoordinates NOTIFY dataChanged)
Q_PROPERTY(QString dayOfWeek READ dayOfWeek WRITE setDayOfWeek NOTIFY dataChanged)
Q_PROPERTY(QString weatherIcon READ weatherIcon WRITE setWeatherIcon NOTIFY dataChanged)
Q_PROPERTY(QString weatherDescription READ weatherDescription WRITE setWeatherDescription NOTIFY dataChanged)
Q_PROPERTY(QString temperature READ temperature WRITE setTemperature NOTIFY dataChanged)
QML_ANONYMOUS

public:
	explicit WeatherData(QObject* parent = nullptr);
	explicit WeatherData(const WeatherData& other);
	explicit WeatherData(const st_WeatherInfo& other);

	QString coordinates() const;
	QString dayOfWeek() const;
	QString weatherIcon() const;
	QString weatherDescription() const;
	QString temperature() const;

	void setCoordinates(const QString& value);
	void setDayOfWeek(const QString& value);
	void setWeatherIcon(const QString& value);
	void setWeatherDescription(const QString& value);
	void setTemperature(const QString& value);

signals:
	void dataChanged();

private:
	QString m_coordinates;
	QString m_dayOfWeek;
	QString m_weather;
	QString m_weatherDescription;
	QString m_temperature;
};

class WeatherInfoPrivate;

class WeatherInfo : public QObject
{

Q_OBJECT

Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)
Q_PROPERTY(bool hasSource READ hasSource NOTIFY readyChanged)
Q_PROPERTY(bool hasValidCity READ hasValidCity NOTIFY cityChanged)
Q_PROPERTY(bool hasValidWeather READ hasValidWeather NOTIFY weatherChanged)
Q_PROPERTY(bool useGps READ useGps WRITE setUseGps NOTIFY useGpsChanged)
Q_PROPERTY(bool canUseGps READ canUseGps CONSTANT)
Q_PROPERTY(QString city READ city WRITE setCity NOTIFY cityChanged)
Q_PROPERTY(WeatherData* weather READ weather NOTIFY weatherChanged)
Q_PROPERTY(QQmlListProperty<WeatherData> forecast READ forecast NOTIFY weatherChanged)

#ifdef Q_OS_ANDROID
Q_PROPERTY(QString gpsCity READ gpsCity WRITE setGpsCity NOTIFY gpsCityChanged)
#endif
QML_ELEMENT

public:
	explicit WeatherInfo(QObject* parent = nullptr);
	~WeatherInfo();

	bool ready() const;
	bool hasSource() const;
	bool useGps() const;
	bool canUseGps() const;
	bool hasValidCity() const;
	bool hasValidWeather() const;
	void setUseGps(const bool value);

	QString city() const;
	void setCity(const QString& value, const bool changeCityOnly = false);

#ifdef Q_OS_ANDROID
	QString gpsCity() const;
	void setGpsCity(const QString& value);
#endif

	WeatherData* weather() const;
	QQmlListProperty<WeatherData> forecast() const;

public slots:
	Q_INVOKABLE void refreshWeather();

private slots:
	void positionUpdated(QGeoPositionInfo gpsPos);
	void positionError(QGeoPositionInfoSource::Error e);
	void handleWeatherData(const st_LocationInfo& location, const QList<st_WeatherInfo>& weatherDetails);
	void switchToNextBackend();

signals:
	void readyChanged();
	void useGpsChanged();
	void cityChanged();
	void weatherChanged();
#ifdef Q_OS_ANDROID
	void gpsCityChanged();
#endif

private:
	bool applyWeatherData(const QString& city, const QList<st_WeatherInfo>& weatherDetails);
	void requestWeatherByCoordinates();
	void requestWeatherByCity();
	void registerBackend(qsizetype index);
	void deregisterCurrentBackend();


	WeatherInfoPrivate* d;
};

#endif // WEATHERINFO_H
