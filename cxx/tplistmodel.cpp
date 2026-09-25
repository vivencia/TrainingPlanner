#include "tplistmodel.h"

#include "tputils.h"

enum metaFields {
	MF_REAL_INDEX = 1000,
	MF_VIRTUAL_INDEX = 1001,
	MF_SELECTED	= 1002,
	MF_VISIBLE	= 1003,
};

enum RoleNames {
	createRole(realIndex,		MF_REAL_INDEX)
	createRole(virtualIndex,	MF_VIRTUAL_INDEX)
	createRole(selected,		MF_SELECTED)
	createRole(itemVisible,		MF_VISIBLE)
};

inline QStringList prepareSearchTerm(const QString &search_term)
{
	return appUtils()->stripDiacriticsFromString(search_term).split(' ', Qt::SkipEmptyParts);
}

TPListModel::TPListModel(QObject *parent, const uint n_cols) : QAbstractListModel{parent}, m_totalCols{n_cols}
{
	roleToString(selected)
	roleToString(itemVisible)
	connect(this, &TPListModel::rowsInserted, this, [this] (const QModelIndex &parent, int first, int last) {
		for (int i{first}; i <= last; ++i)
			insertMetaData(realRow(i));
	});
	connect(this, &TPListModel::rowsRemoved, this, [this] (const QModelIndex &parent, int first, int last) {
		for (int i{last}; i <= first; --i)
			removeMetaData(realRow(i));
	});
	connect(this, &TPListModel::modelReset, this, [this] () {
		clear();
	});
}

void TPListModel::setIsSelected(const int visible_row, const bool selected, const uint visible_column)
{
	const auto real_row{realRow(visible_row)};
	if (real_row < 0)
		return;
	setSelected(real_row, selected, visible_column);
	emit dataChanged(index(visible_row, 0), index(visible_row, m_totalCols), QList<int>{selectedRole});
}

QList<int> TPListModel::selectedInfo(const bool return_real_indices) const
{
	QList<int> selected;
	for (const auto &row_data : std::as_const(m_rowsMetadata)) {
		bool is_selected{false};
		if (m_selectEntireRow)
			is_selected = row_data.selected.at(0);
		else {
			for (const auto col_selected : row_data.selected) {
				if (col_selected) {
					is_selected = true;
					break;
				}
			}
		}
		if (is_selected)
			selected.append(return_real_indices ? row_data.real_index : row_data.virt_index);
	}
	return selected;
}

void TPListModel::applyFilter(const QString &filter, const uint field)
{
	if (field < m_totalCols) {
		if (filter != m_filter || field != m_filterField) {
			m_filter = filter;
			if (!m_filter.isEmpty()) {
				m_filterField = field;
				uint virtual_row{0};
				const QStringList &words_list{prepareSearchTerm(m_filter)};
				for (uint row{0}; row < m_rowsMetadata.count(); ++row) {
					const bool visible{itemShouldBeVisible(row, words_list)};
					m_rowsMetadata[row].virt_index = visible ? virtual_row++ : -1;
					setVisible(row, visible);
				}
			} else {
				m_filterField = -1;
				for (uint row{0}; row < m_rowsMetadata.count(); ++row) {
					m_rowsMetadata[row].virt_index = row;
					setVisible(row, true);
				}
			}
			emit filterAppliedChanged();
			if (m_sortField >= 0)
				sort(true, m_ascendingSort.value(), m_sortField);
			if (m_searchField >= 0)
				search(m_searchTerm, m_searchField);
		}
	}
}

void TPListModel::sort(const bool enable_sorting, const bool ascending, const uint field)
{
	bool do_emit{false};
	if (enable_sorting) {
		if (m_ascendingSort.value() != ascending) {
			m_sortField = field;
			m_ascendingSort = ascending;
			for (uint row{0}; row < m_rowsMetadata.count() - 1; ++row)
				doSort(row);
			do_emit = true;
		}
	} else {
		do_emit = m_sortField >= 0;
		m_sortField = -1;
		if (filterApplied())
			applyFilter(m_filter, m_filterField);
	}
	if (do_emit)
		emit sortingChanged();
}

void TPListModel::search(const QString &search_term, int field)
{
	if (search_term == m_searchTerm)
		return;

	if (search_term.length() <= 3 && !m_searchTerm.isEmpty()) {
		m_searchTerm.clear();
		m_searchField = -1;
		for (auto &row_data : m_rowsMetadata)
			row_data.past_states.clear();
		return;
	} else {
		auto diff{m_searchTerm.length() - search_term.length()};
		if (diff <= 0) {
			for (auto &row_data : m_rowsMetadata)
				fromLastState(row_data.real_index);
		}
	}
	m_searchTerm = search_term;
	m_searchField = field;
	const QStringList &words_list{prepareSearchTerm(m_searchTerm)};

	uint new_virt_index{0};
	for (auto &row_data : m_rowsMetadata) {
		if (row_data.virt_index >= 0) { //visible
			const bool visible{itemShouldRemainVisible(row_data.real_index, words_list)};
			if (!visible)
				setVisible(row_data.real_index, false);
			else
				row_data.virt_index = new_virt_index++;
			toLastState(row_data.real_index, true);
		}
	}
}

int TPListModel::find(const bool visible_rows, const QString &needle, int field) const
{
	for (const auto &row_data : std::as_const(m_rowsMetadata)) {
		if (!visible_rows || visible_rows && row_data.virt_index >= 0) {
			const QStringList &words_list{appUtils()->stripDiacriticsFromString(needle).split(' ', Qt::SkipEmptyParts)};
			bool found{false};
			if (field >= 0) {
				found = appUtils()->containsAllWords(dataValue(row_data.real_index, field), words_list);
			} else {
				for (uint row{0}; row < m_rowsMetadata.count(); ++row) {
					for (uint column{0}; column < m_totalCols; ++column) {
						const QString &field_value{dataValue(row, column)};
						if (appUtils()->containsAllWords(field_value, words_list)) {
							found = true;
							break;
						}
					}
					if (found)
						break;
				}
			}
			if (found)
				return row_data.real_index;
		}
	}
	return -1;
}

QVariant TPListModel::data(const QModelIndex &index, int role) const
{
	const int row{realRow(index.row())};
	if (row >= 0) {
		switch (role) {
		case realIndexRole: return m_rowsMetadata.at(row).real_index;
		case virtualIndexRole: return m_rowsMetadata.at(row).virt_index;
		case selectedRole: return isSelected(row, index.column());
		case itemVisibleRole: return visible(row, index.column());
		default: return data(role, row, index.column());
		}
	}
	return QVariant{};
}

bool TPListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	const int row{realRow(index.row())};
	if (row >= 0) {
		bool ok{true};
		switch (role) {
		case selectedRole: setSelected(index.row(), value.toBool(), index.column()); break;
		case itemVisibleRole: setVisible(index.row(),value.toBool(), index.column()); break;
		default:
			if ((ok = setData(role, row, value, index.column())))
				emit dataChanged(index, index, QList<int>{} << role);
			break;
		}
		return ok;
	}
	return false;
}

void TPListModel::clear()
{
	m_rowsMetadata.clear();
	m_filter.clear();
	m_searchTerm.clear();
	m_ascendingSort = std::nullopt;
	m_nSelected = m_nVisibleRows = 0;
	m_currentRow = m_filterField = m_sortField =  m_searchField = -1;
}

void TPListModel::doSort(const uint row)
{
	if (m_rowsMetadata.at(row).virt_index >= 0) {
		m_rowsMetadata[row].virt_index = 0;
		uint next_row{row + 1};
		while (next_row < m_rowsMetadata.count()) {
			const auto res{dataValue(row, m_sortField).compare(dataValue(next_row, m_sortField))};
			if (res == 0)
				continue;
			if (m_ascendingSort.value()) {
				if (res > 0)
					++(m_rowsMetadata[row].virt_index);
			} else {
				if (res < 0)
					++(m_rowsMetadata[row].virt_index);
			}
			if (m_searchField >= 0)
				toLastState(row, false);
			++next_row;
		}
	}
}

inline const bool TPListModel::visible(const int row, const uint column) const
{
	if (row >= 0) {
		if (m_selectEntireRow || column == 0)
			return m_rowsMetadata.at(row).visible.at(0);
		else if (column < m_totalCols)
			return m_rowsMetadata.at(row).visible.at(column);
	}
	return false;
}

void TPListModel::setVisible(const uint row, bool visible, const uint column)
{
	const bool is_visible{TPListModel::visible(row, column)};
	m_rowsMetadata[row].visible[column] = visible;
	emit dataChanged(index(row, m_selectEntireRow ? 0 : column), index(row, m_selectEntireRow ? m_totalCols : column),
																							QList<int>{itemVisibleRole});
	if (is_visible && !visible)
		--m_nVisibleRows;
	else if (!is_visible && visible)
		--m_nVisibleRows;
	emit countChanged();
	if (m_currentRow >= 0) {
		if (visible && row > m_currentRow) {
			setCurrentRow(row);
		} else if (!visible && row <= m_currentRow) {
			--m_currentRow;
			if (m_currentRow < 0 && count() > 0)
				setCurrentRow(0);
		}
	}
}

bool TPListModel::selected(const int row, const uint visible_column) const
{
	if (row >= 0 && row < m_rowsMetadata.count()) {
		if (m_selectEntireRow)
			return m_rowsMetadata.at(row).selected.at(0);
		else if (visible_column < m_totalCols)
			return m_rowsMetadata.at(row).selected.at(visible_column);
	}
	return false;
}

void TPListModel::setSelected(const int row, const bool selected, const uint visible_column)
{
	if (row >= 0 && row < m_rowsMetadata.count()) {
		const bool is_selected{TPListModel::selected(row, visible_column)};
		m_rowsMetadata[row].selected[visible_column] = selected;
		emit dataChanged(index(row, m_selectEntireRow ? 0 : visible_column),
				index(row, m_selectEntireRow ? m_totalCols : visible_column), QList<int>{selectedRole});
		if (is_selected && !selected)
			--m_nSelected;
		else if (!is_selected && selected)
			++m_nSelected;
		emit selectedChanged();
	}
}

void TPListModel::syncMetadata(const uint modeldata_count)
{
	m_rowsMetadata.reserve(modeldata_count);
	for (uint row{0}; row < modeldata_count; ++row) {
		st_rowData row_data;
		row_data.real_index = row;
		if (m_selectEntireRow) {
			row_data.visible.append(true);
			row_data.selected.append(false);
		} else {
			row_data.visible.reserve(m_totalCols);
			row_data.selected.reserve(m_totalCols);
			for (uint i{0}; i < m_totalCols; ++i) {
				row_data.visible.append(true);
				row_data.selected.append(false);
			}
		}
		m_rowsMetadata.append(std::move(row_data));
	}
}

void TPListModel::insertMetaData(int row)
{
	if (row < 0) {
		if (row == -2)
			row = m_rowsMetadata.count();
		else
			return; //invalid index
	}
	st_rowData row_data;
	row_data.real_index = row_data.virt_index = row;
	bool visible{m_filterField >= 0 ? itemShouldBeVisible(row, prepareSearchTerm(m_filter)) : true};
	if (m_selectEntireRow) {
		row_data.visible.append(visible);
		row_data.selected.append(false);
	} else {
		row_data.visible.reserve(m_totalCols);
		row_data.selected.reserve(m_totalCols);
		for (uint i{0}; i < m_totalCols; ++i) {
			row_data.visible.append(visible);
			row_data.selected.append(false);
		}
	}
	if (visible) {
		if (m_sortField >= 0)
			doSort(row);
		if (m_searchField >= 0) {
			if (m_selectEntireRow) {
				row_data.visible[0] = visible = itemShouldRemainVisible(row, prepareSearchTerm(m_searchTerm));
			} else {
				for (uint i{0}; i < m_totalCols; ++i) {
					if (i != m_searchField)
						row_data.visible[i] = true;
					else
						row_data.visible[i] = itemShouldRemainVisible(row, prepareSearchTerm(m_searchTerm));
				}
			}
		}
		if (visible)
			++m_nVisibleRows;
	}
	m_rowsMetadata.append(std::move(row_data));
}

void TPListModel::removeMetaData(const int row)
{
	const int start_virt_row{m_rowsMetadata.at(row).virt_index};
	int start_row{row};
	m_rowsMetadata.removeAt(row);
	if (m_sortField >= 0)
		start_row = 0;
	for (auto &row_data : m_rowsMetadata | std::views::drop(start_row)) {
		row_data.real_index = start_row++;
		if (row_data.virt_index >= start_virt_row)
			--row_data.virt_index;
	}
}

inline int TPListModel::realRow(const int visible_row) const
{
	if (visible_row >= 0) {
		if (visible_row < m_nVisibleRows) {
			for (const auto &row_data : m_rowsMetadata) {
				if (row_data.virt_index == visible_row)
					return row_data.real_index;
			}
		} else if (visible_row == m_nVisibleRows) {
			return -2;
		}
	}
	return -1;
}

inline bool TPListModel::itemShouldBeVisible(const uint real_row, const QStringList &filters) const
{
	return appUtils()->containsAllWords(dataValue(real_row, m_filterField), filters);
}

inline bool TPListModel::itemShouldRemainVisible(const uint real_row, const QStringList &search_terms) const
{
	bool visible{false};
	if (m_searchField >= 0) {
		visible = appUtils()->containsAllWords(dataValue(real_row, m_searchField), search_terms);
	} else {
		for (uint column{0}; column < m_totalCols; ++column) {
			const QString &field_value{dataValue(real_row, column)};
			if (appUtils()->containsAllWords(field_value, search_terms)) {
				visible = true;
				break;
			}
		}
	}
	return visible;
}

inline void TPListModel::fromLastState(const uint real_row)
{
	if (!m_rowsMetadata.at(real_row).past_states.isEmpty()) {
		auto last_state{m_rowsMetadata.at(real_row).past_states.constLast()};
		m_rowsMetadata[real_row].virt_index = last_state.first;
		setVisible(real_row, last_state.second, m_selectEntireRow ? 0 : m_searchField);
		m_rowsMetadata[real_row].past_states.pop_back();
	}
}

inline void TPListModel::toLastState(const uint real_row, const bool add_new_state)
{
	QPair<int,bool> last_state{m_rowsMetadata.at(real_row).virt_index, m_selectEntireRow
					? m_rowsMetadata.at(real_row).visible[0] : m_rowsMetadata.at(real_row).visible[m_searchField]};
	if (add_new_state)
		m_rowsMetadata[real_row].past_states.append(std::move(last_state));
	else
		m_rowsMetadata[real_row].past_states.last() = last_state;
}
