import QtQuick
import QtQuick.Controls

import "../"
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

TPFloatingControl {
	id: button
	objName: "floatingButton"
	radius: width / 2
	height: cboSetType.height + 10;
	width: cboSetType.width + textAndImageSize + 50
	x: (appSettings.pageWidth-width)/2;
	y: appSettings.pageHeight * 0.5 - height;
	color: appSettings.primaryDarkColor
	dragWidget: buttonText

	property int exerciseIdx
	property string image
	property string text
	property int comboIndex
	property int textAndImageSize: buttonText.width + buttonImage.width

	signal buttonClicked(int settype, int exerciseidx)

	onClicked: buttonClicked(comboIndex, exerciseIdx);

	ToolButton {
		id: btnClose
		width: 20
		height: 20

		anchors {
			left: parent.left
			verticalCenter: parent.verticalCenter
			leftMargin: 5
		}
		onClicked: button.visible = false;

		TPImage {
			anchors.fill: parent
			source: appSettings.iconFolder+"close.png";
		}
	}

	TPComboBox {
		id: cboSetType
		currentIndex: comboIndex
		model: AppGlobals.setTypesModel
		z: 0

		anchors {
			left: parent.left
			verticalCenter: parent.verticalCenter
			leftMargin: 27
		}

		onActivated: { comboIndex = cboSetType.currentValue; }
	}

	TPLabel {
		id: buttonText
		width: 100
		z: 0

		anchors {
			left: cboSetType.right
			verticalCenter: parent.verticalCenter
			leftMargin: 5
		}
	}

	TPImage {
		id: buttonImage
		source: image
		dropShadow: false
		width: 20
		height: 20
		z: 0

		anchors {
			right: parent.right
			rightMargin: 10
			verticalCenter: parent.verticalCenter
		}
	}

	function updateDisplayText(nset: string) {
		buttonText.text = button.text + " #" + nset;
	}
}
