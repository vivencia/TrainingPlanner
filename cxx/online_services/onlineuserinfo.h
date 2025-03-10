#ifndef ONLINEUSERINFO_H
#define ONLINEUSERINFO_H

#include <QAbstractListModel>
#include <QObject>
#include <QQmlEngine>

#define USER_EXTRA_NAME 0
#define USER_EXTRA_SELECTED 1
#define USER_EXTRA_SOURCE 2

#define ASSOCIATED_FILES_PROFILE 1
#define ASSOCIATED_FILES_AVATAR 2
#define ASSOCIATED_FILES_RESUME 3

using namespace Qt::Literals::StringLiterals;

class OnlineUserInfo : public QAbstractListModel
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(int currentRow READ currentRow WRITE setCurrentRow NOTIFY currentRowChanged FINAL)
Q_PROPERTY(uint count READ count NOTIFY countChanged)
Q_PROPERTY(bool anySelected READ anySelected NOTIFY selectedChanged FINAL)
Q_PROPERTY(bool allSelected READ allSelected NOTIFY selectedChanged FINAL)
Q_PROPERTY(bool noneSelected READ noneSelected NOTIFY selectedChanged FINAL)

public:
	explicit OnlineUserInfo(QObject *parent = nullptr);

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
		emit dataChanged(QModelIndex{}, QModelIndex{}, QList<int>{} << Qt::UserRole+user_field);
	}

	inline bool isSelected(const uint row) const
	{
		Q_ASSERT_X(row < count(), "OnlineUserInfo::setSelected", "row out of range");
		return m_extraInfo.at(row).at(USER_EXTRA_SELECTED) == "1"_L1;
	}
	void setSelected(const uint row, bool selected);
	inline uint nSelected() const { return m_nselected; }
	inline bool allSelected() const { return m_nselected == count(); }
	inline bool anySelected() const { return m_nselected > 0; }
	inline bool noneSelected() const { return m_nselected == 0; }

	inline const QString &sourceFile(const uint row) const
	{
		Q_ASSERT_X(row < count(), "OnlineUserInfo::sourceFile", "row out of range");
		return m_extraInfo.at(row).at(USER_EXTRA_SOURCE);
	}
	void setSourceFile(const uint row, const QString &source_file);
	/**
	 * @brief setProfile/setAvatar/setResume
	 * @param row
	 * @param filename: just filename, without path. m_sourceFile's path will be prepended to it along with userId
	 */
	void setProfile(const uint row, const QString &filename);
	void setAvatar(const uint row, const QString &filename);
	void setResume(const uint row, const QString &filename);
	inline const QString &associatedFile(const uint row, const uint af_index) const { return m_extraInfo.at(row).at(USER_EXTRA_SOURCE+af_index); }

	bool dataFromFileSource(const QString &filename);
	bool dataFromString(const QString &user_data);
	void removeUserInfo(const uint row, const bool remove_source);
	//Remove all items from m_modeldata that are not in user_list. Use field to look for matches
	bool sanitize(const QStringList &user_list, const uint field);
	void clear();

	Q_INVOKABLE inline bool isUserDefault(const uint row) const
	{
		Q_ASSERT_X(row < count(), "OnlineUserInfo::data", "row out of range");
		return row == 0 ? m_extraInfo.at(0).at(USER_EXTRA_NAME).at(0) == '*' : false;
	}
	void makeUserDefault(const uint row);

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

	inline int rowCount(const QModelIndex& parent) const override final { Q_UNUSED(parent); return count(); }
	QVariant data(const QModelIndex &index, int role) const override final;
	bool setData(const QModelIndex &index, const QVariant &value, int role) override final;
	// return the roles mapping to be used by QML
	inline QHash<int, QByteArray> roleNames() const override final { return m_roleNames; }

signals:
	void countChanged();
	void selectedChanged();
	void currentRowChanged();

private:
	QHash<int, QByteArray> m_roleNames;
	QList<QStringList> m_modeldata;
	QList<QStringList> m_extraInfo;
	QString m_sourcePath;
	uint m_nselected;
	int m_currentRow;
};

#endif // ONLINEUSERINFO_H
