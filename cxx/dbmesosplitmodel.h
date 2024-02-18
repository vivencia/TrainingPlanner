#ifndef DBMESOSPLITMODEL_H
#define DBMESOSPLITMODEL_H

#include "tplistmodel.h"

static const QLatin1Char fieldSep('|');
static const QLatin1Char subSetSep('#');

class DBMesoSplitModel : public TPListModel
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(int currentEntry READ currentEntry WRITE setCurrentEntry FINAL)
Q_PROPERTY(int nEntries READ nEntries WRITE setNEntries NOTIFY nEntriesChanged)

public:
	// Define the role names to be used
	/*enum RoleNames {
		mesoSplitIdRole = Qt::UserRole,
		mesoSplitMesoIdRole = Qt::UserRole+1,
		splitARole = Qt::UserRole+2,
		splitBRole = Qt::UserRole+3,
		splitCRole = Qt::UserRole+4,
		splitDRole = Qt::UserRole+5,
		splitERole = Qt::UserRole+6,
		splitFRole = Qt::UserRole+7
	};*/

	explicit DBMesoSplitModel(QObject *parent = nullptr) : TPListModel{parent},
			m_currentIndex(0), m_fldExercises(0), m_muscularGroup(0), m_currentEntry(-1), m_nEntries(0) {}

	int currentEntry() const { return m_currentEntry; }
	void setCurrentEntry(const int current_idx) { m_currentEntry = current_idx; }
	uint nEntries() const { return m_nEntries; }
	void setNEntries(const uint n) { m_nEntries = n; }

	Q_INVOKABLE void setWorkingSplit(const uint mesoIdx, QLatin1Char splitLetter);

	Q_INVOKABLE int getExerciseCount() const
	{
		return static_cast<QString>(m_modeldata.at(m_currentIndex).at(m_fldExercises)).split(fieldSep).count();
	}

	Q_INVOKABLE QString getMuscularGroup(const uint n) const
	{
		return static_cast<QString>(m_modeldata.at(m_currentIndex).at(m_muscularGroup)).split(fieldSep).at(n);
	}
	Q_INVOKABLE void changeMuscularGroup(const QString& newMuscularGroup, const uint n, const bool bReplace = true)
	{
		replaceString(m_modeldata[m_currentIndex][m_muscularGroup], newMuscularGroup, n, bReplace);
	}

	Q_INVOKABLE QString getExercise(const uint n) const
	{
		QString exercise(static_cast<QString>(m_modeldata.at(m_currentIndex).at(m_fldExercises)).split(fieldSep).at(n));
		return exercise.replace(QLatin1Char('&'), QStringLiteral(" + "));
	}
	Q_INVOKABLE void changeExercise(const QString& newExercise, const uint n, const bool bReplace = true)
	{
		replaceString(m_modeldata[m_currentIndex][m_fldExercises], newExercise, n, bReplace);
	}
	Q_INVOKABLE void addExercise(const QString& newExercise, const uint n)
	{
		insertString(m_modeldata[m_currentIndex][m_fldExercises], newExercise, n);
		m_nEntries++;
		emit nEntriesChanged();
	}
	Q_INVOKABLE void removeExercise(const uint n)
	{
		removeString(m_modeldata[m_currentIndex][m_fldExercises], n);
		m_nEntries--;
		emit nEntriesChanged();
	}

	Q_INVOKABLE inline uint getSetType(const uint n) const
	{
		return static_cast<QString>(m_modeldata.at(m_currentIndex).at(m_fldExercises+1)).split(fieldSep).at(n).toUInt();
	}
	Q_INVOKABLE void changeSetType(const uint newSetType, const uint n, const bool bReplace = true)
	{
		replaceString(m_modeldata[m_currentIndex][m_fldExercises+1], QString::number(newSetType), n, bReplace);
	}
	Q_INVOKABLE void addSetType(const uint newSetType, const uint n)
	{
		insertString(m_modeldata[m_currentIndex][m_fldExercises+1], QString::number(newSetType), n);
	}
	Q_INVOKABLE void removeSetType(const uint n)
	{
		removeString(m_modeldata[m_currentIndex][m_fldExercises+1], n);
	}

	Q_INVOKABLE inline QString getSetsNumber(const uint n)
	{
		return static_cast<QString>(m_modeldata.at(m_currentIndex).at(m_fldExercises+2)).split(fieldSep).at(n);
	}
	Q_INVOKABLE void changeSetsNumber(const QString& newSetsNumber, const uint n, const bool bReplace = true)
	{
		replaceString(m_modeldata[m_currentIndex][m_fldExercises+2], newSetsNumber, n, bReplace);
	}
	Q_INVOKABLE void addSetsNumber(const QString& newSetsNumber, const uint n)
	{
		insertString(m_modeldata[m_currentIndex][m_fldExercises+2], newSetsNumber, n);
	}
	Q_INVOKABLE void removeSetsNumber(const uint n)
	{
		removeString(m_modeldata[m_currentIndex][m_fldExercises+2], n);
	}

	Q_INVOKABLE inline QString getReps(const uint subSet, const uint n) const
	{
		return static_cast<QString>(m_modeldata.at(m_currentIndex).at(m_fldExercises+3)).split(fieldSep).at(n).split(subSetSep).at(subSet);
	}
	Q_INVOKABLE void changeReps(const uint subSet, const QString& newReps, const uint n, const bool bReplace = true)
	{
		QString newRepsNumber(m_modeldata.at(m_currentIndex).at(m_fldExercises+3).split(fieldSep).at(n));
		replaceString(newRepsNumber, newReps, subSet, bReplace, subSetSep);
		replaceString(m_modeldata[m_currentIndex][m_fldExercises+3], newRepsNumber, n, bReplace);
	}
	Q_INVOKABLE void addReps(const QString& newReps, const uint n)
	{
		insertString(m_modeldata[m_currentIndex][m_fldExercises+3], newReps, n);
	}
	Q_INVOKABLE void removeReps(const uint n)
	{
		removeString(m_modeldata[m_currentIndex][m_fldExercises+3], n);
	}

	Q_INVOKABLE inline QString getWeight(const uint subSet, const uint n) const
	{
		return static_cast<QString>(m_modeldata.at(m_currentIndex).at(m_fldExercises+4)).split(fieldSep).at(n).split(subSetSep).at(subSet);
	}
	Q_INVOKABLE void changeWeigth(const uint subSet, const QString& newWeight, const uint n, const bool bReplace = true)
	{
		QString newWeightNumber(static_cast<QString>(m_modeldata.at(m_currentIndex).at(m_fldExercises+4)).split(fieldSep).at(n));
		replaceString(newWeightNumber, newWeight, subSet, bReplace, subSetSep);
		replaceString(m_modeldata[m_currentIndex][m_fldExercises+4], newWeightNumber, n, bReplace);
	}
	Q_INVOKABLE void addWeight(const QString& newWeight, const uint n)
	{
		insertString(m_modeldata[m_currentIndex][m_fldExercises+4], newWeight, n);
	}
	Q_INVOKABLE void removeWeight(const uint n)
	{
		removeString(m_modeldata[m_currentIndex][m_fldExercises+4], n);
	}

	Q_INVOKABLE int columnCount(const QModelIndex &parent) const override { Q_UNUSED(parent); return 37; }
	Q_INVOKABLE QVariant data(const QModelIndex &, int) const override { return QVariant(); }
	Q_INVOKABLE bool setData(const QModelIndex &, const QVariant &, int) override { return false; }

signals:
	void nEntriesChanged();

private:
	uint m_currentIndex;
	uint m_fldExercises;
	uint m_muscularGroup;
	int m_currentEntry;
	uint m_nEntries;

	void replaceString(QString& oldString, const QString& newString, const uint n, const bool bReplace = true, QLatin1Char sep = fieldSep);
	void insertString(QString& string, const QString& newString, const uint n, QLatin1Char sep = fieldSep);
	void removeString(QString& string, const uint n, QLatin1Char sep = fieldSep);
};

#endif // DBMESOSPLITMODEL_H
