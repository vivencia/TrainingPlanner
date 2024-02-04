#ifndef DBEXERCISESMODEL_H
#define DBEXERCISESMODEL_H

#include <QAbstractListModel>
#include <QtQml/qqml.h>

typedef enum {
	OP_ADD = 0, OP_EDIT = 1, OP_DEL = 2, OP_READ = 3, OP_UPDATE_LIST = 4
} OP_CODES;

class DBExercisesModel : public QAbstractListModel
{

Q_OBJECT
QML_ELEMENT
Q_PROPERTY(uint count READ count NOTIFY countChanged)

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
	inline DBExercisesModel ( const DBExercisesModel& db_model ) : DBExercisesModel ()
	{
		copy ( db_model );
	}

	inline DBExercisesModel ( DBExercisesModel&& other ) : DBExercisesModel ()
	{
		db_exercisesmodel_swap ( *this, other );
	}

	inline const DBExercisesModel& operator= ( DBExercisesModel t_item )
	{
		db_exercisesmodel_swap ( *this, t_item );
		return *this;
	}

	virtual ~DBExercisesModel () override;

	Q_INVOKABLE void setEntireList( const QStringList& newlist );
	Q_INVOKABLE int count() const { return m_data.count(); }
	Q_INVOKABLE void updateList (const QStringList& list, const int row) { m_data.replace(row, list); }
	Q_INVOKABLE void removeFromList (const int row) { m_data.remove(row); emit countChanged(); }
	Q_INVOKABLE void appendList(const QStringList& list) { m_data.append(list); emit countChanged(); }
	Q_INVOKABLE const QString& data(const uint row, int role) const;
	Q_INVOKABLE bool setData(const uint row, const QString& value, int role);

public:
	// QAbstractItemModel interface
	inline virtual int rowCount(const QModelIndex &parent) const override { Q_UNUSED(parent); return m_data.count(); }
	inline virtual QVariant data(const QModelIndex &index, int role) const override { return QVariant(data(index.row(), role)); }
	inline virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override { return setData(index.row(), value.toString(), role); }

signals:
	void countChanged();

protected:
	// return the roles mapping to be used by QML
	inline virtual QHash<int, QByteArray> roleNames() const override { return m_roleNames; }

private:
	QList<QStringList> m_data;
	QHash<int, QByteArray> m_roleNames;

	friend void db_exercisesmodel_swap ( DBExercisesModel& model1, DBExercisesModel& model2 );
	void copy ( const DBExercisesModel& src_item );
};

#endif // DBEXERCISESMODEL_H
