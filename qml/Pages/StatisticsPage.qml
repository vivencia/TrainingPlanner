import QtQuick
import QtQuick.Controls
import QtCharts
import QtQuick.Layouts

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import "../TPWidgets"
import "../Dialogs"
import ".."

TPPage {
	id: statisticsPage
	objectName: "statisticsPage"

	property int selectedMesoIdx: -1
	property string selectedMesoSplit

	onSelectedMesoIdxChanged: if (selectedMesoIdx >= 0) cboUsedSplitsModel.loadData();

	ScrollView {
		ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
		ScrollBar.vertical.policy: ScrollBar.AsNeeded
		contentWidth: availableWidth

		anchors {
			fill: parent
			leftMargin: 5
			rightMargin: 5
			topMargin: 5
			bottomMargin: 10
		}

		ColumnLayout {
			id: mainLayout
			spacing: 10
			anchors.fill: parent

			Rectangle {
				id: mesosRec
				radius: 8
				color: "#3F000000"
				Layout.fillWidth: true
				Layout.preferredHeight: statisticsPage.height*0.15

			ListView {
				id: mesosList
				model: mesocyclesModel
				boundsBehavior: Flickable.StopAtBounds
				spacing: 1
				clip: true
				anchors.fill: parent

				ScrollBar.vertical: ScrollBar {
					policy: ScrollBar.AsNeeded
					active: ScrollBar.AsNeeded
				}

				delegate: ItemDelegate {
					id: statsMesoDelegate
					width: mesosList.width
					height: 25

					background: Rectangle {
						id: backRec
						radius: 6
						color: index === itemManager.appMesocyclesModel.currentMesoIdx ? appSettings.primaryLightColor : appSettings.listEntryColor2
						opacity: 0.8
						anchors.fill: parent
					}

					onClicked: {
						selectedMesoIdx = index;
						setDates();
					}

					contentItem: Label {
						id: mesoNameLabel
						text: mesoName
						elide: Text.ElideRight
						color: index === itemManager.appMesocyclesModel.currentMesoIdx ? "black" : appSettings.fontColor
						topPadding: -3
					}
				}

				Component.onCompleted: {
					currentIndex = itemManager.appMesocyclesModel.currentMesoIdx;
					selectedMesoIdx = currentIndex;
					setDates();
					positionViewAtIndex(currentIndex, ListView.Center);
				}
			}
			}

			Frame {
				height: statisticsPage.height*0.45
				Layout.preferredHeight: height
				Layout.fillWidth: true

				background: Rectangle {
					color: "transparent"
				}

				ColumnLayout {
					spacing: 5

					anchors {
						fill: parent
						topMargin: 0
						leftMargin: 5
						rightMargin: 5
					}

					Row {
						spacing: 30
						padding: 0
						width: parent.width
						Layout.fillWidth: true

						TPLabel {
							text: qsTr("Initial date:")
							width: parent.width/2
						}
						TPLabel {
							text: qsTr("Final date:")
							width: parent.width/2
							Layout.leftMargin: -10
						}
					}

					Row {
						spacing: btnStartDate.width
						padding: 0
						width: parent.width
						Layout.fillWidth: true

						TPTextInput {
							id: txtStartDate
							text: appUtils.formatDate(itemManager.appMesocyclesModel.startDate(selectedMesoIdx))
							readOnly: true
							width: parent.width*0.4

							CalendarDialog {
								id: caldlg
								showDate: itemManager.appMesocyclesModel.startDate(selectedMesoIdx)
								initDate: itemManager.appMesocyclesModel.startDate(selectedMesoIdx)
								finalDate: itemManager.appMesocyclesModel.endDate(selectedMesoIdx)
								parentPage: statisticsPage

								onDateSelected: (date) => {
									txtStartDate.text = appUtils.formatDate(date);
									appStatistics.setStartDate(date);
								}
							}

							TPButton {
								id: btnStartDate
								imageSource: "calendar.png"
								width: 30
								height: 30
								fixedSize: true

								anchors {
									left: txtStartDate.right
									verticalCenter: txtStartDate.verticalCenter
								}

							onClicked: caldlg.open();
							}
						} //TPTextInput: txtStartDate

						TPTextInput {
							id: txtEndDate
							text: appUtils.formatDate(itemManager.appMesocyclesModel.endDate(selectedMesoIdx))
							readOnly: true
							width: parent.width*0.4
							Layout.leftMargin: -10

							CalendarDialog {
								id: caldlg2
								showDate: itemManager.appMesocyclesModel.endDate(selectedMesoIdx)
								initDate: itemManager.appMesocyclesModel.startDate(selectedMesoIdx)
								finalDate: itemManager.appMesocyclesModel.endDate(selectedMesoIdx)
								parentPage: statisticsPage

								onDateSelected: (date) => {
									txtEndDate.text = appUtils.formatDate(date);
									appStatistics.setEndDate(date);
								}
							}

							TPButton {
								id: btnEndDate
								imageSource: "calendar.png"
								width: 30
								height: 30
								fixedSize: true

								anchors {
									left: txtEndDate.right
									verticalCenter: txtEndDate.verticalCenter
								}

								onClicked: caldlg2.open();
							}
						} //TPTextInput: txtEndDate
					} //Row

					TPComboBox {
						id: cboUsedSplits
						width: parent.width
						Layout.fillWidth: true

						model: ListModel {
							id: cboUsedSplitsModel

							Component.onCompleted: {
								itemManager.appMesocyclesModel.usedSplitsChanged.connect(function(meso_idx) {
								if (meso_idx === selectedMesoIdx)
									loadData();
								});
							}

							function loadData(): void {
								clear();
								const splits = itemManager.appMesocyclesModel.usedSplits(selectedMesoIdx);
								for(let i = 0; i < splits.length; ++i)
									append({ "text": splits[i] + ": " + itemManager.appMesocyclesModel.muscularGroup(selectedMesoIdx, splits[i]),
											"value": splits[i], "enabled": true });
							}
						}

						onActivated: (index) => {
							selectedMesoSplit = valueAt(index);
							appStatistics.createDataSet(selectedMesoIdx, selectedMesoSplit);
						}
					} //TPComboBox

					Repeater {
						id: exercisesRepeater
						width: parent.width
						Layout.fillWidth: true

						model: ListModel {
							id: exercisesModel

							Component.onCompleted: {
								appStatistics.exercisesListChanged.connect(function(meso_idx, splitletter) {
								if (meso_idx === selectedMesoIdx && splitletter === selectedMesoSplit)
									loadData();
								});
							}

							function loadData(): void {
								clear();
								const exercises = appStatistics.exercisesList();
								for(let i = 0; i < exercises.length; ++i)
									append({ "text": exercises[i], "value": i });
							}
						}


						TPCheckBox {
							Layout.fillWidth: true
							text: exercisesModel.count > 0 ? exercisesModel.get(index).text : ""
							checked: appStatistics ? appStatistics.exerciseIncluded(index) : false

							onClicked: {
								appStatistics.includeExercise(index, checked);
								//appStatistics.generateData();
							}
						}
					} //Repeater
				}
			} //Frame

			ChartView {
				id: chartView
				antialiasing: true
				height: statisticsPage.height*0.5
				Layout.fillWidth: true
				Layout.preferredHeight: height
				Layout.topMargin: 30

				theme: {
					switch (appSettings.colorScheme) {
						case 0: //blue
							return ChartView.ChartThemeBlueIcy;
						case 3: //dark
							return ChartView.ChartThemeDark;
						case 4: //light
							return ChartView.ChartThemeLight;
						default:
							return ChartView.ChartThemeQt;
					}
				}

				ValueAxis {
					id: axisX
					min: appStatistics.xMin
					max: appStatistics.xMax
				}

				ValueAxis {
					id: axisY
					min: appStatistics.yMin
					max: appStatistics.yMax
					labelFormat: "kg"
				}

				Component.onCompleted: {
					appStatistics.exercisesSeriesChanged.connect(changeSeries);
				}
			} //ChartView
		} //ColumnLayout: mainLayout
	} //ScroolView

	function setDates() {
		appStatistics.setStartDate(itemManager.appMesocyclesModel.startDate(selectedMesoIdx));
		appStatistics.setEndDate(itemManager.appMesocyclesModel.endDate(selectedMesoIdx));
	}

	function changeSeries(type): void {
		if (appStatistics.exercisesSeries === 0)
			chartView.removeAllSeries();
		else {
			const nSeries = appStatistics.exercisesSeries;
			for (let i = 0; i < nSeries; ++i) {
				let series = chartView.createSeries(ChartView.SeriesTypeLine, appStatistics.exerciseName(i), axisX, axisY);
				series.useOpenGL = true;
				appStatistics.update(i, series);
			}
		}
	}
}
