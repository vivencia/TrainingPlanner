import QtQuick
import QtQuick.Controls

import "../ExercisesAndSets"
import "../Pages"

ScrollView {
	required property TPPage parentPage
	property bool navButtonsVisible

	id: control
	contentWidth: availableWidth //stops bouncing to the sides

	anchors {
		fill: parent
		margins: 5
	}

	ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
	ScrollBar.vertical: ScrollBar {
		id: vBar
		policy: ScrollBar.AsNeeded

		anchors {
			top: parent.top
			bottom: parent.bottom
			right: parent.right
		}

		onPositionChanged: {
			if (vBar.position < 0.06) {
				navButtons.showUpButton = false;
				navButtons.showDownButton = true;
			}
			else if (vBar.position - (1 - vBar.size) > -0.029) {
				navButtons.showUpButton = true;
				navButtons.showDownButton = false;
			}
			else {
				navButtons.showUpButton = true;
				navButtons.showDownButton = true;
			}
		}
	}

	Connections {
		target: parentPage
		function onPageActivated(): void {
			if (!navButtons && contentHeight > height)
				createNavButtons();
		}
	}

	function setScrollBarPosition(pos: int): void {
		if (pos === 0)
			vBar.setPosition(0);
		else
			vBar.setPosition(pos - vBar.size);
	}

	property PageScrollButtons navButtons: null
	function createNavButtons(): void {
		let component = Qt.createComponent("qrc:/qml/ExercisesAndSets/PageScrollButtons.qml", Qt.Asynchronous);

		function finishCreation() {
			const coord = control.parentPage.mapToItem(Overlay.overlay, control.parentPage.x, control.parentPage.y + control.height);
			navButtons = component.createObject(control.parentPage, { ownerPage: control.parentPage, showUpButton: false, position: coord,
																										visible: control.navButtonsVisible });
			navButtons.scrollTo.connect(control.setScrollBarPosition);
		}

		if (component.status === Component.Ready)
			finishCreation();
		else
			component.statusChanged.connect(finishCreation);
	}
}
