#ifndef TPLISTMODEL_H
#define TPLISTMODEL_H

#include <QAbstractListModel>
#include <QQmlEngine>

typedef enum {
	OP_ADD = 0, OP_EDIT = 1, OP_DEL = 2, OP_READ = 3, OP_UPDATE_LIST = 4
} OP_CODES;

class TPListModel : public QAbstractListModel
{

Q_OBJECT
QML_ELEMENT
Q_PROPERTY(uint count READ count NOTIFY countChanged)
Q_PROPERTY(int currentRow READ currentRow WRITE setCurrentRow NOTIFY currentRowChanged)

public:

	explicit TPListModel(QObject *parent = 0) : QAbstractListModel(parent), m_currentRow(-1) {}
	inline TPListModel ( const TPListModel& db_model ) : TPListModel ()
	{
		copy ( db_model );
	}

	inline TPListModel ( TPListModel&& other ) : TPListModel ()
	{
		tp_listmodel_swap ( *this, other );
	}

	inline const TPListModel& operator= ( TPListModel t_item )
	{
		copy ( t_item );
		return *this;
	}

	virtual ~TPListModel () override;

	Q_INVOKABLE void setEntireList( const QStringList& newlist );
	Q_INVOKABLE void updateList (const QStringList& list, const int row) { m_modeldata.replace(row, list); }
	Q_INVOKABLE void removeFromList (const int row) { m_modeldata.remove(row); emit countChanged(); }
	Q_INVOKABLE void appendList(const QStringList& list) { m_modeldata.append(list); emit countChanged(); }

	Q_INVOKABLE inline uint count() const { return m_modeldata.count(); }
	Q_INVOKABLE inline int currentRow() const { return m_currentRow; }
	Q_INVOKABLE void setCurrentRow(const int row)
	{
		if (row >= -1 && row < m_modeldata.count())
		{
			m_currentRow = row;
			emit currentRowChanged();
		}
	}

	Q_INVOKABLE virtual const QString& data(const uint row, int role) const;
	Q_INVOKABLE virtual bool setData(const uint row, const QString& value, int role);

public:
	// QAbstractItemModel interface
	inline virtual int rowCount(const QModelIndex &parent) const override { Q_UNUSED(parent); return count(); }
	inline virtual QVariant data(const QModelIndex &index, int role) const override { return QVariant(data(index.row(), role)); }
	inline virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override { return setData(index.row(), value.toString(), role); }

signals:
	void countChanged();
	void currentRowChanged();

protected:
	// return the roles mapping to be used by QML
	inline virtual QHash<int, QByteArray> roleNames() const override { return m_roleNames; }

	QList<QStringList> m_modeldata;
	QHash<int, QByteArray> m_roleNames;
	int m_currentRow;

	friend void tp_listmodel_swap ( TPListModel& model1, TPListModel& model2 );
	void copy ( const TPListModel& src_item );
};

#endif // TPLISTMODEL_H
