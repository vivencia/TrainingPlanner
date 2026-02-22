#pragma once

#include <QMap>

QT_FORWARD_DECLARE_CLASS(TPDatabaseTable)

class DBModelInterface
{

public:
	explicit inline DBModelInterface(QObject *model = nullptr) : m_model(model) {}
	template<typename T> T *model() const { return qobject_cast<T*>(m_model); }
	inline void setModel(QObject* model) { m_model = model; }

	virtual const QList<QStringList> &modelData() const = 0;
	virtual QList<QStringList> &modelData() = 0;

	void clearData(const QList<uint> &excluded_fields = QList<uint> {});
	inline void copyData(DBModelInterface *other_dbmi, const uint row, const uint field)
	{
		setModified(row, field);
		modelData()[row][field] = other_dbmi->modelData().at(row).at(field);
	}

	inline const QMap<uint, QList<int>> &modifiedIndices() const { return m_modifiedIndices; }
	/**
	 * @brief setAllFieldsModified: used by AlterRecords when all fields are modified, i.e. in a swap operation
	 * @param row: required for all operations
	 * @param field: for InsertRecords can be anything because it is not checked. Must be precise for AlterRecords
	 * @param n_fields: setAllFieldsModified: used by AlterRecords when all fields are modified, i.e. in a swap operation
	 * @param &more_fields: insert two or more fields for an AlterRecord operation
	 */
	void setAllFieldsModified(uint row, const uint n_fields);
	void setModified(uint row, const int field);
	void setModified(uint row, const QList<int> &more_fields);
	//For bulk insertions
	inline void setModifiedRows(const uint start_row, const uint end_row)
	{
		for (uint i{start_row}; i < end_row; ++i)
			m_modifiedIndices.insert(i, QList<int>{-1});
	}

	inline void clearModifiedIndices() { m_modifiedIndices.clear(); }
	inline void removeModifiedIndex(const uint row) { m_modifiedIndices.remove(row); }

	inline bool isModified(const uint row, const uint field) const { return m_modifiedIndices.value(row).contains(field); }

	inline const QMap<uint, QList<uint>> &removalInfo() const { return m_removalInfo; }
	inline void setRemovalInfo(const uint row, const QList<uint> &fields) { m_removalInfo.insert(row, fields); }
	//For bulk deletions, eg. remove all rows based on id, meso_id, split letter, or whatever
	inline void setRemovalRows(const uint start_row, const uint end_row, const uint field)
	{
		for (uint i{start_row}; i < end_row; ++i)
			m_removalInfo.insert(i, QList<uint>{1, field});
	}

	inline void clearRemovalIndices() { m_removalInfo.clear(); }

protected:
	QMap<uint, QList<int>> m_modifiedIndices;
	QMap<uint, QList<uint>> m_removalInfo;
	QStringList m_emptyList;
	QObject *m_model;
};
