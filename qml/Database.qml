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
						createSetsInfoTable();
						createTrainingDayTable();
					}
					//console.log(res.rows.item(0).name)
			});
		} catch (error) {
			console.log("Error opening database: " + error);
		}
	}

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
				if (row.number < 0) {
					deleteSetFromSetsInfo(row.id);
					continue;
				}

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
