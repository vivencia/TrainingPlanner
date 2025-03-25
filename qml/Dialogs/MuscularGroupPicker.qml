import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"

TPPopup {
	id: dlgMuscularGroup
	keepAbove: true
	width: appSettings.pageWidth/2
	height: shown ? dlgHeight : 25

	property string buttonLabel: qsTr("Filter")
	property bool useFancyNames: false
	property bool shown: true
	readonly property int dlgHeight: appSettings.pageHeight * 0.5
	signal muscularGroupCreated(group: string);

	property ListModel groupsModel: ListModel {
		ListElement { display: qsTr("Quadriceps"); value: "quadriceps"; selected: false; }
		ListElement { display: qsTr("Hamstrings"); value: "hamstrings"; selected: false; }
		ListElement { display: qsTr("Calves"); value: "calves"; selected: false; }
		ListElement { display: qsTr("Glutes"); value: "glutes"; selected: false; }
		ListElement	{ display: qsTr("Upper Back"); value: "upper back"; selected: false; }
		ListElement { display: qsTr("Middle Back"); value: "middle back"; selected: false; }
		ListElement { display: qsTr("Lower Back"); value: "lower back"; selected: false; }
		ListElement { display: qsTr("Biceps"); value: "biceps"; selected: false; }
		ListElement { display: qsTr("Triceps"); value: "triceps"; selected: false; }
		ListElement { display: qsTr("Forearms"); value: "fore arms"; selected: false; }
		ListElement { display: qsTr("Upper Chest"); value: "upper chest"; selected: false; }
		ListElement { display: qsTr("Middle Chest"); value: "middle chest"; selected: false; }
		ListElement { display: qsTr("Lower Chest"); value: "lower chest"; selected: false; }
		ListElement { display: qsTr("Front Delts"); value: "front delts"; selected: false; }
		ListElement { display: qsTr("Lateral Delts"); value: "lateral delts"; selected: false; }
		ListElement { display: qsTr("Rear Delts"); value: "rear delts"; selected: false; }
		ListElement { display: qsTr("Traps"); value: "traps"; selected: false; }
		ListElement { display: qsTr("Abs"); value: "abs"; selected: false; }
	}

	Behavior on height {
		NumberAnimation {
			easing.type: Easing.InOutBack
		}
	}

	Rectangle {
		id: recTitleBar
		color: appSettings.paneBackgroundColor
		opacity: 0.8
		width: parent.width
		height: 25
		z:0

		gradient: Gradient {
			orientation: Gradient.Horizontal
			GradientStop { position: 0.0; color: appSettings.paneBackgroundColor; }
			GradientStop { position: 0.25; color: appSettings.primaryLightColor; }
			GradientStop { position: 0.50; color: appSettings.primaryColor; }
			GradientStop { position: 0.75; color: appSettings.primaryDarkColor; }
		}

		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
		}

		TPButton {
			id: btnShowHideList
			imageSource: dlgMuscularGroup.shown ? "fold-up.png" : "fold-down.png"
			hasDropShadow: false
			imageSize: 25
			height: 25

			anchors {
				left: parent.left
				verticalCenter: parent.verticalCenter
			}

			onClicked: dlgMuscularGroup.shown = !dlgMuscularGroup.shown;
		}
	} //Rectangle title bar

	ScrollView {
		ScrollBar.horizontal.policy: ScrollBar.AsNeeded
		ScrollBar.vertical.policy: ScrollBar.AsNeeded
		contentWidth: availableWidth

		anchors {
			top: recTitleBar.bottom
			left: parent.left
			right: parent.right
			bottom: btnMakeFilter.top
			bottomMargin: 5
		}

		ColumnLayout {
			id: itemsLayout
			spacing: 0
			anchors.fill: parent
			anchors.leftMargin: 5

			Repeater {
				id: groupsRepeater
				model: groupsModel

				delegate: TPCheckBox {
					text: model.display
					checked: model.selected
					width: itemsLayout.width
					Layout.fillWidth: true
					height: 25

					onCheckedChanged: { model.selected = checked; }
				}
			} //Repeater
		} //ColumnLayout
	} //ScrollView

	TPButton {
		id: btnMakeFilter
		text: buttonLabel
		flat: false
		width: parent.width*0.6
		height: 25
		visible: dlgMuscularGroup.shown

		readonly property int margin: (parent.width - width)/2

		anchors {
			left: parent.left
			leftMargin: btnMakeFilter.margin
			right: parent.right
			rightMargin: btnMakeFilter.margin
			bottom: parent.bottom
		}

		onClicked: {
			let muscularGroup = "";
			for (let i = 0; i < groupsModel.count; ++i) {
				if (groupsModel.get(i).selected) {
					if (!useFancyNames)
						muscularGroup += groupsModel.get(i).value + '|'; //use fancy_record_separator1 as separator
					else
						muscularGroup += groupsModel.get(i).display + '|';
				}
			}
			muscularGroupCreated(muscularGroup);
		}
	}

	function show(initialGroups: string, targetItem: Item, pos: int): void {
		selectInitialGroups(initialGroups);
		shown = true;
		show2(targetItem, pos);
	}

	function selectInitialGroups(initialGroups: string): void {
		const groups = initialGroups.split('|');
		for (let x=0; x < groupsModel.count; ++x) {
			let included = false;
			for (let i=0; i < groups.length; ++i) {
				if (groupsModel.get(x).display === groups[i]) {
					included = true;
					break;
				}
			}
			groupsRepeater.itemAt(x).checked = included;
		}
		//groupsModel.set(x, {selected: groupsModel.get(x).display === groups[i]});
		//groupsModel.setProperty(x, "selected", groupsModel.get(x).display === groups[i]);
	}
}
