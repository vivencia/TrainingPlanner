#ifndef DBMESOSPLITMODEL_H
#define DBMESOSPLITMODEL_H

#include "tplistmodel.h"

static const QLatin1Char fieldSep('|');
static const QLatin1Char subSetSep('#');

typedef struct typeArray6 {
	typeArray6() : x{-1,-1,-1,-1,-1,-1} {}
	int x[6];
} typeArray6;

class DBMesoSplitModel : public TPListModel
{

Q_OBJECT
QML_ELEMENT

public:
	explicit DBMesoSplitModel(QObject *parent = nullptr) : TPListModel{parent},
			m_currentIndex(0), m_fldExercises{ 8,13,18,23,28,33}, m_muscularGroup{2,3,4,5,6,7} {}

	Q_INVOKABLE int currentEntry(const char splitLetter) const { return m_currentEntry.at(m_currentIndex)[letterToIndex(splitLetter)]; }
	Q_INVOKABLE void setCurrentEntry(const char splitLetter, const int current_idx) { m_currentEntry.at(m_currentIndex)[letterToIndex(splitLetter)] = current_idx; }
	Q_INVOKABLE uint nEntries(const char splitLetter) const { return m_nEntries.at(m_currentIndex)[letterToIndex(splitLetter)]; }
	Q_INVOKABLE void setNEntries(const char splitLetter, const uint n) { m_nEntries.at(m_currentIndex)[letterToIndex(splitLetter)] = n; }

	Q_INVOKABLE int getExerciseCount(const char splitLetter) const
	{
		return static_cast<QString>(m_modeldata.at(m_currentIndex).at(m_fldExercises[letterToIndex(splitLetter)])).split(fieldSep).count();
	}

	Q_INVOKABLE QString getMuscularGroup(const char splitLetter, const uint n) const
	{
		return n >= 0 && n < m_nEntries[m_currentIndex][letterToIndex(splitLetter)] ? static_cast<QString>(m_modeldata.at(m_currentIndex).at(m_muscularGroup[letterToIndex(splitLetter)])).split(fieldSep).at(n) : QString();
	}
	Q_INVOKABLE void changeMuscularGroup(const char splitLetter, const QString& newMuscularGroup, const uint n, const bool bReplace = true)
	{
		replaceString(m_modeldata[m_currentIndex][m_muscularGroup[letterToIndex(splitLetter)]], newMuscularGroup, n, bReplace);
	}

	Q_INVOKABLE QString getExercise(const char splitLetter, const uint n) const
	{
		if (n >= 0 && n < m_nEntries.at(m_currentIndex)[letterToIndex(splitLetter)] )
		{
			QString exercise(static_cast<QString>(m_modeldata.at(m_currentIndex).at(m_fldExercises[letterToIndex(splitLetter)])).split(fieldSep).at(n));
			return exercise.replace(QLatin1Char('&'), QStringLiteral(" + "));
		}
		return QString();
	}
	Q_INVOKABLE void changeExercise(const char splitLetter, const QString& newExercise, const uint n, const bool bReplace = true)
	{
		replaceString(m_modeldata[m_currentIndex][m_fldExercises[letterToIndex(splitLetter)]], newExercise, n, bReplace);
	}
	Q_INVOKABLE void addExercise(const char splitLetter, const QString& newExercise, const uint n)
	{
		insertString(m_modeldata[m_currentIndex][m_fldExercises[letterToIndex(splitLetter)]], newExercise, n);
		m_nEntries[letterToIndex(splitLetter)]++;
	}
	Q_INVOKABLE void removeExercise(const char splitLetter, const uint n)
	{
		removeString(m_modeldata[m_currentIndex][m_fldExercises[letterToIndex(splitLetter)]], n);
		m_nEntries[letterToIndex(splitLetter)]--;
	}

	Q_INVOKABLE inline uint getSetType(const char splitLetter, const uint n) const
	{
		return n >= 0 && n < m_nEntries.at(m_currentIndex)[letterToIndex(splitLetter)] ? static_cast<QString>(m_modeldata.at(m_currentIndex).at(m_fldExercises[letterToIndex(splitLetter)]+1)).split(fieldSep).at(n).toUInt() : 0;
	}
	Q_INVOKABLE void changeSetType(const char splitLetter, const uint newSetType, const uint n, const bool bReplace = true)
	{
		replaceString(m_modeldata[m_currentIndex][m_fldExercises[letterToIndex(splitLetter)]+1], QString::number(newSetType), n, bReplace);
	}
	Q_INVOKABLE void addSetType(const char splitLetter, const uint newSetType, const uint n)
	{
		insertString(m_modeldata[m_currentIndex][m_fldExercises[letterToIndex(splitLetter)]+1], QString::number(newSetType), n);
	}
	Q_INVOKABLE void removeSetType(const char splitLetter, const uint n)
	{
		removeString(m_modeldata[m_currentIndex][m_fldExercises[letterToIndex(splitLetter)]+1], n);
	}

	Q_INVOKABLE inline QString getSetsNumber(const char splitLetter, const uint n)
	{
		return n >= 0 && n < m_nEntries.at(m_currentIndex)[letterToIndex(splitLetter)] ? static_cast<QString>(m_modeldata.at(m_currentIndex).at(m_fldExercises[letterToIndex(splitLetter)]+2)).split(fieldSep).at(n) : QString();
	}
	Q_INVOKABLE void changeSetsNumber(const char splitLetter, const QString& newSetsNumber, const uint n, const bool bReplace = true)
	{
		replaceString(m_modeldata[m_currentIndex][m_fldExercises[letterToIndex(splitLetter)]+2], newSetsNumber, n, bReplace);
	}
	Q_INVOKABLE void addSetsNumber(const char splitLetter, const QString& newSetsNumber, const uint n)
	{
		insertString(m_modeldata[m_currentIndex][m_fldExercises[letterToIndex(splitLetter)]+2], newSetsNumber, n);
	}
	Q_INVOKABLE void removeSetsNumber(const char splitLetter, const uint n)
	{
		removeString(m_modeldata[m_currentIndex][m_fldExercises[letterToIndex(splitLetter)]+2], n);
	}

	Q_INVOKABLE inline QString getReps(const char splitLetter, const uint subSet, const uint n) const
	{
		return n >= 0 && n < m_nEntries.at(m_currentIndex)[letterToIndex(splitLetter)] ? static_cast<QString>(m_modeldata.at(m_currentIndex).at(m_fldExercises[letterToIndex(splitLetter)]+3)).split(fieldSep).at(n).split(subSetSep).at(subSet) : QString();
	}
	Q_INVOKABLE void changeReps(const char splitLetter, const uint subSet, const QString& newReps, const uint n, const bool bReplace = true)
	{
		QString newRepsNumber(m_modeldata.at(m_currentIndex).at(m_fldExercises[letterToIndex(splitLetter)]+3).split(fieldSep).at(n));
		replaceString(newRepsNumber, newReps, subSet, bReplace, subSetSep);
		replaceString(m_modeldata[m_currentIndex][m_fldExercises[letterToIndex(splitLetter)]+3], newRepsNumber, n, bReplace);
	}
	Q_INVOKABLE void addReps(const char splitLetter, const QString& newReps, const uint n)
	{
		insertString(m_modeldata[m_currentIndex][m_fldExercises[letterToIndex(splitLetter)]+3], newReps, n);
	}
	Q_INVOKABLE void removeReps(const char splitLetter, const uint n)
	{
		removeString(m_modeldata[m_currentIndex][m_fldExercises[letterToIndex(splitLetter)]+3], n);
	}

	Q_INVOKABLE inline QString getWeight(const char splitLetter, const uint subSet, const uint n) const
	{
		return n >= 0 && n < m_nEntries.at(m_currentIndex)[letterToIndex(splitLetter)] ? static_cast<QString>(m_modeldata.at(m_currentIndex).at(m_fldExercises[letterToIndex(splitLetter)]+4)).split(fieldSep).at(n).split(subSetSep).at(subSet) : QString();
	}
	Q_INVOKABLE void changeWeigth(const char splitLetter, const uint subSet, const QString& newWeight, const uint n, const bool bReplace = true)
	{
		QString newWeightNumber(static_cast<QString>(m_modeldata.at(m_currentIndex).at(m_fldExercises[letterToIndex(splitLetter)]+4)).split(fieldSep).at(n));
		replaceString(newWeightNumber, newWeight, subSet, bReplace, subSetSep);
		replaceString(m_modeldata[m_currentIndex][m_fldExercises[letterToIndex(splitLetter)]+4], newWeightNumber, n, bReplace);
	}
	Q_INVOKABLE void addWeight(const char splitLetter, const QString& newWeight, const uint n)
	{
		insertString(m_modeldata[m_currentIndex][m_fldExercises[letterToIndex(splitLetter)]+4], newWeight, n);
	}
	Q_INVOKABLE void removeWeight(const char splitLetter, const uint n)
	{
		removeString(m_modeldata[m_currentIndex][m_fldExercises[letterToIndex(splitLetter)]+4], n);
	}

	Q_INVOKABLE int columnCount(const QModelIndex &parent) const override { Q_UNUSED(parent); return 37; }
	Q_INVOKABLE QVariant data(const QModelIndex &, int) const override { return QVariant(); }
	Q_INVOKABLE bool setData(const QModelIndex &, const QVariant &, int) override { return false; }

	void appendMesoInfo() { typeArray6 arr; m_currentEntry.append(arr); m_nEntries.append(arr); }

private:
	uint m_currentIndex;
	uint m_fldExercises[6];
	uint m_muscularGroup[6];

	QList<typeArray6> m_currentEntry;
	QList<typeArray6> m_nEntries;

	inline int letterToIndex(const char letter) const { return static_cast<int>(letter) - static_cast<int>('A'); }
	void replaceString(QString& oldString, const QString& newString, const uint n, const bool bReplace = true, QLatin1Char sep = fieldSep);
	void insertString(QString& string, const QString& newString, const uint n, QLatin1Char sep = fieldSep);
	void removeString(QString& string, const uint n, QLatin1Char sep = fieldSep);
};

#endif // DBMESOSPLITMODEL_H
