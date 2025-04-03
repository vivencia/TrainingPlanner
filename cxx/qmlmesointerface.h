#ifndef QMLMESOINTERFACE_H
#define QMLMESOINTERFACE_H

#include <QDate>
#include <QObject>
#include <QVariantMap>

class QmlTDayInterface;
class QmlMesoSplitInterface;
class QmlMesoCalendarInterface;
class DBMesoSplitModel;
class DBTrainingDayModel;

class QQmlComponent;
class QQuickItem;

class QMLMesoInterface : public QObject
{

Q_OBJECT

Q_PROPERTY(bool ownerIsCoach READ ownerIsCoach WRITE setOwnerIsCoach NOTIFY ownerIsCoachChanged FINAL)
Q_PROPERTY(bool hasCoach READ hasCoach WRITE setHasCoach NOTIFY hasCoachChanged FINAL)
Q_PROPERTY(bool realMeso READ realMeso WRITE setRealMeso NOTIFY realMesoChanged FINAL)
Q_PROPERTY(bool ownMeso READ ownMeso WRITE setOwnMeso NOTIFY ownMesoChanged FINAL)
Q_PROPERTY(bool isNewMeso READ isNewMeso NOTIFY isNewMesoChanged FINAL)

Q_PROPERTY(QString nameLabel READ nameLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString coachLabel READ coachLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString clientLabel READ clientLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString typeLabel READ typeLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString startDateLabel READ startDateLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString endDateLabel READ endDateLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString weeksLabel READ weeksLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString splitLabel READ splitLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString notesLabel READ notesLabel NOTIFY labelsChanged FINAL)

Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)
Q_PROPERTY(QString coach READ coach WRITE setCoach NOTIFY coachChanged FINAL)
Q_PROPERTY(QString client READ client WRITE setClient NOTIFY clientChanged FINAL)
Q_PROPERTY(QString type READ type WRITE setType NOTIFY typeChanged FINAL)
Q_PROPERTY(QString fileName READ fileName NOTIFY fileNameChanged FINAL)
Q_PROPERTY(QString file READ file WRITE setFile NOTIFY fileChanged FINAL)
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
		: QObject{parent}, m_mesoComponent{nullptr}, m_mesoIdx{meso_idx}, m_exercisesPage{nullptr}, m_calendarPage{nullptr} {}
	~QMLMesoInterface();

	//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------
	[[nodiscard]] inline const uint mesoIdx() const { return m_mesoIdx; }
	inline void setMesoIdx(const uint new_value) { m_mesoIdx = new_value; }

	[[nodiscard]] inline bool ownerIsCoach() const { return m_bOwnerIsCoach; }
	inline void setOwnerIsCoach(const bool new_value) { if (m_bOwnerIsCoach != new_value) { m_bOwnerIsCoach = new_value; emit ownerIsCoachChanged(); } };

	[[nodiscard]] inline bool hasCoach() const { return m_bHasCoach; }
	inline void setHasCoach(const bool new_value) { if (m_bHasCoach != new_value) { m_bHasCoach = new_value; emit hasCoachChanged(); } };

	[[nodiscard]] inline bool realMeso() const { return m_bRealMeso; }
	void setRealMeso(const bool new_value, const bool bFromQml = true);

	[[nodiscard]] inline bool ownMeso() const { return m_bOwnMeso; }
	void setOwnMeso(const bool new_value, const bool bFromQml = true);

	[[nodiscard]] bool isNewMeso() const;

	QString nameLabel() const;
	QString coachLabel() const;
	QString clientLabel() const;
	QString typeLabel() const;
	QString startDateLabel() const;
	QString endDateLabel() const;
	QString weeksLabel() const;
	QString splitLabel() const;
	QString notesLabel() const;

	[[nodiscard]] inline QString name() const { return m_name; }
	void setName(const QString &new_value, const bool bFromQml = true);
	Q_INVOKABLE void acceptName();

	[[nodiscard]] QString coach() const;
	void setCoach(const QString &new_value, const bool bFromQml = true);

	[[nodiscard]] QString client() const;
	void setClient(const QString &new_value, const bool bFromQml = true);

	[[nodiscard]] inline QString type() const { return m_type; }
	void setType(const QString &new_value, const bool bFromQml = true);

	[[nodiscard]] QString fileName() const;
	[[nodiscard]] inline QString file() const { return m_file; }
	void setFile(const QString &new_value, const bool bFromQml = true);

	[[nodiscard]] inline QString strStartDate() const { return m_strStartDate; }
	[[nodiscard]] inline QDate startDate() const { return m_startDate; }
	[[nodiscard]] inline QDate minimumMesoStartDate() const { return m_minimumMesoStartDate; }
	void setStartDate(const QDate &new_value, const bool bFromQml = true);
	void setMinimumMesoStartDate(const QDate &new_value);
	Q_INVOKABLE void acceptStartDate();

	[[nodiscard]] inline QString strEndDate() const { return m_strEndDate; }
	[[nodiscard]] inline QDate endDate() const { return m_endDate; }
	[[nodiscard]] inline QDate maximumMesoEndDate() const { return m_maximumMesoEndDate; }
	void setEndDate(const QDate &new_value, const bool bFromQml = true);
	void setMaximumMesoEndDate(const QDate &new_value);
	Q_INVOKABLE void acceptEndDate();

	[[nodiscard]] inline QString weeks() const { return m_weeks; }
	void setWeeks(const QString &new_value, const bool bFromQml = true);

	[[nodiscard]] inline QString split() const { return m_split; }
	void setSplit(const QString &new_value, const bool bFromQml = true);

	[[nodiscard]] inline QString notes() const { return m_notes; }
	void setNotes(const QString &new_value, const bool bFromQml = true);

	[[nodiscard]] inline QString muscularGroupA() const { return m_muscularGroup.at(0); }
	void setMuscularGroupA(const QString &new_value, const bool bFromQml = true);
	[[nodiscard]] inline QString muscularGroupB() const { return m_muscularGroup.at(1); }
	void setMuscularGroupB(const QString &new_value, const bool bFromQml = true);
	[[nodiscard]] inline QString muscularGroupC() const { return m_muscularGroup.at(2); }
	void setMuscularGroupC(const QString &new_value, const bool bFromQml = true);
	[[nodiscard]] inline QString muscularGroupD() const { return m_muscularGroup.at(3); }
	void setMuscularGroupD(const QString &new_value, const bool bFromQml = true);
	[[nodiscard]] inline QString muscularGroupE() const { return m_muscularGroup.at(4); }
	void setMuscularGroupE(const QString &new_value, const bool bFromQml = true);
	[[nodiscard]] inline QString muscularGroupF() const { return m_muscularGroup.at(5); }
	void setMuscularGroupF(const QString &new_value, const bool bFromQml = true);
	[[nodiscard]] inline QString muscularGroupR() const { return m_muscularGroup.at(6); }

	[[nodiscard]] inline int newMesoFieldCounter() const { return m_newMesoFieldCounter; }
	void setNewMesoFieldCounter(const int new_value) { m_newMesoFieldCounter = new_value; emit newMesoFieldCounterChanged(new_value);}

	//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------

	Q_INVOKABLE void changeMesoCalendar(const bool preserve_old_cal, const bool preserve_untilyesterday);
	Q_INVOKABLE void getCalendarPage();
	Q_INVOKABLE void getExercisesPlannerPage();
	Q_INVOKABLE void getTrainingDayPage(const QDate &date);

	void getMesocyclePage();
	void exportMeso(const bool bShare, const bool bCoachInfo);
	void importMeso(const QString &filename = QString());
	Q_INVOKABLE void sendMesocycleFileToServer();

	[[nodiscard]] DBMesoSplitModel *plannerSplitModel(const QChar &splitLetter);
	[[nodiscard]] DBTrainingDayModel *tDayModelForToday();

signals:
	void ownerIsCoachChanged();
	void hasCoachChanged();
	void realMesoChanged();
	void ownMesoChanged();
	void isNewMesoChanged();
	void labelsChanged();
	void nameChanged();
	void coachChanged();
	void clientChanged();
	void typeChanged();
	void fileNameChanged();
	void fileChanged();
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

	void displayMessageOnAppWindow(const int message_id, const QString &filename = QString());
	void addPageToMainMenu(const QString &label, QQuickItem *page);
	void removePageFromMainMenu(QQuickItem *page);

private:
	QQmlComponent *m_mesoComponent;
	QQuickItem *m_mesoPage;
	QVariantMap m_mesoProperties;

	//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------
	uint m_mesoIdx;
	bool m_bOwnerIsCoach, m_bHasCoach, m_bRealMeso, m_bOwnMeso;
	QString m_name, m_coach, m_client, m_type, m_file, m_strStartDate, m_strEndDate, m_weeks, m_split, m_notes;
	QDate m_startDate, m_endDate, m_minimumMesoStartDate, m_maximumMesoEndDate;
	QStringList m_muscularGroup;
	int m_newMesoFieldCounter;
	//----------------------------------------------------PAGE PROPERTIES-----------------------------------------------------------------

	QMap<QDate,QmlTDayInterface*> m_tDayPages;
	QmlMesoSplitInterface *m_exercisesPage;
	QmlMesoCalendarInterface *m_calendarPage;

	void createMesocyclePage();
	void createMesocyclePage_part2();
	void setPropertiesBasedOnUseMode();
	void updateMuscularGroupFromOutside(const uint splitIndex);
};

#endif // QMLMESOINTERFACE_H
