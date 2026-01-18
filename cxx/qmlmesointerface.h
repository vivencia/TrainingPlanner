#pragma once

#include <QDate>
#include <QObject>
#include <QVariantMap>

QT_FORWARD_DECLARE_CLASS(DBMesocyclesModel)
QT_FORWARD_DECLARE_CLASS(QmlWorkoutInterface)
QT_FORWARD_DECLARE_CLASS(QmlMesoSplitInterface)
QT_FORWARD_DECLARE_CLASS(QmlMesoCalendarInterface)
QT_FORWARD_DECLARE_CLASS(QQmlComponent)
QT_FORWARD_DECLARE_CLASS(QQuickItem)

class QMLMesoInterface : public QObject
{

Q_OBJECT

Q_PROPERTY(bool mesoNameOK READ mesoNameOK NOTIFY mesoNameOKChanged FINAL)
Q_PROPERTY(bool startDateOK READ startDateOK NOTIFY startDateOKChanged FINAL)
Q_PROPERTY(bool endDateOK READ endDateOK NOTIFY endDateOKChanged FINAL)
Q_PROPERTY(bool splitOK READ splitOK NOTIFY splitOKChanged FINAL)
Q_PROPERTY(bool realMeso READ realMeso WRITE setRealMeso NOTIFY realMesoChanged FINAL)
Q_PROPERTY(bool ownMeso READ ownMeso CONSTANT FINAL)
Q_PROPERTY(bool isTempMeso READ isTempMeso NOTIFY isTempMesoChanged FINAL)
Q_PROPERTY(bool canExport READ canExport NOTIFY canExportChanged FINAL)
Q_PROPERTY(bool coachIsMainUser READ coachIsMainUser CONSTANT FINAL)

Q_PROPERTY(QString mesoNameErrorTooltip READ mesoNameErrorTooltip NOTIFY mesoNameOKChanged FINAL)
Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)
Q_PROPERTY(QString coachName READ coachName CONSTANT FINAL)
Q_PROPERTY(QString client READ client WRITE setClient NOTIFY clientChanged FINAL)
Q_PROPERTY(QString type READ type WRITE setType NOTIFY typeChanged FINAL)
Q_PROPERTY(QString displayFileName READ displayFileName NOTIFY displayFileNameChanged FINAL)
Q_PROPERTY(QString fileName READ fileName WRITE setFileName NOTIFY fileNameChanged FINAL)
Q_PROPERTY(QString strStartDate READ strStartDate NOTIFY startDateChanged FINAL)
Q_PROPERTY(QString strEndDate READ strEndDate NOTIFY endDateChanged FINAL)
Q_PROPERTY(QString weeks READ weeks NOTIFY weeksChanged FINAL)
Q_PROPERTY(QString split READ split WRITE setSplit NOTIFY splitChanged FINAL)
Q_PROPERTY(QString notes READ notes WRITE setNotes NOTIFY notesChanged FINAL)

Q_PROPERTY(QDate startDate READ startDate WRITE setStartDate NOTIFY startDateChanged FINAL)
Q_PROPERTY(QDate endDate READ endDate WRITE setEndDate NOTIFY endDateChanged FINAL)
Q_PROPERTY(QDate minimumMesoStartDate READ minimumMesoStartDate NOTIFY minimumStartDateChanged FINAL)
Q_PROPERTY(QDate maximumMesoEndDate READ maximumMesoEndDate CONSTANT FINAL)

public:
	explicit inline QMLMesoInterface(DBMesocyclesModel *meso_model, const uint meso_idx)
		: QObject{reinterpret_cast<QObject*>(meso_model)}, m_mesoModel{meso_model}, m_mesoComponent{nullptr},
			m_mesoPage{nullptr}, m_mesoIdx{meso_idx}, m_splitsPage{nullptr}, m_calendarPage{nullptr} {}
	inline ~QMLMesoInterface() { cleanUp(); }
	void cleanUp();

	[[nodiscard]] bool mesoNameOK() const;
	[[nodiscard]] bool startDateOK() const;
	[[nodiscard]] bool endDateOK() const;
	[[nodiscard]] bool splitOK() const;

	[[nodiscard]] inline const uint mesoIdx() const { return m_mesoIdx; }
	void setMesoIdx(const uint new_value);

	[[nodiscard]] bool realMeso() const;
	void setRealMeso(const bool new_value);
	[[nodiscard]] bool ownMeso() const;
	[[nodiscard]] bool isTempMeso() const;
	[[nodiscard]] bool canExport() const;
	[[nodiscard]] bool coachIsMainUser() const;

	inline QString mesoNameErrorTooltip() const { return m_nameError; }
	[[nodiscard]] QString name() const;
	void setName(const QString &new_name);

	[[nodiscard]] QString coachName() const;

	[[nodiscard]] QString client() const;
	void setClient(const QString &new_value);

	[[nodiscard]] QString type() const;
	void setType(const QString &new_value);

	[[nodiscard]] QString displayFileName() const;
	[[nodiscard]] QString fileName() const;
	void setFileName(const QString &new_filename);

	[[nodiscard]] QDate startDate() const;
	void setStartDate(const QDate &new_startdate);
	[[nodiscard]] inline QDate minimumMesoStartDate() const { return m_minimumMesoStartDate; }
	void setMinimumMesoStartDate(const QDate &new_value);
	[[nodiscard]] inline QString strStartDate() const { return m_strStartDate; }

	[[nodiscard]] QDate endDate() const;
	void setEndDate(const QDate &new_enddate);
	[[nodiscard]] inline QDate maximumMesoEndDate() const { return m_maximumMesoEndDate; }
	void setMaximumMesoEndDate(const QDate &new_value);
	[[nodiscard]] inline QString strEndDate() const { return m_strEndDate; }

	[[nodiscard]] QString weeks() const;

	[[nodiscard]] QString split() const;
	void setSplit(const QString &new_split);

	[[nodiscard]] QString notes() const;
	void setNotes(const QString &new_value);

	[[nodiscard]] Q_INVOKABLE QString muscularGroup(const QString &split) const;
	Q_INVOKABLE void setMuscularGroup(const QString &split, const QString &new_value);

	Q_INVOKABLE void getCalendarPage();
	Q_INVOKABLE void getExercisesPlannerPage();
	Q_INVOKABLE void getWorkoutPage(const QDate &date);

	void getMesocyclePage(const bool new_meso);
	Q_INVOKABLE void sendMesocycleFileToClient();
	Q_INVOKABLE void incorporateMeso();

signals:
	void mesoNameOKChanged();
	void startDateOKChanged();
	void endDateOKChanged();
	void realMesoChanged();
	void splitOKChanged();
	void isTempMesoChanged();
	void canExportChanged();
	void labelsChanged();
	void nameChanged();
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

private:
	QQmlComponent *m_mesoComponent;
	QQuickItem *m_mesoPage;
	QVariantMap m_mesoProperties;
	DBMesocyclesModel *m_mesoModel;

	uint m_mesoIdx;
	QString m_strStartDate, m_strEndDate, m_nameError;
	QDate m_minimumMesoStartDate, m_maximumMesoEndDate;

	QHash<QDate,QmlWorkoutInterface*> m_workoutPages;
	QmlMesoSplitInterface *m_splitsPage;
	QmlMesoCalendarInterface *m_calendarPage;

	void createMesocyclePage(const bool new_meso);
	void createMesocyclePage_part2();
	void verifyMesoRequiredFieldsStatus();
};
