#ifndef DBEXERCISESMODEL_H
#define DBEXERCISESMODEL_H

#include <QAbstractListModel>
#include <QtCore>
#include <QtGui>

typedef enum {
	OP_ADD = 0, OP_EDIT = 1, OP_DEL = 2, OP_READ
} OP_CODES;

class DBExercisesModel : public QAbstractListModel
{

Q_OBJECT

public:
	// Define the role names to be used
	enum RoleNames {
		exerciseIdRole = Qt::UserRole,
		mainNameRole = Qt::UserRole+2,
		subNameRole = Qt::UserRole+3,
		muscularGroupRole = Qt::UserRole+4,
		nSetsRole = Qt::UserRole+5,
		nRepsRole = Qt::UserRole+6,
		nWeightRole = Qt::UserRole+7,
		uWeightRole = Qt::UserRole+8,
		mediaPathRole = Qt::UserRole+9,
		actualIndexRole = Qt::UserRole+10
	};

	explicit DBExercisesModel(QObject *parent = 0);
	~DBExercisesModel() {}
	void setEntireList( const QStringList& newlist );
	inline void updateList (const QStringList& list, const int row) { m_data.replace(row, list); }
	inline void removeFromList (const int row) { m_data.remove(row); }
	inline void appendList(const QStringList& list) { m_data.append(list); }
	const QString& data(const uint row, int role) const;
	bool setData(const uint row, const QString& value, int role);

public:
	// QAbstractItemModel interface
	inline virtual int rowCount(const QModelIndex &parent) const override { Q_UNUSED(parent); return m_data.count(); }
	inline virtual QVariant data(const QModelIndex &index, int role) const override { return QVariant(data(index.row(), role)); }
	inline virtual bool setData(const QModelIndex &index, const QVariant &value, int role) { return setData(index.row, value.toString(), role); }

protected:
	// return the roles mapping to be used by QML
	inline virtual QHash<int, QByteArray> roleNames() const override { return m_roleNames; }

private:
	QList<QStringList> m_data;
	QHash<int, QByteArray> m_roleNames;
};

#endif // DBEXERCISESMODEL_H
