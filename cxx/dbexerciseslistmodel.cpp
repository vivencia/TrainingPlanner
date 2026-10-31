#include "dbexerciseslistmodel.h"

#include "dbexerciseslisttable.h"
#include "return_codes.h"
#include "tpfilepath.h"
#include "tpsettings.h"
#include "tputils.h"
#include "translationclass.h"

#include <QFile>

#include <ranges>
#include <utility>

DBExercisesListModel *DBExercisesListModel::_app_exercises_list{nullptr};
constexpr int DEFAULT_EXERCISES_NUMBER{306}; //Value hardcode based on the most current exercises list

enum RoleNames {
	createRole(mainName, DBExercisesListModel::MAINNAME)
	createRole(subName, DBExercisesListModel::SUBNAME)
	createRole(muscularGroup, DBExercisesListModel::MUSCULARGROUP)
	createRole(mediaPath, DBExercisesListModel::MEDIAPATH)
};

#include <QThread>
DBExercisesListModel::DBExercisesListModel(QObject *parent) : TPListModel{parent, N_FIELDS}
{
	setObjectName("DBExercisesListModel");
	qDebug() << "DBExercisesListModel::DBExercisesListModel running on thread: " << thread()->isMainThread();
	_app_exercises_list = this;
	REGISTER_QML_SINGLETON(DBExercisesListModel, this);

	roleToString(mainName)
	roleToString(subName)
	roleToString(muscularGroup)
	roleToString(mediaPath)

	m_exercisesData.reserve(DEFAULT_EXERCISES_NUMBER);
	setFieldsNames();
	addFilters();
	m_removeField = MAINNAME;
	connect(this, &TPListModel::itemRemoved, this, [this] (const int index){
		m_dbModelInterface->setRemovalInfo(index, {ID});
		appThreadManager()->runAction(m_db, ThreadManager::DeleteRecords, m_dbModelInterface);
		beginRemoveRows(QModelIndex{}, index, index);
		m_exercisesData.remove(index);
		endRemoveRows();
	});
	connect(appTr(), &TranslationClass::applicationLanguageChanged, this, &DBExercisesListModel::labelsChanged);
}

void DBExercisesListModel::initExercisesList()
{
	if (m_exercisesData.isEmpty()) {
		readExercisesList();
		m_dbModelInterface = new DBModelInterfaceExercisesList;
		m_db = new DBExercisesListTable{m_dbModelInterface};
		appThreadManager()->runAction(m_db, ThreadManager::CreateTable);
		appThreadManager()->runAction(m_db, ThreadManager::ReadAllRecords);
	}
}

TPFilePathPtr DBExercisesListModel::suggestedName() const
{
	return TPFilePath::newTPFilePath(tr("New Exercises") % TPUtils::TP_FILE_EXTENSION, appSettings()->currentUser());
}

void DBExercisesListModel::setMainName(const uint index, const QString &new_name)
{
	if (index < m_exercisesData.count()) {
		m_exercisesData[index][MAINNAME] = new_name;
		emit dataChanged(TPListModel::index(index, 0), TPListModel::index(index, 0), {mainNameRole});
	}
}

void DBExercisesListModel::setSubName(const uint index, const QString &new_subname)
{
	if (index < m_exercisesData.count()) {
		m_exercisesData[index][SUBNAME] = new_subname;
		emit dataChanged(TPListModel::index(index, 0), TPListModel::index(index, 0), {subNameRole});
	}
}

void DBExercisesListModel::setMuscularGroup(const uint index, const QString &new_group)
{
	if (index < m_exercisesData.count()) {
		m_exercisesData[index][MUSCULARGROUP] = new_group;
		emit dataChanged(TPListModel::index(index, 0), TPListModel::index(index, 0), {muscularGroupRole});
	}
}

void DBExercisesListModel::setMediaPath(const uint index, const QString &new_path)
{
	if (index < m_exercisesData.count()) {
		m_exercisesData[index][MEDIAPATH] = new_path;
		emit dataChanged(TPListModel::index(index, 0), TPListModel::index(index, 0), {mediaPathRole});
	}
}

void DBExercisesListModel::newExercise(const QString &name, const QString &subname, const QString &muscular_group)
{
	appendList(std::move(QStringList{} << std::move("-1"_L1) << name << subname << muscular_group << QString{} <<
					std::move("0"_L1) << std::move(QString::number(m_exercisesData.count())) << std::move("0"_L1)));
}

bool DBExercisesListModel::collectExportData()
{
	m_exportRows.clear();
	for (uint i{DEFAULT_EXERCISES_NUMBER}; i < m_exercisesData.count(); ++i)
		m_exportRows.append(i);
	return m_exportRows.count() > 0;
}

void DBExercisesListModel::appendList(const QStringList &list)
{
	beginInsertRows(QModelIndex{}, m_exercisesData.count(), m_exercisesData.count() + list.count() - 1);
	m_dbModelInterface->setModified(m_exercisesData.count(), 0);
	m_exercisesData.append(list);
	endInsertRows();
}

void DBExercisesListModel::clear()
{
	m_exercisesData.clear();
	m_exportRows.clear();
	TPListModel::clear();
}

int DBExercisesListModel::exportToFile(const TPFilePath &filename) const
{
	const auto ret{appUtils()->writeDataToFile(filename.toString(), appUtils()->exercisesListFileIdentifier,
																					m_exercisesData, m_exportRows)};
	return ret;
}

int DBExercisesListModel::exportToFormattedFile(const TPFilePath &filename) const
{
	const QList<std::function<QString(void)>> &field_description{QList<std::function<QString(void)>>{} <<
											nullptr <<
											[this] () { return exerciseNameLabel(); } <<
											[this] () { return exerciseSpecificsLabel(); } <<
											[this] () { return muscularGroupsLabel(); } <<
											nullptr <<
											nullptr <<
											nullptr <<
											nullptr <<
											nullptr
	};

	const int ret{appUtils()->writeDataToFormattedFile(filename.toString(),
														appUtils()->exercisesListFileIdentifier,
														m_exercisesData,
														field_description,
														nullptr,
														m_exportRows,
														QString{tr("Exercises List") + "\n\n"_L1})
	};
	return ret;
}

int DBExercisesListModel::importFromFile(const TPFilePath& filename)
{
	beginInsertRows(QModelIndex{}, count(), count());
	int ret{appUtils()->readDataFromFile(filename.toString(), m_exercisesData, N_FIELDS,
																			appUtils()->exercisesListFileIdentifier)};
	if (ret == TP_RET_CODE_IMPORT_OK)
		emit countChanged();
	endInsertRows();
	return ret;
}

int DBExercisesListModel::importFromFormattedFile(const TPFilePath &filename)
{
	const uint first_imported_idx{count()};
	beginInsertRows(QModelIndex{}, count(), count());
	const int ret{appUtils()->readDataFromFormattedFile(filename.toString(), m_exercisesData, N_FIELDS,
																	appUtils()->exercisesListFileIdentifier, nullptr)};

	if (ret == TP_RET_CODE_IMPORT_OK) {
		uint actual_index{first_imported_idx};
		for (auto &&data : m_exercisesData | std::views::drop(first_imported_idx) ) {
			data[0] = std::move(QString::number(m_exercisesData.last().at(ID).toUInt() + 1));
			data.append(std::move("(kg)"_L1));
			data.append(std::move("0"_L1));
			data.append(std::move(QString::number(actual_index++)));
			data.append(std::move("0"_L1));
		}
		emit countChanged();
	}
	endInsertRows();
	return ret;
}

int DBExercisesListModel::newExerciseFromFile(const TPFilePath &filename, const std::optional<bool> &file_formatted)
{
	int import_result{TP_RET_CODE_IMPORT_FAILED};
	if (file_formatted.has_value()) {
		if (file_formatted.value())
			import_result = importFromFormattedFile(filename);
		else
			import_result = importFromFile(filename);
	} else {
		import_result = importFromFile(filename);
		if (import_result == TP_RET_CODE_WRONG_IMPORT_FILE_TYPE)
			import_result = importFromFormattedFile(filename);
	}
	return import_result;
}

void DBExercisesListModel::setFieldsNames()
{
	m_identifier = std::move(tr("Exercises Database"));
	m_fieldsNames.append(std::move(tr("Name")));
	m_fieldsNames.append(std::move(tr("Extra info")));
	m_fieldsNames.append(std::move(tr("Muscular Group")));
}

QVariant DBExercisesListModel::data(const uint role, const uint row, const int column) const
{
	switch (role) {
	case mainNameRole: return m_exercisesData.at(row).at(MAINNAME);
	case subNameRole: return m_exercisesData.at(row).at(SUBNAME);
	case muscularGroupRole: return m_exercisesData.at(row).at(MUSCULARGROUP);
	case mediaPathRole: return m_exercisesData.at(row).at(MEDIAPATH);
	default: Q_UNREACHABLE_RETURN(QVariant{});
	}
}

void DBExercisesListModel::addFilters()
{
	TPFilterModel *filter{new TPFilterModel{this}};
	filter->setDataAcquisitionFunc([this] (uint index, uint column) -> QPair<bool,QString> {
		if (index < m_exercisesData.count())
			return {true, dataValue(index, column)};
		else
			return {false, QString{}};
	});
	for (uint i{MAINNAME}; i <= MUSCULARGROUP; ++i)
		filter->addFilterField(i, [this,i] () -> QString { return m_fieldsNames.at(i+1); }, std::move(QString{}));
	setFiltersManager(filter);
	setPersistentFilterCompareFunction(true, MUSCULARGROUP, QString{}, [this] (bool positive, int row) -> bool {
		if (m_mgf.isEmpty()) {
			return true;
		} else {
			bool ret{false};
			if (row > 0)
				ret = exerciseData(MUSCULARGROUP, row).contains(m_mgf);
			return ret;
		}
	});
}

void DBExercisesListModel::readExercisesList()
{
	const QString &list_filename{":/extras/exercises_%1.lst"_L1};
	QFile *file{appUtils()->openFile(list_filename.arg(appSettings()->userLocale()))};
	if (!file)
		file = appUtils()->openFile(list_filename.arg("en_US"_L1));
	if (file) {
		QString line{256, QChar{0}};
		QTextStream stream{file};
		stream.readLineInto(&line);
		while (stream.readLineInto(&line)) {
			QStringList fields{std::move<QStringList>(line.split(';'))};
			m_exercisesData.append({std::move("-1"_L1), std::move(fields[0]), std::move(fields[1]),std::move(fields[2]),
				QString{}, std::move("1"_L1), std::move(QString::number(m_exercisesData.count())), std::move("0"_L1)});
		}
		file->close();
		delete file;
		syncMetadata(m_exercisesData.count());
	}
}
