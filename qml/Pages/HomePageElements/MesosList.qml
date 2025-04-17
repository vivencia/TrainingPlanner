import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

import "../"
import "../../TPWidgets"
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

Item {
	required property HomePageMesoModel mesoModel
	required property bool mainUserPrograms

	property int viewedMesoIdx
	property bool viewedMesoCanBeExported

	ListView {
		id: mesosListView
		model: mesoModel
		boundsBehavior: Flickable.StopAtBounds
		spacing: 10
		width: parent.width
		height: parent.height*0.8

		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
			margins: 5
		}

		ScrollBar.vertical: ScrollBar {
			policy: ScrollBar.AsNeeded
			active: ScrollBar.AsNeeded
		}

		Connections {
			target: mesocyclesModel
			function onCanExportChanged(meso_idx: int, can_export: bool) : void {
				viewedMesoIdx = meso_idx;
				viewedMesoCanBeExported = can_export;
			}
		}

		delegate: SwipeDelegate {
			id: mesoDelegate
			width: parent ? parent.width : 0

			onClicked: mesocyclesModel.getMesocyclePage(mesoModel.mesoRow(index));
			onPressAndHold: mesocyclesModel.currentMesoIdx = mesoModel.mesoRow(index);
			swipe.onOpened: mesocyclesModel.setCurrentlyViewedMeso(mesoModel.mesoRow(index));

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
					imageSize: 30
					backgroundColor: "transparent"
					textUnderIcon: true
					rounded: false
					flat: false
					fixedSize: true
					width: parent.width/2 - 10
					height: parent.height/2 - 10
					z:1

					anchors {
						top: parent.top
						topMargin: 5
						left: parent.left
						leftMargin: 5
					}

					onClicked: mesocyclesModel.getMesocyclePage(mesoModel.mesoRow(index));
				}

				TPButton {
					id: btnMesoCalendar
					text: qsTr("Calendar")
					imageSource: "meso-calendar.png"
					imageSize: 30
					backgroundColor: "transparent"
					rounded: false
					flat: false
					textUnderIcon: true
					fixedSize: true
					width: parent.width/2 - 10
					height: parent.height/2 - 10
					z:1

					anchors {
						top: parent.top
						topMargin: 5
						left: btnMesoInfo.right
						leftMargin: 5
					}

					onClicked: mesocyclesModel.getMesoCalendarPage(mesoModel.mesoRow(index));
				}

				TPButton {
					id: btnMesoPlan
					text: qsTr("Exercises Table")
					imageSource: "meso-splitplanner.png"
					imageSize: 30
					backgroundColor: "transparent"
					rounded: false
					flat: false
					textUnderIcon: true
					fixedSize: true
					width: parent.width/2 - 10
					height: parent.height/2 - 10
					z:1

					anchors {
						top: btnMesoInfo.bottom
						topMargin: 5
						left: parent.left
						leftMargin: 5
					}

					onClicked: mesocyclesModel.getExercisesPlannerPage(mesoModel.mesoRow(index));
				}

				TPButton {
					id: btnExport
					text: qsTr("Export")
					imageSource: "export.png"
					imageSize: 30
					backgroundColor: "transparent"
					rounded: false
					flat: false
					textUnderIcon: true
					fixedSize: true
					enabled: viewedMesoIdx === mesoModel.mesoRow(index) ? viewedMesoCanBeExported : false
					width: parent.width/2 - 10
					height: parent.height/2 - 10
					z:1

					anchors {
						top: btnMesoCalendar.bottom
						topMargin: 5
						left: btnMesoPlan.right
						leftMargin: 5
					}

					onClicked: showExportMenu(mesoModel.mesoRow(index), this);
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
					z: 2

					anchors {
						horizontalCenter: parent.horizontalCenter
						verticalCenter: parent.verticalCenter
					}

					onClicked: msgDlg.init(mesoModel.mesoRow(index));
				}

				TPBalloonTip {
					id: msgDlg
					title: qsTr("Remove Program?")
					message: qsTr("This action cannot be undone. Note: removing a Program does not remove the workout records.")
					imageSource: "remove"
					keepAbove: true
					parentPage: homePage

					property int mesoidx
					onButton1Clicked: mesocyclesModel.removeMesocycle(mesoidx);

					function init(meso_idx: int): void {
						mesoidx = meso_idx;
						show(-1);
					}
				}
			} //swipe.right

			Rectangle {
				id: backRec
				anchors.fill: parent
				radius: 6
				layer.enabled: true
				color: mesoModel.mesoRow(index) === mesocyclesModel.currentMesoIdx ? appSettings.primaryColor : appSettings.listEntryColor2
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
					visible: mainUserPrograms
				}
				TPLabel {
					text: mesoClient
					fontColor: appSettings.fontColor
					Layout.maximumWidth: parent.width
					visible: !mainUserPrograms
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
		id: quickActionToolbar
		width: parent.width
		height: parent.height*0.2

		anchors {
			left: parent.left
			right: parent.right
			bottom: parent.bottom
		}

		TPButton {
			id: btnAddMeso
			text: qsTr("New Training Program")
			imageSource: "mesocycle-add.png"
			backgroundColor: "transparent"
			width: parent.width

			anchors {
				horizontalCenter: parent.horizontalCenter
				verticalCenter: parent.verticalCenter
				verticalCenterOffset: mainUserPrograms ? -30 : -15
			}

			onClicked: mesocyclesModel.startNewMesocycle(true, mainUserPrograms);
		}

		TPButton {
			id: btnImportMeso
			text: qsTr("Import program from file")
			imageSource: "import.png"
			backgroundColor: "transparent"
			width: parent.width

			anchors {
				horizontalCenter: parent.horizontalCenter
				verticalCenter: parent.verticalCenter
				verticalCenterOffset: mainUserPrograms ? 0 : 15
			}

			onClicked: itemManager.chooseFileToImport();
		}

		TPButton {
			id: btnWorkout
			text: qsTr("Today's workout")
			imageSource: "workout.png"
			backgroundColor: "transparent"
			visible: mainUserPrograms
			enabled: mesocyclesModel.canHaveTodaysWorkout
			width: parent.width

			anchors {
				horizontalCenter: parent.horizontalCenter
				verticalCenter: parent.verticalCenter
				verticalCenterOffset: 30
			}

			onClicked: mesocyclesModel.todaysWorkout();
		}
	}

	property TPFloatingMenuBar exportMenu: null
	function showExportMenu(meso_idx: int, callButton: TPButton): void {
		if (exportMenu === null) {
			let exportMenuComponent = Qt.createComponent("qrc:/qml/TPWidgets/TPFloatingMenuBar.qml");
			exportMenu = exportMenuComponent.createObject(homePage, { parentPage: homePage });
			if (!mesocyclesModel.isOwnMeso(meso_idx)) {
				const userid = mesocyclesModel.mesoClient(meso_idx);
				if (userid !== "")
					exportMenu.addEntry(qsTr("Send to ") + userModel.userNameFromId(userid), userModel.avatarFromId(userid), 10, true);
			}
			exportMenu.addEntry(qsTr("Export"), "save-day.png", 20, true);

			if (Qt.platform.os === "android")
				exportMenu.addEntry(qsTr("Share"), "export.png", 30, true);
			exportMenu.menuEntrySelected.connect(function(id) {
				switch (id) {
					case 10: mesocyclesModel.sendMesoToUser(meso_idx); break;
					case 20: exportTypeTip.init(meso_idx, false); break;
					case 30: exportTypeTip.init(meso_idx, true); break;
				}
			});
		}
		exportMenu.show2(callButton, 0);
	}

	Loader {
		id: exportTypeTip
		asynchronous: true
		sourceComponent: TPComplexDialog {
			id: dialog
			customStringProperty1: bShare ? qsTr("Share complete program?") : qsTr("Export complete program to file?")
			customStringProperty2: qsTr("Include Coach data?")
			customStringProperty3: "export.png"
			button1Text: qsTr("Yes")
			button2Text: qsTr("No")
			customItemSource: "TPDialogWithMessageAndCheckBox.qml"
			closeButtonVisible: true
			parentPage: homePage

			onButton1Clicked: {
				mesocyclesModel.exportMeso(mesoIdx, bShare, customBoolProperty1);
				exportTypeTip.active = false;
			}
		}

		property int mesoIdx
		property bool bShare

		function init(meso_idx: int, share: bool): void {
			active = true;
			mesoIdx = meso_idx;
			bShare = share;
			item.show(-1);
		}
	}
} //ListView
