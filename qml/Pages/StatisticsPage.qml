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
	property int dataSet: -1
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

	ColumnLayout {
		id: mainLayout
		spacing: 5

		anchors {
			fill: parent
			leftMargin: 5
			rightMargin: 5
			topMargin: 5
			bottomMargin: 5
		}

		ListView {
			id: mesosList
			model: mesocyclesModel
			boundsBehavior: Flickable.StopAtBounds
			spacing: 0
			height: parent.height*0.2
			Layout.fillWidth: true
			Layout.minimumHeight: height
			Layout.maximumHeight: height

			ScrollBar.vertical: ScrollBar {
				policy: ScrollBar.AsNeeded
				active: ScrollBar.AsNeeded
			}

			delegate: ItemDelegate {
				id: statsMesoDelegate
				width: mesosList.width
				height: mesosList.height*0.25

				background: Rectangle {
					id: backRec
					radius: 6
					color: index === mesocyclesModel.currentMesoIdx ? appSettings.primaryLightColor : appSettings.listEntryColor2
					opacity: 0.8
					anchors.fill: parent
				}

				onClicked: selectedMesoIdx = index;

				contentItem: Label {
					id: mesoNameLabel
					text: mesoName
					color: index === mesocyclesModel.currentMesoIdx ? "black" : appSettings.fontColor
				}
			}

			Component.onCompleted: {
				currentIndex = mesocyclesModel.currentMesoIdx;
				selectedMesoIdx = currentIndex;
				positionViewAtIndex(currentIndex, ListView.Center);
			}
		}

		Frame {
			height: parent.height*0.4
			spacing: 0
			padding: 0
			Layout.preferredHeight: height
			Layout.fillWidth: true

			ColumnLayout {
				spacing: 0

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

							onDateSelected: (date) => txtStartDate.text = appUtils.formatDate(date);
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

							onDateSelected: (date) => txtEndDate.text = appUtils.formatDate(date);
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

					model: ListModel {
						id: cboUsedSplitsModel

						Component.onCompleted: {
							mesocyclesModel.usedSplitsChanged.connect(function(meso_idx) {
							if (meso_idx === selectedMesoIdx)
								loadData();
							});
						}

						function loadData() {
							const splits = mesocyclesModel.usedSplits(selectedMesoIdx);
							for(var i = 0; i < splits.length; ++i)
								append({ "text": splits[i] + ": " + mesocyclesModel.muscularGroup(selectedMesoIdx, splits[i]),
										"value": splits[i], "enabled": true });
						}
					}

					width: parent.width
					Layout.fillWidth: true

					onActivated: (index) => {
						selectedMesoSplit = valueAt(index);
						dataSet = appStatistics.createDataSet(selectedMesoIdx, selectedMesoSplit);
					}
				} //TPComboBox

				Repeater {
					id: exercisesRepeater
					model: ListModel {
						id: exercisesModel

						Component.onCompleted: {
							appStatistics.exercisesListChanged(function(meso_idx, splitletter) {
							if (meso_idx === selectedMesoIdx && splitletter === selectedMesoSplit)
								loadData();
							});
						}

						function loadData() {
							const exercises = appStatistics.exercisesList();
							for(var i = 0; i < exercises.length; ++i)
								append({ "text": exercises[i], "value": i });
						}
					}

					width: parent.width
					Layout.fillWidth: true

					ColumnLayout {
						spacing: 0
						Layout.fillWidth: true

						TPCheckBox {
							Layout.fillWidth: true
							text: exercisesModel.get(index).text

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
			height: parent.height*0.5
			Layout.fillWidth: true
			Layout.preferredHeight: height

			ValueAxis {
				id: axisY1
				min: -1
				max: 4
			}

			ValueAxis {
				id: axisY2
				min: -10
				max: 5
			}

			ValueAxis {
				id: axisX
				min: 0
				max: 1024
			}

			Component.onCompleted: {
				changeSeriesType("line");
				appStatistics.update(chartView.series(0));
				appStatistics.update(chartView.series(1));
			}
		} //ChartView
	} //ColumnLayout: mainLayout

	function changeSeriesType(type) {
		chartView.removeAllSeries();

		// Create two new series of the correct type. Axis x is the same for both of the series,
		// but the series have their own y-axes to make it possible to control the y-offset
		// of the "signal sources".
		var series1
		var series2
		if (type === "line") {
			series1 = chartView.createSeries(ChartView.SeriesTypeLine, "signal 1",
											 axisX, axisY1);
			series1.useOpenGL = true;

			series2 = chartView.createSeries(ChartView.SeriesTypeLine, "signal 2",
											 axisX, axisY2);
			series2.useOpenGL = true;
		} else {
			series1 = chartView.createSeries(ChartView.SeriesTypeScatter, "signal 1",
											 axisX, axisY1);
			series1.markerSize = 2;
			series1.borderColor = "transparent";
			series1.useOpenGL = true;

			series2 = chartView.createSeries(ChartView.SeriesTypeScatter, "signal 2",
											 axisX, axisY2);
			series2.markerSize = 2;
			series2.borderColor = "transparent";
			series2.useOpenGL = true;
		}
	}

	function createAxis(min, max) {
		// The following creates a ValueAxis object that can be then set as a x or y axis for a series
		return Qt.createQmlObject("import QtQuick; import QtCharts; ValueAxis { min: "
								  + min + "; max: " + max + " }", chartView);
	}
}
