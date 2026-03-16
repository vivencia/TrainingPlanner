import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

import "../../TPWidgets"

import TpQml

Item {
	id: control

	required property HomePageMesoModel mesoSubModel

	TPLabel {
		id: lblTitle
		text: control.mesoSubModel.ownMesosModel ? qsTr("My Programs") : qsTr("Clients' Programs")
		useBackground: true
		backgroundColor: control.mesoSubModel.ownMesosModel ? appSettings.primaryLightColor : appSettings.primaryColor

		anchors {
			top: parent.top
			horizontalCenter: parent.horizontalCenter
			margins: 5
		}
	}

	TPListView {
		id: mesosListView
		model: control.mesoSubModel
		spacing: 10
		width: parent.width
		height: parent.height * 0.8 - lblTitle.height - 10

		anchors {
			top: lblTitle.bottom
			left: parent.left
			right: parent.right
			margins: 5
		}

		delegate: SwipeDelegate {
			id: mesoDelegate
			width: parent.width

			onClicked: mesoModel.getMesocyclePage(mesoIdx, false);
			onPressAndHold: control.mesoSubModel.currentIndex = index;
			swipe.onCompleted: control.mesoSubModel.currentIndex = index;

			Rectangle {
				id: optionsRec
				color: appSettings.primaryDarkColor
				radius: 6
				layer.enabled: true
				visible: false
				anchors.fill: parent
			}

			swipe.left: MultiEffect {
				id: optionsEffect
				anchors.fill: parent
				source: optionsRec
				shadowEnabled: true
				shadowOpacity: 0.5
				blurMax: 16
				shadowBlur: 1
				shadowHorizontalOffset: 5
				shadowVerticalOffset: 5
				shadowColor: "black"
				shadowScale: 1
				opacity: mesoDelegate.swipe.complete ? 0.8 : mesoDelegate.swipe.position
				Behavior on opacity { NumberAnimation {} }

				TPButton {
					id: btnMesoInfo
					text: qsTr("View Program")
					imageSource: "mesocycle.png"
					textUnderIcon: true
					rounded: false
					width: parent.width/2 - 10
					height: parent.height/2 - 10
					z:1

					anchors {
						top: parent.top
						topMargin: 5
						left: parent.left
						leftMargin: 5
					}

					onClicked: mesoModel.getMesocyclePage(mesoIdx, false);
				}

				TPButton {
					id: btnMesoCalendar
					text: qsTr("Calendar")
					imageSource: "meso-calendar.png"
					rounded: false
					textUnderIcon: true
					enabled: haveCalendar
					width: parent.width/2 - 10
					height: parent.height/2 - 10
					z: 1

					anchors {
						top: parent.top
						topMargin: 5
						left: btnMesoInfo.right
						leftMargin: 5
					}

					onClicked: mesoModel.getMesoCalendarPage(mesoIdx);
				}

				TPButton {
					id: btnMesoPlan
					text: qsTr("Exercises Sheet")
					imageSource: "meso-splitplanner.png"
					rounded: false
					enabled: mesoSplitsAvailable
					textUnderIcon: true
					width: parent.width/2 - 10
					height: parent.height/2 - 10
					z: 1

					anchors {
						top: btnMesoInfo.bottom
						topMargin: 5
						left: parent.left
						leftMargin: 5
					}

					onClicked: mesoModel.getExercisesPlannerPage(mesoIdx);
				}

				TPButton {
					id: btnExport
					text: qsTr("Export")
					imageSource: "export.png"
					rounded: false
					textUnderIcon: true
					enabled: mesoExportable
					width: parent.width/2 - 10
					height: parent.height/2 - 10
					z: 1

					anchors {
						top: btnMesoCalendar.bottom
						topMargin: 5
						left: btnMesoPlan.right
						leftMargin: 5
					}

					onClicked: showExportMenu(mesoIdx, this);
				}
			} //swipe.left: Rectangle

			Rectangle {
				id: removeBackground
				anchors.fill: parent
				color: "lightgray"
				radius: 6
				layer.enabled: true
				visible: false
			}

			swipe.right: MultiEffect {
				id: removeRec
				anchors.fill: parent
				source: removeBackground
				shadowEnabled: true
				shadowOpacity: 0.5
				blurMax: 16
				shadowBlur: 1
				shadowHorizontalOffset: 5
				shadowVerticalOffset: 5
				shadowColor: "black"
				shadowScale: 1
				opacity: mesoDelegate.swipe.complete ? 0.8 : 0-mesoDelegate.swipe.position
				Behavior on opacity { NumberAnimation {} }

				TPButton {
					text: qsTr("Remove Program")
					imageSource: "remove"
					hasDropShadow: false
					autoSize: true
					z: 1

					anchors {
						horizontalCenter: parent.horizontalCenter
						verticalCenter: parent.verticalCenter
					}

					onClicked: {
						msgDlg.meso_name = mesoName;
						msgDlg.show(-1);
					}
				}

				TPBalloonTip {
					id: msgDlg
					title: qsTr("Remove ") + meso_name + "?"
					message: qsTr("This action cannot be undone.")
					imageSource: "remove"
					keepAbove: true
					parentPage: homePage

					property string meso_name
					onButton1Clicked: mesoModel.removeMesocycle(mesoIdx);
				}
			} //swipe.right

			Rectangle {
				id: backRec
				anchors.fill: parent
				radius: 8
				layer.enabled: true
				color: control.mesoSubModel.ownMesosModel ? appSettings.primaryColor : appSettings.primaryDarkColor
				border.color: index === control.mesoSubModel.currentIndex ? appSettings.fontColor : "transparent"
				visible: false
			}

			background: MultiEffect {
				id: mesoEntryEffect
				visible: true
				source: backRec
				shadowEnabled: true
				shadowOpacity: 0.5
				blurMax: 16
				shadowBlur: 1
				shadowHorizontalOffset: 5
				shadowVerticalOffset: 5
				shadowColor: "black"
				shadowScale: 1
				opacity: 0.8
			}

			contentItem: ColumnLayout {
				id: mesoContent
				spacing: 2

				TPLabel {
					text: mesoName
					fontColor: appSettings.fontColor
					horizontalAlignment: Text.AlignHCenter
					Layout.bottomMargin: 10
					Layout.maximumWidth: parent.width
				}
				TPLabel {
					text: mesoCoach
					fontColor: appSettings.fontColor
					Layout.maximumWidth: parent.width
					visible: control.mesoSubModel.ownMesosModel
				}
				TPLabel {
					text: mesoClient
					fontColor: appSettings.fontColor
					Layout.maximumWidth: parent.width
					visible: !control.mesoSubModel.ownMesosModel
				}
				TPLabel {
					text: mesoStartDate
					fontColor: appSettings.fontColor
				}
				TPLabel {
					text: mesoEndDate
					fontColor: appSettings.fontColor
				}
				TPLabel {
					text: mesoSplit
					fontColor: appSettings.fontColor
					Layout.maximumWidth: parent.width
				}
			}
		} //delegate
	} //ListView

	TPToolBar {
		height: parent.height * (Qt.platform.os !== "android" ? 0.2 : 0.25)

		anchors {
			left: parent.left
			right: parent.right
			bottom: parent.bottom
		}

		ColumnLayout {
			spacing: 5
			anchors.fill: parent
			anchors.margins: 5

			TPButton {
				id: btnAddMeso
				text: qsTr("New Training Program")
				imageSource: "mesocycle-add.png"
				Layout.preferredWidth: preferredWidth
				Layout.maximumWidth: parent.width
				Layout.maximumHeight: appSettings.itemDefaultHeight
				Layout.alignment: Qt.AlignCenter

				onClicked: mesoModel.startNewMesocycle(control.mesoSubModel.ownMesosModel);
			}

			TPButton {
				id: btnImportMeso
				text: qsTr("Import program from file")
				imageSource: "import.png"
				Layout.preferredWidth: preferredWidth
				Layout.maximumWidth: parent.width
				Layout.maximumHeight: appSettings.itemDefaultHeight
				Layout.alignment: Qt.AlignCenter

				onClicked: itemManager.chooseFileToImport();
			}

			TPButton {
				id: btnWorkout
				text: qsTr("Today's workout")
				imageSource: "workout.png"
				visible: control.mesoSubModel.ownMesosModel
				enabled: control.mesoSubModel.canHaveTodaysWorkout
				Layout.preferredWidth: preferredWidth
				Layout.maximumHeight: appSettings.itemDefaultHeight
				Layout.alignment: Qt.AlignCenter

				onClicked: mesoModel.startTodaysWorkout(control.mesoSubModel.currentMesoIdx());
			}
		} //ColumnLayout
	}

	FileOperations {
		id: fileOps
		fileType: TPUtils.FT_TP_PROGRAM
	}

	property TPFloatingMenuBar exportMenu: null
	function showExportMenu(mesoidx: int, callButton: TPButton): void {
		if (exportMenu === null) {
			let exportMenuComponent = Qt.createComponent("qrc:/TpQml/qml/TPWidgets/TPFloatingMenuBar.qml");
			exportMenu = exportMenuComponent.createObject(homePage, { parentPage: homePage, titleHeader: qsTr("Export options") });
			fileOps.mesoIdx = mesoidx;
			for(let i = 0; i < fileOps.operationsCount; ++i)
				exportMenu.addEntry(fileOps.operationsList[i], "", i, true);

			exportMenu.menuEntrySelected.connect(function(id) { fileOps.doFileOperation(id); });
		}
		exportMenu.show2(callButton, 0);
	}
} //ListView
