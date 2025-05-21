#ifndef QMLMESOSPLITINTERFACE_H
#define QMLMESOSPLITINTERFACE_H

#include <QObject>
#include <QVariantMap>

QT_FORWARD_DECLARE_CLASS(DBExercisesModel);
QT_FORWARD_DECLARE_CLASS(QQmlComponent);
QT_FORWARD_DECLARE_CLASS(QQuickItem);

Q_DECLARE_OPAQUE_POINTER(DBExercisesModel*)
Q_DECLARE_OPAQUE_POINTER(QQuickItem*)

class QmlMesoSplitInterface : public QObject
{

Q_OBJECT

Q_PROPERTY(QQuickItem* currentPage READ currentPage NOTIFY currentPageChanged FINAL)
Q_PROPERTY(DBExercisesModel* currentSplitModel READ currentSplitModel NOTIFY currentPageChanged FINAL)
Q_PROPERTY(QChar currentSplitLetter READ currentSplitLetter NOTIFY currentPageChanged FINAL)
Q_PROPERTY(QChar currentSwappableLetter READ currentSwappableLetter NOTIFY currentPageChanged FINAL)
Q_PROPERTY(bool hasExercises READ hasExercises NOTIFY currentPageChanged FINAL)
Q_PROPERTY(bool canSwapExercises READ canSwapExercises NOTIFY currentPageChanged FINAL)

public:
	explicit inline QmlMesoSplitInterface(QObject* parent, const uint meso_idx)
		: QObject{parent}, m_plannerComponent{nullptr}, m_currentSplitPage{nullptr}, m_splitComponent{nullptr}, m_mesoIdx{meso_idx} {}
	~QmlMesoSplitInterface();

	inline uint mesoIdx() const { return m_mesoIdx; }
	inline void setMesoIdx(const uint new_meso_idx) { m_mesoIdx = new_meso_idx; }

	Q_INVOKABLE void getExercisesPlannerPage();
	Q_INVOKABLE void moveRow(const uint from, const uint to, DBExercisesModel *splitModel);
	Q_INVOKABLE void removeRow();
	Q_INVOKABLE void swapMesoPlans();
	Q_INVOKABLE void loadSplitFromPreviousMeso();
	Q_INVOKABLE void simpleExercisesList(DBExercisesModel* splitModel, const bool show, const bool multi_sel = false, const uint exercise_idx = 0);
	Q_INVOKABLE void exportMesoSplit(const bool bShare, const QString& splitLetter);
	Q_INVOKABLE void importMesoSplit(const QString& filename = QString());
	Q_INVOKABLE QString findSwappableModel() const;
	Q_INVOKABLE QString prevMesoName() const { return m_prevMesoName; }
	Q_INVOKABLE QQuickItem* setCurrentPage(const int index);

	DBExercisesModel* currentSplitModel() const;
	inline QQuickItem* getSplitPage(const QChar& splitLetter) const { return m_splitPages.value(splitLetter); }
	inline QQuickItem* currentPage() const { return m_currentSplitPage; }
	inline QChar currentSplitLetter() const { return m_currentSplitLetter; }
	inline QChar currentSwappableLetter() const { return m_currentSwappableLetter; }
	inline bool hasExercises() const { return m_bHasExercises; }
	inline bool canSwapExercises() const { return !m_currentSwappableLetter.isEmpty(); }

signals:
	void plannerPageCreated();
	void displayMessageOnAppWindow(const int message_id, const QString& filename = QString());
	void addPageToMainMenu(const QString& label, QQuickItem* page);
	void removePageFromMainMenu(QQuickItem* page);
	void currentPageChanged();

public slots:
	void exerciseSelected();
	void hideSimpleExercisesList();

private:
	QQmlComponent* m_plannerComponent;
	QQuickItem* m_plannerPage, *m_currentSplitPage;
	QVariantMap m_plannerProperties;

	QQmlComponent* m_splitComponent;
	QMap<QChar,QQuickItem*> m_splitPages;
	QVariantMap m_splitProperties;
	uint m_mesoIdx;

	DBExercisesModel* m_simpleExercisesListRequester;
	uint m_simpleExercisesListExerciseIdx;
	QString m_prevMesoName;
	QChar m_currentSplitLetter, m_currentSwappableLetter;
	bool m_bHasExercises;
	int m_prevMesoId;

	void createPlannerPage();
	void createPlannerPage_part2();
	void createMesoSplitPage(const QChar& splitletter);
	void createMesoSplitPage_part2(const QChar& splitletter);
	void initializeSplitModels();
	void setSplitPageProperties(QQuickItem* splitPage, const DBExercisesModel* const splitModel);
};

#endif // QMLMESOSPLITINTERFACE_H
