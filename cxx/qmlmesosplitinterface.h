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
	explicit inline QmlMesoSplitInterface(QObject *parent, const uint meso_idx)
		: QObject{parent}, m_plannerPage{nullptr}, m_currentSplitPage{nullptr}, m_splitComponent{nullptr}, m_mesoIdx{meso_idx} {}
	inline ~QmlMesoSplitInterface() { cleanUp(); }
	void cleanUp();

	inline uint mesoIdx() const { return m_mesoIdx; }
	inline void setMesoIdx(const uint new_meso_idx) { m_mesoIdx = new_meso_idx; }

	Q_INVOKABLE void getExercisesPlannerPage();
	Q_INVOKABLE void addExercise();
	Q_INVOKABLE void removeExercise(const int exercise_number = -1);
	Q_INVOKABLE void swapMesoPlans();
	Q_INVOKABLE void loadSplitFromPreviousMeso();
	Q_INVOKABLE void simpleExercisesList(const bool show);
	Q_INVOKABLE void exportMesoSplit(const bool bShare);
	Q_INVOKABLE void exportAllMesoSplits(const bool bShare);
	Q_INVOKABLE void importMesoSplit(const QString &filename = QString{});
	Q_INVOKABLE QString prevMesoName() const { return m_prevMesoName; }
	Q_INVOKABLE QQuickItem *setCurrentPage(const int index);

	inline DBExercisesModel *currentSplitModel() const { return m_splitModels.value(m_currentSplitLetter); }
	inline QQuickItem *currentPage() const { return m_currentSplitPage; }
	inline QChar currentSplitLetter() const { return m_currentSplitLetter; }
	inline QChar currentSwappableLetter() const { return m_currentSwappableLetter; }
	bool hasExercises() const;
	inline bool canSwapExercises() const { return m_currentSwappableLetter != 'N'; }

public slots:
	void changeExerciseName();

signals:
	void plannerPageCreated();
	void currentPageChanged();

private:
	QQmlComponent* m_plannerComponent;
	QQuickItem* m_plannerPage, *m_currentSplitPage;
	QVariantMap m_plannerProperties;

	QQmlComponent* m_splitComponent;
	QMap<QChar,QQuickItem*> m_splitPages;
	QMap<QChar,DBExercisesModel*> m_splitModels;
	QMap<QChar,bool> m_hasPreviousPlan;
	QVariantMap m_splitProperties;
	uint m_mesoIdx;

	uint m_simpleExercisesListExerciseIdx;
	QString m_prevMesoName, m_prevMesoId;
	QChar m_currentSplitLetter, m_currentSwappableLetter;

	void createPlannerPage();
	void createPlannerPage_part2();
	void createMesoSplitPages();
	void createMesoSplitPages_part2();
	void setSplitPageProperties(const QChar &split_letter);
	void syncSplitPagesWithMesoSplit();
	void addPage(const QChar &split_letter, const uint index);
	void removePage(const QChar &split_letter);
	QChar findSwappableModel() const;
};

#endif // QMLMESOSPLITINTERFACE_H
