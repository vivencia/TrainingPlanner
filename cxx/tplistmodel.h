#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

class TPListModel : public QAbstractListModel
{

Q_OBJECT
QML_ELEMENT
QML_VALUE_TYPE(TPListModel)

Q_PROPERTY(int currentRow READ currentRow WRITE setCurrentRow NOTIFY currentRowChanged FINAL)
Q_PROPERTY(uint count READ count NOTIFY countChanged)
Q_PROPERTY(uint colCount READ colCount NOTIFY colCountChanged)
Q_PROPERTY(bool selectEntireRow READ selectEntireRow WRITE setSelectEntireRow NOTIFY selectEntireRowChanged FINAL)
Q_PROPERTY(bool anySelected READ anySelected NOTIFY selectedChanged FINAL)
Q_PROPERTY(bool allSelected READ allSelected NOTIFY selectedChanged FINAL)
Q_PROPERTY(bool noneSelected READ noneSelected NOTIFY selectedChanged FINAL)

public:
	explicit TPListModel(QObject *parent, const uint n_cols);

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

	inline const QList<QStringList> &modelData() const { return m_modelData; }

	Q_INVOKABLE inline void clearSelection()
	{
		for(int i{0}; i < m_rowsMetadata.count(); ++i)
			setSelected(i, false);
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
	Q_INVOKABLE inline void setIsSelected(const int visible_row, const bool selected, const uint visible_column = 0);
	//return row(index) of all selected(therefore visible) items
	Q_INVOKABLE QList<int> selectedInfo(const bool return_real_indices = true) const;
	//field = -1 means return the index(row), field = 0 means id, and so forth
	Q_INVOKABLE QStringList selectedInfo(const int field = -1, const bool return_real_indices = true) const;

	/**
	 * Functions as a first round of searching, but works on real rows/indices.
	 * Limits the visible items to a certain condition, e.g. split A(filter = "A",
	 * field = MESO_FIELD_SPLITA), or completed exercises (filter = "1", field =  EXERCISES_FIELD_COMPLETED)
	 * @see sort() and search()
	 */
	Q_INVOKABLE void applyFilter(const QString &filter, const uint field);

	/**
	 * Works on visible/virtual rows/indices. Second round of searching. Uses QString::compare()
	 * @see applyFilter() and search()
	 **/
	Q_INVOKABLE void sort(const bool enable_sorting = true, const bool ascending = true, const uint field = 0);

	/**
	 * If field < 0, search()/find() will look in all fields of the visible/virtual rows/indices.
	 * Modifies the visibilty of already visible items to false if there is no match
	 * This is the third round of searching
	 * @see applyFilter() and sort()
	 **/
	Q_INVOKABLE void search(const QString &search_term, int field = -1);

	/** Searches the model without any sort of change
	 *  @note Does not modify the visibility of an item.
	 *  @return -1 if nothing is found, real row/index if visible_rows is false, virtual/visible row/index otherwise
	 **/
	Q_INVOKABLE int find(const bool visible_rows, const QString &needle, int field = -1) const;

	inline int rowCount(const QModelIndex & = QModelIndex{}) const override final { return count(); }
	inline int columnCount(const QModelIndex & = QModelIndex{}) const override final { return m_totalCols; }

	QVariant data(const QModelIndex &index, int role) const final override;
	bool setData(const QModelIndex &index, const QVariant &value, int role) final override;
	// return the roles mapping to be used by QML
	inline QHash<int, QByteArray> roleNames() const override final { return m_roleNames; }

public slots:
	void userModified(const uint user_idx, const uint field);

signals:
	void selectEntireRowChanged();
	void currentRowChanged();
	void countChanged();
	void selectedChanged();
	void visibleChanged();

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

	void insertMetaData(const int row);
	void removeMetaData(const int row); //row must be real/visible row/index
	int realRow(const int visible_row) const;
	QHash<int, QByteArray> m_roleNames;
	QList<QStringList> m_modelData;

private:
	struct st_rowData {
		int real_index{0}, virt_index{0};
		QList<bool> visible;
		QList<bool> selected;
		QList<QPair<int,bool>> past_states; //search history;
	};

	QList<st_rowData> m_rowsMetadata;
	QString m_filter, m_searchTerm;
	uint m_nSelected{0}, m_totalCols{0}, m_nVisibleRows{0};
	int m_currentRow{-1}, m_fieldFilter{-1}, m_sortField{-1}, m_searchField{-1};
	bool m_selectEntireRow{false};
	std::optional<bool> m_ascendingSort{std::nullopt};

	void clear();
	void doSort(const uint row);
	const bool visible(const int row, const uint column = 0) const;
	void setVisible(const uint row, bool visible, const uint column = 0);
	bool selected(const int row, const uint visible_column = 0) const;
	void setSelected(const int row, const bool selected, const uint visible_column = 0);
	bool itemShouldBeVisible(const uint real_row, const QStringList &filters) const;
	bool itemShouldRemainVisible(const uint real_row, const QStringList &search_terms) const;
	void fromLastState(const uint real_row);
	void toLastState(const uint real_row, const bool add_new_state);
};
