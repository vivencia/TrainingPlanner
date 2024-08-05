#include "dbusermodel.h"

DBUserModel::DBUserModel(QObject *parent)
	: TPListModel(parent)
{
	m_tableId = EXERCISES_TABLE_ID;
	setObjectName(DBExercisesObjectName);

	mColumnNames.reserve(USER_TOTAL_COLS);
	mColumnNames.append(QString());
	mColumnNames.append(tr("Name: "));
	mColumnNames.append(tr("Birthday: "));
	mColumnNames.append(tr("Sex: "));
	mColumnNames.append(tr("Phone(s): "));
	mColumnNames.append(tr("E-mail(s): "));
	mColumnNames.append(tr("Social Media: "));
	mColumnNames.append(tr("Role: "));
	mColumnNames.append(tr("Goal: "));
	mColumnNames.append(QString());
	mColumnNames.append(tr("Coach: "));
}

bool DBUserModel::updateFromModel(const TPListModel* model)
{
	if (model->count() > 0)
	{
		QList<QStringList>::const_iterator lst_itr(model->m_modeldata.constBegin());
		const QList<QStringList>::const_iterator lst_itrend(model->m_modeldata.constEnd());
		uint lastIndex(m_modeldata.count());
		do {
			m_modifiedIndices.append(lastIndex++);
			appendList((*lst_itr));
		} while (++lst_itr != lst_itrend);
		return true;
	}
	return false;
}

void DBUserModel::exportToText(QFile* outFile, const bool bFancy) const
{
	QString strHeader;
	if (bFancy)
		strHeader = u"##"_qs + objectName() + u"\n\n"_qs;
	else
		strHeader = u"##0x0"_qs + QString::number(m_tableId) + u"\n"_qs;

	outFile->write(strHeader.toUtf8().constData());
	outFile->write(exportExtraInfo().toUtf8().constData());
	if (bFancy)
		outFile->write("\n\n", 2);
	else
		outFile->write("\n", 1);

	QList<QStringList>::const_iterator itr(m_modeldata.constBegin());
	const QList<QStringList>::const_iterator itr_end(m_modeldata.constEnd());

	while (itr != itr_end)
	{
		for (uint i(0); i < (*itr).count(); ++i)
		{
			if (bFancy)
			{
				if (i < mColumnNames.count())
				{
					if (!mColumnNames.at(i).isEmpty())
					{
						outFile->write(mColumnNames.at(i).toUtf8().constData());
						outFile->write((*itr).at(i).toUtf8().constData());
						outFile->write("\n", 1);
					}
				}
			}
			else
			{
				outFile->write((*itr).at(i).toUtf8().constData());
					outFile->write(QByteArray(1, record_separator.toLatin1()), 1);
			}
		}
		if (bFancy)
			outFile->write("\n", 1);
		else
			outFile->write(QByteArray(1, record_separator2.toLatin1()), 1);
		++itr;
	}

	if (bFancy)
		outFile->write(tr("##End##\n").toUtf8().constData());
	else
		outFile->write("##end##");
}

bool DBUserModel::importFromFancyText(QFile* inFile, QString& inData)
{
	char buf[256];
	QStringList modeldata;
	uint col(1);
	QString value;

	//Because a DBMesocyclesModel does not have an extra info to export nor import, inFile is already at the
	//first relevant information of the meso, its name
	inData.chop(1);
	int sep_idx(inData.indexOf(':'));
	if (sep_idx != -1)
	{
		value = inData.right(inData.length() - sep_idx - 2);
		modeldata.append(u"-1"_qs); //id
		modeldata.append(value); //name
		col++;
	}
	else
		return false;

	while (inFile->readLine(buf, sizeof(buf)) != -1) {
		inData = buf;
		inData.chop(1);
		if (inData.isEmpty())
		{
			if (!modeldata.isEmpty())
			{
				appendList(modeldata);
				modeldata.clear();
				col = 1;
			}
		}
		else
		{
			sep_idx = inData.indexOf(':');
			if (sep_idx != -1)
			{
				value = inData.right(inData.length() - sep_idx - 2);
				modeldata.append(value);
				col++;
			}
			else
			{
				if (inData.contains(u"##"_qs))
					break;
			}
		}
	}
	return count() > 0;
}

QVariant DBUserModel::data(const QModelIndex &index, int role) const
{
	const int row(index.row());
	if( row >= 0 && row < m_modeldata.count() )
	{
		switch(role) {
			case idRole:
				return static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole)).toUInt();
			case birthdayRole:
				return QDate::fromJulianDay(static_cast<QString>(m_modeldata.at(row).at(role-Qt::UserRole)).toLongLong());
			default:
				return m_modeldata.at(row).at(role-Qt::UserRole);
		}
	}
	return QVariant();
}

bool DBUserModel::setData(const QModelIndex &index, const QVariant& value, int role)
{
	const int row(index.row());
	if( row >= 0 && row < m_modeldata.count() )
	{
		switch(role) {
			default:
				m_modeldata[row][role-Qt::UserRole] = value.toString();
				emit dataChanged(index, index, QList<int>() << role);
			break;
			case birthdayRole:
				m_modeldata[row][role-Qt::UserRole] = QString::number(value.toDate().toJulianDay());
				emit dataChanged(index, index, QList<int>() << role);
			break;
		}
		return true;
	}
	return false;
}
