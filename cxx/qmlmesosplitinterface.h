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
	explicit QmlMesoSplitInterface(QObject* parent, QQmlApplicationEngine* qmlEngine, QQuickWindow* mainWindow, const uint meso_idx);
	~QmlMesoSplitInterface();

	Q_INVOKABLE void getExercisesPlannerPage();
	Q_INVOKABLE void getMesoSplitPage(const uint page_index);
	Q_INVOKABLE void changeMuscularGroup(const QString& new_musculargroup, DBMesoSplitModel* splitModel, const uint initiator_id);
	Q_INVOKABLE void swapMesoPlans(const QString& splitLetter1, const QString& splitLetter2);
	Q_INVOKABLE void loadSplitFromPreviousMeso(DBMesoSplitModel* splitModel);
	Q_INVOKABLE void exportMesoSplit(const bool bShare, const QString& splitLetter, const QString& filePath = QString(), const bool bJustExport = false);
	Q_INVOKABLE void importMesoSplit(const QString& filename = QString());

	DBMesoSplitModel* getSplitModel(const QChar& splitLetter);
	inline QMap<QChar,DBMesoSplitModel*>& allSplitModels() { initializeSplitModels(); return m_splitModels; }
	inline QQuickItem* getSplitPage(const QChar& splitLetter) const { return m_splitPages.value(splitLetter); }

signals:
	void plannerPageCreated();
	void displayMessageOnAppWindow(const int message_id, const QString& filename = QString());
	void addPageToMainMenu(const QString& label, QQuickItem* page);
	void removePageFromMainMenu(QQuickItem* page);

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
	uint m_mesoIdx, m_splitMuscularGroupId;
	QString m_splitLetters;

	int splitLetterToPageIndex(const DBMesoSplitModel* const splitModel);
	void createPlannerPage();
	void createPlannerPage_part2();
	void createMesoSplitPage(const uint page_index);
	void initializeSplitModels();
	void setSplitPageProperties(QQuickItem* splitPage, const DBMesoSplitModel* const splitModel);
	void updateMuscularGroup(DBMesoSplitModel* splitModel);
};

#endif // QMLMESOSPLITINTERFACE_H
