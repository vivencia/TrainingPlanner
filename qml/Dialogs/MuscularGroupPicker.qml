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
	property string groupsSeparator
	property bool shown: true
	readonly property int dlgHeight: appSettings.pageHeight * 0.5
	signal muscularGroupCreated(group: string);

	property ListModel groupsModel: ListModel {
		ListElement { display: QT_TR_NOOP("Quadriceps"); value: "quadriceps"; selected: false; }
		ListElement { display: QT_TR_NOOP("Hamstrings"); value: "hamstrings"; selected: false; }
		ListElement { display: QT_TR_NOOP("Calves"); value: "calves"; selected: false; }
		ListElement { display: QT_TR_NOOP("Glutes"); value: "glutes"; selected: false; }
		ListElement	{ display: QT_TR_NOOP("Upper Back"); value: "upper back"; selected: false; }
		ListElement { display: QT_TR_NOOP("Middle Back"); value: "middle back"; selected: false; }
		ListElement { display: QT_TR_NOOP("Lower Back"); value: "lower back"; selected: false; }
		ListElement { display: QT_TR_NOOP("Biceps"); value: "biceps"; selected: false; }
		ListElement { display: QT_TR_NOOP("Triceps"); value: "triceps"; selected: false; }
		ListElement { display: QT_TR_NOOP("Forearms"); value: "fore arms"; selected: false; }
		ListElement { display: QT_TR_NOOP("Upper Chest"); value: "upper chest"; selected: false; }
		ListElement { display: QT_TR_NOOP("Middle Chest"); value: "middle chest"; selected: false; }
		ListElement { display: QT_TR_NOOP("Lower Chest"); value: "lower chest"; selected: false; }
		ListElement { display: QT_TR_NOOP("Front Delts"); value: "front delts"; selected: false; }
		ListElement { display: QT_TR_NOOP("Lateral Delts"); value: "lateral delts"; selected: false; }
		ListElement { display: QT_TR_NOOP("Rear Delts"); value: "rear delts"; selected: false; }
		ListElement { display: QT_TR_NOOP("Traps"); value: "traps"; selected: false; }
		ListElement { display: QT_TR_NOOP("Abs"); value: "abs"; selected: false; }
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
			width: btnClose.width
			height: width

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
		flat: false
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
			let muscularGroup = "";
			for (let i = 0; i < groupsModel.count; ++i) {
				if (groupsModel.get(i).selected) {
					if (!useFancyNames)
						muscularGroup += groupsModel.get(i).value + groupsSeparator;
					else
						muscularGroup += qsTr(groupsModel.get(i).display) + groupsSeparator;
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
		const groups = initialGroups.split(groupsSeparator);
		for (let x = 0; x < groupsModel.count; ++x) {
			let included = false;
			for (let i=0; i < groups.length; ++i) {
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
