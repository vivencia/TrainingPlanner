#ifndef DBMESOSPLITMODEL_H
#define DBMESOSPLITMODEL_H

#include "tplistmodel.h"

static const QLatin1Char fieldSep('|');
static const QLatin1Char subSetSep('#');

class DBMesoSplitModel : public TPListModel
{

Q_OBJECT
QML_ELEMENT

public:
	explicit DBMesoSplitModel(QObject *parent = nullptr) : TPListModel{parent},
			m_currentIndex(0), m_fldExercises{ 8,13,18,23,28,33}, m_muscularGroup{2,3,4,5,6,7}, m_currentEntry{-1}, m_nEntries{0} {}

	int currentEntry(const char splitLetter) const { return m_currentEntry[letterToIndex(splitLetter)]; }
	void setCurrentEntry(const char splitLetter, const int current_idx) { m_currentEntry[letterToIndex(splitLetter)] = current_idx; }
	uint nEntries(const char splitLetter) const { return m_nEntries[letterToIndex(splitLetter)]; }
	void setNEntries(const char splitLetter, const uint n) { m_nEntries[letterToIndex(splitLetter)] = n; }

	Q_INVOKABLE int getExerciseCount(const char splitLetter) const
	{
		return static_cast<QString>(m_modeldata.at(m_currentIndex).at(m_fldExercises[letterToIndex(splitLetter)])).split(fieldSep).count();
	}

	Q_INVOKABLE QString getMuscularGroup(const char splitLetter,const uint n) const
	{
		return n >= 0 && n < m_nEntries ? static_cast<QString>(m_modeldata.at(m_currentIndex).at(m_muscularGroup)).split(fieldSep).at(n) : QString();
	}
	Q_INVOKABLE void changeMuscularGroup(const QString& newMuscularGroup, const uint n, const bool bReplace = true)
	{
		replaceString(m_modeldata[m_currentIndex][m_muscularGroup], newMuscularGroup, n, bReplace);
	}

	Q_INVOKABLE QString getExercise(const uint n) const
	{
		if (n >= 0 && n < m_nEntries )
		{
			QString exercise(static_cast<QString>(m_modeldata.at(m_currentIndex).at(m_fldExercises)).split(fieldSep).at(n));
			return exercise.replace(QLatin1Char('&'), QStringLiteral(" + "));
		}
		return QString();
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
		return n >= 0 && n < m_nEntries ? static_cast<QString>(m_modeldata.at(m_currentIndex).at(m_fldExercises+1)).split(fieldSep).at(n).toUInt() : 0;
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
		return n >= 0 && n < m_nEntries ? static_cast<QString>(m_modeldata.at(m_currentIndex).at(m_fldExercises+2)).split(fieldSep).at(n) : QString();
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
		return n >= 0 && n < m_nEntries ? static_cast<QString>(m_modeldata.at(m_currentIndex).at(m_fldExercises+3)).split(fieldSep).at(n).split(subSetSep).at(subSet) : QString();
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
		return n >= 0 && n < m_nEntries ? static_cast<QString>(m_modeldata.at(m_currentIndex).at(m_fldExercises+4)).split(fieldSep).at(n).split(subSetSep).at(subSet) : QString();
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
	void currentEntryChanged();
	void nEntriesChanged();

private:
	uint m_currentIndex;
	uint m_fldExercises[6];
	uint m_muscularGroup[6];
	int m_currentEntry[6];
	uint m_nEntries[6];

	inline int letterToIndex(const char letter) const { return static_cast<int>(letter) - static_cast<int>('A'); }
	void replaceString(QString& oldString, const QString& newString, const uint n, const bool bReplace = true, QLatin1Char sep = fieldSep);
	void insertString(QString& string, const QString& newString, const uint n, QLatin1Char sep = fieldSep);
	void removeString(QString& string, const uint n, QLatin1Char sep = fieldSep);
};

#endif // DBMESOSPLITMODEL_H
