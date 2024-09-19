#include "tplistmodel.h"

#include <QRegularExpression>

TPListModel::TPListModel(QObject* parent, int meso_idx)
	: QAbstractListModel(parent), m_mesoIdx(meso_idx), m_currentRow(-1), m_tableId(0), m_bFilterApplied(false),
		m_bReady(false), m_bModified(false), filterSearch_Field1(0), filterSearch_Field2(0)
{}

void tp_listmodel_swap(TPListModel& model1, TPListModel& model2)
{
	using std::swap;
	swap (model1.m_modeldata, model2.m_modeldata);
	swap (model1.m_roleNames, model2.m_roleNames);
	swap (model1.m_indexProxy, model2.m_indexProxy);
}

void TPListModel::copy(const TPListModel& src_item)
{
	m_modeldata = src_item.m_modeldata;
	m_roleNames = src_item.m_roleNames;
	m_indexProxy = src_item.m_indexProxy;
}

TPListModel::~TPListModel()
{
	m_modeldata.clear();
	m_roleNames.clear();
	m_indexProxy.clear();
}

void TPListModel::updateList(const QStringList& list, const int row)
{
	const uint actual_row(m_indexProxy.at(row));
	m_modeldata.replace(actual_row, list);
	emit dataChanged(index(row, 0), index(row, list.count()-1));
}

void TPListModel::removeFromList(const int row)
{
	if (row < count())
	{
		beginRemoveRows(QModelIndex(), row, row);
		m_modeldata.remove(row);
		if (!m_bFilterApplied)
			m_indexProxy.remove(row);
		else
		{
			const int proxy_index(m_indexProxy.indexOf(row));
			if (proxy_index >= 0)
			{
				m_indexProxy.remove(proxy_index);
				for(uint i(proxy_index); i < m_indexProxy.count(); ++i)
					m_indexProxy[i] = i-1;
			}
		}
		if (m_currentRow >= row)
			setCurrentRow(m_currentRow - 1);
		emit countChanged();
		endRemoveRows();
	}
}

void TPListModel::appendList(const QStringList& list)
{
	beginInsertRows(QModelIndex(), count(), count());
	m_modeldata.append(list);
	m_indexProxy.append(m_modeldata.count() - 1);
	emit countChanged();
	endInsertRows();
}

void TPListModel::clear()
{
	beginRemoveRows(QModelIndex(), 0, count()-1);
	m_modeldata.clear();
	m_indexProxy.clear();
	setReady(false);
	setModified(true);
	emit countChanged();
	endRemoveRows();
}

void TPListModel::setCurrentRow(const int row)
{
	if (row >= -1 && row < m_indexProxy.count())
	{
		m_currentRow = row;
		emit currentRowChanged();
	}
}

void TPListModel::moveRow(const uint from, const uint to)
{
	if (from < count() && to < count())
	{
		const QModelIndex sourceParent(index(from < to ? from : to, 0));
		const QModelIndex destinationParent(index(to > from ? to : from, 0));
		const QStringList tempList(m_modeldata.at(from));
		if (to > from)
		{
			for(uint i(from); i < to; ++i)
				m_modeldata[i] = m_modeldata.at(i+1);
		}
		else
		{
			for(uint i(from); i > to; --i)
				m_modeldata[i] = m_modeldata.at(i-1);
		}
		m_modeldata[to] = tempList;
		QList<int> roles;
		for (uint role(0); role < m_roleNames.count(); ++role)
			roles.append(Qt::UserRole+role);
		emit dataChanged(sourceParent, destinationParent, roles);
		setModified(true);
	}
}

void TPListModel::setFilter(const QString &filter, const bool resetSelection)
{
	if (filter.length() >=3)
	{
		QList<QStringList>::const_iterator lst_itr(m_modeldata.constBegin());
		const QList<QStringList>::const_iterator lst_itrend(m_modeldata.constEnd());
		uint idx(0);
		bool bFound(false), bFirst(true);

		const QRegularExpression regex(filter, QRegularExpression::CaseInsensitiveOption);
		for ( ; lst_itr != lst_itrend; ++lst_itr, ++idx )
		{
			bFound = regex.match(static_cast<QStringList>(*lst_itr).at(filterSearch_Field1)).hasMatch();
			if (!bFound)
				bFound = regex.match(static_cast<QStringList>(*lst_itr).at(filterSearch_Field2)).hasMatch();

			if (bFound)
			{
				if (bFirst)
				{
					bFirst = false;
					beginRemoveRows(QModelIndex(), 0, count()-1);
					m_indexProxy.clear();
					if (resetSelection)
					{
						resetPrivateData();
						setCurrentRow(-1);
					}
					endRemoveRows();
				}
				beginInsertRows(QModelIndex(), count(), count());
				m_indexProxy.append(idx);
				endInsertRows();
			}
		}
		m_bFilterApplied = m_indexProxy.count() != m_modeldata.count();
	}
	else
	{
		if (m_bFilterApplied)
		{
			m_bFilterApplied = false;
			beginRemoveRows(QModelIndex(), 0, count()-1);
			m_indexProxy.clear();
			if (resetSelection)
			{
				resetPrivateData();
				setCurrentRow(-1);
			}
			endRemoveRows();
			beginInsertRows(QModelIndex(), 0, m_modeldata.count());
			for( uint i (0); i < m_modeldata.count(); ++i )
				m_indexProxy.append(i);
			endInsertRows();
		}
	}
}

void TPListModel::makeFilterString(const QString& text)
{
	m_filterString = text;
	m_filterString = m_filterString.replace(',', ' ').simplified();
	const QStringList words(m_filterString.split(' '));

	if ( words.count() > 0)
	{
		QStringList::const_iterator itr(words.begin());
		const QStringList::const_iterator itr_end(words.end());
		m_filterString.clear();

		do
		{
			if(static_cast<QString>(*itr).length() < 3)
				continue;
			if (!m_filterString.isEmpty())
				m_filterString.append('|');
			m_filterString.append(static_cast<QString>(*itr).toLower());
			if (m_filterString.endsWith('s', Qt::CaseInsensitive) )
				m_filterString.chop(1);
			m_filterString.remove('.');
			m_filterString.remove('(');
			m_filterString.remove(')');
		} while (++itr != itr_end);
	}
}

void TPListModel::setExportFiter(const QString& filter, const uint field)
{
	const QRegularExpression regex(filter, QRegularExpression::CaseInsensitiveOption);
	QList<QStringList>::const_iterator lst_itr(m_modeldata.constBegin());
	const QList<QStringList>::const_iterator lst_itrend(m_modeldata.constEnd());
	uint row(0);
	m_exportRows.clear();

	for ( ; lst_itr != lst_itrend; ++lst_itr, ++row )
	{
		if (regex.match(static_cast<QStringList>(*lst_itr).at(field)).hasMatch())
			m_exportRows.append(row);
	}
}

bool TPListModel::exportToFile(const QString& filename) const
{
	QFile* outFile{new QFile(filename)};
	const bool bOK(outFile->open(QIODeviceBase::ReadWrite|QIODeviceBase::Append|QIODeviceBase::Text));
	if (bOK)
	{
		const QString strHeader(u"## "_qs + exportName() + u"\n\n"_qs);
		outFile->write(strHeader.toUtf8().constData());

		QString value;
		if (m_exportRows.isEmpty())
		{
			QList<QStringList>::const_iterator itr(m_modeldata.constBegin());
			const QList<QStringList>::const_iterator itr_end(m_modeldata.constEnd());

			while (itr != itr_end)
			{
				for (uint i(0); i < (*itr).count(); ++i)
				{
					if (i < mColumnNames.count())
					{
						if (!mColumnNames.at(i).isEmpty())
						{
							outFile->write(mColumnNames.at(i).toUtf8().constData());
							if (!isFieldFormatSpecial(i))
								value = (*itr).at(i);
							else
								value = formatFieldToExport(i, (*itr).at(i));
							outFile->write(value.replace(subrecord_separator, '|').toUtf8().constData());
							outFile->write("\n", 1);
						}
					}
				}
				outFile->write("\n", 1);
				++itr;
			}
		}
		else
		{
			for (uint x(0); x < m_exportRows.count(); ++x)
			{
				for (uint i(0); i < m_modeldata.at(m_exportRows.at(x)).count(); ++i)
				{
					if (i < mColumnNames.count())
					{
						if (!mColumnNames.at(i).isEmpty())
						{
							outFile->write(mColumnNames.at(i).toUtf8().constData());
							if (!isFieldFormatSpecial(i))
								value = m_modeldata.at(m_exportRows.at(x)).at(i);
							else
								value = formatFieldToExport(i, m_modeldata.at(m_exportRows.at(x)).at(i));
							outFile->write(value.replace(subrecord_separator, '|').toUtf8().constData());
							outFile->write("\n", 1);
						}
					}
				}
				outFile->write("\n", 1);
			}
		}
		outFile->write(STR_END_EXPORT.toUtf8().constData());
		outFile->close();
	}
	delete outFile;
	return bOK;
}
