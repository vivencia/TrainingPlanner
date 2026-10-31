#include "tplistmodel.h"

#include "qmlitemmanager.h"
#include "tpsettings.h"
#include "tputils.h"
#include "translationclass.h"

#include <QQmlComponent>
#include <QQmlApplicationEngine>

enum metaFields {
	MF_REAL_INDEX = 200,
	MF_VIRTUAL_INDEX = 201,
	MF_SELECTED	= 202,
	MF_VISIBLE	= 203,
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

	m_fieldsNames.append(std::move(tr("Unsorted")));
	m_fieldsNames.append(std::move("--"));
	connect(appTr(), &TranslationClass::applicationLanguageChanged, this, [this] () {
		m_fieldsNames.clear();
		m_fieldsNames.append(std::move(tr("Unsorted")));
		m_fieldsNames.append(std::move("--"));
		setFieldsNames();
		emit fieldsNamesChanged();
	});
	doConnectionsWork(true);
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
					appItemManager()->log(TPLogs::LT_DEBUG, TPLogs::originQml,
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

void TPListModel::applySecondaryFilters(const bool do_layout_signals)
{
	if (m_rowsMetadata.isEmpty())
		return;
	if (do_layout_signals)
		emit layoutAboutToBeChanged();
	if (m_enableFilters && !m_filters->filters().isEmpty()) {
		int new_virt_index{0};
		for (uint row{0}; row < m_rowsMetadata.count(); ++row) {
			if (visible(row)) {
				const bool visible{itemShouldBeVisible_2(row)};
				m_rowsMetadata[row].virt_index = visible ? new_virt_index++ : -1;
				setVisible(row, visible, 0, false);
			}
		}
	} else {
		applyPersistentFilters(false);
	}

	if (do_layout_signals) {
		if (m_sortDirection != NO_SORT)
			sort(false);
		if (m_searchTerm.length() > 3)
			search(m_searchTerm, m_searchField);
		setCurrentRow(count() > 0 ? 0 : -1);
		emit layoutChanged();
	}
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

	auto diff{search_term.length() - m_searchTerm.length()};
	if (diff < 0) {
		if (do_layout_signals)
			emit layoutAboutToBeChanged();
		for (auto &row_data : m_rowsMetadata)
			restoreState(row_data.real_index, (-1 * diff) < row_data.past_states.count() ? diff : -row_data.past_states.count());
	}

	m_searchTerm = search_term;
	if (diff <= 0 && do_layout_signals) {
		emit layoutChanged();
		return;
	} else if (m_searchTerm.length() <= 3)
		return;

	if (diff > 0 && do_layout_signals)
		emit layoutAboutToBeChanged();

	m_searchField = field;
	const QStringList &words_list{prepareSearchTerm(m_searchTerm)};

	//Save the state of all hidden items first. The save states must be consistent for all items and correspond to the length
	//(from the 4th position on) of the search string, i.e a search string of 13 characters ("search term 1") will produce 10
	//save states(1 = 9, ' ' = 8, m = 7, r = 6, e = 5, t = 4, ' ' = 3, h = 2, c = 1, r = 0). When/if the 'r' is removed, the
	//first save state will be popped out and the model will return to the pre-search state.
	for (uint i{0}; i < m_rowsMetadata.count(); ++i) {
		if (m_rowsMetadata.at(i).virt_index < 0)
			saveState(m_rowsMetadata.at(i).real_index, true);
	}

	//Now go in -visible- order, respecting any sorting that might be occurring, and see if the item should or not continue to be visible
	uint next_virt_index{0};
	uint new_virt_index{0};
	for (int i{0}; i < m_rowsMetadata.count(); ++i) {
		if (m_rowsMetadata.at(i).virt_index == next_virt_index) {
			saveState(m_rowsMetadata.at(i).real_index, true);
			if (!itemShouldRemainVisible(m_rowsMetadata.at(i).real_index, words_list)) {
				setVisible(m_rowsMetadata.at(i).real_index, false);
				m_rowsMetadata[i].virt_index = -1;
			} else {
				m_rowsMetadata[i].virt_index = new_virt_index++;
			}
			++next_virt_index;
			i = -1;
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

bool TPListModel::askBeforeRemove() const
{
	if (m_askBeforeRemove.has_value())
		return m_askBeforeRemove.value();
	else
		return appSettings()->alwaysAskConfirmation();
}

void TPListModel::syncMetadata(const uint modeldata_count)
{
	if (modeldata_count > m_rowsMetadata.count()) {
		m_rowsMetadata.reserve(m_rowsMetadata.count() + modeldata_count);
		if (modeldata_count <= 10) {
			beginInsertRows(QModelIndex{}, count(), count() + modeldata_count - 1);
			endInsertRows();
		} else {
			doConnectionsWork(false);
			beginInsertRows(QModelIndex{}, count(), count() + modeldata_count - 1);
			for (uint row{0}; row < modeldata_count; ++row) {
				st_rowData row_data;
				row_data.real_index = row_data.virt_index = row;
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
			m_nVisibleRows = modeldata_count;
			if (m_hasPersistentFilters)
				applyPersistentFilters(false);
			if (m_enableFilters)
				applySecondaryFilters(false);
			if (m_sortDirection != NO_SORT)
				sort(false);
			if (m_searchTerm.length() > 3)
				search(m_searchTerm, m_searchField);
			setCurrentRow(0);
			endInsertRows();
			emit countChanged();
			doConnectionsWork(true);
		}
	}
}

void TPListModel::insertMetaData(const int row)
{
	if (row < m_rowsMetadata.count()) {
		for (auto &row_data : m_rowsMetadata | std::views::drop(row)) {
			row_data.real_index++;
			if (m_sortDirection == NO_SORT)
				row_data.virt_index++;
		}
	}
	st_rowData row_data;
	row_data.real_index = row;
	bool visible{true};
	if (m_hasPersistentFilters)
		visible = itemShouldBeVisible_1(row);
	if (visible && m_enableFilters)
		visible = itemShouldBeVisible_2(row);
	if (m_searchTerm.length() > 3) {
		visible = itemShouldRemainVisible(row, prepareSearchTerm(m_searchTerm));
		syncNewItemStates(row, visible);
	}
	row_data.virt_index = visible ? nextVirtualIndex() : -1;
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

	if (m_sortDirection != NO_SORT)
		insertSort(row);
	if (visible) {
		++m_nVisibleRows;
		emit visibleCountChanged();
	} else if (m_sortDirection != NO_SORT) {
		emit dataChanged(index(row, 0), index(row, 0));
	}
	setCurrentRow(row_data.virt_index);
	emit countChanged();
	emit itemAdded(row);
}

void TPListModel::removeMetaData(const int row)
{
	emit itemRemoved(row);
	const int start_virt_row{m_rowsMetadata.at(row).virt_index};
	m_rowsMetadata.removeAt(row);
	emit layoutAboutToBeChanged();
	for (auto &row_data : m_rowsMetadata) {
		if (row_data.real_index > row)
			--row_data.real_index;
		if (row_data.virt_index > start_virt_row)
			--row_data.virt_index;
	}
	emit layoutChanged();
	emit countChanged();
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

void TPListModel::remove(const bool from_qml, const bool bypass_confirmation)
{
	if (!m_rowsToRemove.isEmpty()) {
		if (!bypass_confirmation && from_qml && askBeforeRemove()) {
			const int requestid{appUtils()->generateUniqueId()};
			auto conn{std::make_shared<QMetaObject::Connection>()};
			*conn = connect(appItemManager(), &QmlItemManager::removeDialogClosed, this, [this,requestid,conn]
								(const int request_id, const int close_action_type, const bool dont_ask_again) {
				if (requestid == request_id) {
					disconnect(*conn);
					if (close_action_type == 0) {
						setAskBeforeRemove(!dont_ask_again);
						remove(true, true);
					} else {
						emit cancelRemove(-1);
					}
				}
			});
			const QString &title{!mf_removeTitle ? tr("Remove ") % m_identifier : mf_removeTitle(m_rowsToRemove)};
			QString message;
			if (m_removeField < 0) {
				message = !mf_removeMessage ? std::move(tr("Remove the selected entry(ies)?")) : std::move(mf_removeMessage(m_rowsToRemove));
			} else {
				if (m_rowsToRemove.count() == 1) {
					message = !mf_removeMessage
						? std::move(tr("Remove %1?").arg(dataValue(m_rowsToRemove.constFirst(), m_removeField)))
						: std::move(mf_removeMessage(m_rowsToRemove));
				} else {
					if (!mf_removeMessage) {
						message = dataValue(m_rowsToRemove.at(0), m_removeField) % "<br>"_L1;
						message += dataValue(m_rowsToRemove.at(1), m_removeField) % "<br>"_L1;
						if (m_rowsToRemove.count() > 2)
							message += "...<br>"_L1 % dataValue(m_rowsToRemove.constLast(), m_removeField) % "<br>"_L1;
						message += std::move(tr("Remove these entries?"));
					} else {
						message = std::move(mf_removeMessage(m_rowsToRemove));
					}
				}
			}
			appItemManager()->showRemoveDialog(requestid, m_parentPage, m_identifier, title, message);
			return;
		}
		if (from_qml) {
			beginRemoveRows(QModelIndex{}, m_rowsToRemove.constFirst(), m_rowsToRemove.constLast());
			endRemoveRows();
		}
	}
}

uint TPListModel::nextVirtualIndex() const
{
	uint virt_index{0};
	for (const auto &row_data : std::as_const(m_rowsMetadata)) {
		if (row_data.virt_index >= virt_index) {
			virt_index = row_data.virt_index + 1;
			if (virt_index >= m_nVisibleRows)
				break;
		}
	}
	return virt_index;
}

void TPListModel::resetVirtualIndices()
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
		const QString &data_value{dataValue(row, m_sortField)};
		uint next_virt_index{0};
		do {
			auto other_row{0};
			for (; other_row < m_rowsMetadata.count(); ++other_row) {
				if (other_row != row) {
					const int other_virt_index{m_searchTerm.length() <= 3 ? m_rowsMetadata.at(other_row).virt_index :
														m_rowsMetadata.at(other_row).past_states.constFirst().first};
					if (other_virt_index == next_virt_index) {
						const auto res{compareStrings(data_value, dataValue(other_row, m_sortField))};
						const bool insert_here{m_sortDirection == SORT_DOWN ? res <= 0 : res > 0};
						if (insert_here) {
							int first_modified_row{-1}, last_modified_row{-1};
							m_rowsMetadata[row].virt_index = other_virt_index;
							for (other_row = 0; other_row < m_rowsMetadata.count(); ++other_row) {
								if (other_row != row) {
									if (m_searchTerm.length() <= 3) { //Search not active
										if (m_rowsMetadata.at(other_row).virt_index >= other_virt_index) {
											m_rowsMetadata[other_row].virt_index++;
											if (first_modified_row == -1)
												first_modified_row = other_row;
											last_modified_row = other_row;
										}
									} else { //Search active, modify baseline states only
										if (m_rowsMetadata.at(other_row).past_states.constFirst().first >= other_virt_index) {
											m_rowsMetadata[other_row].past_states.first().first++;
											if (first_modified_row == -1)
												first_modified_row = other_row;
											last_modified_row = other_row;
										}
									}
								}
							} //for () after insertion place found
							emit dataChanged(index(qMin(static_cast<int>(row), first_modified_row), 0),
															index(qMax(static_cast<int>(row), last_modified_row), 0));
							return;
						} //insert_here
						break; //once the expected virtual index is found, if not the insertion place, stop and look for the next
					}
				} //(other_row != row)
			} //for()
		} while (++next_virt_index < m_rowsMetadata.count());
	}
}

inline const bool TPListModel::visible(const int row, const int column) const
{
	if (row >= 0) {
		if (m_selectEntireRow || column == -1)
			return m_rowsMetadata.at(row).visible.at(0);
		else if (column < m_totalCols)
			return m_rowsMetadata.at(row).visible.at(column);
	}
	return false;
}

void TPListModel::setVisible(const uint row, bool visible, const int column, const bool emit_signals)
{
	const bool is_visible{TPListModel::visible(row, column)};
	if (m_selectEntireRow || column == -1)
		m_rowsMetadata[row].visible[0] = visible;
	else
		m_rowsMetadata[row].visible[column] = visible;
	if (emit_signals)
		emit dataChanged(index(row, m_selectEntireRow ? 0 : column), index(row, m_selectEntireRow ? m_totalCols : column),
																								QList<int>{itemVisibleRole});
	if (is_visible && !visible)
		--m_nVisibleRows;
	else if (!is_visible && visible)
		++m_nVisibleRows;
	if (emit_signals) {
		emit visibleCountChanged();
		if (m_currentRow >= 0) {
			if (!visible && row <= m_currentRow) {
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

void TPListModel::setFiltersManager(TPFilterModel *filter_model, const bool use_default)
{
	if (filter_model) {
		if (m_filters) {
			disconnect(m_filters, nullptr, nullptr, nullptr);
			delete m_filters;
		}
		m_filters = filter_model;
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
	if (m_filters) {
		connect(m_filters, &TPFilterModel::filtersChanged, this, &TPListModel::applySecondaryFilters);
		connect(m_filters, &TPFilterModel::hasFilterChanged, this, [this] () {
			emit hasFilterChanged();
			if (m_filters->filters().isEmpty())
				setEnableFilters(false);
		});
	}
}

void TPListModel::applyPersistentFilters(const bool do_layout_signals)
{
	if (m_rowsMetadata.isEmpty())
		return;
	if (do_layout_signals)
		emit layoutAboutToBeChanged();
	if (m_filters->positiveFilter().first >= 0 || m_filters->negativeFilter().first >= 0) {
		int new_virt_index{0};
		for (uint row{0}; row < m_rowsMetadata.count(); ++row) {
			const bool visible{itemShouldBeVisible_1(row)};
			m_rowsMetadata[row].virt_index = visible ? new_virt_index++ : -1;
			setVisible(row, visible, 0, false);
		}
	} else {
		for (uint row{0}; row < m_rowsMetadata.count(); ++row) {
			m_rowsMetadata[row].virt_index = row;
			setVisible(row, true, 0, false);
		}
	}

	if (do_layout_signals) {
		if (m_enableFilters)
			applySecondaryFilters(false);
		if (m_sortDirection != NO_SORT)
			sort(false);
		if (m_searchTerm.length() > 3)
			search(m_searchTerm, m_searchField);
		setCurrentRow(count() > 0 ? 0 : -1);
		emit layoutChanged();
	}
}

inline bool TPListModel::itemShouldBeVisible_1(const uint real_row) const
{
	bool visible{true};
	if (m_filters->positiveFilter().first >= 0) {
		if (m_compareFunc)
			visible = m_compareFunc(true, real_row);
		else
			visible = dataValue(real_row, m_filters->positiveFilter().first) == m_filters->positiveFilter().second;
	}
	if (m_filters->negativeFilter().first >= 0) {
		if (m_compareFunc)
			visible = m_compareFunc(false, real_row);
		else
			visible = dataValue(real_row, m_filters->negativeFilter().first) != m_filters->negativeFilter().second;
	}
	return visible;
}

//Assumes item in real_row is visible
inline bool TPListModel::itemShouldBeVisible_2(const uint real_row) const
{
	if (!m_filters->filters().isEmpty()) {
		for (const auto &filter : std::as_const(m_filters->filters())) {
			if (dataValue(real_row, m_filterField) == filter)
				return true;
		}
	}
	return false;
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

inline void TPListModel::restoreState(const uint real_row, const int index)
{
	if (index < 0 && !m_rowsMetadata.at(real_row).past_states.isEmpty()) {
		auto state_index{m_rowsMetadata.at(real_row).past_states.count() + index};
		auto state{m_rowsMetadata.at(real_row).past_states.at(state_index)};
		m_rowsMetadata[real_row].virt_index = state.first;
		setVisible(real_row, state.second, m_searchField);
		m_rowsMetadata[real_row].past_states.resize(state_index);
	}
}

inline void TPListModel::saveState(const uint real_row, const bool add_new_state)
{
	QPair<int,bool> last_state{m_rowsMetadata.at(real_row).virt_index, m_selectEntireRow
					? m_rowsMetadata.at(real_row).visible[0] : m_rowsMetadata.at(real_row).visible[m_searchField]};
	if (add_new_state)
		m_rowsMetadata[real_row].past_states.append(std::move(last_state));
	else
		m_rowsMetadata[real_row].past_states.last() = last_state;
}

void TPListModel::syncNewItemStates(const uint real_row, const bool laststate_visible)
{
	//1 - Save the baseline state. i.e. how the item is before first search(character)
	m_rowsMetadata[real_row].past_states.append(std::move(
			QPair<int,bool>{m_rowsMetadata.at(real_row).virt_index, m_rowsMetadata.at(real_row).virt_index >= 0}));
	//2 - Now that the baseline is saved, update the item to its current state
	if (m_selectEntireRow) {
		m_rowsMetadata[real_row].visible[0] = laststate_visible;
	} else {
		for (uint i{0}; i < m_totalCols; ++i)
			m_rowsMetadata[real_row].visible[i] = i != m_searchField ? true : laststate_visible;
	}
	if (!laststate_visible)
		m_rowsMetadata[real_row].virt_index = -1;

	//3 - 3 means the minimum search term length(i.e. a quantity to be ignored, no states up untill then); +1 means
	//ignore the first(baseline) state because we already set it up
	const auto n_chars{m_searchTerm.length()};
	const auto n_states{n_chars - (3 + 1)};
	//4 - If the item's baseline state is to be hidden(because of a filter, then all subsequent states must reflect that
	if (m_rowsMetadata.at(real_row).past_states.constFirst().first < 0) {
		for (auto i{0}; i < n_states; ++i)
			m_rowsMetadata[real_row].past_states.append(std::move(QPair<int,bool>{-1, false}));
	} else {
		int pos{3};
		while ((pos + n_states) < n_chars) {
			const QStringList &words_list{prepareSearchTerm(m_searchTerm.first(pos))};
			if (itemShouldRemainVisible(real_row, words_list)) { //visible
				if (m_sortDirection == NO_SORT)
					m_rowsMetadata[real_row].past_states.append(std::move(QPair<int,bool>{real_row, true}));
				else
					//5 - the other item's state index never accounted for this new item, so they need to be updated
					alterStates(real_row, pos);
			} else { //not visible
				m_rowsMetadata[real_row].past_states.append(std::move(QPair<int,bool>{-1, false}));
			}
			pos++;
		};
	}
}

void TPListModel::alterStates(const uint new_item_row, uint state_index)
{
	const QString &data_value{dataValue(new_item_row, m_sortField)};
	auto new_virt_index{0};
	for (auto other_row{0}; other_row < m_rowsMetadata.count(); ++other_row) {
		if (new_item_row != other_row && visible(other_row)) {
			const auto res{compareStrings(data_value, dataValue(other_row, m_sortField))};
			const bool insert_here{m_sortDirection == SORT_DOWN ? res <= 0 : res > 0};
			if (insert_here) {
				m_rowsMetadata[new_item_row].past_states.append(std::move(QPair<int,bool>{new_virt_index, true}));
				for (; other_row < m_rowsMetadata.count(); ++other_row) {
					if (other_row != new_item_row && m_rowsMetadata.at(other_row).past_states.at(state_index).first >= new_virt_index)
						m_rowsMetadata[other_row].past_states[state_index].first++;
				}
				return;
			}
			++new_virt_index;
		}
	}
}

void TPListModel::doConnectionsWork(const bool do_connect)
{
	if (do_connect) {
		connect(this, &TPListModel::rowsInserted, this, [this] (const QModelIndex &parent, int first, int last) {
			for (int i{first}; i <= last; ++i)
				insertMetaData(i);
		});
		connect(this, &TPListModel::rowsRemoved, this, [this] (const QModelIndex &parent, int first, int last) {
			for (int i{last}; i >= first; --i)
				removeMetaData(i);
		});
		connect(this, &TPListModel::modelReset, this, [this] () {
			clear();
		});
	} else {
		disconnect(this, &TPListModel::rowsInserted, this, nullptr);
		disconnect(this, &TPListModel::rowsRemoved, this, nullptr);
		disconnect(this, &TPListModel::modelReset, this, nullptr);
	}
}
