#ifndef TPWORKOUTSCALENDAR_H
#define TPWORKOUTSCALENDAR_H

#include <QAbstractListModel>
#include <QQmlEngine>

class TPWorkoutsCalendar : public QAbstractListModel
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(uint count READ count NOTIFY countChanged)
Q_PROPERTY(QDate initialDate READ initialDate WRITE setInitialDate NOTIFY initialDateChanged FINAL)
Q_PROPERTY(QDate finalDate READ finalDate WRITE setFinalDate NOTIFY finalDateChanged FINAL)

enum RoleNames {
	yearRole = Qt::UserRole,
	monthRole = Qt::UserRole+1,
	dayRole = Qt::UserRole+2
};

public:
	explicit inline TPWorkoutsCalendar(QObject* parent = nullptr) : QAbstractListModel{parent}
	{
		m_roleNames[yearRole] = std::move("year");
		m_roleNames[monthRole] = std::move("month");
		//m_roleNames[dayRole] = std::move("day");
	}
	~TPWorkoutsCalendar();
	inline uint count() const { return m_monthsList.count(); }

	void scanMesocycles(const uint startMesoIdx = 0);

	inline int rowCount(const QModelIndex& parent) const override { Q_UNUSED(parent); return count(); }
	QVariant data(const QModelIndex&, int) const override final;
	// return the roles mapping to be used by QML
	inline QHash<int, QByteArray> roleNames() const override final { return m_roleNames; }

	inline QDate initialDate() const { return m_initialDate; }
	inline void setInitialDate(const QDate& new_date) { m_initialDate = new_date; emit initialDateChanged(); }
	inline QDate finalDate() const { return m_finalDate; }
	inline void setFinalDate(const QDate& new_date) { m_finalDate = new_date; emit finalDateChanged(); }

public slots:
	void reScanMesocycles(const uint meso_idx, const uint extra_info);

signals:
	void countChanged();
	void initialDateChanged();
	void finalDateChanged();

private:
	struct dayInfo {
		int meso_id;
		int meso_idx;
		QDate date;
		int tday_id;

		explicit inline dayInfo() : meso_id{-1}, meso_idx{-1}, tday_id{-1} {}
	};

	QList<QList<dayInfo*>> m_monthsList;
	QHash<int, QByteArray> m_roleNames;

	QDate m_initialDate, m_finalDate;
};

#endif // TPWORKOUTSCALENDAR_H
