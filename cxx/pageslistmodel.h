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
QML_VALUE_TYPE(PagesListModel)
QML_UNCREATABLE("")

Q_PROPERTY(uint count READ count NOTIFY countChanged)
Q_PROPERTY(uint currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged FINAL)

public:
	explicit PagesListModel(QObject *parent = nullptr);
	#ifndef Q_OS_ANDROID
	void userSwitchingActions();
	#endif

	inline uint count() const { return m_pagesData.count(); }
	inline uint currentIndex() const { return m_pagesIndex; }
	void setCurrentIndex(const uint new_index);

	void removeEventFilter();
	void reinstallEventFilter();
	void insertHomePage(QQuickItem *page);
	void openPage(QQuickItem *page, QString &&label = QString{}, const std::function<void(void)> &clean_up_func = nullptr);
	void closePage(QQuickItem *page);
	Q_INVOKABLE void closePage(const uint index);
	Q_INVOKABLE void openMainMenuShortCut(const uint index, const bool change_order = true);
	void changeLabel(QQuickItem *page, QString &&new_label);

	Q_INVOKABLE QQuickItem *homePage() const { return m_pagesData.at(0)->page; }
	Q_INVOKABLE void openMainMenu();
	Q_INVOKABLE inline int backKey() const { return m_backKey; }
	Q_INVOKABLE inline void goHome() { openMainMenuShortCut(0); }
	Q_INVOKABLE inline void prevPage() { if (m_pagesIndex > 0) openMainMenuShortCut(m_pagesIndex - 1, false); }
	Q_INVOKABLE inline void nextPage() { if (m_pagesIndex < m_pagesData.count() - 1) openMainMenuShortCut(m_pagesIndex + 1, false); }

	Q_INVOKABLE void openPopup(QObject *popup, QQuickItem *parentPage, const int position = Qt::AlignCenter, QQuickItem *widget = nullptr);
	Q_INVOKABLE void raisePopup(QObject *popup);
	Q_INVOKABLE void hidePopup(QObject *popup);
	Q_INVOKABLE bool isPopupAboveAllOthers(QObject *popup) const;

	inline int rowCount(const QModelIndex &parent) const override final { Q_UNUSED(parent); return count(); }
	QVariant data(const QModelIndex&, int) const override final;
	inline bool setData(const QModelIndex&, const QVariant &, int) override final { return false; }
	// return the roles mapping to be used by QML
	inline QHash<int, QByteArray> roleNames() const override final { return m_roleNames; }

public slots:
	void popupClosed(QObject *popup);

signals:
	void countChanged();
	void currentIndexChanged();
	void appSettingsButtonEnabled(const bool enabled);
	void userSettingsButtonEnabled(const bool enabled);

protected:
	bool eventFilter(QObject *obj, QEvent *event) override;

private:
	struct pageInfo {
		QString displayText;
		QQuickItem *page{nullptr};
		std::function<void(void)> cleanUpFunc{nullptr};
		QList<QObject*> tpPopups;
	};

	QList<pageInfo*> m_pagesData;
	QList<uint> m_pagesMesoIdx;
	QHash<int, QByteArray> m_roleNames;
	uint m_pagesIndex{0};
	int m_backKey;

	pageInfo *getPageInfo(QQuickItem *qml_page) const;
	pageInfo *getPageInfo(QObject *tp_popup) const;
	void openQMLPage(const uint index);
	void activateQmlPage(const uint index);
	void deActivateQmlPage(const uint index);
	void changePopupStackOrder(QObject *popup, pageInfo *page_info);

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
