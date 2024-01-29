// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

pragma Singleton

import QtQml
import QtQuick.LocalStorage

import "jsfunctions.js" as JSF

QtObject {
	id: root

	property var db
	property int exercisesTableLastId: 1000

	function init_database() {
		try {
			db = LocalStorage.openDatabaseSync("TrainingPlanner", "1.0", "Training application database");
			db.transaction(
				function (tx) {
					var res = tx.executeSql(`SELECT name FROM sqlite_schema WHERE type='table' AND name='mesocycles'`);
					if (!res.rows.length) {
						createMesoTable();
						createMesoDivisionTable();
						createMesoCalendarTable();
						createExercisesTable();
						createSetsInfoTable();
						createTrainingDayTable();
						if (AppSettings.exercisesListVersion === 0.0) { //First run. Insert exercises list into database
							var exercisesList = runCmd.getExercisesList();
							if (exercisesList.length > 0)
								appDB.updateExercisesList(exercisesList);
						}
					}
					//console.log(res.rows.item(0).name)
			});
		} catch (error) {
			console.log("Error opening database: " + error);
		}
	}

	function createMesoTable() {
		db.transaction(function (tx) {
			tx.executeSql(`CREATE TABLE IF NOT EXISTS mesocycles (
				meso_id INTEGER PRIMARY KEY AUTOINCREMENT,
				meso_name TEXT,
				meso_start_date INTEGER,
				meso_end_date INTEGER,
				meso_note TEXT,
				meso_nweeks INTEGER,
				meso_split TEXT,
				meso_drugs TEXT
			)`);
		});
	}

	function createMesoDivisionTable() {
		db.transaction(function (tx) {
			tx.executeSql(`CREATE TABLE IF NOT EXISTS mesocycle_division (
				division_id INTEGER PRIMARY KEY AUTOINCREMENT,
				meso_id INTEGER,
				splitA TEXT,
				splitA_exercisesnames TEXT,
				splitA_exercisesset_types TEXT,
				splitA_exercisesset_n TEXT,
				splitA_exercisesset_reps TEXT,
				splitA_exercisesset_weight TEXT,
				splitB TEXT,
				splitB_exercisesnames TEXT,
				splitB_exercisesset_types TEXT,
				splitB_exercisesset_n TEXT,
				splitB_exercisesset_reps TEXT,
				splitB_exercisesset_weight TEXT,
				splitC TEXT,
				splitC_exercisesnames TEXT,
				splitC_exercisesset_types TEXT,
				splitC_exercisesset_n TEXT,
				splitC_exercisesset_reps TEXT,
				splitC_exercisesset_weight TEXT,
				splitD TEXT,
				splitD_exercisesnames TEXT,
				splitD_exercisesset_types TEXT,
				splitD_exercisesset_n TEXT,
				splitD_exercisesset_reps TEXT,
				splitD_exercisesset_weight TEXT,
				splitE TEXT,
				splitE_exercisesnames TEXT,
				splitE_exercisesset_types TEXT,
				splitE_exercisesset_n TEXT,
				splitE_exercisesset_reps TEXT,
				splitE_exercisesset_weight TEXT,
				splitF TEXT,
				splitF_exercisesnames TEXT,
				splitF_exercisesset_types TEXT,
				splitF_exercisesset_n TEXT,
				splitF_exercisesset_reps TEXT,
				splitF_exercisesset_weight TEXT
			)`);
		});
	}

	function createMesoCalendarTable() {
		db.transaction(function (tx) {
			tx.executeSql(`CREATE TABLE IF NOT EXISTS mesocycle_calendar (
				id INTEGER PRIMARY KEY AUTOINCREMENT,
				meso_id INTEGER,
				cal_date INTEGER,
				training_day INTEGER,
				training_split TEXT
			)`);
		});
	}

	function createExercisesTable() {
		db.transaction(function (tx) {
			tx.executeSql(`CREATE TABLE IF NOT EXISTS exercises_table (
				id INTEGER PRIMARY KEY,
				primary_name TEXT CHECK(primary_name != ''),
				secondary_name TEXT,
				muscular_group TEXT,
				sets INTEGER,
				reps REAL,
				weight REAL,
				weight_unit TEXT,
				media_path TEXT,
				from_list INTEGER
			)`);
		});
	}

	/*function updateExercisesTable() {
		db.transaction(function (tx) {
			tx.executeSql(`CREATE TABLE IF NOT EXISTS exercises_table2 (
				id INTEGER PRIMARY KEY AUTOINCREMENT,
				primary_name TEXT CHECK(primary_name != ''),
				secondary_name TEXT CHECK(secondary_name != ''),
				muscular_group TEXT,
				sets INTEGER,
				reps REAL,
				weight REAL,
				weight_unit TEXT,
				media_path TEXT
			)`);
		});

		let exercises = getExercises();
		for (var i = 0; i < exercises.length; i++) {
			db.transaction(function (tx) {
			tx.executeSql("INSERT INTO exercises_table2
						(primary_name,secondary_name,muscular_group,sets,reps,weight,weight_unit,media_path) VALUES(?,?,?,?,?,?,?,?)",
						[exercises[i].mainName, exercises[i].subName, " ", exercises[i].nSets, exercises[i].nReps,
						exercises[i].nWeight, "kg(s)", exercises[i].mediaPath]);
			});
		}

		removeExercisesTable();
		db.transaction(function (tx) {
			tx.executeSql("ALTER TABLE `exercises_table2` RENAME TO `exercises_table`");
		});
	}*/

	function createSetsInfoTable() {
		db.transaction(function (tx) {
			tx.executeSql(`CREATE TABLE IF NOT EXISTS set_info (
				id INTEGER PRIMARY KEY AUTOINCREMENT,
				trainingday_id INTEGER,
				trainingday_exercise_idx INTEGER,
				type INTEGER,
				number INTEGER,
				reps TEXT,
				weight TEXT,
				weight_unit TEXT,
				subsets INTEGER,
				resttime TEXT,
				notes TEXT
			)`);
		});
	}

	/*function updateSetsInfoTable() {
		db.transaction(function (tx) {
			tx.executeSql(`CREATE TABLE IF NOT EXISTS set_info2 (
				id INTEGER PRIMARY KEY AUTOINCREMENT,
				trainingday_id INTEGER,
				trainingday_exercise_idx INTEGER,
				type INTEGER,
				number INTEGER,
				reps TEXT,
				weight TEXT,
				weight_unit TEXT,
				subsets INTEGER,
				resttime TEXT,
				notes TEXT
			)`);
		});

		let setsInfo = [];
		db.transaction(function (tx) {
			let results = tx.executeSql("SELECT * FROM set_info");
			for (let i = 0; i < results.rows.length; i++) {
				let row = results.rows.item(i);
				db.transaction(function (tx) {
						tx.executeSql("INSERT INTO set_info2
							(trainingday_id,trainingday_exercise_idx,type,number,reps,weight,weight_unit,subsets,resttime,notes) VALUES(?,?,?,?,?,?,?,?,?,?)",
							[row.trainingday_id, row.trainingday_exercise_idx, row.type, row.number,
							row.reps, row.weight, "kg(s)", row.subsets, row.resttime, row.notes]);
				});
			}
		});

		removeSetsInfoTable();
		db.transaction(function (tx) {
			tx.executeSql("ALTER TABLE `set_info2` RENAME TO `set_info`");
		});
	}*/

	function createTrainingDayTable() {
		db.transaction(function (tx) {
			tx.executeSql(`CREATE TABLE IF NOT EXISTS training_day (
				id INTEGER PRIMARY KEY AUTOINCREMENT,
				date INTEGER,
				meso_id INTEGER,
				exercises TEXT,
				day_number INTEGER,
				split_letter TEXT,
				time_in TEXT,
				time_out TEXT,
				location TEXT,
				notes TEXT
			)`);
		});
	}

	function getMesos() {
		let mesos = [];
		db.transaction(function (tx) {
			let results = tx.executeSql("SELECT * FROM mesocycles");
			for (let i = 0; i < results.rows.length; i++) {
				var bRealMeso = results.rows.item(i).meso_end_date !== 0;
				mesos.push({
					"mesoId": results.rows.item(i).meso_id,
					"mesoName": results.rows.item(i).meso_name,
					"mesoStartDate": new Date(results.rows.item(i).meso_start_date),
					"mesoEndDate": bRealMeso ? new Date(results.rows.item(i).meso_end_date) : new Date(),
					"mesoNote": results.rows.item(i).meso_note,
					"nWeeks": results.rows.item(i).meso_nweeks,
					"mesoSplit": results.rows.item(i).meso_split,
					"mesoDrugs": results.rows.item(i).meso_drugs,
					"realMeso": bRealMeso
				});
			}
		});
		return mesos;
	}

	function getMesoInfo(mesoId) {
		let mesoinfo = [];
		db.transaction(function (tx) {
			let results = tx.executeSql("SELECT * FROM mesocycles WHERE meso_id=?", [mesoId]);
			const bRealMeso = results.rows.item(0).meso_end_date !== 0;
			mesoinfo.push({
					"mesoId": mesoId,
					"mesoName": results.rows.item(0).meso_name,
					"mesoStartDate": results.rows.item(0).meso_start_date,
					"mesoEndDate": bRealMeso ? new Date(results.rows.item(0).meso_end_date) : new Date(),
					"mesoNote": results.rows.item(0).meso_note,
					"nWeeks": results.rows.item(0).meso_nweeks,
					"mesoSplit": results.rows.item(0).meso_split,
					"mesoDrugs": results.rows.item(0).meso_drugs,
					"realMeso": bRealMeso
			});
		});
		return mesoinfo;
	}

	function getPreviousMesoEndDate(mesoId) {
		var nrecs;
		db.transaction(function (tx) { //Unless the table is deleted, meso_id gets incremented every time a new record is saved
			var res = tx.executeSql("SELECT count(*) as cnt FROM mesocycles where meso_id>=1");
			nrecs = res.rows.item(0).cnt;
		});

		var enddate;
		if (nrecs <=1) // 1 or 0 meso records = no previous meso. The first meso can start anywhere in 2023
			enddate = new Date(2023, 0, 2);
		else {
			var prev_meso_id = mesoId - 1;
			var query_res;
			db.transaction(function (tx) {
				while (prev_meso_id > 0) {
					query_res = tx.executeSql("SELECT meso_end_date FROM mesocycles WHERE meso_id=?",[prev_meso_id]);
					if (query_res.rows.length === 1) {
						enddate = new Date(query_res.rows.item(0).meso_end_date);
						break;
					}
					prev_meso_id--;
				}
			});
		}
		return enddate;
	}

	function getNextMesoStartDate(mesoId) {
		var startdate;
		var next_meso_id = mesoId + 1;
		let query_res;
		db.transaction(function (tx) {
			query_res = tx.executeSql("SELECT meso_end_date,meso_id,MAX(meso_id) FROM mesocycles");
		});
		let max_id = -1;
		if (query_res.rows.length === 1)
			max_id = query_res.rows.item(0).meso_id;

		if (mesoId === max_id) { //This is the most current meso. The cut off date for it is undetermined. So we set a value that is 6 months away
			startdate = JSF.createFutureDate(new Date(query_res.rows.item(0).meso_end_date), 0, 6, 0);
		}
		else {
			db.transaction(function (tx) {
				while (next_meso_id < max_id) {
					let results = tx.executeSql("SELECT meso_start_date FROM mesocycles WHERE meso_id=?",[next_meso_id]);
					if (results.rows.length === 1) {
						startdate = new Date(results.rows.item(0).meso_start_date);
						break;
					}
					next_meso_id++;
				}
			});
		}
		return startdate;
	}

	function getLastMesoEndDate() {
		var date;
		let results;
		db.transaction(function (tx) {
			results = tx.executeSql("SELECT meso_end_date,MAX(meso_id) FROM mesocycles");
			if (results.rows.item(0).meso_end_date)
				date = new Date(results.rows.item(0).meso_end_date);
			else
				date = new Date();
		});
		return date;
	}

	function newMeso(mesoName, mesoStartDate, mesoEndDate, mesoNote, nWeeks, mesoSplit, mesoDrugs) {
		let results;
		db.transaction(function (tx) {
			results = tx.executeSql("INSERT INTO mesocycles
							(meso_name,meso_start_date,meso_end_date,meso_note,meso_nweeks,meso_split,meso_drugs) VALUES(?,?,?,?,?,?,?)",
							[mesoName, mesoStartDate, mesoEndDate, mesoNote, nWeeks, mesoSplit, mesoDrugs]);
		});
		return results;
	}

	function updateMeso(mesoId, mesoName, mesoStartDate, mesoEndDate, mesoNote, nWeeks, mesoSplit, mesoDrugs) {
		db.transaction(function (tx) {
			tx.executeSql("UPDATE mesocycles set meso_name=?,meso_start_date=?,meso_end_date=?,meso_note=?,meso_nweeks=?,
							meso_split=?,meso_drugs=? WHERE meso_id=?",
							[mesoName, mesoStartDate, mesoEndDate, mesoNote, nWeeks, mesoSplit, mesoDrugs, mesoId]);
		});
	}

	function deleteMeso(mesoId) {
		db.transaction(function (tx) {
			tx.executeSql("DELETE FROM mesocycles WHERE meso_id=?", [mesoId]);
		});
		deleteMesoDivision(mesoId);
		deleteMesoCalendar(mesoId);
	}

	function removeMesoTable() {
		db.transaction(function (tx) {
			tx.executeSql("DROP TABLE mesocycles");
		});
	}

	function getDivisionForMeso(mesoId) {
		let mesodivision = [];
		db.transaction(function (tx) {
			let results = tx.executeSql("SELECT division_id,meso_id,splitA,splitB,splitC,splitD,splitE,splitF FROM mesocycle_division where meso_id=?", [mesoId]);
			for (let i = 0; i < results.rows.length; i++) {
				let divisionRow = results.rows.item(i);
				mesodivision.push({
					"divisionId": divisionRow.division_id,
					"mesoId": divisionRow.meso_id,
					"splitA": divisionRow.splitA,
					"splitB": divisionRow.splitB,
					"splitC": divisionRow.splitC,
					"splitD": divisionRow.splitD,
					"splitE": divisionRow.splitE,
					"splitF": divisionRow.splitF,
				});
			}
		});
		return mesodivision;
	}

	function getDivisionIdForMeso(mesoId) {
		var divisionid = -1;
		db.transaction(function (tx) {
			let results = tx.executeSql("SELECT division_id FROM mesocycle_division where meso_id=?", [mesoId]);
			if (results.rows.length > 0)
				divisionid = results.rows.item(0).division_id;
		});
		return divisionid
	}

	function getExercisesFromDivisionAForMeso(mesoId) {
		var result = "";
		db.transaction(function (tx) {
			let results = tx.executeSql("SELECT splitA_exercisesnames FROM mesocycle_division WHERE meso_id=?", [mesoId]);
			if (results.rows.length > 0)
				result = results.rows.item(0).splitA_exercisesnames;
		});
		return result;
	}

	function getCompleteDivisionAForMeso(mesoId) {
		let mesodivision = [];
		db.transaction(function (tx) {
			let results = tx.executeSql("SELECT division_id,splitA,splitA_exercisesnames,splitA_exercisesset_types,splitA_exercisesset_n,
					splitA_exercisesset_reps,splitA_exercisesset_weight FROM mesocycle_division WHERE meso_id=?", [mesoId]);
			for (let i = 0; i < results.rows.length; i++) {
				let divisionRow = results.rows.item(i);
				mesodivision.push({
					"divisionId": divisionRow.division_id,
					"splitLetter": "A",
					"splitText": divisionRow.splitA,
					"splitExercises": divisionRow.splitA_exercisesnames,
					"splitSetTypes": divisionRow.splitA_exercisesset_types,
					"splitNSets": divisionRow.splitA_exercisesset_n,
					"splitNReps": divisionRow.splitA_exercisesset_reps,
					"splitNWeight": divisionRow.splitA_exercisesset_weight
				});
			}
		});
		return mesodivision;
	}

	function getExercisesFromDivisionBForMeso(mesoId) {
		var result = "";
		db.transaction(function (tx) {
			let results = tx.executeSql("SELECT splitB_exercisesnames FROM mesocycle_division WHERE meso_id=?", [mesoId]);
			if (results.rows.length > 0)
				 result = results.rows.item(0).splitB_exercisesnames;
		});
		return result;
	}

	function getCompleteDivisionBForMeso(mesoId) {
		let mesodivision = [];
		db.transaction(function (tx) {
			let results = tx.executeSql("SELECT division_id,splitB,splitB_exercisesnames,splitB_exercisesset_types,splitB_exercisesset_n,
				splitB_exercisesset_reps,splitB_exercisesset_weight FROM mesocycle_division WHERE meso_id=?", [mesoId]);
			for (let i = 0; i < results.rows.length; i++) {
				let divisionRow = results.rows.item(i);
				mesodivision.push({
					"divisionId": divisionRow.division_id,
					"splitLetter": "B",
					"splitText": divisionRow.splitB,
					"splitExercises": divisionRow.splitB_exercisesnames,
					"splitSetTypes": divisionRow.splitB_exercisesset_types,
					"splitNSets": divisionRow.splitB_exercisesset_n,
					"splitNReps": divisionRow.splitB_exercisesset_reps,
					"splitNWeight": divisionRow.splitB_exercisesset_weight
				});
			}
		});
		return mesodivision;
	}

	function getExercisesFromDivisionCForMeso(mesoId) {
		var result = "";
		db.transaction(function (tx) {
			let results = tx.executeSql("SELECT splitC_exercisesnames FROM mesocycle_division WHERE meso_id=?", [mesoId]);
			if (results.rows.length > 0)
				result = results.rows.item(0).splitC_exercisesnames;
		});
		return result;
	}

	function getCompleteDivisionCForMeso(mesoId) {
		let mesodivision = [];
		db.transaction(function (tx) {
			let results = tx.executeSql("SELECT division_id,splitC,splitC_exercisesnames,splitC_exercisesset_types,splitC_exercisesset_n,
				splitC_exercisesset_reps,splitC_exercisesset_weight FROM mesocycle_division WHERE meso_id=?", [mesoId]);
			for (let i = 0; i < results.rows.length; i++) {
				let divisionRow = results.rows.item(i);
				mesodivision.push({
					"divisionId": divisionRow.division_id,
					"splitLetter": "C",
					"splitText": divisionRow.splitC,
					"splitExercises": divisionRow.splitC_exercisesnames,
					"splitSetTypes": divisionRow.splitC_exercisesset_types,
					"splitNSets": divisionRow.splitC_exercisesset_n,
					"splitNReps": divisionRow.splitC_exercisesset_reps,
					"splitNWeight": divisionRow.splitC_exercisesset_weight
				});
			}
		});
		return mesodivision;
	}

	function getExercisesFromDivisionDForMeso(mesoId) {
		var result = "";
		db.transaction(function (tx) {
			let results = tx.executeSql("SELECT splitD_exercisesnames FROM mesocycle_division WHERE meso_id=?", [mesoId]);
			if (results.rows.length > 0)
				result = results.rows.item(0).splitD_exercisesnames;
		});
		return result;
	}

	function getCompleteDivisionDForMeso(mesoId) {
		let mesodivision = [];
		db.transaction(function (tx) {
			let results = tx.executeSql("SELECT division_id,splitD,splitD_exercisesnames,splitD_exercisesset_types,splitD_exercisesset_n,
				splitD_exercisesset_reps,splitD_exercisesset_weight FROM mesocycle_division WHERE meso_id=?", [mesoId]);
			for (let i = 0; i < results.rows.length; i++) {
				let divisionRow = results.rows.item(i);
				mesodivision.push({
					"divisionId": divisionRow.division_id,
					"splitLetter": "D",
					"splitText": divisionRow.splitD,
					"splitExercises": divisionRow.splitD_exercisesnames,
					"splitSetTypes": divisionRow.splitD_exercisesset_types,
					"splitNSets": divisionRow.splitD_exercisesset_n,
					"splitNReps": divisionRow.splitD_exercisesset_reps,
					"splitNWeight": divisionRow.splitD_exercisesset_weight
				});
			}
		});
		return mesodivision;
	}

	function getExercisesFromDivisionEForMeso(mesoId) {
		var result = "";
		db.transaction(function (tx) {
			let results = tx.executeSql("SELECT splitE_exercisesnames FROM mesocycle_division WHERE meso_id=?", [mesoId]);
			if (results.rows.length > 0)
				result = results.rows.item(0).splitE_exercisesnames;
		});
		return result;
	}

	function getCompleteDivisionEForMeso(mesoId) {
		let mesodivision = [];
		db.transaction(function (tx) {
			let results = tx.executeSql("SELECT division_id,splitE,splitE_exercisesnames,splitE_exercisesset_types,splitE_exercisesset_n,
				splitE_exercisesset_reps,splitE_exercisesset_weight FROM mesocycle_division WHERE meso_id=?", [mesoId]);
			for (let i = 0; i < results.rows.length; i++) {
				let divisionRow = results.rows.item(i);
				mesodivision.push({
					"divisionId": divisionRow.division_id,
					"splitLetter": "E",
					"splitText": divisionRow.splitE,
					"splitExercises": divisionRow.splitE_exercisesnames,
					"splitSetTypes": divisionRow.splitE_exercisesset_types,
					"splitNSets": divisionRow.splitE_exercisesset_n,
					"splitNReps": divisionRow.splitE_exercisesset_reps,
					"splitNWeight": divisionRow.splitE_exercisesset_weight
				});
			}
		});
		return mesodivision;
	}

	function getExercisesFromDivisionFForMeso(mesoId) {
		var result = "";
		db.transaction(function (tx) {
			let results = tx.executeSql("SELECT splitF_exercisesnames FROM mesocycle_division WHERE meso_id=?", [mesoId]);
			if (results.rows.length > 0)
				result = results.rows.item(0).splitF_exercisesnames;
		});
		return result;
	}

	function getCompleteDivisionFForMeso(mesoId) {
		let mesodivision = [];
		db.transaction(function (tx) {
			let results = tx.executeSql("SELECT division_id,splitF,splitF_exercisesnames,splitF_exercisesset_types,splitF_exercisesset_n,
				splitF_exercisesset_reps,splitF_exercisesset_weight FROM mesocycle_division WHERE meso_id=?", [mesoId]);
			for (let i = 0; i < results.rows.length; i++) {
				let divisionRow = results.rows.item(i);
				mesodivision.push({
					"divisionId": divisionRow.division_id,
					"splitLetter": "F",
					"splitText": divisionRow.splitF,
					"splitExercises": divisionRow.splitF_exercisesnames,
					"splitSetTypes": divisionRow.splitF_exercisesset_types,
					"splitNSets": divisionRow.splitF_exercisesset_n,
					"splitNReps": divisionRow.splitF_exercisesset_reps,
					"splitNWeight": divisionRow.splitF_exercisesset_weight
				});
			}
		});
		return mesodivision;
	}

	function updateMesoDivisionComplete(id, split, splitgroup, exercises, types, nsets, nreps, nweights) {
		db.transaction(function (tx) {
			tx.executeSql("UPDATE mesocycle_division SET split" + split + "=?, split" + split + "_exercisesnames=?, split" +
			split + "_exercisesset_types=?, split" + split + "_exercisesset_n=?, split" + split + "_exercisesset_reps=?, split" +
			split + "_exercisesset_weight=? WHERE division_id=?",
			[splitgroup, exercises, types, nsets, nreps, nweights, id]);
		});
	}

	function updateMesoDivision_OnlyExercises(id, split, exercises, types, nsets, nreps, nweights) {
		db.transaction(function (tx) {
			tx.executeSql("UPDATE mesocycle_division SET split" + split + "_exercisesnames=?, split" +
			split + "_exercisesset_types=?, split" + split + "_exercisesset_n=?, split" + split + "_exercisesset_reps=?, split" +
			split + "_exercisesset_weight=? WHERE division_id=?",
			[exercises, types, nsets, nreps, nweights, id]);
		});
	}

	function newMesoDivision(mesoId, splitA, splitB, splitC, splitD, splitE, splitF) {
		let results;
		db.transaction(function (tx) {
			results = tx.executeSql("INSERT INTO mesocycle_division
							(meso_id,splitA,splitB,splitC,splitD,splitE,splitF) VALUES(?,?,?,?,?,?,?)",
							[mesoId, splitA, splitB, splitC, splitD, splitE, splitF]);
		});
		return results;
	}

	function updateMesoDivision(mesoID, splitFieldA, splitFieldB, splitFieldC, splitFieldD, splitFieldE, splitFieldF) {
		db.transaction(function (tx) {
			tx.executeSql("UPDATE mesocycle_division SET splitA=?,splitB=?,splitC=?,splitD=?,splitE=?,splitA=F WHERE meso_id=?",
			[splitFieldA,splitFieldB,splitFieldC,splitFieldD,splitFieldE,splitFieldF, mesoID]);
		});
	}

	function deleteMesoDivision(mesoId) {
		db.transaction(function (tx) {
			tx.executeSql("DELETE FROM mesocycle_division WHERE meso_id=?", [mesoId]);
		});
	}

	function removeMesoDivisionTable() {
		db.transaction(function (tx) {
			tx.executeSql("DROP TABLE mesocycle_division");
		});
	}

	function createMesocycleCalendar() {
		db.transaction(function (tx) {
			tx.executeSql(`CREATE TABLE IF NOT EXISTS mesocycle_calendar (
						  id INTEGER PRIMARY KEY AUTOINCREMENT,
						  meso_id INTEGER,
						  cal_date INTEGER,
						  training_day INTEGER,
						  training_split TEXT
			)`);
		});
	}

	function getMesoCalendar(mesoId) {
		let calendar = [];
		db.transaction(function (tx) {
			let results = tx.executeSql("SELECT * FROM mesocycle_calendar where meso_id=?", [mesoId]);
			for (let i = 0; i < results.rows.length; i++) {
				let row = results.rows.item(i);
				calendar.push({
					"mesoCalId": row.id,
					"mesoCalMesoId": row.meso_id,
					"mesoCalDate": row.cal_date,
					"mesoCalnDay": row.training_day,
					"mesoCalSplit": row.training_split
				});
			}
		});
		return calendar;
	}

	function getMesoCalendarDate(date) {
		let calendarDate = [];
		var calDate = date.getTime();
		db.transaction(function (tx) {
			let results = tx.executeSql("SELECT * FROM mesocycle_calendar where cal_date=?", [calDate]);
			for (let i = 0; i < results.rows.length; i++) {
				let row = results.rows.item(i);
				calendarDate.push({
					"mesoCalId": row.id,
					"mesoCalMesoId": row.meso_id,
					"mesoCalDate": row.cal_date,
					"mesoCalnDay": row.training_day,
					"mesoCalSplit": row.training_split
				});
			}
		});
		return calendarDate;
	}

	function getPartialMesoCalendar_From(cal_id) {
		let calendarDate = [];
		db.transaction(function (tx) {
			let results = tx.executeSql("SELECT * FROM mesocycle_calendar where id>=?", [cal_id]);
			for (let i = 0; i < results.rows.length; i++) {
				let row = results.rows.item(i);
				calendarDate.push({
					"mesoCalId": row.id,
					"mesoCalMesoId": row.meso_id,
					"mesoCalDate": row.cal_date,
					"mesoCalnDay": row.training_day,
					"mesoCalSplit": row.training_split
				});
			}
		});
		return calendarDate;
	}

	function newMesoCalendarEntry(mesoId, calDate, calnDay, calSplit) {
		let results;
		db.transaction(function (tx) {
			results = tx.executeSql("INSERT INTO mesocycle_calendar
							(meso_id, cal_date, training_day, training_split) VALUES(?,?,?,?)",
							[mesoId, calDate, calnDay, calSplit]);
		});
		return results;
	}

	function updateMesoCalendarDate(calId, newDate) {
		db.transaction(function (tx) {
			tx.executeSql("UPDATE mesocycle_calendar set cal_date=? WHERE id=?", [newDate, calId]);
		});
	}

	function updateMesoCalendarTrainingDay(calId, newNDay) {
		db.transaction(function (tx) {
			tx.executeSql("UPDATE mesocycle_calendar set training_day=? WHERE id=?", [newNDay, calId]);
		});
	}

	function updateMesoCalendarDaySplit(calId, splitLetter) {
		db.transaction(function (tx) {
			tx.executeSql("UPDATE mesocycle_calendar set training_split=? WHERE id=?", [splitLetter, calId]);
		});
	}

	function checkIfCalendarForMesoExists(mesoId) {
		var ok = false;
		db.transaction(function (tx) {
			var res = tx.executeSql("SELECT count(*) as cnt FROM mesocycle_calendar where meso_id=?", [mesoId]);
			if (res.rows.item(0).cnt > 0)
				ok = true;
		});
		return ok;
	}

	//TODO do not delete entries, just change meso_id to -1
	function deleteMesoCalendar(mesoId) {
		db.transaction(function (tx) {
			tx.executeSql("DELETE FROM mesocycle_calendar WHERE meso_id=?", [mesoId]);
		});
	}

	function removeMesoCalendarTable() {
		db.transaction(function (tx) {
			tx.executeSql("DROP TABLE mesocycle_calendar");
		});
	}

	function removeUnwantedTables() {
		db.transaction(function (tx) {
			tx.executeSql("DROP TABLE exercise");
		});
		db.transaction(function (tx) {
			tx.executeSql("DROP TABLE day_info");
		});
		db.transaction(function (tx) {
			tx.executeSql("DROP TABLE weeks");
		});
	}

	function getExercises() {
		let exercises = [];
		db.transaction(function (tx) {
			let results = tx.executeSql("SELECT * FROM exercises_table");
			for (let i = 0; i < results.rows.length; i++) {
				let row = results.rows.item(i);
				if (row.id > 1000)
					exercisesTableLastId = row.id;
				exercises.push({
					"exerciseId": row.id,
					"mainName": row.primary_name,
					"subName": row.secondary_name,
					"muscularGroup": row.muscular_group,
					"nSets": row.sets,
					"nReps": row.reps,
					"nWeight": row.weight,
					"uWeight": row.weight_unit,
					"mediaPath": row.media_path,
					"actualIndex": i
				});
			}
		});
		return exercises;
	}

	function newExercise(mainName, subName, muscularGroup, nSets, nReps, nWeight, uWeight, mediaPath) {
		let results;
		db.transaction(function (tx) {
			results = tx.executeSql("INSERT INTO exercises_table
							(id,primary_name,secondary_name,muscular_group,sets,reps,weight,weight_unit,media_path,from_list) VALUES(?,?,?,?,?,?,?,?,?,?)",
							[++exercisesTableLastId, mainName, subName, muscularGroup, nSets, nReps, nWeight, uWeight, mediaPath,0]);
		});
		return results;
	}

	function updateExercise(exerciseId, mainName, subName, muscularGroup, nSets, nReps, nWeight, mediaPath) {
		let results;
		db.transaction(function (tx) {
			results = tx.executeSql("UPDATE exercises_table SET primary_name=?, secondary_name=?, muscular_group=?,
									sets=?, reps=?, weight=?, media_path=? WHERE id=?",
									[mainName, subName, muscularGroup, nSets, nReps, nWeight, mediaPath, exerciseId]);
		});
		return results;
	}

	function updateExerciseMainName(exerciseId, newName) {
		db.transaction(function (tx) {
			tx.executeSql("UPDATE exercises_table set primary_name=? WHERE id=?", [newName, exerciseId]);
		});
	}

	function updateExerciseSubName(exerciseId, newName) {
		db.transaction(function (tx) {
			tx.executeSql("UPDATE exercises_table set secondary_name=? WHERE id=?", [newName, exerciseId]);
		});
	}

	function updateExerciseMuscularGroup(exerciseId, newGroup) {
		db.transaction(function (tx) {
			tx.executeSql("UPDATE exercises_table set muscular_group=? WHERE id=?", [newGroup, exerciseId]);
		});
	}

	function updateExerciseSets(exerciseId, nSets) {
		db.transaction(function (tx) {
			tx.executeSql("UPDATE exercises_table set sets=? WHERE id=?", [nSets, exerciseId]);
		});
	}

	function updateExerciseReps(exerciseId, nReps) {
		db.transaction(function (tx) {
			tx.executeSql("UPDATE exercises_table set reps=? WHERE id=?", [nReps, exerciseId]);
		});
	}

	function updateExerciseWeight(exerciseId, nWeight) {
		db.transaction(function (tx) {
			tx.executeSql("UPDATE exercises_table set weight=? WHERE id=?", [nWeight, exerciseId]);
		});
	}

	function updateExerciseWeightUnit(exerciseId, uWeight) {
		db.transaction(function (tx) {
			tx.executeSql("UPDATE exercises_table set weight_unit=? WHERE id=?", [uWeight, exerciseId]);
		});
	}

	function updateExerciseMediaPath(exerciseId, mediaPath) {
		db.transaction(function (tx) {
			tx.executeSql("UPDATE exercises_table set media_path=? WHERE id=?", [mediaPath, exerciseId]);
		});
	}

	function deleteExerciseFromExercises(exerciseId) {
		db.transaction(function (tx) {
			tx.executeSql("DELETE FROM exercises_table WHERE id=?", [exerciseId]);
		});
	}

	function removeExercisesTable() {
		db.transaction(function (tx) {
			tx.executeSql("DROP TABLE exercises_table");
		});
	}

	function getExercise(exerciseId) {
		if (!exerciseId)
			return;

		let exercise = [];
		db.transaction(function (tx) {
			let results = tx.executeSql("SELECT * FROM exercises_table WHERE id = " + exerciseId);
			for (let i = 0; i < results.rows.length; i++) {
				let row = results.rows.item(i);

				exercise.push({
					"exerciseId": row.id,
					"exercisePName": row.primary_name,
					"exerciseSName": row.secondary_name,
					"exerciseMuscularGroup": row.muscular_group,
					"exerciseSets": row.sets,
					"exerciseReps": row.reps,
					"exerciseWeight": row.weight,
					"exerciseUnitWeight": row.weight_unit,
					"exerciseNotes": row.notes,
					"exerciseMedia": row.media
				});
			}
		});
		return exercise;
	}

	function getTrainingDay(date) {
		let dayinfo = [];
		db.transaction(function (tx) {
			let results = tx.executeSql("SELECT * FROM training_day WHERE date = " + date);
			for (let i = 0; i < results.rows.length; i++) {
				let row = results.rows.item(i);
				dayinfo.push({
					"dayId": row.id,
					"dayDate": row.date,
					"mesoId": row.meso_id,
					"exercisesNames": row.exercises,
					"dayNumber": row.day_number,
					"daySplitLetter": row.split_letter,
					"dayTimeIn": row.time_in,
					"dayTimeOut": row.time_out,
					"dayLocation": row.location,
					"dayNotes": row.notes
				});
			}
		});
		return dayinfo;
	}

	function getAllTrainingDays(mesoId) {
		let dayinfo = [];
		db.transaction(function (tx) {
			let results = tx.executeSql("SELECT * FROM training_day WHERE meso_id = " + mesoId);
			for (let i = 0; i < results.rows.length; i++) {
				let row = results.rows.item(i);
					console.info();
					console.info();
					console.log("dayId    ", row.id);
					console.log("dayDate   ", new Date(row.date).toDateString());
					console.log("mesoId   ", row.meso_id);
					console.log("exercisesNames   ", row.exercises);
					console.log("dayNumber   ", row.day_number);
					console.log("daySplitLetter   ", row.split_letter);
					console.log("dayTimeIn   ", row.time_in);
					console.log("dayTimeOut   ", row.time_out);
					console.log("dayLocation   ", row.location);
					console.log("dayNotes   ", row.notes);
					console.info();
					console.info();
			}
		});
		return dayinfo;
	}

	function newTrainingDay(date, mesoId, exercisesNames, dayNumber, splitLetter, timeIn, timeOut, location, notes) {
		let results;
		db.transaction(function (tx) {
			results = tx.executeSql("INSERT INTO training_day
							(date,meso_id,exercises,day_number,split_letter,time_in,time_out,location,notes) VALUES(?,?,?,?,?,?,?,?,?)",
							[date,mesoId,exercisesNames,dayNumber,splitLetter,timeIn,timeOut,location,notes]);
		});
		return results;
	}

	function updateTrainingDay(id, exercisesNames, dayNumber, splitLetter, timeIn, timeOut, location, notes) {
		db.transaction(function (tx) {
			tx.executeSql("UPDATE training_day set exercises=?,day_number=?,split_letter=?,time_in=?,time_out=?,location=?, notes=? WHERE id=?",
			[exercisesNames, dayNumber, splitLetter, timeIn, timeOut, location, notes, id]);
		});
	}

	function updateTrainingDay_MesoId(id, newMesoId) {
		db.transaction(function (tx) {
			tx.executeSql("UPDATE training_day set meso_id=? WHERE id=?", [newMesoId, id]);
		});
	}

	function deleteTraingDay(id) {
		db.transaction(function (tx) {
			tx.executeSql("DELETE FROM training_day WHERE id=?", [id]);
		});
	}

	function removeTrainingDayTable() {
		db.transaction(function (tx) {
			tx.executeSql("DROP TABLE training_day");
		});
	}

	function alterTableTrainingDay() {
		db.transaction(function (tx) {
			tx.executeSql("ALTER TABLE training_day ADD notes TEXT");
		});
	}

	function isTrainingDayTableEmpty(mesoId) {
		var ok = true;
		db.transaction(function (tx) {
			var res = tx.executeSql("SELECT count(*) as cnt FROM training_day where meso_id=?", [mesoId]);
			if (res.rows.item(0).cnt > 0)
				ok = false;
		});
		return ok;
	}

	function getPreviousTrainingDayForDivision(division, tday, mesoId) {
		let dayinfo = [];
		db.transaction(function (tx) {
			let results = tx.executeSql("SELECT * FROM training_day WHERE meso_id=? AND split_letter=? AND day_number<?", [mesoId, division, tday]);
			const len = results.rows.length;
			if(len > 0) {
				var i = len - 1; //array insertion order: most recent to least recent
				do {
					console.log("getPreviousTrainingDayForDivision. Found day number:   ", results.rows.item(i).day_number);
					let row = results.rows.item(i);
					dayinfo.push({
						"dayId": row.id,
						"dayDate": row.date,
						"mesoId": row.meso_id,
						"exercisesNames": row.exercises,
						"dayNumber": row.day_number,
						"daySplitLetter": row.split_letter,
						"dayTimeIn": row.time_in,
						"dayTimeOut": row.time_out,
						"dayLocation": row.location,
						"dayNotes": row.notes
					});
				} while (--i >= 0);
			}
		});
		return dayinfo;
	}

	function getMostRecentTrainingDay() {
		let dayinfo = [];
		db.transaction(function (tx) {
			let results = tx.executeSql("SELECT *,MAX(id) FROM training_day");
			if(results.rows.length > 0) {
				let row = results.rows.item(0);
				dayinfo.push({
					"dayId": row.id,
					"dayDate": row.date,
					"mesoId": row.meso_id,
					"exercisesNames": row.exercises,
					"dayNumber": row.day_number,
					"daySplitLetter": row.split_letter,
					"dayTimeIn": row.time_in,
					"dayTimeOut": row.time_out,
					"dayLocation": row.location,
					"dayNotes": row.notes
				});
			}
		});
		return dayinfo;
	}

	/*function getPreviousTrainingDayForDivision(division, tday, mesoId) {
		let dayinfo = [];
		db.transaction(function (tx) {
			let results = tx.executeSql("SELECT * FROM training_day");
			const len = results.rows.length;
			console.log(len);
			if(len > 0) {
				var i = len - 1;
				do {
					let row = results.rows.item(i);
					//if (row.exercises_ids) {
						//dayinfo.push({
						console.log("dayId   ", row.id);
						console.log("dayDate   ", new Date(row.date).toDateString());
						console.log("mesoId   ", row.meso_id);
						console.log("exercisesNames   ", row.exercises);
						console.log("dayNumber   ", row.day_number);
						console.log("daySplitLetter   ", row.split_letter);
						console.log("dayTimeIn   ", row.time_in);
						console.log("dayTimeOut   ", row.time_out);
						console.log("dayLocation   ", row.location);
						console.log("dayNotes   ", row.notes);
						//});
					//}
				} while (--i >= 0);
			}
		});
		return dayinfo;
	}*/

	function getSetsInfo(tdayId) {
		let setsInfo = [];
		db.transaction(function (tx) {
			let results = tx.executeSql("SELECT * FROM set_info WHERE trainingday_id = " + tdayId);
			for (let i = 0; i < results.rows.length; i++) {
				let row = results.rows.item(i);
				setsInfo.push({
					"setId": row.id,
					"setTrainingdDayId": row.trainingday_id,
					"setExerciseIdx": row.trainingday_exercise_idx,
					"setType": row.type,
					"setNumber": row.number,
					"setReps": row.reps,
					"setWeight": row.weight,
					"setWeightUnit": row.weight_unit,
					"setSubSets": row.subsets,
					"setRestTime": row.resttime,
					"setNotes": row.notes
				});
			}
		});
		return setsInfo;
	}

	function getAllSetsInfo() {
		db.transaction(function (tx) {
			let results = tx.executeSql("SELECT * FROM set_info");
			for (let i = 0; i < results.rows.length; i++) {
				let row = results.rows.item(i);
				console.info();
				console.info();
				console.log("setId   ", row.id);
				console.log("setTrainingdDayId   ", row.trainingday_id);
				console.log("setExerciseIdx   ", row.trainingday_exercise_idx);
				console.log("setType   ", row.type);
				console.log("setNumber   ", row.number);
				console.log("setReps   ", row.reps);
				console.log("setWeight   ", row.weight);
				console.log("setWeightUnit   ", row.weight_unit);
				console.log("setSubSets   ", row.subsets);
				console.log("setRestTime   ", row.resttime);
				console.log("setNotes   ", row.note);
				console.info();
				console.info();
			}
		});
	}

	function updateSetsInfoTable() {
		db.transaction(function (tx) {
			let results = tx.executeSql("SELECT reps,weight FROM set_info WHERE id >=0");
			var newreps = "", newweight = "";
			for (let i = 0; i < results.rows.length; i++) {
				let row = results.rows.item(i);
				if (row.reps.indexOf('|') !== -1) {
					newreps = row.reps.replace('|', '#');
					newreps = row.weight.replace('|', '#');

					db.transaction(function (ty) {
						ty.executeSql("UPDATE set_info set reps=?,weight=? WHERE id=?", [newreps, newweight,row.id]);
					});
				}
			}
		});
	}

	function newSetInfo(tDayId, exerciseIdx, setType, setNumber, setReps, setWeight, setWeightUnit, setSubSets, setRestTime, setNotes) {
		let results;
		db.transaction(function (tx) {
			results = tx.executeSql("INSERT INTO set_info
							(trainingday_id,trainingday_exercise_idx,type,number,reps,weight,weight_unit,subsets,resttime,notes) VALUES(?,?,?,?,?,?,?,?,?,?)",
							[tDayId, exerciseIdx, setType, setNumber, setReps, setWeight, setWeightUnit, setSubSets, setRestTime, setNotes]);
		});
		return results;
	}

	function updateSetInfo(setId, exerciseIdx, setNumber, setReps, setWeight, setSubSets, setRestTime, setNotes) {
		let results;
		db.transaction(function (tx) {
			results = tx.executeSql("UPDATE set_info set
						trainingday_exercise_idx=?, number=?, reps=?,weight=?, subsets=?, resttime=?, notes=? WHERE id=?",
						[exerciseIdx, setNumber, setReps, setWeight, setSubSets, setRestTime, setNotes, setId]);
		});
		return results;
	}

	function updateSetInfo_setExerciseId(setId, exerciseIdx) {
		db.transaction(function (tx) {
			tx.executeSql("UPDATE set_info set trainingday_exercise_idx=? WHERE id=?", [exerciseIdx, setId]);
		});
	}

	function updateSetInfo_setType(setId, setType) {
		db.transaction(function (tx) {
			tx.executeSql("UPDATE set_info set type=? WHERE id=?", [setType, setId]);
		});
	}

	function updateSetInfo_setNumber(setId, setNumber) {
		db.transaction(function (tx) {
			tx.executeSql("UPDATE set_info set number=? WHERE id=?", [setNumber, setId]);
		});
	}

	function updateSetInfo_setReps(setId, setReps) {
		db.transaction(function (tx) {
			tx.executeSql("UPDATE set_info set reps=? WHERE id=?", [setReps, setId]);
		});
	}

	function updateSetInfo_setWeight(setId, setWeight) {
		db.transaction(function (tx) {
			tx.executeSql("UPDATE set_info set weight=? WHERE id=?", [setWeight, setId]);
		});
	}

	function updateSetInfo_setWeightUnit(setId, setWeightUnit) {
		db.transaction(function (tx) {
			tx.executeSql("UPDATE set_info set weight_unit=? WHERE id=?", [setWeightUnit, setId]);
		});
	}

	function updateSetInfo_setSubSets(setId, setSubSets) {
		db.transaction(function (tx) {
			tx.executeSql("UPDATE set_info set subsets=? WHERE id=?", [setSubSets, setId]);
		});
	}

	function updateSetInfo_setRestTime(setId, setRestTime) {
		db.transaction(function (tx) {
			tx.executeSql("UPDATE set_info set resttime=? WHERE id=?", [setRestTime, setId]);
		});
	}

	function updateSetInfo_setNotes(setId, setNotes) {
		db.transaction(function (tx) {
			tx.executeSql("UPDATE set_info set notes=? WHERE id=?", [setNotes, setId]);
		});
	}

	//Called when the exercise name is edited and changed
	function updateSetInfo_Exercise(setId, setExerciseIdx) {
		db.transaction(function (tx) {
			tx.executeSql("UPDATE set_info set trainingday_exercise_idx=? WHERE id=?", [setExerciseIdx, setId]);
		});
	}

	//Called when a set is removed
	function deleteSetFromSetsInfo(setId) {
		db.transaction(function (tx) {
			tx.executeSql("DELETE FROM set_info WHERE id=?", [setId]);
		});
	}

	function deleteSets() {
		db.transaction(function (tx) {
			tx.executeSql("DELETE FROM set_info WHERE id>=213");
		});
	}

	//Called when an exercise is removed
	function deleteSetsForExercise(setExerciseIdx) {
		db.transaction(function (tx) {
			tx.executeSql("DELETE * FROM set_info WHERE trainingday_exercise_idx=?", [setExerciseIdx]);
		});
	}

	function removeSetsInfoTable() {
		db.transaction(function (tx) {
			tx.executeSql("DROP TABLE set_info");
		});
	}
}
