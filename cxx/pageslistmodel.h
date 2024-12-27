#ifndef PAGESLISTMODEL_H
#define PAGESLISTMODEL_H

#include <QAbstractListModel>
#include <QQmlEngine>

class QQuickItem;

class PagesListModel : public QAbstractListModel
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(uint count READ count NOTIFY countChanged)

enum RoleNames {
	displayTextRole = Qt::UserRole,
	pageRole = Qt::UserRole+1
};

public:
	explicit inline PagesListModel(QObject* parent = nullptr) : QAbstractListModel{parent}
	{
		m_roleNames[displayTextRole] = std::move("displayText");
		m_roleNames[pageRole] = std::move("page");
	}
	~PagesListModel();

	inline uint count() const { return m_modeldata.count(); }

	void addMainMenuShortCut(const QString& label, QQuickItem* page);
	void removeMainMenuShortCut(QQuickItem* page);
	Q_INVOKABLE void removeMainMenuShortCut(const uint index);
	Q_INVOKABLE void openMainMenuShortCut(const uint index) const;

	inline int rowCount(const QModelIndex& parent) const override final { Q_UNUSED(parent); return count(); }
	QVariant data(const QModelIndex&, int) const override final;
	inline bool setData(const QModelIndex&, const QVariant &, int) override final { return false; }
	// return the roles mapping to be used by QML
	inline QHash<int, QByteArray> roleNames() const override final { return m_roleNames; }

signals:
	void countChanged();

private:
	struct pageInfo {
		QString displayText;
		QQuickItem* page;

		explicit inline pageInfo() : page{nullptr} {}
	};

	QList<pageInfo*> m_modeldata;
	QHash<int, QByteArray> m_roleNames;
};

#endif // PAGESLISTMODEL_H
