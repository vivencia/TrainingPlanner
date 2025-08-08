#pragma once

#include <QAbstractListModel>
#include <QKeyEvent>
#include <QQmlEngine>
#include <QQuickItem>

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
	explicit inline PagesListModel(QObject *parent = nullptr) : QAbstractListModel{parent}, m_pagesIndex{0}
	{
		m_roleNames[displayTextRole] = std::move("displayText");
		m_roleNames[pageRole] = std::move("page");
#ifdef Q_OS_ANDROID
		m_backKey = Qt::Key_Back;
#else
		m_backKey = Qt::Key_Left;
#endif
	}

	inline uint count() const { return m_pagesData.count(); }
	inline uint currentIndex() const { return m_pagesIndex; }
	inline void setCurrentIndex(const uint new_index) { if (m_pagesIndex != new_index) { m_pagesIndex = new_index; emit currentIndexChanged(); } }

	Q_INVOKABLE void insertHomePage(QQuickItem *page);
	Q_INVOKABLE void openPage(const QString &label, QQuickItem *page, const std::function<void(void)> &clean_up_func = nullptr);
	Q_INVOKABLE void closePage(QQuickItem *page);
	Q_INVOKABLE void closePage(const uint index);
	Q_INVOKABLE void openMainMenuShortCut(const uint index, const bool change_order = true);

	inline int rowCount(const QModelIndex &parent) const override final { Q_UNUSED(parent); return count(); }
	QVariant data(const QModelIndex&, int) const override final;
	inline bool setData(const QModelIndex&, const QVariant &, int) override final { return false; }
	// return the roles mapping to be used by QML
	inline QHash<int, QByteArray> roleNames() const override final { return m_roleNames; }

	Q_INVOKABLE void prevPage() { if (m_pagesIndex > 0) openMainMenuShortCut(m_pagesIndex - 1, false); }
	Q_INVOKABLE void nextPage() { if (m_pagesIndex < m_pagesData.count() - 1) openMainMenuShortCut(m_pagesIndex + 1, false); }

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
	QHash<int, QByteArray> m_roleNames;
	uint m_pagesIndex;
	int m_backKey;
};
