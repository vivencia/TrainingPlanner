#pragma once

#include <QAbstractListModel>
#include <QKeyEvent>
#include <QQmlEngine>
#include <QQuickItem>

#ifndef Q_OS_ANDROID
#include "tpsettings.h"
#endif

class PagesListModel : public QAbstractListModel
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(uint count READ count NOTIFY countChanged)
Q_PROPERTY(uint currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged FINAL)

enum RoleNames {
	displayTextRole = Qt::UserRole,
	pageRole = Qt::UserRole+1
};

public:
	explicit PagesListModel(QObject *parent = nullptr);
	#ifndef Q_OS_ANDROID
	void userSwitchingActions();
	#endif

	inline uint count() const { return m_pagesData.count(); }
	inline uint currentIndex() const { return m_pagesIndex; }
	inline void setCurrentIndex(const uint new_index) { if (m_pagesIndex != new_index) { m_pagesIndex = new_index; emit currentIndexChanged(); } }

	void insertHomePage(QQuickItem *page);
	void openPage(QQuickItem *page, QString &&label = QString{}, const std::function<void(void)> &clean_up_func = nullptr);
	void closePage(QQuickItem *page);
	Q_INVOKABLE void closePage(const uint index);
	Q_INVOKABLE void openMainMenuShortCut(const uint index, const bool change_order = true);
	void changeLabel(QQuickItem *page, QString &&new_label);

	Q_INVOKABLE inline void prevPage() { if (m_pagesIndex > 0) openMainMenuShortCut(m_pagesIndex - 1, false); }
	Q_INVOKABLE inline void nextPage() { if (m_pagesIndex < m_pagesData.count() - 1) openMainMenuShortCut(m_pagesIndex + 1, false); }
	Q_INVOKABLE void popupOpened(QObject* popup);
	Q_INVOKABLE void popupClosed(QObject* popup);
	Q_INVOKABLE void raisePopup(QObject* popup);
	Q_INVOKABLE bool isPopupAboveAllOthers(QObject* popup) const;

	inline int rowCount(const QModelIndex &parent) const override final { Q_UNUSED(parent); return count(); }
	QVariant data(const QModelIndex&, int) const override final;
	inline bool setData(const QModelIndex&, const QVariant &, int) override final { return false; }
	// return the roles mapping to be used by QML
	inline QHash<int, QByteArray> roleNames() const override final { return m_roleNames; }

signals:
	void countChanged();
	void currentIndexChanged();

protected:
	bool eventFilter(QObject *obj, QEvent *event) override;

private:
	struct pageInfo {
		QString displayText;
		QQuickItem *page;
		std::function<void(void)> cleanUpFunc;
		explicit inline pageInfo() : page{nullptr} {}
	};

	QList<pageInfo*> m_pagesData;
	QList<QObject*> m_popupsOpen;
	QHash<int, QByteArray> m_roleNames;
	uint m_pagesIndex;
	int m_backKey;

	#ifndef Q_OS_ANDROID
	static QHash<QString,PagesListModel*> app_Pages_list_models;
	#else
	static PagesListModel *app_Pages_list_model;
	#endif
	friend PagesListModel *appPagesListModel();
};

#ifndef Q_OS_ANDROID
inline PagesListModel *appPagesListModel() { return PagesListModel::app_Pages_list_models.value(appSettings()->currentUser()); }
#else
inline PagesListModel* appPagesListModel() { return PagesListModel::app_Pages_list_model; }
#endif
