import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"

TPPopup {
	id: dlgMuscularGroup
	keepAbove: true
	width: appSettings.pageWidth/2
	height: shown ? dlgHeight : appSettings.itemDefaultHeight

	property string buttonLabel: qsTr("Filter")

	property bool shown: true
	readonly property string groupsSeparator: '|'
	readonly property int dlgHeight: appSettings.pageHeight * 0.5

	signal muscularGroupsCreated(groups: string);

	property ListModel groupsModel: ListModel {
		ListElement { display: qsTr("Quadriceps"); value: ""; selected: false; }
		ListElement { display: qsTr("Hamstrings"); value: ""; selected: false; }
		ListElement { display: qsTr("Calves"); value: ""; selected: false; }
		ListElement { display: qsTr("Glutes"); value: ""; selected: false; }
		ListElement	{ display: qsTr("Upper Back"); value: ""; selected: false; }
		ListElement { display: qsTr("Middle Back"); value: ""; selected: false; }
		ListElement { display: qsTr("Lower Back"); value: ""; selected: false; }
		ListElement { display: qsTr("Biceps"); value: ""; selected: false; }
		ListElement { display: qsTr("Triceps"); value: ""; selected: false; }
		ListElement { display: qsTr("Forearms"); value: ""; selected: false; }
		ListElement { display: qsTr("Upper Chest"); value: ""; selected: false; }
		ListElement { display: qsTr("Middle Chest"); value: ""; selected: false; }
		ListElement { display: qsTr("Lower Chest"); value: ""; selected: false; }
		ListElement { display: qsTr("Front Delts"); value: ""; selected: false; }
		ListElement { display: qsTr("Lateral Delts"); value: ""; selected: false; }
		ListElement { display: qsTr("Rear Delts"); value: ""; selected: false; }
		ListElement { display: qsTr("Traps"); value: ""; selected: false; }
		ListElement { display: qsTr("Abs"); value: ""; selected: false; }
	}

	Behavior on height {
		NumberAnimation {
			easing.type: Easing.InOutBack
		}
	}

	TPButton {
		id: btnShowHideList
		imageSource: dlgMuscularGroup.shown ? "fold-up.png" : "fold-down.png"
		hasDropShadow: false
		width: btnClose.width
		height: width
		z: 1

		anchors {
			top: parent.top
			topMargin: 2
			left: parent.left
			leftMargin: 2
		}

		onClicked: dlgMuscularGroup.shown = !dlgMuscularGroup.shown;
	}

	ScrollView {
		ScrollBar.horizontal.policy: ScrollBar.AsNeeded
		ScrollBar.vertical.policy: ScrollBar.AsNeeded
		contentWidth: availableWidth

		anchors {
			top: titleBar.bottom
			left: parent.left
			right: parent.right
			bottom: btnMakeFilter.top
			bottomMargin: 5
		}

		ColumnLayout {
			id: itemsLayout
			spacing: 0
			anchors {
				fill: parent
				leftMargin: 5
				topMargin: 5
			}

			Repeater {
				id: groupsRepeater
				model: groupsModel

				delegate: TPRadioButtonOrCheckBox {
					text: qsTr(model.display)
					radio: false
					checked: model.selected
					width: itemsLayout.width
					Layout.fillWidth: true

					onCheckedChanged: { model.selected = checked; }
				}
			} //Repeater
		} //ColumnLayout
	} //ScrollView

	TPButton {
		id: btnMakeFilter
		text: buttonLabel
		autoSize: true
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
			let muscularGroups = "";
			for (let i = 0; i < groupsModel.count; ++i) {
				if (groupsModel.get(i).selected)
					muscularGroups += qsTr(groupsModel.get(i).display) + groupsSeparator;
			}
			muscularGroupsCreated(muscularGroups);
		}
	}

	function show(targetItem: Item, pos: int): void {
		selectInitialGroups();
		shown = true;
		show2(targetItem, pos);
	}

	function selectInitialGroups(): void {
		const groups = exercisesListModel.filter().split(groupsSeparator);
		for (let x = 0; x < groupsModel.count; ++x) {
			let included = false;
			for (let i = 0; i < groups.length; ++i) {
				if (qsTr(groupsModel.get(x).display) === groups[i]) {
					included = true;
					break;
				}
				else if (groupsModel.get(x).display === groups[i]) {
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
