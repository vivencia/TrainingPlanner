import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

TPPage {
	id: homePage

	property date minimumStartDate;

	header: TPToolBar {
		bottomPadding: 20

		TPImage {
			id: imgAppIcon
			source: "app_icon"
			dropShadow: false
			width: 50
			height: 50

			anchors {
				top: parent.top
				left: parent.left
				leftMargin: 5
			}
		}

		TPLabel {
			text: qsTr("Training Organizer")
			font: AppGlobals.extraLargeFont
			width: parent.width - imgAppIcon.width - 15
			height: parent.height

			anchors {
				top: parent.top
				topMargin: 5
				left: imgAppIcon.right
				leftMargin: 5
				right: parent.right
				rightMargin: 10
			}
		}
	}

	Loader {
		active: appSettings.mainUserConfigured
		asynchronous: true
		source: "qrc:/qml/Pages/HomePageElements/MesosList.qml"

		anchors {
			fill: parent
			leftMargin: 10
			rightMargin: 10
			topMargin: 40
		}
	}

	footer: Loader {
		active: appSettings.mainUserConfigured
		asynchronous: true
		source: "qrc:/qml/Pages/HomePageElements/Footer.qml"
	} // footer

	property TPFloatingMenuBar exportMenu: null
	function showExportMenu(meso_idx): void {
		if (exportMenu === null) {
			let exportMenuComponent = Qt.createComponent("qrc:/qml/TPWidgets/TPFloatingMenuBar.qml");
			exportMenu = exportMenuComponent.createObject(homePage, { parentPage: homePage });
			exportMenu.addEntry(qsTr("Export"), "save-day.png", 0, true);
			exportMenu.addEntry(qsTr("Share"), "export.png", 1, true);
			exportMenu.menuEntrySelected.connect(function(id) { exportTypeTip.init(meso_idx, id === 1); });
		}
		exportMenu.show2(btnImExport, 0);
	}

	Loader {
		id: exportTypeTip
		active: mesocyclesModel.currentMesoHasData
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
			dialog.show(-1);
		}
	}
} //Page

