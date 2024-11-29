#ifndef QMLMESOSPLITINTERFACE_H
#define QMLMESOSPLITINTERFACE_H

#include <QObject>
#include <QVariantMap>

class DBMesoSplitModel;

class QQmlApplicationEngine;
class QQmlComponent;
class QQuickItem;
class QQuickWindow;

class QmlMesoSplitInterface : public QObject
{

Q_OBJECT

public:
	explicit inline QmlMesoSplitInterface(QObject* parent, QQmlApplicationEngine* qmlEngine, QQuickWindow* mainWindow, const uint meso_idx)
		: QObject{parent}, m_qmlEngine(qmlEngine), m_mainWindow(mainWindow), m_plannerComponent(nullptr), m_splitComponent(nullptr), m_mesoIdx(meso_idx) {}
	~QmlMesoSplitInterface();

	void setMesoIdx(const uint new_meso_idx);

	Q_INVOKABLE void getExercisesPlannerPage();
	Q_INVOKABLE void changeMuscularGroup(const QString& new_musculargroup, DBMesoSplitModel* splitModel);
	Q_INVOKABLE void swapMesoPlans(const QString& splitLetter1, const QString& splitLetter2);
	Q_INVOKABLE void loadSplitFromPreviousMeso(DBMesoSplitModel* splitModel);
	Q_INVOKABLE void simpleExercisesList(DBMesoSplitModel* splitModel, const bool show, const bool multi_sel = false, const uint exercise_idx = 0);
	Q_INVOKABLE void exportMesoSplit(const bool bShare, const QString& splitLetter);
	Q_INVOKABLE void importMesoSplit(const QString& filename = QString());

	inline QQuickItem* getSplitPage(const QChar& splitLetter) const { return m_splitPages.value(splitLetter); }
	inline DBMesoSplitModel* splitModel(const QChar& splitLetter) { return m_splitModels.value(splitLetter); } //will return nullptr if the page was naver created

signals:
	void plannerPageCreated();
	void displayMessageOnAppWindow(const int message_id, const QString& filename = QString());
	void addPageToMainMenu(const QString& label, QQuickItem* page);
	void removePageFromMainMenu(QQuickItem* page);

public slots:
	void exerciseSelected();
	void hideSimpleExercisesList();

private:
	QQmlApplicationEngine* m_qmlEngine;
	QQuickWindow* m_mainWindow;
	QQmlComponent* m_plannerComponent;
	QQuickItem* m_plannerPage;
	QVariantMap m_plannerProperties;

	QQmlComponent* m_splitComponent;
	QMap<QChar,QQuickItem*> m_splitPages;
	QMap<QChar,DBMesoSplitModel*> m_splitModels;
	QVariantMap m_splitProperties;
	uint m_mesoIdx;

	DBMesoSplitModel* m_simpleExercisesListRequester;
	uint m_simpleExercisesListExerciseIdx;

	void createPlannerPage();
	void createPlannerPage_part2();
	void createMesoSplitPage(const QChar& splitletter);
	void createMesoSplitPage_part2(const QChar& splitletter);
	void initializeSplitModels();
	void setSplitPageProperties(QQuickItem* splitPage, const DBMesoSplitModel* const splitModel);
	void updateMuscularGroup(DBMesoSplitModel* splitModel);
};

#endif // QMLMESOSPLITINTERFACE_H
