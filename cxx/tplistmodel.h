#ifndef TPLISTMODEL_H
#define TPLISTMODEL_H

#include <QAbstractListModel>
#include <QQmlEngine>
#include <QDate>

class TPListModel : public QAbstractListModel
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(uint count READ count NOTIFY countChanged)
Q_PROPERTY(int currentRow READ currentRow WRITE setCurrentRow NOTIFY currentRowChanged)
Q_PROPERTY(int mesoIdx READ mesoIdx WRITE setMesoIdx NOTIFY mesoIdxChanged)

public:
	explicit inline TPListModel(QObject* parent = nullptr, int meso_idx = -1)
		: QAbstractListModel{parent}, m_mesoIdx(meso_idx), m_currentRow(-1), m_bReady(false), m_bImportMode(false) {}

	inline TPListModel(const TPListModel& other) : TPListModel {other.parent()} { copy(other); }

	inline const TPListModel& operator=(TPListModel t_item)
	{
		copy (t_item);
		return *this;
	}

	virtual ~TPListModel() override;

	inline int tableID() const { return m_tableId; }
	inline uint numberOfFields() const { return m_fieldCount; }
	inline int mesoIdx() const { return m_mesoIdx; }
	inline void setMesoIdx(const int new_mesoidx)
	{
		if (new_mesoidx != m_mesoIdx)
		{
			m_mesoIdx = new_mesoidx;
			emit mesoIdxChanged();
		}
	}

	void appendList(const QStringList& list);
	virtual void clear();

	Q_INVOKABLE inline void appendRow() { appendList(QStringList(numberOfFields())); setCurrentRow(count() - 1); }
	Q_INVOKABLE void removeRow (const uint row);
	Q_INVOKABLE inline uint count() const { return m_modeldata.count(); }
	Q_INVOKABLE inline int currentRow() const { return m_currentRow; }
	Q_INVOKABLE void setCurrentRow(const int row);
	Q_INVOKABLE void moveRow(const uint from, const uint to);

	Q_INVOKABLE QString columnLabel(const uint col) const { return mColumnNames.at(col); }

	virtual int exportToFile(const QString& filename, const bool writeHeader = true, const bool writeEnd = true) const;
	virtual int importFromFile(const QString& filename) { Q_UNUSED(filename); return false; }
	virtual bool updateFromModel(const TPListModel* const) { return false; }
	virtual QString formatFieldToExport(const uint field, const QString& fieldValue) const { Q_UNUSED(field); return fieldValue; }

	inline const QString& exportName() const { return m_exportName; }
	inline void setExportRow(const int row) { Q_ASSERT_X(row >= 0, "TPListModel::setExportRow", "row < 0"); m_exportRows.clear(); m_exportRows.append(row); }
	void setExportFilter(const QString& filter, const uint field);
	virtual inline bool isFieldFormatSpecial (const uint) const { return false; }

	inline const QString& extraInfo(const uint pos) const { return m_extraInfo.at(pos); }
	inline const QStringList& getRow_const(const uint row) const { return m_modeldata.at(row); }

	inline const bool isReady() const { return m_bReady; }
	inline void setReady(const bool bready) { m_bReady = bready; }
	inline bool importMode() const { return m_bImportMode; }
	inline void setImportMode(const bool bimportmode) { m_bImportMode = bimportmode; }

	inline virtual void resetPrivateData() {}

	// QAbstractItemModel interface
	inline virtual int columnCount(const QModelIndex &parent) const override { Q_UNUSED(parent); return 1; }
	inline virtual int rowCount(const QModelIndex &parent) const override { Q_UNUSED(parent); return count(); }
	inline virtual QVariant data(const QModelIndex &, int) const override { return QVariant(); }
	inline virtual bool setData(const QModelIndex &, const QVariant &, int) override { return false; }

signals:
	void countChanged();
	void mesoIdxChanged();
	void currentRowChanged();

protected:
	// return the roles mapping to be used by QML
	inline virtual QHash<int, QByteArray> roleNames() const override { return m_roleNames; }

	QList<QStringList> m_modeldata;
	QList<uint> m_exportRows;
	QStringList m_extraInfo;
	QHash<int, QByteArray> m_roleNames;
	QList<QString> mColumnNames;

	int m_mesoIdx;
	int m_currentRow;
	uint m_tableId, m_fieldCount;
	bool m_bReady, m_bModified, m_bImportMode;
	QString m_filterString, m_exportName;

	void copy(const TPListModel& src_item);

	friend class DBExercisesModel;
	friend class DBMesocyclesModel;
	friend class DBMesoSplitModel;
	friend class DBMesoCalendarModel;
	friend class DBTrainingDayModel;
	friend class DBUserModel;
};

#endif // TPLISTMODEL_H
