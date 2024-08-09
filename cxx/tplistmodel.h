#ifndef TPLISTMODEL_H
#define TPLISTMODEL_H

#include <QAbstractListModel>
#include <QQmlEngine>
#include <QDate>
#include <QFile>

#ifdef DEBUG
#define MSG_OUT(message) qDebug() << message;
#else
#define MSG_OUT(message)
#endif

static const QLatin1Char record_separator(29);
static const QLatin1Char record_separator2(30);
static const QLatin1Char subrecord_separator(31);

static const uint APP_TABLES_NUMBER(6);
static const uint EXERCISES_TABLE_ID(0x0001);
static const uint MESOCYCLES_TABLE_ID(0x0002);
static const uint MESOSPLIT_TABLE_ID(0x0003);
static const uint MESOCALENDAR_TABLE_ID(0x0004);
static const uint TRAININGDAY_TABLE_ID(0x0005);
static const uint USER_TABLE_ID(0x0006);

static const QString DBExercisesObjectName(QStringLiteral("Exercises"));
static const QString DBMesocyclesObjectName(QStringLiteral("Mesocycles"));
static const QString DBMesoSplitObjectName(QStringLiteral("MesocyclesSplits"));
static const QString DBMesoCalendarObjectName(QStringLiteral("MesoCalendar"));
static const QString DBTrainingDayObjectName(QStringLiteral("TrainingDay"));
static const QString DBUserObjectName(QStringLiteral("UserProfile"));

class TPListModel : public QAbstractListModel
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(uint count READ count NOTIFY countChanged)
Q_PROPERTY(int currentRow READ currentRow WRITE setCurrentRow NOTIFY currentRowChanged)
Q_PROPERTY(bool modified READ modified WRITE setModified NOTIFY modifiedChanged)

public:

	explicit TPListModel(QObject *parent = nullptr);
	inline TPListModel(const TPListModel& db_model) : TPListModel ()
	{
		copy (db_model);
	}

	inline TPListModel(TPListModel&& other) : TPListModel ()
	{
		tp_listmodel_swap (*this, other);
	}

	inline const TPListModel& operator=(TPListModel t_item)
	{
		copy (t_item);
		return *this;
	}

	virtual ~TPListModel() override;

	inline int tableID() const { return m_tableId; }
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
	Q_INVOKABLE virtual void clear();

	Q_INVOKABLE inline uint count() const { return m_indexProxy.count(); }
	Q_INVOKABLE inline int currentRow() const { return m_currentRow; }
	Q_INVOKABLE void setCurrentRow(const int row);
	Q_INVOKABLE void moveRow(const uint from, const uint to);

	Q_INVOKABLE void setFilter(const QString& filter, const bool resetSelection);
	Q_INVOKABLE void makeFilterString(const QString& text);
	Q_INVOKABLE QString getFilter() const { return m_filterString; }

	Q_INVOKABLE QString columnLabel(const uint col) const { return mColumnNames.at(col); }
	inline void setExportRow(const uint row) { m_exportRows.clear(); m_exportRows.append(row); }
	void setExportFiter(const QString& filter, const uint field);
	virtual void exportToText(QFile* outFile, const bool bFancy) const { Q_UNUSED(outFile); Q_UNUSED(bFancy); }
	virtual bool importFromFancyText(QFile* inFile, QString& inData) { Q_UNUSED(inFile); Q_UNUSED(inData); return false; }
	virtual bool importFromText(const QString& data);

	inline uint modifiedIndicesCount() const { return m_modifiedIndices.count(); }
	inline uint modifiedIndex(const uint pos) const { return m_modifiedIndices.at(pos); }
	inline void clearModifiedIndices() { m_modifiedIndices.clear(); }

	Q_INVOKABLE const QString get(const uint row, const uint field) const
	{
		if (row >= 0 && row < m_indexProxy.count())
			return static_cast<QString>(m_modeldata.at(m_indexProxy.at(row)).at(field));
		else
			return QString();
	}

	inline const QString& getFast(const uint row, const uint field) const
	{
		return m_modeldata.at(row).at(field);
	}

	Q_INVOKABLE bool set(const uint row, const uint field, const QString& value)
	{
		if (row >= 0 && row < m_indexProxy.count())
		{
			if (getFast(m_indexProxy.at(row), field) != value)
			{
				m_modeldata[m_indexProxy.at(row)][field] = value;
				setModified(true);
				return true;
			}
		}
		return false;
	}

	inline void setFast(const uint row, const uint field, const QString& value)
	{
		m_modeldata[row][field] = value;
	}

	Q_INVOKABLE int getInt(const uint row, const uint field) const
	{
		if (row >= 0 && row < m_indexProxy.count())
			return static_cast<QString>(m_modeldata.at(m_indexProxy.at(row)).at(field)).toInt();
		else
			return -1;
	}

	inline int getIntFast(const uint row, const uint field) const
	{
		return row < m_modeldata.count() ? m_modeldata.at(row).at(field).toInt() : -1;
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
			return QDate::currentDate();
	}

	inline const QDate getDateFast(const uint row, const uint field) const
	{
		return QDate::fromJulianDay(m_modeldata.at(row).at(field).toLongLong());
	}

	Q_INVOKABLE bool setDate(const uint row, const uint field, const QDate& date)
	{
		if (row >= 0 && row < m_indexProxy.count())
		{
			if (getDateFast(m_indexProxy.at(row), field) != date)
			{
				m_modeldata[m_indexProxy.at(row)][field] = QString::number(date.toJulianDay());
				setModified(true);
				return true;
			}
		}
		return false;
	}

	inline const QString& extraInfo(const uint pos) const { return m_extraInfo.at(pos); }

	inline const QStringList& getRow_const(const uint row) const { return m_modeldata.at(m_indexProxy.at(row)); }
	inline QStringList& getRow(const uint row) { return m_modeldata[m_indexProxy.at(row)]; }

	inline const bool isReady() const { return m_bReady; }
	inline void setReady(const bool bready) { m_bReady = bready; }

	inline virtual void resetPrivateData() {}
	inline virtual const QString exportExtraInfo() const { return QString(); }
	inline virtual bool importExtraInfo(const QString& ) { return true; }
	inline virtual bool updateFromModel(const TPListModel*) { return false; }

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
	QList<uint> m_modifiedIndices;
	QList<uint> m_exportRows;
	QStringList m_extraInfo;
	QHash<int, QByteArray> m_roleNames;
	QList<QString> mColumnNames;

	int m_currentRow;
	uint m_tableId;
	bool m_bFilterApplied, m_bReady, m_bModified;
	uint filterSearch_Field1;
	uint filterSearch_Field2;
	QString m_filterString;

	friend void tp_listmodel_swap ( TPListModel& model1, TPListModel& model2 );
	void copy ( const TPListModel& src_item );

	friend class DBExercisesModel;
	friend class DBMesocyclesModel;
	friend class DBMesoSplitModel;
	friend class DBMesoCalendarModel;
	friend class DBTrainingDayModel;
	friend class DBUserModel;
};

#endif // TPLISTMODEL_H
