#ifndef QMLMESOINTERFACE_H
#define QMLMESOINTERFACE_H

#include <QDate>
#include <QObject>
#include <QVariantMap>

class QmlWorkoutInterface;
class QmlMesoSplitInterface;
class QmlMesoCalendarInterface;
class DBMesoSplitModel;
class DBWorkoutModel;

class QQmlComponent;
class QQuickItem;

class QMLMesoInterface : public QObject
{

Q_OBJECT

Q_PROPERTY(bool mesoNameOK READ mesoNameOK WRITE setMesoNameOK NOTIFY mesoNameOKChanged FINAL)
Q_PROPERTY(bool startDateOK READ startDateOK WRITE setStartDateOK NOTIFY startDateOKChanged FINAL)
Q_PROPERTY(bool endDateOK READ endDateOK WRITE setEndDateOK NOTIFY endDateOKChanged FINAL)
Q_PROPERTY(bool realMeso READ realMeso WRITE setRealMeso NOTIFY realMesoChanged FINAL)
Q_PROPERTY(bool ownMeso READ ownMeso NOTIFY ownMesoChanged FINAL)
Q_PROPERTY(bool splitOK READ splitOK NOTIFY splitOKChanged FINAL)
Q_PROPERTY(bool isNewMeso READ isNewMeso NOTIFY isNewMesoChanged FINAL)
Q_PROPERTY(bool isTempMeso READ isTempMeso NOTIFY isTempMesoChanged FINAL)
Q_PROPERTY(bool canExport READ canExport NOTIFY canExportChanged FINAL)

Q_PROPERTY(QString mesoNameErrorTooltip READ mesoNameErrorTooltip NOTIFY mesoNameOKChanged FINAL)
Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)
Q_PROPERTY(QString coach READ coach WRITE setCoach NOTIFY coachChanged FINAL)
Q_PROPERTY(QString client READ client WRITE setClient NOTIFY clientChanged FINAL)
Q_PROPERTY(QString type READ type WRITE setType NOTIFY typeChanged FINAL)
Q_PROPERTY(QString displayFileName READ displayFileName NOTIFY displayFileNameChanged FINAL)
Q_PROPERTY(QString fileName READ fileName WRITE setFileName NOTIFY fileNameChanged FINAL)
Q_PROPERTY(QString strStartDate READ strStartDate NOTIFY startDateChanged FINAL)
Q_PROPERTY(QString strEndDate READ strEndDate NOTIFY endDateChanged FINAL)
Q_PROPERTY(QString weeks READ weeks NOTIFY weeksChanged FINAL)
Q_PROPERTY(QString split READ split WRITE setSplit NOTIFY splitChanged FINAL)
Q_PROPERTY(QString notes READ notes WRITE setNotes NOTIFY notesChanged FINAL)
Q_PROPERTY(QString muscularGroupA READ muscularGroupA WRITE setMuscularGroupA NOTIFY muscularGroupAChanged FINAL)
Q_PROPERTY(QString muscularGroupB READ muscularGroupB WRITE setMuscularGroupB NOTIFY muscularGroupBChanged FINAL)
Q_PROPERTY(QString muscularGroupC READ muscularGroupC WRITE setMuscularGroupC NOTIFY muscularGroupCChanged FINAL)
Q_PROPERTY(QString muscularGroupD READ muscularGroupD WRITE setMuscularGroupD NOTIFY muscularGroupDChanged FINAL)
Q_PROPERTY(QString muscularGroupE READ muscularGroupE WRITE setMuscularGroupE NOTIFY muscularGroupEChanged FINAL)
Q_PROPERTY(QString muscularGroupF READ muscularGroupF WRITE setMuscularGroupF NOTIFY muscularGroupFChanged FINAL)
Q_PROPERTY(QString muscularGroupR READ muscularGroupR CONSTANT FINAL)

Q_PROPERTY(int newMesoFieldCounter READ newMesoFieldCounter WRITE setNewMesoFieldCounter NOTIFY newMesoFieldCounterChanged FINAL)

Q_PROPERTY(QDate startDate READ startDate WRITE setStartDate NOTIFY startDateChanged FINAL)
Q_PROPERTY(QDate endDate READ endDate WRITE setEndDate NOTIFY endDateChanged FINAL)
Q_PROPERTY(QDate minimumMesoStartDate READ minimumMesoStartDate NOTIFY minimumStartDateChanged FINAL)
Q_PROPERTY(QDate maximumMesoEndDate READ maximumMesoEndDate CONSTANT FINAL)

public:
	explicit inline QMLMesoInterface(QObject *parent, const uint meso_idx)
		: QObject{parent}, m_mesoComponent{nullptr}, m_mesoIdx{meso_idx}, m_exercisesPage{nullptr}, m_calendarPage{nullptr},
				m_canSendMesoToServer{false} {}
	inline ~QMLMesoInterface() { cleanUp(); }

	void cleanUp();
	//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------
	bool isMesoNameOK(const QString &meso_name) const;
	[[nodiscard]] inline bool mesoNameOK() const { return m_mesoNameOK; }
	void setMesoNameOK(const bool nameok);

	[[nodiscard]] inline bool startDateOK() const { return m_startDateOK; }
	void setStartDateOK(const bool dateok);
	[[nodiscard]] inline bool endDateOK() const { return m_endDateOK; }
	void setEndDateOK(const bool dateok);

	[[nodiscard]] inline const uint mesoIdx() const { return m_mesoIdx; }
	inline void setMesoIdx(const uint new_value) { m_mesoIdx = new_value; }

	[[nodiscard]] bool realMeso() const;
	void setRealMeso(const bool new_value);

	[[nodiscard]] bool splitOK() const;

	[[nodiscard]] bool ownMeso() const;
	[[nodiscard]] bool isNewMeso() const;
	[[nodiscard]] bool isTempMeso() const;
	[[nodiscard]] inline bool canExport() const { return m_bCanExport; }

	inline QString mesoNameErrorTooltip() const { return m_nameError; }
	[[nodiscard]] QString name() const;
	void setName(const QString &new_name);

	[[nodiscard]] QString coach() const;
	void setCoach(const QString &new_value);

	[[nodiscard]] QString client() const;
	void setClient(const QString &new_value);

	[[nodiscard]] QString type() const;
	void setType(const QString &new_value);

	[[nodiscard]] QString displayFileName() const;
	[[nodiscard]] QString fileName() const;
	void setFileName(const QString &new_filename);

	[[nodiscard]] inline QString strStartDate() const { return m_strStartDate; }
	[[nodiscard]] QDate startDate() const;
	[[nodiscard]] inline QDate minimumMesoStartDate() const { return m_minimumMesoStartDate; }
	void setStartDate(const QDate &new_startdate);
	void setMinimumMesoStartDate(const QDate &new_value);

	[[nodiscard]] inline QString strEndDate() const { return m_strEndDate; }
	[[nodiscard]] inline QDate endDate() const { return m_endDate; }
	[[nodiscard]] inline QDate maximumMesoEndDate() const { return m_maximumMesoEndDate; }
	void setEndDate(const QDate &new_enddate);
	void setMaximumMesoEndDate(const QDate &new_value);

	[[nodiscard]] QString weeks() const;

	[[nodiscard]] QString split() const;
	void setSplit(const QString &new_split);

	[[nodiscard]] QString notes() const;
	void setNotes(const QString &new_value);

	[[nodiscard]] QString muscularGroupA() const;
	void setMuscularGroupA(const QString &new_value);
	[[nodiscard]] QString muscularGroupB() const;
	void setMuscularGroupB(const QString &new_value);
	[[nodiscard]] QString muscularGroupC() const;
	void setMuscularGroupC(const QString &new_value);
	[[nodiscard]] QString muscularGroupD() const;
	void setMuscularGroupD(const QString &new_value);
	[[nodiscard]] QString muscularGroupE() const;
	void setMuscularGroupE(const QString &new_value);
	[[nodiscard]] QString muscularGroupF() const;
	void setMuscularGroupF(const QString &new_value);
	[[nodiscard]] QString muscularGroupR() const;

	[[nodiscard]] inline int newMesoFieldCounter() const { return m_newMesoFieldCounter; }
	void setNewMesoFieldCounter(const int new_value) { m_newMesoFieldCounter = new_value; emit newMesoFieldCounterChanged(new_value);}

	//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------

	Q_INVOKABLE void changeMesoCalendar(const bool preserve_old_cal);
	Q_INVOKABLE void getCalendarPage();
	Q_INVOKABLE void getExercisesPlannerPage();
	Q_INVOKABLE void getWorkoutPage(const QDate &date);

	void getMesocyclePage();
	Q_INVOKABLE inline bool canSendMesoToServer() const { return m_canSendMesoToServer; }
	Q_INVOKABLE void sendMesocycleFileToServer();
	Q_INVOKABLE void incorporateMeso();

signals:
	void mesoNameOKChanged();
	void startDateOKChanged();
	void endDateOKChanged();
	void realMesoChanged();
	void ownMesoChanged();
	void splitOKChanged();
	void isNewMesoChanged();
	void isTempMesoChanged();
	void canExportChanged();
	void labelsChanged();
	void nameChanged();
	void coachChanged();
	void clientChanged();
	void typeChanged();
	void displayFileNameChanged();
	void fileNameChanged();
	void startDateChanged();
	void endDateChanged();
	void minimumStartDateChanged();
	void weeksChanged();
	void splitChanged();
	void notesChanged();
	void muscularGroupAChanged();
	void muscularGroupBChanged();
	void muscularGroupCChanged();
	void muscularGroupDChanged();
	void muscularGroupEChanged();
	void muscularGroupFChanged();
	void newMesoFieldCounterChanged(const int fieldCounter);
	//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------

private:
	QQmlComponent *m_mesoComponent;
	QQuickItem *m_mesoPage;
	QVariantMap m_mesoProperties;

	//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------
	uint m_mesoIdx;
	bool m_bCanExport, m_mesoNameOK, m_startDateOK, m_endDateOK, m_canSendMesoToServer;
	QString m_strStartDate, m_strEndDate, m_nameError;
	QDate m_startDate, m_endDate, m_minimumMesoStartDate, m_maximumMesoEndDate;
	int m_newMesoFieldCounter;
	//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------

	QHash<QDate,QmlWorkoutInterface*> m_workoutPages;
	QmlMesoSplitInterface *m_exercisesPage;
	QmlMesoCalendarInterface *m_calendarPage;

	void createMesocyclePage();
	void createMesocyclePage_part2();
	inline bool isSplitOK(const QString &split) const;
};

#endif // QMLMESOINTERFACE_H
