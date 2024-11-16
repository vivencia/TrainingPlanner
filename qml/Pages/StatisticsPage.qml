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

				onClicked: mesocyclesModel.getMesocyclePage(index);

				contentItem: Label {
					id: mesoNameLabel
					text: mesoName
					color: index === mesocyclesModel.currentMesoIdx ? "black" : appSettings.fontColor
				}
			}

			Component.onCompleted: positionViewAtIndex(mesocyclesModel.currentMesoIdx, ListView.Center);
		}

		Frame {
			height: parent.height*0.3
			spacing: 0
			padding: 0
			Layout.preferredHeight: height
			Layout.fillWidth: true

			GridLayout {
				columns: 2
				rows: 5
				columnSpacing: 0
				rowSpacing: 0
				anchors {
					fill: parent
					topMargin: 0
					leftMargin: 5
					rightMargin: 5
				}

				TPLabel {
					text: qsTr("Date range:")
					Layout.row: 0
					Layout.column: 0
					Layout.columnSpan: 2
				}

				TPTextInput {
					id: txtStartDate
					text: appUtils.formatDate(mesocyclesModel.startDate(mesosList.currentIndex))
					readOnly: true
					width: parent.width*0.4
					Layout.preferredWidth: width
					Layout.row: 1
					Layout.column: 0

					CalendarDialog {
						id: caldlg
						showDate: mesocyclesModel.startDate(mesosList.currentIndex)
						initDate: mesocyclesModel.startDate(mesosList.currentIndex)
						finalDate: mesocyclesModel.endDate(mesosList.currentIndex)
						parentPage: statisticsPage

						onDateSelected: (date) => txtStartDate.text = appUtils.formatDate(date);
					}

					TPButton {
						id: btnStartDate
						imageSource: "calendar.png"

						anchors {
							left: txtStartDate.right
							leftMargin: -5
							verticalCenter: txtStartDate.verticalCenter
						}

						onClicked: caldlg.open();
					}
				}

				TPTextInput {
					id: txtEndDate
					text: appUtils.formatDate(mesocyclesModel.endDate(mesosList.currentIndex))
					readOnly: true
					width: parent.width*0.4
					Layout.preferredWidth: width
					Layout.row: 1
					Layout.column: 1

					CalendarDialog {
						id: caldlg2
						showDate: mesocyclesModel.endDate(mesosList.currentIndex)
						initDate: mesocyclesModel.startDate(mesosList.currentIndex)
						finalDate: mesocyclesModel.endDate(mesosList.currentIndex)
						parentPage: statisticsPage

						onDateSelected: (date) => txtEndDate.text = appUtils.formatDate(date);
					}

					TPButton {
						id: btnEndDate
						imageSource: "calendar.png"

						anchors {
							left: txtEndDate.right
							leftMargin: -5
							verticalCenter: txtEndDate.verticalCenter
						}

						onClicked: caldlg2.open();
					}
				}
			}
		}

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
		return Qt.createQmlObject("import QtQuick 2.0; import QtCharts 2.0; ValueAxis { min: "
								  + min + "; max: " + max + " }", chartView);
	}
}
