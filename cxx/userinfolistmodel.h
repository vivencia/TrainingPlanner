#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

class UserInfoListModel : public QAbstractListModel
{

Q_OBJECT
QML_ELEMENT
QML_VALUE_TYPE(UserInfoModel)

Q_PROPERTY(int currentUserIdx READ currentUserIdx NOTIFY currentUserIdxChanged FINAL)
Q_PROPERTY(int currentRow READ currentRow WRITE setCurrentRow NOTIFY currentRowChanged FINAL)
Q_PROPERTY(uint count READ count NOTIFY countChanged)
Q_PROPERTY(bool anySelected READ anySelected NOTIFY selectedChanged FINAL)
Q_PROPERTY(bool allSelected READ allSelected NOTIFY selectedChanged FINAL)
Q_PROPERTY(bool noneSelected READ noneSelected NOTIFY selectedChanged FINAL)
Q_PROPERTY(bool showClients READ showClients WRITE setShowClients NOTIFY showClientsChanged FINAL)
Q_PROPERTY(bool showCoaches READ showCoaches WRITE setShowCoaches NOTIFY showCoachesChanged FINAL)
Q_PROPERTY(bool showConfirmed READ showConfirmed WRITE setShowConfirmed NOTIFY showConfirmedChanged FINAL)
Q_PROPERTY(bool showAvailable READ showAvailable WRITE setShowAvailable NOTIFY showAvailableChanged FINAL)

#ifndef Q_OS_ANDROID
Q_PROPERTY(bool allUsersList READ allUsersList WRITE setAllUsersList NOTIFY allUsersListChanged FINAL)
#endif

public:
	explicit UserInfoListModel(QObject *parent = nullptr);
	inline uint count() const { return m_nVisibleRows; }

	inline int currentUserIdx() const { return userIdx(m_currentRow); }
	Q_INVOKABLE inline int userIdx(const int row) const
	{
		return row >= 0 && row < count() ? _userIdx(rowFromVisibleRow(row)) : -1;
	}
	inline int currentRow() const { return m_currentRow; }
	inline void setCurrentRow(const int new_row)
	{
		if (m_currentRow != new_row) {
			m_currentRow = new_row;
			emit currentRowChanged();
		}
	}

	Q_INVOKABLE inline void clearSelection()
	{
		for(int i{0}; i < m_extraInfo.count(); ++i) {
			if (rowVisible(i))
				setSelected(i, false);
		}
	}
	Q_INVOKABLE bool isSelected(const int visible_row, const int column = 0) const;
	Q_INVOKABLE void setSelected(const int visible_row, const bool selected, const int column = 0);
	Q_INVOKABLE QStringList selectedUsers() const; //Only from visible rows
	inline uint nSelected() const { return m_nSelected; }
	inline bool allSelected() const { return m_nSelected == count(); }
	inline bool anySelected() const { return m_nSelected > 0; }
	inline bool noneSelected() const { return m_nSelected == 0; }
	inline bool selectEntireRow() const { return m_selectEntireRow; }
	inline void setSelectEntireRow(const bool full_sel) { m_selectEntireRow = full_sel; }

	inline const bool showClients() const { return m_showClients; }
	inline void setShowClients(const bool show)
	{
		if (show != m_showClients) {
			m_showClients = show;
			changeVisibilityAsPerCategory();
			emit showClientsChanged();
		}
	}
	inline const bool showCoaches() const { return m_showCoaches; }
	inline void setShowCoaches(const bool show)
	{
		if (show != m_showCoaches) {
			m_showCoaches = show;
			changeVisibilityAsPerCategory();
			emit showCoachesChanged();
		}
	}
	inline const bool showConfirmed() const { return m_showConfirmed; }
	inline void setShowConfirmed(const bool show)
	{
		if (show != m_showConfirmed) {
			m_showConfirmed = show;
			changeVisibilityAsPerCategory();
			emit showConfirmedChanged();
		}
	}
	inline const bool showAvailable() const { return m_showAvailable; }
	inline void setShowAvailable(const bool show)
	{
		if (show != m_showAvailable) {
			m_showAvailable = show;
			changeVisibilityAsPerCategory();
			emit showAvailableChanged();
		}
	}

	Q_INVOKABLE void applyFilter(const QString &filter, int field = -1);
	inline QString data(const uint field, const uint row, const int column = 0) const
	{
		return data(index(row, column), Qt::UserRole + field).toString();
	}

#ifndef Q_OS_ANDROID
	inline bool allUsersList() const { return m_allUsers; }
	inline void setAllUsersList(const bool all_users) { m_allUsers = all_users; emit allUsersListChanged(); }
	bool dataFromString(const QString &users_data);
#endif

	inline int rowCount(const QModelIndex & = QModelIndex{}) const override final { return count(); }
	inline int columnCount(const QModelIndex & = QModelIndex{}) const override final { return m_totalCols; }

	QVariant data(const QModelIndex &index, int role) const override final;
	bool setData(const QModelIndex &index, const QVariant &value, int role) override final;
	QVariant headerData(int section, Qt::Orientation orientation, int header_role) const override final;
	// return the roles mapping to be used by QML
	inline QHash<int, QByteArray> roleNames() const override final { return m_roleNames; }

#ifndef Q_OS_ANDROID
	void clear();
	QVariant allUsersData(const int role, const int row = -1, const int column = 0) const;
	bool setAllUsersData(const uint user_idx, const int row, const int column, int role, const QVariant &value);
	void removeUserInfo(const int row = -1);
#endif

public slots:
	void userModified(const uint user_idx, const uint field);

signals:
	void currentRowChanged();
	void currentUserIdxChanged();
	void countChanged();
	void selectedChanged();
	void visibleChanged();
	void showClientsChanged();
	void showCoachesChanged();
	void showConfirmedChanged();
	void showAvailableChanged();
#ifndef Q_OS_ANDROID
	void allUsersListChanged();
#endif

private:
	QHash<int, QByteArray> m_roleNames;
	QList<QVariantList> m_extraInfo;
	QString m_filter;
	uint m_nSelected{0}, m_totalCols{0}, m_nVisibleRows{0};
	int m_currentRow{-1}, m_fieldFilter{-1};
	bool m_selectEntireRow{false}, m_showClients{false}, m_showCoaches{false}, m_showConfirmed{false}, m_showAvailable{false};
#ifndef Q_OS_ANDROID
	bool m_allUsers{false};
	QList<QStringList> m_allUsersData;
#endif

	int _userIdx(const uint row) const;
	void changeNumberOfVisibleRows(const bool add);
	void insertUserInfo(const uint user_idx);
	void removeUserInfo(const uint user_idx);
	const bool rowVisible(const int row) const;
	void setRowVisible(const uint row, bool visible, const int column = 0);
	void changeVisibilityAsPerCategory();
	int findRow(const uint user_idx) const;
	int rowFromVisibleRow(const uint visible_row) const;
	void gatherAllUsersInfo();
};
