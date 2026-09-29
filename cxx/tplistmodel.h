#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

class TPListModel : public QAbstractListModel
{

Q_OBJECT
QML_ELEMENT
QML_VALUE_TYPE(TPListModel)
QML_UNCREATABLE("")

Q_PROPERTY(uint count READ count NOTIFY countChanged)
Q_PROPERTY(uint colCount READ colCount CONSTANT FINAL)
Q_PROPERTY(int currentRow READ currentRow WRITE setCurrentRow NOTIFY currentRowChanged FINAL)
Q_PROPERTY(int sortField READ sortField WRITE setSortField NOTIFY sortFieldChanged FINAL)
Q_PROPERTY(int sortDirection READ sortDirection WRITE setSortDirection NOTIFY sortChanged FINAL)
Q_PROPERTY(int filterField READ filterField WRITE setFilterField NOTIFY filterFieldChanged FINAL)
Q_PROPERTY(bool filterApplied READ filterApplied NOTIFY filterAppliedChanged FINAL)
Q_PROPERTY(bool canApplyFilter READ canApplyFilter WRITE setCanApplyFilter NOTIFY canApplyFilterChanged FINAL)
Q_PROPERTY(bool selectEntireRow READ selectEntireRow WRITE setSelectEntireRow NOTIFY selectEntireRowChanged FINAL)
Q_PROPERTY(bool anySelected READ anySelected NOTIFY selectedChanged FINAL)
Q_PROPERTY(bool allSelected READ allSelected NOTIFY selectedChanged FINAL)
Q_PROPERTY(bool noneSelected READ noneSelected NOTIFY selectedChanged FINAL)
Q_PROPERTY(QStringList filters READ filters WRITE setFilters NOTIFY filterChanged FINAL)
Q_PROPERTY(QStringList fieldsNames READ fieldsNames NOTIFY fieldsNamesChanged FINAL)

public:
	enum SortDirection {
		SORT_UP,
		SORT_DOWN,
		NO_SORT
	};
	Q_ENUM(SortDirection)

	explicit TPListModel(QObject *parent = nullptr, const uint n_cols = 1);

	inline uint count() const { return m_nVisibleRows; }
	inline uint colCount() const { return m_totalCols; }

	inline int currentRow() const { return m_currentRow; }
	inline void setCurrentRow(const int new_row)
	{
		if (m_currentRow != new_row) {
			m_currentRow = new_row;
			emit currentRowChanged();
		}
	}

	inline bool canApplyFilter() const { return m_canApplyFilter; }
	void setCanApplyFilter(const bool apply)
	{
		if (m_canApplyFilter != apply) {
			m_canApplyFilter = apply;
			emit canApplyFilterChanged();
		}
	}
	inline int filterField() const { return m_filterField; }
	inline void setFilterField(const uint filter_field)
	{
		if (filter_field < m_totalCols && filter_field != m_filterField) {
			const bool re_filter{m_filterField >= 0};
			m_filterField = filter_field;
			emit filterFieldChanged();
			if (re_filter)
				applyFilters();
		}
	}
	inline QStringList filters() const { return m_filters; }
	inline void setFilters(const QStringList &filters)
	{
			m_filters = filters;
			emit filterChanged();
	}
	Q_INVOKABLE void insertFilter(const QString &filter)
	{
		if (!filter.isEmpty() && !m_filters.contains(filter)) {
			m_filters.append(filter);
			emit filterChanged();
		}
	}
	Q_INVOKABLE void removeFilter(const QString &filter)
	{
		if (!filter.isEmpty()) {
			const auto f_index{m_filters.indexOf(filter)};
			if (f_index >= 0) {
				m_filters.removeAt(f_index);
				emit filterChanged();
			}
		}
	}
	inline bool filterApplied() const { return m_filterApplied; }

	/**
	 * Name of the sortable fields to be included in TPListViewHeader. Don't override setFieldsNames() and the selectable
	 * combobox will not appear on the header and the default sort field will be used
	 */
	inline QStringList fieldsNames() const { return m_fieldsNames; }
	virtual void setFieldsNames() {}
	inline int sortField() const { return m_sortField; }
	inline void setSortField(const int sort_field)
	{
		if (sort_field != m_sortField) {
			m_sortField = sort_field;
			emit sortFieldChanged();
			if (m_sortField < 0)
				setSortDirection(NO_SORT);
			else if (m_sortDirection != NO_SORT)
				sort();
		}
	}

	inline int sortDirection() const { return static_cast<int>(m_sortDirection); }
	inline void setSortDirection(int s_dire)
	{
		if (m_sortDirection != s_dire) {
			if (s_dire == NO_SORT) {
				m_sortDirection = NO_SORT;
				fixVirtualIndices();
				emit sortChanged();
			} else {
				m_sortDirection = static_cast<SortDirection>(s_dire);
				if (m_sortField <= 0)
					m_sortField = 0;
				sort();
			}
		}
	}

	Q_INVOKABLE inline void clearSelection()
	{
		for(int i{0}; i < m_rowsMetadata.count(); ++i)
			setSelected(i, false);
	}
	Q_INVOKABLE inline void selectAll()
	{
		for(int i{0}; i < m_rowsMetadata.count(); ++i)
			setSelected(i, true);
	}

	inline uint nSelected() const { return m_nSelected; }
	inline bool allSelected() const { return m_nSelected == rowCount(); }
	inline bool anySelected() const { return m_nSelected > 0; }
	inline bool noneSelected() const { return m_nSelected == 0; }
	//This property must be constant and only set once because the logic to convert the metadata from entire
	//row to individual columns is not implemented
	inline bool selectEntireRow() const { return m_selectEntireRow; }
	inline void setSelectEntireRow(const bool full_sel) { m_selectEntireRow = full_sel; emit selectEntireRowChanged(); }

	//visible_column = -1 means entire row.
	Q_INVOKABLE inline bool isSelected(const int visible_row, const uint visible_column = 0) const
	{
		const auto real_row(realRow(visible_row));
		return selected(real_row, visible_column);
	}
	Q_INVOKABLE void setIsSelected(const int visible_row, const bool selected, const uint visible_column = 0);
	//return row(index) of all selected(therefore visible) items
	QList<int> selectedInfo(const bool return_real_indices = true) const;

	/**
	 * Functions as a first round of searching, but works on real rows/indices.
	 * Limits the visible items to a certain condition, e.g. split A(filter = "A",
	 * field = MESO_FIELD_SPLITA), or completed exercises (filter = "1", field =  EXERCISES_FIELD_COMPLETED)
	 * @see sort() and search()
	 */
	Q_INVOKABLE void applyFilters();

	/**
	 * Works on visible/virtual rows/indices. Second round of searching. Uses QString::compare()
	 * @field: -1 gives the chance for a derived class to set m_sortField before calling this and the sort will
	 * be done on that field. Otherwise, the default sort field is 0.
	 * @see applyFilter() and search()
	 **/
	void sort();

	/**
	 * If field < 0, search()/find() will look in all fields of the visible/virtual rows/indices.
	 * Modifies the visibilty of already visible items to false if there is no match.
	 * This is the third round of searching.
	 * @see applyFilter() and sort()
	 **/
	Q_INVOKABLE void search(const QString &search_term, int field = -1);

	/** Searches the model without any sort of change
	 *  @note Does not modify the visibility of an item.
	 *  @return -1 if nothing is found, real row/index if visible_rows is false, virtual/visible row/index otherwise
	 **/
	Q_INVOKABLE int find(const bool visible_rows, const QString &needle, int field = -1) const;

	/**
	 * @brief dataValue
	 * Reimplement when the container for the model data is not the default m_modelData
	 * @return
	 */
	virtual inline const QString &dataValue(const uint real_row, const uint column) const
	{
		return m_modelData.at(real_row).at(column);
	}

	inline int rowCount(const QModelIndex & = QModelIndex{}) const override final { return count(); }
	inline int columnCount(const QModelIndex & = QModelIndex{}) const override final { return colCount(); }

	QVariant data(const QModelIndex &index, int role) const final override;
	bool setData(const QModelIndex &index, const QVariant &value, int role) final override;
	// return the roles mapping to be used by QML
	inline QHash<int, QByteArray> roleNames() const override final { return m_roleNames; }

signals:
	void selectEntireRowChanged();
	void countChanged();
	void filterFieldChanged();
	void sortFieldChanged();
	void currentRowChanged();
	void selectedChanged();
	void visibleChanged();
	void filterChanged();
	void fieldsNamesChanged();
	void filterAppliedChanged();
	void canApplyFilterChanged();
	void sortChanged();

protected:
	/** Derived classes *must* implement this function and "can not" implement data(const QModelIndex &index, int role)
	 *  @return The return value will be used by data(const QModelIndex &index, int role)
	 **/
	virtual QVariant data(const uint role, const uint row, const int column = -1) const = 0;

	/** Derived classes must implement this function if they need setData functionality
	 *  and "can not" implement setData(const QModelIndex &index, const QVariant &value, int role)
	 *  @return The return value will be used by setData(const QModelIndex &index, const QVariant &value, int role)
	 *			The default implementation does nothing.
	 **/
	virtual bool setData(const uint role, const uint row, const QVariant &value, const int column = -1)
	{
		Q_UNUSED(role); Q_UNUSED(row); Q_UNUSED(value); Q_UNUSED(column);
		return false;
	}

	/**
	 * @brief syncMetadata
	 * Call after m_modelData is filled with data from batch operations(e.g. after a Database read).
	 */
	void syncMetadata(const uint modeldata_count);

	/**
	 * @brief insertMetaData/removeMetaData
	 * Automatically called after a row insertion/deletion as long as beginInsertRows()/beginRemoveRows() is used
	 * Can also be called independently
	 * @param row = real row/index
	 */
	void insertMetaData(int row);
	void removeMetaData(const int row);
	int realRow(const int visible_row) const;

	QHash<int, QByteArray> m_roleNames;
	QList<QStringList> m_modelData;
	QString m_searchTerm;
	int m_filterField{-1}, m_sortField{-1}, m_searchField{-1};
	QStringList m_filters, m_fieldsNames;

private:
	struct st_rowData {
		int real_index{0}, virt_index{0};
		QList<bool> visible;
		QList<bool> selected;
		QList<QPair<int,bool>> past_states; //search history;
	};

	QList<st_rowData> m_rowsMetadata;
	uint m_nSelected{0}, m_totalCols{1}, m_nVisibleRows{0};
	int m_currentRow{-1};
	bool m_selectEntireRow{true}, m_filterApplied{false}, m_canApplyFilter{false};
	SortDirection m_sortDirection{NO_SORT};

	void clear();
	void fixVirtualIndices();
	void insertSort(const uint row);
	const bool visible(const int row, const uint column = 0) const;
	void setVisible(const uint row, bool visible, const uint column = 0);
	bool selected(const int row, const uint visible_column = 0) const;
	void setSelected(const int row, const bool selected, const uint visible_column = 0);
	bool itemShouldBeVisible(const uint real_row) const;
	bool itemShouldRemainVisible(const uint real_row, const QStringList &search_terms) const;
	void fromLastState(const uint real_row);
	void toLastState(const uint real_row, const bool add_new_state);
};
