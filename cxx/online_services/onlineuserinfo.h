#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

#define USER_EXTRA_NAME 0
#define USER_EXTRA_SELECTED 1
#define USER_EXTRA_SOURCE 2
#define USER_EXTRA_ISCOACH 3
#define USER_EXTRA_VISIBLE 4

class OnlineUserInfo : public QAbstractListModel
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(int currentRow READ currentRow WRITE setCurrentRow NOTIFY currentRowChanged FINAL)
Q_PROPERTY(uint count READ count NOTIFY countChanged)
Q_PROPERTY(bool anySelected READ anySelected NOTIFY selectedChanged FINAL)
Q_PROPERTY(bool allSelected READ allSelected NOTIFY selectedChanged FINAL)
Q_PROPERTY(bool noneSelected READ noneSelected NOTIFY selectedChanged FINAL)

#ifndef Q_OS_ANDROID
Q_PROPERTY(QString userId READ userId CONSTANT FINAL)
#endif

public:
	explicit OnlineUserInfo(QObject *parent);

#ifndef Q_OS_ANDROID
	inline QString userId() const
	{
		Q_ASSERT_X(m_currentRow >= 0 && m_currentRow < m_modeldata.count(), "OnlineUserInfo::userId", "row out of range");
		return m_modeldata.at(m_currentRow).at(0);
	}
#endif

	inline uint count() const { return m_modeldata.count(); }
	inline const QString &sourcePath()const { return m_sourcePath; }
	inline const QString &data(const uint row, const uint user_field) const
	{
		Q_ASSERT_X(row < count(), "OnlineUserInfo::data", "row out of range");
		return m_modeldata.at(row).at(user_field);
	}
	inline void setData(const uint row, const uint user_field, const QString &value)
	{
		Q_ASSERT_X(row < count(), "OnlineUserInfo::setData", "row out of range");
		m_modeldata[row][user_field] = value;
	}
	inline void setData(const uint row, const uint user_field, QString &&value)
	{
		Q_ASSERT_X(row < count(), "OnlineUserInfo::setData", "row out of range");
		m_modeldata[row][user_field] = std::move(value);
		emit dataChanged(index(row), index(row), QList<int>{} << Qt::UserRole + user_field);
	}

	bool isSelected(const uint row, const int column = 0) const;
	Q_INVOKABLE void setSelected(const uint row, const bool selected, const int column = 0);
	inline uint nSelected() const { return m_nselected; }
	inline bool allSelected() const { return m_nselected == count(); }
	inline bool anySelected() const { return m_nselected > 0; }
	inline bool noneSelected() const { return m_nselected == 0; }
	inline bool selectEntireRow() const { return m_selectEntireRow; }
	inline void setSelectEntireRow(const bool full_sel) { m_selectEntireRow = full_sel; }

	inline const QString &extraName(const uint row) const
	{
		Q_ASSERT_X(row < count(), "OnlineUserInfo::extraName", "row out of range");
		return m_extraInfo.at(row).at(USER_EXTRA_NAME);
	}
	void setExtraName(const uint row, const QString &extra_name);

	inline const QString &sourceFile(const uint row) const
	{
		Q_ASSERT_X(row < count(), "OnlineUserInfo::sourceFile", "row out of range");
		return m_extraInfo.at(row).at(USER_EXTRA_SOURCE);
	}
	void setSourceFile(const uint row, const QString &source_file);

	inline const bool isCoach(const uint row) const
	{
		Q_ASSERT_X(row < count(), "OnlineUserInfo::isCoach", "row out of range");
		return m_extraInfo.at(row).at(USER_EXTRA_ISCOACH).at(0) == '1';
	}
	void setIsCoach(const uint row, bool coach);

	inline const bool visible(const uint row) const
	{
		Q_ASSERT_X(row < count(), "OnlineUserInfo::visible", "row out of range");
		return m_extraInfo.at(row).at(USER_EXTRA_VISIBLE).at(0) == '1';
	}
	void setVisible(const uint row, bool visible, const int column = 0);

	bool dataFromFileSource(const QString &filename);
	bool dataFromString(const QString &user_data);
	void dataFromUserModel(const uint user_idx);
	void dataFromOnlineUserInfo(const OnlineUserInfo *other_userinfo, const int other_row = -1);

	void removeUserInfo(const uint row, const bool remove_source);
	//Remove all items from m_modeldata that are not in user_list. Use field to look for matches
	bool sanitize(const QStringList &user_list, const uint field);
	void clear();

	Q_INVOKABLE inline bool isUserDefault(const uint row) const
	{
		return row == 0 ? m_extraInfo.isEmpty() ? false : m_extraInfo.at(0).at(USER_EXTRA_NAME).at(0) == '*' : false;
	}
	void makeUserDefault(const uint row);

	int getRowFromUserIdx(const uint user_idx) const;
	Q_INVOKABLE int getUserIdx(int row = -1, const bool exact_match = true) const;

	inline QStringList &modeldata(const uint row) { return m_modeldata[row]; } //used to move data into appUserModel()::m_modeldata
	inline const QStringList &modeldata(const uint row) const { return m_modeldata.at(row); } //used to copy data into appUserModel()::m_modeldata
	inline void setModelData(QStringList &&model) { m_modeldata.append(std::move(model)); setCurrentRow(count()-1); }
	inline void setModelData(const QStringList &model) { m_modeldata.append(model); setCurrentRow(count()-1); }

	inline int currentRow() const { return m_currentRow; }
	inline void setCurrentRow(const int new_row)
	{
		if (m_currentRow != new_row)
		{
			m_currentRow = new_row;
			emit currentRowChanged();
		}
	}

	bool containsUser(const QString &userid) const;

	inline int rowCount(const QModelIndex & = QModelIndex{}) const override final { return count(); }
	inline int columnCount(const QModelIndex & = QModelIndex{}) const override final { return m_totalCols; }

	QString fieldValueFromAnotherFieldValue(const uint target_field, const uint needle_field, const QString &needle) const;
	inline const QList<QStringList> &userInfo() const { return m_modeldata; }

	QVariant data(const QModelIndex &index, int role) const override final;
	bool setData(const QModelIndex &index, const QVariant &value, int role) override final;
	QVariant headerData(int section, Qt::Orientation orientation, int header_role) const override final;
	// return the roles mapping to be used by QML
	inline QHash<int, QByteArray> roleNames() const override final { return m_roleNames; }

signals:
	void currentRowChanged();
	void countChanged();
	void selectedChanged();
	void visibleChanged();

private:
	QHash<int, QByteArray> m_roleNames;
	QList<QStringList> m_modeldata;
	QList<QStringList> m_extraInfo;
	QString m_sourcePath;
	uint m_nselected, m_totalCols;
	int m_currentRow;
	bool m_selectEntireRow;

	void setupExtraInfo(const uint row, const QString &source_file = QString{});
};
