#include "dbmodelinterface.h"

void DBModelInterface::setAllFieldsModified(uint row, const uint n_fields)
{
	QList<uint> fields{std::move(m_modifiedIndices.value(row))};
	fields.clear();
	for (uint i{0}; i < n_fields; ++i)
		fields.append(i);
	m_modifiedIndices.insertOrAssign(std::move(row), std::move(fields));
}

void DBModelInterface::setModified(uint row, const uint field)
{
	QList<uint> fields{std::move(m_modifiedIndices.value(row))};
	if (!fields.contains(field))
		fields.append(field);
	m_modifiedIndices.insertOrAssign(std::move(row), std::move(fields));
}

void DBModelInterface::setModified(uint row, const QList<uint> &more_fields)
{
	QList<uint> fields{std::move(m_modifiedIndices.value(row))};
	for (const auto field : std::as_const(more_fields))
	{
		if (!fields.contains(field))
			fields.append(field);
	}
	m_modifiedIndices.insertOrAssign(std::move(row), std::move(fields));
}

void DBModelInterface::setRemovalIndex(const int removal_index, const uint field)
{
	for (const auto ri : std::as_const(m_removalInfo))
	{
		if (ri->index == removal_index && ri->fields.contains(field))
			return;
	}
	stRemovalInfo *stri{new stRemovalInfo};
	stri->index = removal_index;
	stri->fields.append(field);
	stri->values.append(modelData().at(removal_index).at(field));
}
