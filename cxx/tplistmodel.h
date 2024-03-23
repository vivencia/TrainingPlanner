#ifndef TPLISTMODEL_H
#define TPLISTMODEL_H

#include <QAbstractListModel>
#include <QQmlEngine>
#include <QDate>

#ifdef DEBUG
#define MSG_OUT(message) qDebug() << message;
#else
#define MSG_OUT(message)
#endif

static const QLatin1Char record_separator(29);
static const QLatin1Char record_separator2(30);
static const QLatin1Char subrecord_separator(31);

class TPListModel : public QAbstractListModel
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(uint count READ count NOTIFY countChanged)
Q_PROPERTY(int currentRow READ currentRow WRITE setCurrentRow NOTIFY currentRowChanged)
Q_PROPERTY(bool modified READ modified WRITE setModified NOTIFY modifiedChanged)

public:

	explicit TPListModel(QObject *parent = nullptr) : QAbstractListModel(parent),
		m_currentRow(-1), m_bFilterApplied(false),  m_bReady(false), filterSearch_Field1(0), filterSearch_Field2(0) {}
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

	bool modified() const { return m_bModified; }
	void setModified(const bool bModified)
	{
		if (m_bModified != bModified)
		{
			m_bModified = bModified;
			emit modifiedChanged();
		}
	}

	Q_INVOKABLE void updateList (const QStringList& list, const int row);
	Q_INVOKABLE void removeFromList (const int row);
	Q_INVOKABLE void appendList(const QStringList& list);
	Q_INVOKABLE void clear();

	Q_INVOKABLE inline uint count() const { return m_indexProxy.count(); }
	Q_INVOKABLE inline int currentRow() const { return m_currentRow; }
	Q_INVOKABLE void setCurrentRow(const int row);
	Q_INVOKABLE void moveRow(const uint from, const uint to);

	Q_INVOKABLE void setFilter(const QString& filter);
	Q_INVOKABLE void makeFilterString(const QString& text);
	Q_INVOKABLE QString getFilter() const { return m_filterString; }

	inline const QString& getFast(const uint row, const uint field) const
	{
		return m_modeldata.at(row).at(field);
	}

	inline const QDate getDateFast(const uint row, const uint field) const
	{
		return QDate::fromJulianDay(m_modeldata.at(row).at(field).toLongLong());
	}

	Q_INVOKABLE const QString get(const uint row, const uint field) const
	{
		if (row >= 0 && row < m_indexProxy.count())
			return static_cast<QString>(m_modeldata.at(m_indexProxy.at(row)).at(field));
		else
			return QString();
	}

	Q_INVOKABLE int getInt(const uint row, const uint field) const
	{
		if (row >= 0 && row < m_indexProxy.count())
			return static_cast<QString>(m_modeldata.at(m_indexProxy.at(row)).at(field)).toInt();
		else
			return -1;
	}

	Q_INVOKABLE float getFloat(const uint row, const uint field) const
	{
		if (row >= 0 && row < m_indexProxy.count())
			return static_cast<QString>(m_modeldata.at(m_indexProxy.at(row)).at(field)).toFloat();
		else
			return -1.0;
	}

	Q_INVOKABLE QDate getDate(const uint row, const uint field) const
	{
		if (row >= 0 && row < m_indexProxy.count())
			return QDate::fromJulianDay(static_cast<QString>(m_modeldata.at(m_indexProxy.at(row)).at(field)).toLongLong());
		else
			return QDate();
	}

	inline const QStringList& getRow_const(const uint row) const { return m_modeldata.at(m_indexProxy.at(row)); }
	inline QStringList& getRow(const uint row) { return m_modeldata[m_indexProxy.at(row)]; }

	inline const bool isReady() const { return m_bReady; }
	inline void setReady(const bool bready) { m_bReady = bready; }

	// QAbstractItemModel interface
	inline virtual int columnCount(const QModelIndex &parent) const override { Q_UNUSED(parent); return 1; }
	inline virtual int rowCount(const QModelIndex &parent) const override { Q_UNUSED(parent); return count(); }
	inline virtual QVariant data(const QModelIndex &, int) const override { return QVariant(); }
	inline virtual bool setData(const QModelIndex &, const QVariant &, int) override { return false; }

signals:
	void countChanged();
	void currentRowChanged();
	void modifiedChanged();

protected:
	// return the roles mapping to be used by QML
	inline virtual QHash<int, QByteArray> roleNames() const override { return m_roleNames; }

	QList<QStringList> m_modeldata;
	QList<uint> m_indexProxy;
	QHash<int, QByteArray> m_roleNames;
	int m_currentRow;
	bool m_bFilterApplied, m_bReady, m_bModified;
	uint filterSearch_Field1;
	uint filterSearch_Field2;
	QString m_filterString;

	friend void tp_listmodel_swap ( TPListModel& model1, TPListModel& model2 );
	void copy ( const TPListModel& src_item );
};

#endif // TPLISTMODEL_H
