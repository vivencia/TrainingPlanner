#pragma once

#include "tplistmodel.h"

class UserInfoListModel : public TPListModel
{

Q_OBJECT
QML_ELEMENT
QML_VALUE_TYPE(UserInfoModel)

Q_PROPERTY(bool showClients READ showClients WRITE setShowClients NOTIFY showClientsChanged FINAL)
Q_PROPERTY(bool showCoaches READ showCoaches WRITE setShowCoaches NOTIFY showCoachesChanged FINAL)
Q_PROPERTY(bool showConfirmed READ showConfirmed WRITE setShowConfirmed NOTIFY showConfirmedChanged FINAL)
Q_PROPERTY(bool showAvailable READ showAvailable WRITE setShowAvailable NOTIFY showAvailableChanged FINAL)
Q_PROPERTY(int selectedUser READ selectedUser WRITE setSelectedUser NOTIFY selectedUserChanged FINAL)

public:
	explicit UserInfoListModel(QObject *parent = nullptr);

	/**
	 * If other_data is not empty(and it should not, if possible), the QStringList part must be compatible with UsersManager field setup
	 * The reason for this flexible data source model is to be able to deal with online users list,
	 * all local users(linux only, for server management/develoment), etc.
	 */
	void setModelData(QList<QStringList> &&other_data)
	{
		emit layoutAboutToBeChanged();
		m_otherModelData = std::forward<QList<QStringList>>(other_data);
		m_modelData = &m_otherModelData;
		if (count() > 0) {
			clear();
			syncMetadata(m_modelData->count(), true);
		}
		emit layoutChanged();
	}

	inline const bool showClients() const { return m_showClients; }
	inline void setShowClients(const bool show)
	{
		if (show != m_showClients) {
			m_showClients = show;
			emit showClientsChanged();
			applyFilters();
		}
	}
	inline const bool showCoaches() const { return m_showCoaches; }
	inline void setShowCoaches(const bool show)
	{
		if (show != m_showCoaches) {
			m_showCoaches = show;
			emit showCoachesChanged();
			applyFilters();
		}
	}
	inline const bool showConfirmed() const { return m_showConfirmed; }
	inline void setShowConfirmed(const bool show)
	{
		if (show != m_showConfirmed) {
			m_showConfirmed = show;
			emit showConfirmedChanged();
			applyFilters();
		}
	}
	inline const bool showAvailable() const { return m_showAvailable; }
	inline void setShowAvailable(const bool show)
	{
		if (show != m_showAvailable) {
			m_showAvailable = show;
			emit showClientsChanged();
			applyFilters();
		}
	}
	inline const int selectedUser() const { return realRow(currentRow()); }
	inline void setSelectedUser(const int user_idx) { setCurrentRowViaRealRow(user_idx); }

	QVariant headerData(int section, Qt::Orientation orientation, int header_role) const override final;
	const QString &dataValue(const uint real_row, const uint column) const override final;
	QVariant data(const uint role, const uint row, const int column = -1) const override final;
	bool setData(const uint role, const uint row, const QVariant &value, const int column = -1) override final;

public slots:
	void userModified(const uint user_idx, const uint field);

signals:
	void showClientsChanged();
	void showCoachesChanged();
	void showConfirmedChanged();
	void showAvailableChanged();
	void selectedUserChanged();
#ifndef Q_OS_ANDROID
	void allUsersListChanged();
#endif

private:
	bool m_useOtherModel{false}, m_showClients{false}, m_showCoaches{false}, m_showConfirmed{false}, m_showAvailable{false};
	QList<QStringList> *m_modelData{nullptr};
	QList<QStringList> m_otherModelData;
};
