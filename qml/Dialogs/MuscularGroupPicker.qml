import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"

TPPopup {
	id: dlgMuscularGroup
	bKeepAbove: true
	width: appSettings.pageWidth/2
	height: shown ? dlgHeight : 25

	readonly property int dlgHeight: appSettings.pageHeight * 0.5
	property bool shown: true
	signal muscularGroupCreated(group: string);

	property ListModel groupsModel: ListModel {
		ListElement { text: qsTr("Quadriceps"); value: false; }
		ListElement { text: qsTr("Hamstrings"); value: false; }
		ListElement { text: qsTr("Calves"); value: false; }
		ListElement	{ text: qsTr("Upper Back"); value: false; }
		ListElement { text: qsTr("Middle Back"); value: false; }
		ListElement { text: qsTr("Lower Back"); value: false; }
		ListElement { text: qsTr("Biceps"); value: false; }
		ListElement { text: qsTr("Triceps"); value: false; }
		ListElement { text: qsTr("Forearms"); value: false; }
		ListElement { text: qsTr("Upper Chest"); value: false; }
		ListElement { text: qsTr("Middle Chest"); value: false; }
		ListElement { text: qsTr("Lower Chest"); value: false; }
		ListElement { text: qsTr("Front Shoulders"); value: false; }
		ListElement { text: qsTr("Lateral Shoulders"); value: false; }
		ListElement { text: qsTr("Rear Shoulders"); value: false; }
		ListElement { text: qsTr("Traps"); value: false; }
		ListElement { text: qsTr("Abdominals"); value: false; }
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

				TPCheckBox {
					text: model.text
					checked: model.value
					width: parent.width
					height: 25

					onClicked: model.value = checked;
				}
			} //Repeater
		} //ColumnLayout
	} //ScrollView

	TPButton {
		id: btnMakeFilter
		text: qsTr("Filter")
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
			for (let i = 0; i < groupsModel.count; ++i)
			{
				if (groupsModel.get(i).value)
					muscularGroup += groupsModel.get(i).text + ' ';
			}
			muscularGroupCreated(muscularGroup);
		}
	}
}
