#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

QT_FORWARD_DECLARE_CLASS(TPListModel)

class TPFilterModel : public QAbstractListModel
{

Q_OBJECT
QML_ELEMENT
QML_VALUE_TYPE(TPFilterModel)

Q_PROPERTY(uint count READ count NOTIFY countChanged)

public:

	enum FILTER_FIELDS {
		FF_FIELD,
		FF_DISPLAY,
		FF_ICON,
		FF_VISIBLE,
		FF_ENABLED,
		FF_SELECTED,
		FF_VALUES_LIST,
		FF_SELECTED_VALUES,
		FF_COUNT
	};

	explicit TPFilterModel(TPListModel *parent = nullptr);
    TPFilterModel(const TPFilterModel &other);
    ~TPFilterModel() = default;
	inline uint count() const { return m_filterElements.count(); }
	inline void clear()
	{
		m_filters.clear();
		for (auto &filter : m_filterElements) {
			filter.enabled = filter.visible = true;
			filter.selected = false;
			filter.values.clear();
		}
	}

	int addFilterField(const uint field, const std::function<QString()> &display_func, QString &&image,
											const bool visible = true, const bool enabled = true, const bool selected = false);
	void removeFilter(const uint filter_idx);
	inline void setDataAcquisitionFunc(const std::function<QPair<bool,QString>(uint index, uint column)> &func) { m_getData = func; }

	inline const QStringList &filters() const { return m_filters; }

	Q_INVOKABLE void clearSelection();
	Q_INVOKABLE void selectAll();
	Q_INVOKABLE inline void addValue(const uint filter_index, const uint value_index)
	{
		if (filter_index < m_filterElements.count() && value_index < m_filterElements.at(filter_index).selected_values.count()) {
			if (!m_filterElements.at(filter_index).selected_values.at(value_index)) {
				m_filterElements[filter_index].selected_values[value_index] = true;
				m_filters.append(m_filterElements.at(filter_index).values.at(value_index));
			}
		}
	}
	Q_INVOKABLE inline void delValue(const uint filter_index, const uint value_index)
	{
		if (filter_index < m_filterElements.count() && value_index < m_filterElements.at(filter_index).selected_values.count()) {
			if (m_filterElements.at(filter_index).selected_values.at(value_index)) {
				m_filterElements[filter_index].selected_values[value_index] = false;
				m_filters.removeOne(m_filterElements.at(filter_index).values.at(value_index));
			}
		}
	}

	void initFilterValues(const uint field);

	inline int rowCount(const QModelIndex & = QModelIndex{}) const override final { return count(); }
	QVariant data(const QModelIndex &index, int role) const final override;
	bool setData(const QModelIndex &index, const QVariant &value, int role) final override;
	// return the roles mapping to be used by QML
	inline QHash<int, QByteArray> roleNames() const override final { return m_roleNames; }

signals:
	void countChanged();
	void filtersChanged(const bool); //Signature matching

private:
	struct st_Filter {
		QString display, image;
		bool visible{true}, enabled{true}, selected{false};
		QList<bool> selected_values;
		QStringList values;
		int field{-1};
	};

	TPListModel *m_parentModel{nullptr};
	QList<st_Filter> m_filterElements;
	QList<std::function<QString()>> m_filterDisplayStringFuncs;
	QStringList m_filters;
	std::function<QPair<bool,QString>(uint index, uint column)> m_getData{nullptr};
	QHash<int, QByteArray> m_roleNames;

	bool hasValue(const st_Filter &filter, const QString &value) const;
};
