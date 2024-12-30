#ifndef TPWORKOUTSCALENDAR_H
#define TPWORKOUTSCALENDAR_H

#include <QAbstractListModel>
#include <QQmlEngine>

class TPWorkoutsCalendar : public QAbstractListModel
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)
Q_PROPERTY(uint count READ count NOTIFY countChanged)
Q_PROPERTY(QDate initialDate READ initialDate WRITE setInitialDate NOTIFY initialDateChanged FINAL)
Q_PROPERTY(QDate finalDate READ finalDate WRITE setFinalDate NOTIFY finalDateChanged FINAL)
Q_PROPERTY(QDate selectedDate READ selectedDate WRITE setSelectedDate NOTIFY selectedDateChanged FINAL)
Q_PROPERTY(QString mesoName READ mesoName NOTIFY selectedDateChanged FINAL)

enum RoleNames {
	yearRole = Qt::UserRole,
	monthRole = Qt::UserRole+1
};

public:
	explicit inline TPWorkoutsCalendar(QObject* parent = nullptr) : QAbstractListModel{parent}, m_selectedDay{&noDataDay}, m_bReady{false}
	{
		m_roleNames[yearRole] = std::move("year");
		m_roleNames[monthRole] = std::move("month");
	}
	~TPWorkoutsCalendar();

	void scanMesocycles(const uint startMesoIdx = 0);
	Q_INVOKABLE int indexOf(const QDate& date) const;

	inline bool ready() const { return m_bReady; }
	inline uint count() const { return m_monthsList.count(); }
	inline QDate initialDate() const { return m_initialDate; }
	inline void setInitialDate(const QDate& new_date) { m_initialDate = new_date; emit initialDateChanged(); }
	inline QDate finalDate() const { return m_finalDate; }
	inline void setFinalDate(const QDate& new_date) { m_finalDate = new_date; emit finalDateChanged(); }
	inline QDate selectedDate() const { return m_selectedDate; }
	inline void setSelectedDate(const QDate& new_date) { m_selectedDate = new_date; findDateInList(); emit selectedDateChanged(); }
	QString mesoName() const;

	inline int rowCount(const QModelIndex& parent) const override { Q_UNUSED(parent); return count(); }
	QVariant data(const QModelIndex&, int) const override final;
	// return the roles mapping to be used by QML
	inline QHash<int, QByteArray> roleNames() const override final { return m_roleNames; }

public slots:
	void reScanMesocycles(const uint meso_idx, const uint extra_info);

signals:
	void readyChanged();
	void countChanged();
	void initialDateChanged();
	void finalDateChanged();
	void selectedDateChanged();

private:
	struct dayInfo {
		int meso_id;
		int meso_idx;
		int start_day;
		QDate date;
		int tday_id;

		explicit inline dayInfo() : meso_id{-1}, meso_idx{-1}, tday_id{-1}, start_day{-1} {}
	};

	QList<QList<dayInfo*>> m_monthsList;
	QHash<int, QByteArray> m_roleNames;
	dayInfo* m_selectedDay;

	bool m_bReady;
	int m_mesoid, m_mesoidx, m_tdayid;
	QDate m_initialDate, m_finalDate, m_selectedDate;
	static dayInfo noDataDay;

	void findDateInList();
};

#endif // TPWORKOUTSCALENDAR_H
