pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import TpQml
import TpQml.Widgets

TPPopup {
	id: _dlgMuscularGroup
	keepAbove: true
	width: AppSettings.pageWidth / 2
	height: shown ? dlgHeight : AppSettings.itemDefaultHeight

	onOpened: {
		shown = true;
		if (initialGroups !== "") {
			const groups = initialGroups.split(groupsSeparator);
			for (let x = 0; x < groupsModel.count; ++x) {
				let included = false;
				for (let i = 0; i < groups.length; ++i) {
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

//public:
	property string buttonLabel: qsTr("Filter")
	property string initialGroups
	signal muscularGroupsCreated(groups: string);

//private:
	property bool shown: true
	readonly property string groupsSeparator: '|'
	readonly property int dlgHeight: AppSettings.pageHeight * 0.5

	property ListModel groupsModel: ListModel {
		ListElement { display: qsTr("Quadriceps");		value: ""; selected: false; }
		ListElement { display: qsTr("Hamstrings");		value: ""; selected: false; }
		ListElement { display: qsTr("Calves");			value: ""; selected: false; }
		ListElement { display: qsTr("Glutes");			value: ""; selected: false; }
		ListElement	{ display: qsTr("Upper Back");		value: ""; selected: false; }
		ListElement { display: qsTr("Middle Back");		value: ""; selected: false; }
		ListElement { display: qsTr("Lower Back");		value: ""; selected: false; }
		ListElement { display: qsTr("Biceps");			value: ""; selected: false; }
		ListElement { display: qsTr("Triceps");			value: ""; selected: false; }
		ListElement { display: qsTr("Forearms");		value: ""; selected: false; }
		ListElement { display: qsTr("Upper Chest");		value: ""; selected: false; }
		ListElement { display: qsTr("Middle Chest");	value: ""; selected: false; }
		ListElement { display: qsTr("Lower Chest");		value: ""; selected: false; }
		ListElement { display: qsTr("Front Delts");		value: ""; selected: false; }
		ListElement { display: qsTr("Lateral Delts");	value: ""; selected: false; }
		ListElement { display: qsTr("Rear Delts");		value: ""; selected: false; }
		ListElement { display: qsTr("Traps");			value: ""; selected: false; }
		ListElement { display: qsTr("Abs");				value: ""; selected: false; }
	}

	Behavior on height {
		NumberAnimation {
			easing.type: Easing.InOutBack
		}
	}

	TPButton {
		id: btnShowHideList
		imageSource: _dlgMuscularGroup.shown ? "fold-up.png" : "fold-down.png"
		hasDropShadow: false
		width: _dlgMuscularGroup.btnClose.width
		height: width
		z: 1

		anchors {
			top: _dlgMuscularGroup.contentItem.top
			topMargin: 2
			left: _dlgMuscularGroup.contentItem.left
			leftMargin: 2
		}

		onClicked: _dlgMuscularGroup.shown = !_dlgMuscularGroup.shown;
	}

	ScrollView {
		ScrollBar.horizontal.policy: ScrollBar.AsNeeded
		ScrollBar.vertical.policy: ScrollBar.AsNeeded
		contentWidth: availableWidth

		anchors {
			top: _dlgMuscularGroup.titleBar.bottom
			left: _dlgMuscularGroup.contentItem.left
			right: _dlgMuscularGroup.contentItem.right
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
				model: _dlgMuscularGroup.groupsModel

				delegate: TPRadioButtonOrCheckBox {
					text: qsTr(display)
					radio: false
					checked: selected
					width: itemsLayout.width
					Layout.fillWidth: true

					required property int index
					required property string display
					required property bool selected

					onCheckedChanged: { _dlgMuscularGroup.groupsModel.set(index, { "selected": checked }); }
				}
			} //Repeater
		} //ColumnLayout
	} //ScrollView

	TPButton {
		id: btnMakeFilter
		text: _dlgMuscularGroup.buttonLabel
		autoSize: true
		//visible: _dlgMuscularGroup.shown

		readonly property int margin: (_dlgMuscularGroup.width - width)/2

		anchors {
			left: _dlgMuscularGroup.contentItem.left
			leftMargin: btnMakeFilter.margin
			right: _dlgMuscularGroup.contentItem.right
			rightMargin: btnMakeFilter.margin
			bottom: _dlgMuscularGroup.contentItem.bottom
		}

		onClicked: {
			let muscularGroups = "";
			for (let i = 0; i < _dlgMuscularGroup.groupsModel.count; ++i) {
				if (_dlgMuscularGroup.groupsModel.get(i).selected)
					muscularGroups += qsTr(_dlgMuscularGroup.groupsModel.get(i).display) + _dlgMuscularGroup.groupsSeparator;
			}
			_dlgMuscularGroup.muscularGroupsCreated(muscularGroups);
		}
	}
}
