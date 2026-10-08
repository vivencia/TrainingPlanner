#include "tplistmodel.h"

#include "qmlitemmanager.h"
#include "tpsettings.h"
#include "tputils.h"
#include "translationclass.h"

#include <QQmlComponent>
#include <QQmlApplicationEngine>

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

static inline QStringList prepareSearchTerm(const QString &search_term)
{
	return appUtils()->stripDiacriticsFromString(search_term).split(' ', Qt::SkipEmptyParts);
}

static int compareStrings(const QString &str1, const QString &str2)
{
	const auto len{qMin(str1.size(), str2.size())};
	auto doSum = [] (uint start_pos, uint sum, const uint len, const QString &str) -> uint {
		for (; start_pos < len; ++start_pos) {
			if (str.at(start_pos).isLetterOrNumber()) {
				if (!str.at(start_pos).isDigit()) {
					sum += static_cast<uint>(str.at(start_pos).toLatin1());
				} else {
					QString number{str.at(start_pos)};
					while (++start_pos < str.size()) {
						if (str.at(start_pos).isDigit())
							number += str.at(start_pos);
						else
							break;
					}
					sum += number.toUInt();
				}
			}
		}
		return sum;
	};

	uint sum1{doSum(0, 0, len, str1)}, sum2{doSum(0, 0, len, str2)};
	if (str1.size() > str2.size())
		sum1 = doSum(str2.size(), sum1, str1.size(), str1);
	else if (str2.size() > str1.size())
		sum2 = doSum(str1.size(), sum2, str2.size(), str2);

	return sum1 == sum2 ? 0 : sum1 > sum2 ? 1 : -1;
}

TPListModel::TPListModel(QObject *parent, const uint n_cols) : QAbstractListModel{parent}, m_totalCols{n_cols}
{
	roleToString(selected)
	roleToString(itemVisible)

	auto addNoSortField = [this] () -> void {
		m_fieldsNames.append(std::move(tr("Unsorted")));
		m_fieldsNames.append(std::move("--"));
		setFieldsNames();
	};

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

	addNoSortField();
	connect(appTr(), &TranslationClass::applicationLanguageChanged, this, [this,addNoSortField] () {
		m_fieldsNames.clear();
		addNoSortField();
		emit fieldsNamesChanged();
	});
}

void TPListModel::clear()
{
	m_rowsMetadata.clear();
	m_filters->clear();
}

int TPListModel::realRow(const int visible_row) const
{
	if (visible_row >= 0 && visible_row < m_nVisibleRows) {
		for (const auto &row_data : m_rowsMetadata) {
			if (row_data.virt_index == visible_row)
				return row_data.real_index;
		}
	} else if (visible_row == m_nVisibleRows) {
		return -2;
	}
	return -1;
}

void TPListModel::showFiltersDialog()
{
	if (!m_filtersDialogComponent) {
		m_filtersDialogComponent = new QQmlComponent{appQmlEngine(), "TpQml.Dialogs"_L1, "ModelFiltersDialog"_L1, QQmlComponent::Asynchronous};
		connect(m_filtersDialogComponent, &QQmlComponent::statusChanged, this, [this] (QQmlComponent::Status status) { showFiltersDialog(); });
	} else {
		if (!m_filtersDialog) {
			switch (m_filtersDialogComponent->status()) {
			case QQmlComponent::Ready:
				m_filtersDialogComponent->disconnect();
				m_filtersDialog = m_filtersDialogComponent->createWithInitialProperties({
									{"filtersModel"_L1, QVariant::fromValue(m_filters)},
									{"title"_L1, tr("Filter Logs")}}, appQmlEngine()->rootContext());
#ifndef QT_NO_DEBUG
				if (!m_filtersDialog) {
					appItemManager()->log(TPLogs::LT_DEBUG, QmlItemManager::logOriginQmlEngine,
						std::move("Component creation failed"_L1), std::move(m_filtersDialogComponent->errorString()));
					return;
				}
#endif
				appQmlEngine()->setObjectOwnership(m_filtersDialog, QQmlEngine::CppOwnership);
				showFiltersDialog();
				break;
			case QQmlComponent::Loading:
				return;
			case QQmlComponent::Null:
			case QQmlComponent::Error:
#ifndef QT_NO_DEBUG
				qDebug() << m_filtersDialogComponent->errorString();
#endif
				return;
			}
		} else {
			appItemManager()->appPagesManager()->openPopup(m_filtersDialog, appItemManager()->appHomePage());
		}
	}
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

void TPListModel::applyFilters(const bool do_layout_signals)
{
	if (m_rowsMetadata.isEmpty())
		return;
	if (do_layout_signals)
		emit layoutAboutToBeChanged();
	if (m_enableFilters && !m_filters->filters().isEmpty()) {
		int new_virt_index{0};
		for (uint row{0}; row < m_rowsMetadata.count(); ++row) {
			const bool visible{itemShouldBeVisible(row)};
			m_rowsMetadata[row].virt_index = visible ? new_virt_index++ : -1;
			setVisible(row, visible, 0, false);
		}
	} else {
		for (uint row{0}; row < m_rowsMetadata.count(); ++row) {
			m_rowsMetadata[row].virt_index = row;
			setVisible(row, true, 0, false);
		}
	}

	if (m_sortDirection != NO_SORT)
		sort(false);
	if (m_searchField >= 0)
		search(m_searchTerm, m_searchField);
	setCurrentRow(count() > 0 ? 0 : -1);
	if (do_layout_signals)
		emit layoutChanged();
}

void TPListModel::sort(const bool do_layout_signals)
{
	if (do_layout_signals)
		emit layoutAboutToBeChanged();
	for (uint row{0}; row < m_rowsMetadata.count(); ++row) {
		if (!visible(row))
			continue;
		auto new_virt_index{m_sortDirection == SORT_DOWN ? 0 : m_nVisibleRows - 1};
		auto other_row{0};
		const QString &data_value{dataValue(row, m_sortField)};
		do {
			if (row != other_row && visible(other_row)) {
				const auto res{compareStrings(data_value, dataValue(other_row, m_sortField))};
				if (m_sortDirection == SORT_DOWN) {
					if (res >= 0)
						++new_virt_index;
				} else {
					if (res > 0)
						--new_virt_index;
				}
				if (m_searchField >= 0)
					toLastState(row, false);
			}
		} while (++other_row  < m_rowsMetadata.count());
		m_rowsMetadata[row].virt_index = new_virt_index;
	}
	/*int virt_row = 0;
	do {
		for (uint row{0}; row < m_rowsMetadata.count(); ++row){
			if (m_rowsMetadata.at(row).virt_index == virt_row) {
				qDebug() << dataValue(row, 2);
				break;
			}
		}
	} while (++virt_row < m_rowsMetadata.count());*/
	emit sortChanged();
	if (do_layout_signals)
		emit layoutChanged();
}

void TPListModel::search(const QString &search_term, int field, const bool do_layout_signals)
{
	if (search_term == m_searchTerm)
		return;

	if (search_term.length() <= 3) {
		if (!m_searchTerm.isEmpty()) {
			m_searchTerm.clear();
			m_searchField = -1;
			for (auto &row_data : m_rowsMetadata)
				row_data.past_states.clear();
		}
		return;
	} else {
		auto diff{m_searchTerm.length() - search_term.length()};
		if (diff <= 0) {
			for (auto &row_data : m_rowsMetadata)
				fromLastState(row_data.real_index);
		}
	}
	if (do_layout_signals)
		emit layoutAboutToBeChanged();

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
	if (do_layout_signals)
		emit layoutChanged();
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

void TPListModel::remove(const bool from_qml)
{
	if (!m_rowsToRemove.isEmpty()) {
		if (from_qml && appSettings()->alwaysAskConfirmation()) {
			const int requestid{appUtils()->generateUniqueId()};
			auto conn{std::make_shared<QMetaObject::Connection>()};
			*conn = connect(appItemManager(), &QmlItemManager::removeDialogClosed, this, [this,requestid,conn]
																	(const int request_id, const int close_action_type) {
				if (requestid == request_id) {
					disconnect(*conn);
					if (close_action_type == 0)
						remove(false);
				}
			});
			QString message;
			if (m_removeField < 0)
				message = std::move(tr("Remove the selected entry(ies)?"));
			else {
				if (m_rowsToRemove.count() == 1)
					message = std::move(tr("Remove %1?").arg(dataValue(m_rowsToRemove.constFirst(), m_removeField)));
				else {
					message = dataValue(m_rowsToRemove.at(0), m_removeField) % "<br>"_L1;
					message += dataValue(m_rowsToRemove.at(1), m_removeField) % "<br>"_L1;
					if (m_rowsToRemove.count() > 2)
						message += "...<br>"_L1 % dataValue(m_rowsToRemove.constLast(), m_removeField) % "<br>"_L1;
					message += std::move(tr("Remove these entries?"));
				}
			}
			appItemManager()->showRemoveDialog(requestid, m_parentPage, m_identifier, tr("Remove ") % m_identifier, message);
			return;
		}
		for (const auto row : m_rowsToRemove | std::views::reverse)
			removeMetaData(row);
	}
}


void TPListModel::fixVirtualIndices()
{
	emit layoutAboutToBeChanged();
	int virt_index{0};
	for (auto &row_data : m_rowsMetadata)
		row_data.virt_index = row_data.visible.at(0) ? virt_index++ : -1;
	emit layoutChanged();
}

void TPListModel::insertSort(const uint row)
{
	if (m_rowsMetadata.at(row).virt_index >= 0) {
		auto new_virt_index{m_sortDirection == SORT_DOWN ? 0 : m_nVisibleRows > 0 ? m_rowsMetadata.count() - 1 : 0};
		auto other_row{0};
		const QString &data_value{dataValue(row, m_sortField)};
		do {
			if (row != other_row && visible(other_row)) {
				const auto res{compareStrings(data_value, dataValue(other_row, m_sortField))};
				if (m_sortDirection == SORT_DOWN) {
					if (res >= 0)
						++new_virt_index;
				} else {
					if (res > 0)
						--new_virt_index;
				}
				if (m_searchField >= 0)
					toLastState(row, false);
			}
		} while (++other_row  < m_rowsMetadata.count());
		const bool row_moved{m_rowsMetadata[row].virt_index != new_virt_index};
		m_rowsMetadata[row].virt_index = new_virt_index;
		if (row_moved) {
			if (m_sortDirection == SORT_DOWN) {
				for (auto &row_data : m_rowsMetadata) {
					if (row_data.real_index != row) {
						if (row_data.virt_index <= new_virt_index)
							++(row_data.virt_index);
					}
				}
			} else {
				for (auto &row_data : m_rowsMetadata) {
					if (row_data.real_index != row) {
						if (row_data.virt_index >= new_virt_index)
							++(row_data.virt_index);
					}
				}
			}
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

void TPListModel::setVisible(const uint row, bool visible, const uint column, const bool emit_signals)
{
	const bool is_visible{TPListModel::visible(row, column)};
	m_rowsMetadata[row].visible[column] = visible;
	if (emit_signals)
		emit dataChanged(index(row, m_selectEntireRow ? 0 : column), index(row, m_selectEntireRow ? m_totalCols : column),
																								QList<int>{itemVisibleRole});
	if (is_visible && !visible)
		--m_nVisibleRows;
	else if (!is_visible && visible)
		++m_nVisibleRows;
	if (emit_signals) {
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

void TPListModel::syncMetadata(const uint modeldata_count, const bool do_layout_signals)
{
	if (modeldata_count > 0) {
		if (do_layout_signals)
			emit layoutAboutToBeChanged();
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
		if (m_enableFilters)
			applyFilters(false);
		if (m_sortDirection != NO_SORT)
			sort(false);
		if (m_searchField >= 0)
			search(m_searchTerm, m_searchField);
		setCurrentRow(0);
		emit countChanged();
		if (do_layout_signals)
			emit layoutChanged();
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
	if (row < m_rowsMetadata.count()) {
		for (auto &row_data : m_rowsMetadata | std::views::drop(row)) {
			row_data.real_index++;
			if (m_sortDirection == NO_SORT)
				row_data.virt_index++;
		}
	}
	st_rowData row_data;
	row_data.real_index = row_data.virt_index = row;
	bool visible{m_enableFilters ? itemShouldBeVisible(row) : true};
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
	m_rowsMetadata.insert(row, std::move(row_data));

	if (visible) {
		if (m_sortDirection != NO_SORT)
			insertSort(row);
		if (m_searchField >= 0) {
			if (m_selectEntireRow) {
				m_rowsMetadata[row].visible[0] = visible = itemShouldRemainVisible(row, prepareSearchTerm(m_searchTerm));
			} else {
				for (uint i{0}; i < m_totalCols; ++i) {
					if (i != m_searchField)
						m_rowsMetadata[row].visible[i] = true;
					else
						m_rowsMetadata[row].visible[i] = itemShouldRemainVisible(row, prepareSearchTerm(m_searchTerm));
				}
			}
		}
		if (visible)
			++m_nVisibleRows;
	}
	emit countChanged();
	emit itemAdded(row);
}

void TPListModel::removeMetaData(const int row)
{
	const int start_virt_row{m_rowsMetadata.at(row).virt_index};
	int start_row{row};
	m_rowsMetadata.removeAt(row);
	if (m_sortDirection != NO_SORT)
		start_row = 0;
	for (auto &row_data : m_rowsMetadata | std::views::drop(start_row)) {
		row_data.real_index = start_row++;
		if (row_data.virt_index >= start_virt_row)
			--row_data.virt_index;
	}
	emit itemRemoved(row);
	emit countChanged();
}

void TPListModel::setFiltersManager(TPFilterModel *filter_model, const bool use_default)
{
	if (filter_model) {
		if (m_filters) {
			disconnect(m_filters, nullptr, nullptr, nullptr);
			delete m_filters;
		}
		m_filters = filter_model;
		connect(m_filters, &TPFilterModel::filtersChanged, this, &TPListModel::applyFilters);
	} else {
		if (!use_default) {
			if (m_filters) {
				setEnableFilters(false);
				disconnect(m_filters, nullptr, nullptr, nullptr);
				delete m_filters;
				m_filters = nullptr;
			}
		} else {
			m_filters = new TPFilterModel{this};
			m_filters->setDataAcquisitionFunc([this] (uint index, uint column) -> QPair<bool,QString> {
				if (index < m_rowsMetadata.count())
					return {true, dataValue(index, column)};
				else
					return {false, QString{}};
			});
			for (uint i{0}; i < m_totalCols; ++i) {
				m_filters->addFilterField(i, nullptr, std::move(QString{}));
				m_filters->initFilterValues(i);
			}
		}
	}
	applyFilters();
}

inline bool TPListModel::itemShouldBeVisible(const uint real_row) const
{
		bool visible{false};
		for (const auto &filter : std::as_const(m_filters->filters())) {
			if (dataValue(real_row, m_filterField) == filter) {
				visible = true;
				break;
			}
		}
	return visible;
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
