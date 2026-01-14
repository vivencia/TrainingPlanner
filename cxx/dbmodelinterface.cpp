#include "dbmodelinterface.h"

void DBModelInterface::clearData(const QList<uint> &excluded_fields)
{
	if (excluded_fields.isEmpty())
		modelData().clear();
	else
	{
		uint modified_row{0};
		for (auto &data : modelData())
		{
			for (uint field{0}; field < data.count(); ++field)
			{
				if (!excluded_fields.contains(field))
				{
					data[field].clear();
					setModified(modified_row, field);
				}
			}
			++modified_row;
		}
	}
}

void DBModelInterface::setAllFieldsModified(uint row, const uint n_fields)
{
	QList<int> fields{std::move(m_modifiedIndices.value(row))};
	fields.clear();
	for (int i{0}; i < n_fields; ++i)
		fields.append(i);
	m_modifiedIndices[row] = std::move(fields);
}

void DBModelInterface::setModified(uint row, const int field)
{
	QList<int> fields{m_modifiedIndices.value(row)};
	if (!fields.contains(field))
		fields.append(field);
	m_modifiedIndices[row] = std::move(fields);
}

void DBModelInterface::setModified(uint row, const QList<int> &more_fields)
{
	QList<int> fields{std::move(m_modifiedIndices.value(row))};
	for (const auto field : std::as_const(more_fields))
	{
		if (!fields.contains(field))
			fields.append(field);
	}
	m_modifiedIndices[row] = std::move(fields);
}

void DBModelInterface::setRemovalInfo(const uint row, const QList<uint> &fields)
{
	for (const auto field : std::as_const(fields))
	{
		bool found{false};
		for (const auto ri : std::as_const(m_removalInfo))
		{
			if (ri->index == row && ri->fields.contains(field))
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			stRemovalInfo *stri{new stRemovalInfo};
			stri->index = row;
			stri->fields.append(field);
			stri->values.append(modelData().at(row).at(field));
		}
	}
}
