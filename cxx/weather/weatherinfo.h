#ifndef WEATHERINFO_H
#define WEATHERINFO_H

#include "providerbackend.h"

#include <QObject>
#include <QString>
#include <QtPositioning/qgeopositioninfo.h>
#include <QtPositioning/qgeopositioninfosource.h>
#include <QtQml/qqml.h>
#include <QtQml/qqmllist.h>

class QTimer;
class QNetworkAccessManager;

class	WeatherData : public QObject
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

	void setWeatherInfo(const st_WeatherInfo& w_info);

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

Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)
Q_PROPERTY(QString loadMessage READ loadMessage NOTIFY readyChanged)
Q_PROPERTY(bool hasSource READ hasSource NOTIFY readyChanged)
Q_PROPERTY(bool hasValidCity READ hasValidCity NOTIFY cityChanged)
Q_PROPERTY(bool hasValidWeather READ hasValidWeather NOTIFY weatherChanged)
Q_PROPERTY(bool useGps READ useGps WRITE setUseGps NOTIFY useGpsChanged)
Q_PROPERTY(bool canUseGps READ canUseGps CONSTANT)
Q_PROPERTY(QString city READ city WRITE setCity NOTIFY cityChanged)
Q_PROPERTY(QStringList locationList READ locationList NOTIFY locationListChanged)
Q_PROPERTY(WeatherData* weather READ weather NOTIFY weatherChanged)
Q_PROPERTY(QQmlListProperty<WeatherData> forecast READ forecast NOTIFY weatherChanged)

#ifdef Q_OS_ANDROID
Q_PROPERTY(QString gpsCity READ gpsCity WRITE setGpsCity NOTIFY gpsCityChanged)
#endif

public:
	explicit WeatherInfo(QObject* parent = nullptr);
	~WeatherInfo();

	bool ready() const;
	QString loadMessage() const;

	bool hasSource() const;
	bool useGps() const;
	bool canUseGps() const;
	bool hasValidCity() const;
	bool hasValidWeather() const;
	void setUseGps(const bool value);

	QString city() const;
	void setCity(const QString& value, const bool changeCityOnly = false);

	QStringList locationList() const { return m_locationList; }
	Q_INVOKABLE void placeLookUp(const QString& place);
	Q_INVOKABLE void locationSelected(const uint index);

#ifdef Q_OS_ANDROID
	QString gpsCity() const;
	void setGpsCity(const QString& value);
#endif

	WeatherData* weather() const;
	QQmlListProperty<WeatherData> forecast() const;

public slots:
	Q_INVOKABLE void refreshWeather();

private slots:
#ifdef Q_OS_ANDROID
	void positionUpdated(const QGeoPositionInfo& gpsPos);
	void positionError(QGeoPositionInfoSource::Error e);
#endif
	void handleWeatherData(const st_LocationInfo& location, const QList<st_WeatherInfo>& weatherDetails);
	void switchToNextBackend();

signals:
	void readyChanged();
	void useGpsChanged();
	void cityChanged();
	void weatherChanged();
	void locationListChanged();
#ifdef Q_OS_ANDROID
	void gpsCityChanged();
#endif

private:
	bool applyWeatherData(const QString& city, const QList<st_WeatherInfo>& weatherDetails);
	void requestWeatherByCoordinates();
	void requestWeatherByCity();
	void registerBackend(qsizetype index);
	void deregisterCurrentBackend();
	void buildLocationList(const QByteArray& net_data);

	WeatherInfoPrivate* d;
	QNetworkAccessManager* m_netAccessManager;
	QMap<QString, st_LocationInfo> m_usedLocations;
	QList<st_LocationInfo> m_foundLocations;
	QStringList m_locationList;

#ifdef Q_OS_ANDROID
	QTimer* gpsWaitTimer;
	uint m_gpsOnAttempts, m_gpsPosError;
#endif
};

#endif // WEATHERINFO_H
