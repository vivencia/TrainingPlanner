#include "tpfiltermodel.h"

#include "tplistmodel.h"
#include "tputils.h"
#include "translationclass.h"

enum RoleNames {
	createRole(field,		TPFilterModel::FF_FIELD)
	createRole(display,		TPFilterModel::FF_DISPLAY)
	createRole(icon,		TPFilterModel::FF_ICON)
	createRole(itemVisible,	TPFilterModel::FF_VISIBLE)
	createRole(itemEnabled,	TPFilterModel::FF_ENABLED)
	createRole(selected,	TPFilterModel::FF_SELECTED)
	createRole(values,		TPFilterModel::FF_VALUES_LIST)
	createRole(selValues,	TPFilterModel::FF_SELECTED_VALUES)
};

TPFilterModel::TPFilterModel(TPListModel *parent)
	: QAbstractListModel{parent}, m_parentModel{parent}
{
	roleToString(field)
	roleToString(display)
	roleToString(icon)
	roleToString(itemVisible)
	roleToString(itemEnabled)
	roleToString(selected)
	roleToString(values)
	roleToString(selValues)

	connect(m_parentModel, &TPListModel::itemAdded, this, [this] (const int index) {
		//When an item is added to the parent model, go through every filter element and add the relevant value its list
		//of values only if the filter as been initialized
		for (uint i{0}; i < m_filterElements.count(); ++i) {
			if (!m_filterElements.at(i).values.isEmpty()) {
				const QPair<bool,QString> &value{m_getData(index, m_filterElements.at(index).field)};
				if (!hasValue(m_filterElements.at(i), value.second)) {
					m_filterElements[i].selected_values.append(std::move(false));
					m_filterElements[i].values.append(std::move(value.second));
				}
			}
		}

	});
	connect(m_parentModel, &TPListModel::itemRemoved, this, [this] (const int index) {
		//When an item is removed from the parent model, go through every filter element and every filter value under it and
		//compare it to the remaining field values of all the items in the model. When the filter value is not found,
		//meaning that filter no longer could apply to the model because the model is void of it, remove it
		for (uint i{0}; i < m_filterElements.count(); ++i) {
			if (!m_filterElements.at(i).values.isEmpty()) {
				auto value_idx{m_filterElements.at(i).values.count() - 1};
				for (auto x{value_idx}; x >= 0; --x) {
					uint data_index{0};
					bool found{false};
					const auto &value{m_filterElements.at(i).values.at(x)};
					do {
						const QPair<bool,QString> &model_value{m_getData(data_index++, m_filterElements.at(i).field)};
						if (model_value.first) {
							if (model_value.second == value) {
								found = true;
								break;
							}
						} else {
							break;
						}
					} while (true);
					if (!found)
						m_filterElements.removeAt(x);
				}
			}
		}
	});

	connect(appTr(), &TranslationClass::applicationLanguageChanged, this, [this] () {
		uint i{0};
		for (auto &filter : m_filterElements)
			filter.display = std::move(m_filterDisplayStringFuncs.at(i++)());
		emit dataChanged(index(0, 0), index(0, i));
	});
}

int TPFilterModel::addFilterField(const uint field, const std::function<QString()> &display_func, QString &&image,
							 const bool visible, const bool enabled, const bool selected)
{
	const auto filter_idx{m_filterElements.count()};
	beginInsertRows(QModelIndex{}, filter_idx, filter_idx);
	st_Filter new_filter;
	new_filter.field = field;
	new_filter.display = std::move(display_func());
	new_filter.image = std::move(image);
	new_filter.visible = visible;
	new_filter.enabled = enabled;
	new_filter.selected = selected;
	m_filterDisplayStringFuncs.append(display_func);
	m_filterElements.append(std::move(new_filter));
	emit filtersChanged();
	endInsertRows();
	emit countChanged();
	return filter_idx;
}

void TPFilterModel::removeFilter(const uint filter_idx)
{
	if (filter_idx < m_filterElements.count()) {
		beginRemoveRows(QModelIndex{}, filter_idx, filter_idx);
		m_filterElements.removeAt(filter_idx);
		emit filtersChanged();
		emit countChanged();
		endRemoveRows();
	}
}

void TPFilterModel::clearSelection()
{
	for (auto &filter : m_filterElements)
		filter.selected = false;
	emit dataChanged(index(0, 0), index(count() - 1, 0), QList<int>{} << selectedRole);
}

void TPFilterModel::selectAll()
{
	for (auto &filter : m_filterElements)
		filter.selected = true;
	emit dataChanged(index(0, 0), index(count() - 1, 0), QList<int>{} << selectedRole);
}

void TPFilterModel::initFilterValues(const uint field)
{
	if (m_getData) {
		st_Filter *filter{nullptr};
		for (uint i{0}; i < m_filterElements.count(); ++i) {
			if (m_filterElements.at(i).field == field) {
				filter = &m_filterElements[i];
				break;
			}
		}
		uint data_index{0};
		do {
			const QPair<bool,QString> &value{m_getData(data_index++, field)};
			if (value.first) {
				if (!hasValue(std::as_const(*filter), value.second)) {
					filter->selected_values.append(false);
					filter->values.append(std::move(value.second));
				}
			} else {
				break;
			}
		} while (true);
	}
}

QVariant TPFilterModel::data(const QModelIndex &index, int role) const
{
	const int row{index.row()};
	if (row >= 0) {
		switch (role) {
		case fieldRole:			return m_filterElements.at(row).field;
		case displayRole:		return m_filterElements.at(row).display;
		case iconRole:			return m_filterElements.at(row).image;
		case itemVisibleRole:	return m_filterElements.at(row).visible;
		case itemEnabledRole:	return m_filterElements.at(row).enabled;
		case selectedRole:		return m_filterElements.at(row).selected;
		case valuesRole:		return m_filterElements.at(row).values;
		default: break;
		}
	}
	return QVariant{};
}

bool TPFilterModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	const int row{index.row()};
	if (row >= 0 && role == selectedRole) {
		st_Filter &filter{m_filterElements[row]};
		filter.selected = true;
		int last_select_row{0};
		for (auto &_filter : m_filterElements) {
			if (_filter.field == m_parentModel->filterField()) {
				m_filters.clear();
				_filter.selected = false;
				for (auto &selected_value : _filter.selected_values)
					selected_value = false;
				QModelIndex prev_index{TPFilterModel::index(last_select_row, 0)};
				emit dataChanged(prev_index, prev_index, QList<int>{} << valuesRole);
				emit dataChanged(prev_index, prev_index, QList<int>{} << selectedRole);
				break;
			}
			++last_select_row;
		}
		filter.selected = true;
		emit dataChanged(index, index, QList<int>{} << selectedRole);
		m_parentModel->setFilterField(filter.field);
		if (m_filterElements.at(row).values.isEmpty())
			const_cast<TPFilterModel*>(this)->initFilterValues(m_filterElements.at(row).field);
		emit dataChanged(index, index, QList<int>{} << valuesRole);
		return true;
	}
	return false;
}

inline bool TPFilterModel::hasValue(const st_Filter &filter, const QString &value) const
{
	const auto &itr{std::find_if(filter.values.cbegin(), filter.values.cend(), [value] (const QString &filter_value) {
		return filter_value == value;
	})};
	return itr != filter.values.cend();
}
