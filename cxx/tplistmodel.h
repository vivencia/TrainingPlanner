#pragma once
#include <QAbstractListModel>
#include <QQmlEngine>
#include <QDate>

class TPListModel : public QAbstractListModel
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(uint count READ count NOTIFY countChanged)
Q_PROPERTY(int currentRow READ currentRow WRITE setCurrentRow NOTIFY currentRowChanged)

public:
	inline TPListModel(const TPListModel& other)  = delete;
	inline const TPListModel& operator=(TPListModel t_item) = delete;
	inline TPListModel& operator=(const TPListModel& t_item) = delete;

	inline int tableID() const { return m_tableId; }
	inline uint numberOfFields() const { return m_fieldCount; }
	inline int mesoIdx() const { return m_mesoIdx; }
	inline void setMesoIdx(const int new_mesoidx) { m_mesoIdx = new_mesoidx; }

	void appendList(const QStringList &list);
	void appendList(QStringList &&list);
	inline void appendList_fast(QStringList &&list) { m_modeldata.append(std::move(list)); }
	virtual void clear();
	void clearFast();

	virtual inline uint count() const { return m_modeldata.count(); }
	inline int currentRow() const { return m_currentRow; }
	void setCurrentRow(const int row);
	Q_INVOKABLE void removeRow (const uint row);
	void moveRow(const uint from, const uint to);

	[[nodiscard]] inline const QString &columnLabel(const uint col) const { return mColumnNames.at(col); }

	bool isDifferent(const TPListModel* const model) const;
	bool exportContentsOnlyToFile(const QString &filename, const bool appendInfo = false);
	virtual int exportToFile(const QString &filename, const bool writeHeader = true, const bool writeEnd = true, const bool appendInfo = true) const;
	virtual int importFromFile(const QString &filename) { Q_UNUSED(filename); return false; }
	virtual bool updateFromModel(TPListModel*) { return false; }
	virtual QString formatFieldToExport(const uint field, const QString &fieldValue) const { Q_UNUSED(field); return fieldValue; }

	inline const QString &exportName() const { return m_exportName; }
	inline void setExportRow(const int row) { Q_ASSERT_X(row >= 0, "TPListModel::setExportRow", "row < 0"); m_exportRows.append(row); }
	void setExportFilter(const QString &filter, const uint field);
	virtual inline bool isFieldFormatSpecial (const uint) const { return false; }

	inline const QString &extraInfo(const uint pos) const { return m_extraInfo.at(pos); }
	inline const QStringList &getRow_const(const uint row) const { return m_modeldata.at(row); }
	inline QStringList &getRow(const uint row) { return m_modeldata[row]; }

	inline bool importMode() const { return m_bImportMode; }
	inline void setImportMode(const bool bimportmode) { m_bImportMode = bimportmode; }
	inline const bool isReady() const { return m_bReady; }
	inline void setReady(const bool bready) { m_bReady = bready; }

	inline virtual void resetPrivateData() {}

	// QAbstractItemModel interface
	inline virtual int rowCount(const QModelIndex& parent) const override { Q_UNUSED(parent); return count(); }
	inline virtual QVariant data(const QModelIndex&, int) const override { return QVariant(); }
	inline virtual bool setData(const QModelIndex&, const QVariant &, int) override { return false; }
	// return the roles mapping to be used by QML
	inline QHash<int, QByteArray> roleNames() const override final { return m_roleNames; }

signals:
	void countChanged();
	void currentRowChanged();

protected:
	explicit inline TPListModel(QObject* parent = nullptr, int meso_idx = -1)
		: QAbstractListModel{parent}, m_mesoIdx(meso_idx), m_currentRow(-1), m_bReady(false), m_bImportMode(false) {}

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

	friend class DBExercisesModel;
	friend class DBMesocyclesModel;
	friend class DBMesoSplitModel;
	friend class DBMesoCalendarModel;
	friend class DBTrainingDayModel;
	friend class DBUserModel;
};
