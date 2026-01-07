import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

import "../"
import "../../TPWidgets"
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

Item {
	required property HomePageMesoModel mesoSubModel

	TPLabel {
		id: lblTitle
		text: mesoSubModel.ownMesosModel ? qsTr("My Programs") : qsTr("Clients' Programs")
		useBackground: true
		backgroundColor: mesoSubModel.ownMesosModel ? appSettings.primaryLightColor : appSettings.primaryColor

		anchors {
			top: parent.top
			horizontalCenter: parent.horizontalCenter
			margins: 5
		}
	}

	TPListView {
		id: mesosListView
		model: mesoSubModel
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
			width: parent ? parent.width : 0

			onClicked: mesoModel.getMesocyclePage(mesoIdx, false);
			onPressAndHold: mesoSubModel.currentIndex = index;
			swipe.onCompleted: mesoSubModel.currentIndex = index;

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
					imageSize: appSettings.itemDefaultHeight
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

					onClicked: mesoModel.getMesocyclePage(mesoIdx);
				}

				TPButton {
					id: btnMesoCalendar
					text: qsTr("Calendar")
					imageSource: "meso-calendar.png"
					imageSize: appSettings.itemDefaultHeight
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
					text: qsTr("Exercises Table")
					imageSource: "meso-splitplanner.png"
					imageSize: appSettings.itemDefaultHeight
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
					imageSize: appSettings.itemDefaultHeight
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
						msgDlg.mesoidx = mesoIdx;
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
					property int mesoidx
					onButton1Clicked: mesoModel.removeMesocycle(mesoidx);
				}
			} //swipe.right

			Rectangle {
				id: backRec
				anchors.fill: parent
				radius: 8
				layer.enabled: true
				color: mesoSubModel.ownMesosModel ? appSettings.primaryColor : appSettings.primaryDarkColor
				border.color: index === mesoSubModel.currentIndex ? appSettings.fontColor : "transparent"
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
					visible: mesoSubModel.ownMesosModel
				}
				TPLabel {
					text: mesoClient
					fontColor: appSettings.fontColor
					Layout.maximumWidth: parent.width
					visible: !mesoSubModel.ownMesosModel
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
		height: parent.height * (Qt.platform.os !== "android" ? 0.2 : 0.25)

		anchors {
			left: parent.left
			right: parent.right
			bottom: parent.bottom
		}

		ColumnLayout {
			anchors.fill: parent
			spacing: 5

			TPButton {
				id: btnAddMeso
				text: qsTr("New Training Program")
				imageSource: "mesocycle-add.png"
				Layout.preferredWidth: preferredWidth
				Layout.maximumWidth: parent.width - 20
				Layout.maximumHeight: appSettings.itemDefaultHeight
				Layout.alignment: Qt.AlignCenter

				onClicked: mesoModel.startNewMesocycle(mesoSubModel.ownMesosModel);
			}

			TPButton {
				id: btnImportMeso
				text: qsTr("Import program from file")
				imageSource: "import.png"
				Layout.preferredWidth: preferredWidth
				Layout.maximumWidth: parent.width - 20
				Layout.maximumHeight: appSettings.itemDefaultHeight
				Layout.alignment: Qt.AlignCenter

				onClicked: itemManager.chooseFileToImport();
			}

			TPButton {
				id: btnWorkout
				text: qsTr("Today's workout")
				imageSource: "workout.png"
				visible: mesoSubModel.ownMesosModel
				enabled: mesoSubModel.canHaveTodaysWorkout
				Layout.preferredWidth: preferredWidth
				Layout.maximumHeight: appSettings.itemDefaultHeight
				Layout.alignment: Qt.AlignCenter

				onClicked: mesoModel.startTodaysWorkout(mesoIdx);
			}
		}
	}

	property TPFloatingMenuBar exportMenu: null
	function showExportMenu(meso_idx: int, callButton: TPButton): void {
		if (exportMenu === null) {
			let exportMenuComponent = Qt.createComponent("qrc:/qml/TPWidgets/TPFloatingMenuBar.qml");
			exportMenu = exportMenuComponent.createObject(homePage, { parentPage: homePage });
			if (!mesoModel.isOwnMeso(meso_idx)) {
				const userid = mesoModel.mesoClient(meso_idx);
				if (userid !== "")
					exportMenu.addEntry(qsTr("Send to ") + userModel.userNameFromId(userid), userModel.avatarFromId(userid), 10, true);
			}
			exportMenu.addEntry(qsTr("Export"), "save-day.png", 20, true);

			if (Qt.platform.os === "android")
				exportMenu.addEntry(qsTr("Share"), "export.png", 30, true);
			exportMenu.menuEntrySelected.connect(function(id) {
				switch (id) {
					case 10: mesoModel.sendMesoToUser(meso_idx); break;
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
		active: false

		sourceComponent: TPComplexDialog {
			customStringProperty1: bShare ? qsTr("Share complete program?") : qsTr("Export complete program to file?")
			customStringProperty2: qsTr("Include Coach data?")
			customStringProperty3: "export.png"
			button1Text: qsTr("Yes")
			button2Text: qsTr("No")
			customItemSource: "TPDialogWithMessageAndCheckBox.qml"
			closeButtonVisible: true
			parentPage: homePage

			onButton1Clicked: {
				mesoModel.exportMeso(mesoIdx, bShare, customBoolProperty1);
				exportTypeTip.active = false;
			}
		}

		onLoaded: item.show(-1);

		property int mesoIdx
		property bool bShare

		function init(meso_idx: int, share: bool): void {
			active = true;
			mesoIdx = meso_idx;
			bShare = share;
		}
	}
} //ListView
