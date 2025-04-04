#include "tplistmodel.h"
#include "tputils.h"
#include "tpglobals.h"

#include <QFile>
#include <QRegularExpression>

void TPListModel::removeRow(const uint row)
{
	Q_ASSERT_X(row < m_modeldata.count(), "TPListModel::removeRow", "out of range row");
	beginRemoveRows(QModelIndex{}, row, row);
	m_modeldata.remove(row);
	if (m_currentRow >= row)
		setCurrentRow(m_currentRow > 0 ? m_currentRow - 1 : 0);
	emit countChanged();
	endRemoveRows();
}

void TPListModel::appendList(const QStringList &list)
{
	beginInsertRows(QModelIndex{}, count(), count());
	m_modeldata.append(list);
	emit countChanged();
	endInsertRows();
}

void TPListModel::appendList(QStringList &&list)
{
	beginInsertRows(QModelIndex{}, count(), count());
	m_modeldata.append(std::move(list));
	emit countChanged();
	endInsertRows();
}

void TPListModel::clear()
{
	beginResetModel();
	//beginRemoveRows(QModelIndex{}, 0, count()-1);
	m_modeldata.clear();
	m_exportRows.clear();
	setReady(false);
	emit countChanged();
	//endRemoveRows();
	endResetModel();
}

void TPListModel::clearFast()
{
	m_modeldata.clear();
	setReady(false);
}

void TPListModel::setCurrentRow(const int row)
{
	if (m_currentRow != row)
	{
		m_currentRow = row;
		emit currentRowChanged();
	}
}

void TPListModel::moveRow(const uint from, const uint to)
{
	if (from < count() && to < count())
	{
		QStringList tempList{std::move(m_modeldata[from])};

		if (to > from)
		{
			beginMoveRows(QModelIndex{}, from, from, QModelIndex{}, to+1);
			for(uint i{from}; i < to; ++i)
				m_modeldata[i] = std::move(m_modeldata[i+1]);
		}
		else
		{
			beginMoveRows(QModelIndex{}, to, to, QModelIndex{}, from+1);
			for(uint i{from}; i > to; --i)
				m_modeldata[i] = std::move(m_modeldata[i-1]);
		}
		m_modeldata[to] = std::move(tempList);
		endMoveRows();
		/*const QModelIndex& sourceParent(index(from < to ? from : to, 0));
		const QModelIndex& destinationParent(index(to > from ? to : from, 0));
		QList<int> roles;
		for (uint role(0); role < m_roleNames.count(); ++role)
			roles.append(Qt::UserRole+role);
		emit dataChanged(sourceParent, destinationParent, roles);*/
	}
}

//Called when importing from a text file
bool TPListModel::isDifferent(const TPListModel *const model) const
{
	if (model->count() > 0)
	{
		if (count() == 0)
			return true;
	}
	else
		return false; //model is not usefull

	bool bEqual{true};
	for (uint n{0}; n < count(); ++n)
	{
		for (uint i{1}; i < model->m_modeldata.at(0).count(); ++i)
		{
			if (m_modeldata.at(n).at(i) != model->m_modeldata.at(0).at(i))
			{
				bEqual = false;
				break;
			}
		}
		if (bEqual)
			return false;
		bEqual = true;
	}
	return true;
}

bool TPListModel::exportContentsOnlyToFile(const QString &filename, const bool useRealId, const bool appendInfo) const
{
	QFile *outFile{appUtils()->openFile(filename, appendInfo ? QIODeviceBase::ReadWrite|QIODeviceBase::Append|QIODeviceBase::Text :
							QIODeviceBase::WriteOnly|QIODeviceBase::Truncate|QIODeviceBase::Text)};
	if (!outFile)
		return false;

	if (m_exportRows.isEmpty())
	{
		for (const auto &itr : m_modeldata)
		{
			uint i{0};
			if (!useRealId)
			{
				outFile->write("-1", 2);
				i = 1;
			}
			for (; i < itr.count(); ++i)
			{
				outFile->write(itr.at(i).toUtf8().constData());
				outFile->write("\n", 1);
			}
		}
	}
	else
	{
		for (uint x{0}; x < m_exportRows.count(); ++x)
		{
			for (const auto &modeldata : m_modeldata.at(m_exportRows.at(x)))
			{
				outFile->write(modeldata.toUtf8().constData());
				outFile->write("\n", 1);
			}
		}
		const_cast<TPListModel*>(this)->m_exportRows.clear();
	}
	outFile->close();
	delete outFile;
	return true;
}

int TPListModel::importFromContentsOnlyFile(const QString &filename)
{
	QFile *inFile{appUtils()->openFile(filename, QIODeviceBase::ReadOnly|QIODeviceBase::Text)};
	if (!inFile)
		return -1;

	char buf[512];
	QStringList modeldata(m_fieldCount);
	while (inFile->readLine(buf, sizeof(buf)) != -1)
		modeldata.append(QString::fromLocal8Bit(buf));
	inFile->close();
	delete inFile;
	if (modeldata.count() < m_fieldCount)
		return -1;
	if (modeldata.count() > m_fieldCount)
		modeldata.resize(m_fieldCount);
	m_modeldata.append(std::move(modeldata));
	return m_modeldata.count() - 1;
}

int TPListModel::exportToFile(const QString &filename, const bool writeHeader, const bool writeEnd, const bool appendInfo) const
{
	QFile *outFile{appUtils()->openFile(filename, appendInfo ? QIODeviceBase::ReadWrite|QIODeviceBase::Append|QIODeviceBase::Text :
							QIODeviceBase::WriteOnly|QIODeviceBase::Truncate|QIODeviceBase::Text)};

	if (!outFile)
		return APPWINDOW_MSG_OPEN_CREATE_FILE_FAILED;

	if (writeHeader)
	{
		const QString &strHeader{"## "_L1 + exportName() + " - 0x000"_L1 + QString::number(tableID()) + "\n\n"_L1};
		outFile->write(strHeader.toUtf8().constData());
	}

	QString value;
	if (m_exportRows.isEmpty())
	{
		QList<QStringList>::const_iterator itr{m_modeldata.constBegin()};
		const QList<QStringList>::const_iterator& itr_end{m_modeldata.constEnd()};

		while (itr != itr_end)
		{
			for (uint i{0}; i < (*itr).count(); ++i)
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
						outFile->write(value.replace(comp_exercise_separator, comp_exercise_fancy_separator).toUtf8().constData());
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
		for (uint x{0}; x < m_exportRows.count(); ++x)
		{
			const QStringList &modeldata{m_modeldata.at(m_exportRows.at(x))};
			for (uint i{0}; i < modeldata.count(); ++i)
			{
				if (i < mColumnNames.count())
				{
					if (!mColumnNames.at(i).isEmpty())
					{
						outFile->write(mColumnNames.at(i).toUtf8().constData());
						if (!isFieldFormatSpecial(i))
							value = modeldata.at(i);
						else
							value = std::move(formatFieldToExport(i, modeldata.at(i)));
						outFile->write(value.replace(comp_exercise_separator, comp_exercise_fancy_separator).toUtf8().constData());
						outFile->write("\n", 1);
					}
				}
			}
			outFile->write("\n", 1);
		}
		const_cast<TPListModel*>(this)->m_exportRows.clear();
	}
	if (writeEnd)
		outFile->write(STR_END_EXPORT.toUtf8().constData());
	outFile->close();
	delete outFile;
	return APPWINDOW_MSG_EXPORT_OK;
}

void TPListModel::setExportFilter(const QString &filter, const uint field)
{
	const QRegularExpression regex{filter, QRegularExpression::CaseInsensitiveOption};
	QList<QStringList>::const_iterator lst_itr(m_modeldata.constBegin());
	const QList<QStringList>::const_iterator& lst_itrend(m_modeldata.constEnd());
	uint row(0);
	m_exportRows.clear();

	for (; lst_itr != lst_itrend; ++lst_itr, ++row)
	{
		if (regex.match((*lst_itr).at(field)).hasMatch())
			m_exportRows.append(row);
	}
}
