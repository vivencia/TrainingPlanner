import QtQuick
import QtQuick.Controls
import QtQuick.Shapes
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

	//Shape because Rectangle does not support diagonal gradient
	Shape {
		preferredRendererType: Shape.CurveRenderer

		ShapePath {
			strokeWidth: 0
			startX: 0
			startY: 0

			PathLine { x: appSettings.pageWidth; y: 0 }
			PathLine { x: appSettings.pageWidth; y: appSettings.pageHeight }
			PathLine { x: 0; y: appSettings.pageHeight }
			fillGradient: LinearGradient {
				x1: 0
				y1: appSettings.pageHeight / 4
				x2: appSettings.pageWidth
				y2:  appSettings.pageHeight / 4 * 3
				GradientStop { position: 0.0; color: "#31adff" }
				GradientStop { position: 1.0; color: "#d0e3ff" }
			}
		}
	}

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

			ListView {
				id: mesosList
				model: mesocyclesModel
				boundsBehavior: Flickable.StopAtBounds
				spacing: 1
				clip: true
				Layout.fillWidth: true
				Layout.minimumHeight: statisticsPage.height*0.08
				Layout.maximumHeight: statisticsPage.height*0.15

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
						color: index === mesocyclesModel.currentMesoIdx ? appSettings.primaryLightColor : appSettings.listEntryColor2
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
						color: index === mesocyclesModel.currentMesoIdx ? "black" : appSettings.fontColor
						topPadding: -3
					}
				}

				Component.onCompleted: {
					currentIndex = mesocyclesModel.currentMesoIdx;
					selectedMesoIdx = currentIndex;
					setDates();
					positionViewAtIndex(currentIndex, ListView.Center);
				}
			}

			Frame {
				spacing: 0
				padding: 0
				bottomInset: 0
				topInset: 0
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
							text: appUtils.formatDate(mesocyclesModel.startDate(selectedMesoIdx))
							readOnly: true
							width: parent.width*0.4

							CalendarDialog {
								id: caldlg
								showDate: mesocyclesModel.startDate(selectedMesoIdx)
								initDate: mesocyclesModel.startDate(selectedMesoIdx)
								finalDate: mesocyclesModel.endDate(selectedMesoIdx)
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
							text: appUtils.formatDate(mesocyclesModel.endDate(selectedMesoIdx))
							readOnly: true
							width: parent.width*0.4
							Layout.leftMargin: -10

							CalendarDialog {
								id: caldlg2
								showDate: mesocyclesModel.endDate(selectedMesoIdx)
								initDate: mesocyclesModel.startDate(selectedMesoIdx)
								finalDate: mesocyclesModel.endDate(selectedMesoIdx)
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
								mesocyclesModel.usedSplitsChanged.connect(function(meso_idx) {
								if (meso_idx === selectedMesoIdx)
									loadData();
								});
							}

							function loadData() {
								clear();
								const splits = mesocyclesModel.usedSplits(selectedMesoIdx);
								for(var i = 0; i < splits.length; ++i)
									append({ "text": splits[i] + ": " + mesocyclesModel.muscularGroup(selectedMesoIdx, splits[i]),
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

							function loadData() {
								clear();
								const exercises = appStatistics.exercisesList();
								for(var i = 0; i < exercises.length; ++i)
									append({ "text": exercises[i], "value": i });
							}
						}

						ColumnLayout {
							spacing: 5
							Layout.fillWidth: true

							TPCheckBox {
								Layout.fillWidth: true
								text: exercisesModel.count > 0 ? exercisesModel.get(index).text : ""
								checked: appStatistics ? appStatistics.exerciseIncluded(index) : false

								onClicked: {
									appStatistics.includeExercise(index, checked);
									//appStatistics.generateData();
								}
							}
						}
					}
				} //GridLayout
			} //Frame

			ChartView {
				id: chartView
				antialiasing: true
				height: statisticsPage.height*0.5
				Layout.fillWidth: true
				Layout.preferredHeight: height

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
					min: appSettings.xMin
					max: appSettings.xMax
				}

				ValueAxis {
					id: axisY
					min: appSettings.yMin
					max: appSettings.yMax
					labelFormat: "kg"
				}

				Component.onCompleted: {
					appStatistics.exercisesSeriesChanged.connect(changeSeries);
				}
			} //ChartView
		} //ColumnLayout: mainLayout
	} //ScroolView

	function setDates() {
		appStatistics.setStartDate(mesocyclesModel.startDate(selectedMesoIdx));
		appStatistics.setEndDate(mesocyclesModel.endDate(selectedMesoIdx));
	}

	function changeSeries(type) {
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
