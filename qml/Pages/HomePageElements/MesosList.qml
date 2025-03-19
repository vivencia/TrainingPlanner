import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

import "../"
import "../../TPWidgets"
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

ListView {
	id: mesosListView
	model: mesocyclesModel
	boundsBehavior: Flickable.StopAtBounds
	spacing: 10

	ScrollBar.vertical: ScrollBar {
		policy: ScrollBar.AsNeeded
		active: ScrollBar.AsNeeded
	}

	property bool viewedMesoHasData
	Connections {
		target: mesocyclesModel
		function onViewedMesoHasDataChanged() {
			viewedMesoHasData = mesocyclesModel.viewedMesoHasData();
		}
	}

	delegate: SwipeDelegate {
		id: mesoDelegate
		width: parent ? parent.width : 0

		onClicked: mesocyclesModel.getMesocyclePage(index);
		onPressAndHold: mesocyclesModel.currentMesoIdx = index;
		swipe.onOpened: mesocyclesModel.setCurrentlyViewedMeso(index);

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

				onClicked: mesocyclesModel.getMesocyclePage(index);
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

				onClicked: mesocyclesModel.getMesoCalendarPage(index);
			}

			TPButton {
				id: btnMesoPlan
				text: qsTr("Exercises Table")
				imageSource: "meso-splitplanner.png"
				imageSize: 30
				backgroundColor: "transparent"
				rounded: false
				flat: false
				autoResize: true
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

				onClicked: mesocyclesModel.getExercisesPlannerPage(index);
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
				enabled: viewedMesoHasData
				width: parent.width/2 - 10
				height: parent.height/2 - 10
				z:1

				anchors {
					top: btnMesoCalendar.bottom
					topMargin: 5
					left: btnMesoPlan.right
					leftMargin: 5
				}

				onClicked: {
					if (Qt.platform.os === "android")
						showExportMenu(index);
					else
						exportTypeTip.init(index, false);
				}
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

				onClicked: msgDlg.init(index);
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
			color: index === mesocyclesModel.currentMesoIdx ? appSettings.primaryColor : appSettings.listEntryColor2
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

			readonly property string fntColor: index === mesocyclesModel.currentMesoIdx ? appSettings.fontColor : appSettings.disabledFontColor

			TPLabel {
				text: mesoName
				fontColor: mesoContent.fntColor
				horizontalAlignment: Text.AlignHCenter
				Layout.bottomMargin: 10
				Layout.maximumWidth: parent.width
			}
			TPLabel {
				text: mesoCoach
				fontColor: mesoContent.fntColor
				Layout.maximumWidth: parent.width
			}
			TPLabel {
				text: mesoClient
				fontColor: mesoContent.fntColor
				Layout.maximumWidth: parent.width
			}
			TPLabel {
				text: mesoStartDate
				fontColor: mesoContent.fntColor
			}
			TPLabel {
				text: mesoEndDate
				fontColor: mesoContent.fntColor
			}
			TPLabel {
				text: mesoSplit
				fontColor: mesoContent.fntColor
				Layout.maximumWidth: parent.width
			}
		}
	} //delegate

	property TPFloatingMenuBar exportMenu: null
	function showExportMenu(meso_idx): void {
		if (exportMenu === null) {
			let exportMenuComponent = Qt.createComponent("qrc:/qml/TPWidgets/TPFloatingMenuBar.qml");
			exportMenu = exportMenuComponent.createObject(homePage, { parentPage: homePage });
			exportMenu.addEntry(qsTr("Export"), "save-day.png", 0, true);
			exportMenu.addEntry(qsTr("Share"), "export.png", 1, true);
			exportMenu.menuEntrySelected.connect(function(id) { exportTypeTip.init(meso_idx, id === 1); });
		}
		exportMenu.show2(btnExport, 0);
	}

	Loader {
		id: exportTypeTip
		active: viewedMesoHasData
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

			onButton1Clicked: mesocyclesModel.exportMeso(mesoIdx, bShare, customBoolProperty1);
		}

		property int mesoIdx
		property bool bShare
		function init(meso_idx: int, share: bool): void {
			mesoIdx = meso_idx;
			bShare = share;
			item.show(-1);
		}
	}
} //ListView
