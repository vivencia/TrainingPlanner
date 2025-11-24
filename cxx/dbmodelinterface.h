#pragma once

#include <QtGlobal>
#include <QHash>

QT_FORWARD_DECLARE_CLASS(TPDatabaseTable)

class DBModelInterface
{

public:

	struct stRemovalInfo
	{
		uint index;
		QList<uint> fields;
		QStringList values;
	};

	explicit inline DBModelInterface(QObject *model = nullptr) : m_model(model) {}
	template<typename T> T *model() const { return qobject_cast<T*>(m_model); }
	inline void setModel(QObject* model) { m_model = model; }

	inline void setTPDatabaseTable(TPDatabaseTable *db) { m_db = db; }
	virtual const QList<QStringList> &modelData() const = 0;
	virtual QList<QStringList> &modelData() = 0;

	void clearData(const QList<uint> &excluded_fields = QList<uint> {});
	inline void copyData(DBModelInterface *other_dbmi, const uint row, const uint field)
	{
		setModified(row, field);
		modelData()[row][field] = other_dbmi->modelData().at(row).at(field);
	}

	inline QHash<uint, QList<uint>> &modifiedIndices() { return m_modifiedIndices; }
	inline const QHash<uint, QList<uint>> &modifiedIndices() const { return m_modifiedIndices; }
	inline const QList<stRemovalInfo*> &removalInfo() const { return m_removalInfo; }

	void setAllFieldsModified(uint row, const uint n_fields);
	void setModified(uint row, const uint field);
	void setModified(uint row, const QList<uint> &more_fields);

	inline void clearModifiedIndices()
	{
		m_modifiedIndices.clear();
	}

	inline bool isModified(const uint row, const uint field) const
	{
		return m_modifiedIndices.value(row).contains(field);
	}

	void setRemovalInfo(const uint row, const QList<uint> &fields);
	inline void clearRemovalIndices()
	{
		qDeleteAll(m_removalInfo);
	}

protected:
	QHash<uint, QList<uint>> m_modifiedIndices;
	QList<stRemovalInfo*> m_removalInfo;
	TPDatabaseTable *m_db;
	QStringList m_emptyList;
	QObject *m_model;
};
