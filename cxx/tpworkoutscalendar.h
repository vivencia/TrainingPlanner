#ifndef TPWORKOUTSCALENDAR_H
#define TPWORKOUTSCALENDAR_H

#include <QAbstractListModel>
#include <QQmlEngine>

struct st_workoutDayInfo;

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
Q_PROPERTY(QString trainingDay READ trainingDay NOTIFY workoutChanged FINAL)
Q_PROPERTY(QString splitLetter READ splitLetter NOTIFY workoutChanged FINAL)
Q_PROPERTY(bool workoutCompleted READ workoutCompleted NOTIFY workoutChanged FINAL)
Q_PROPERTY(bool canViewWorkout READ canViewWorkout WRITE setCanViewWorkout NOTIFY canViewWorkoutChanged FINAL)

enum RoleNames {
	yearRole = Qt::UserRole,
	monthRole = Qt::UserRole+1
};

public:
	explicit inline TPWorkoutsCalendar(QObject* parent = nullptr) : QAbstractListModel{parent}, m_selectedDay{&noDataDay},
					m_selectedDayWorkoutInfo{noWorkoutDay}, m_bReady{false}, m_bCanViewWorkout{false}
	{
		m_roleNames[yearRole] = std::move("year");
		m_roleNames[monthRole] = std::move("month");
	}
	~TPWorkoutsCalendar();

	void scanMesocycles(const uint startMesoIdx = 0);
	Q_INVOKABLE int indexOf(const QDate& date) const;
	Q_INVOKABLE void viewSelectedWorkout();

	inline bool ready() const { return m_bReady; }
	inline uint count() const { return m_monthsList.count(); }
	inline QDate initialDate() const { return m_initialDate; }
	inline void setInitialDate(const QDate& new_date) { m_initialDate = new_date; emit initialDateChanged(); }
	inline QDate finalDate() const { return m_finalDate; }
	inline void setFinalDate(const QDate& new_date) { m_finalDate = new_date; emit finalDateChanged(); }
	inline QDate selectedDate() const { return m_selectedDate; }
	inline void setSelectedDate(const QDate& new_date) { m_selectedDate = new_date; findDateInList(); emit selectedDateChanged(); }
	QString mesoName() const;
	QString trainingDay() const;
	QString splitLetter() const;
	bool workoutCompleted() const;
	inline bool canViewWorkout() const { return m_bCanViewWorkout; }
	inline void setCanViewWorkout(const bool can_view) { m_bCanViewWorkout = can_view; emit canViewWorkoutChanged(); }

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
	void workoutChanged();
	void canViewWorkoutChanged();

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
	st_workoutDayInfo* m_selectedDayWorkoutInfo;
	QList<st_workoutDayInfo*> m_workoutInfoList;

	bool m_bReady, m_bCanViewWorkout;
	int m_mesoid, m_mesoidx, m_tdayid;
	QDate m_initialDate, m_finalDate, m_selectedDate;
	static dayInfo noDataDay;
	static st_workoutDayInfo* noWorkoutDay;

	void findDateInList();
	void getWorkoutInfo();
	void findWorkoutInList();
};

#endif // TPWORKOUTSCALENDAR_H
