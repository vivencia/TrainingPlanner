#ifndef TPLISTMODEL_H
#define TPLISTMODEL_H

#include <QAbstractListModel>
#include <QQmlEngine>

#ifdef DEBUG
#define MSG_OUT(message) qDebug() << message;
#else
#define MSG_OUT(message)
#endif

class TPListModel : public QAbstractListModel
{

Q_OBJECT
QML_ELEMENT
Q_PROPERTY(uint count READ count NOTIFY countChanged)
Q_PROPERTY(int currentRow READ currentRow WRITE setCurrentRow NOTIFY currentRowChanged)

public:

	explicit TPListModel(QObject *parent = 0) : QAbstractListModel(parent),
		m_currentRow(-1), bFilterApplied(false), filterSearch_Field1(0), filterSearch_Field2(0) {}
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
	Q_INVOKABLE void updateList (const QStringList& list, const int row);
	Q_INVOKABLE void removeFromList (const int row);
	Q_INVOKABLE void appendList(const QStringList& list);
	Q_INVOKABLE void clear();

	Q_INVOKABLE inline uint count() const { return m_indexProxy.count(); }
	Q_INVOKABLE inline int currentRow() const { return m_currentRow; }
	Q_INVOKABLE void setCurrentRow(const int row);

	Q_INVOKABLE void setFilter(const QString& filter);
	Q_INVOKABLE const QString get(const uint row, const uint field) const { return m_modeldata.at(m_indexProxy.at(row)).at(field); }
	Q_INVOKABLE const int getInt(const uint row, const uint field) const { return m_modeldata.at(m_indexProxy.at(row)).at(field).toInt(); }
	Q_INVOKABLE const float getFloat(const uint row, const uint field) const { return m_modeldata.at(m_indexProxy.at(row)).at(field).toFloat(); }
	Q_INVOKABLE const QStringList getRow(const uint row) const { return m_modeldata.at(m_indexProxy.at(row)); }

public:
	// QAbstractItemModel interface
	inline virtual int columnCount(const QModelIndex &parent) const override { Q_UNUSED(parent); return 1; }
	inline virtual int rowCount(const QModelIndex &parent) const override { Q_UNUSED(parent); return count(); }
	virtual QVariant data(const QModelIndex &index, int role) const override;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override;

signals:
	void countChanged();
	void currentRowChanged();

protected:
	// return the roles mapping to be used by QML
	inline virtual QHash<int, QByteArray> roleNames() const override { return m_roleNames; }

	QList<QStringList> m_modeldata;
	QList<uint> m_indexProxy;
	QHash<int, QByteArray> m_roleNames;
	int m_currentRow;
	bool bFilterApplied;
	uint filterSearch_Field1;
	uint filterSearch_Field2;

	friend void tp_listmodel_swap ( TPListModel& model1, TPListModel& model2 );
	void copy ( const TPListModel& src_item );
};

#endif // TPLISTMODEL_H
