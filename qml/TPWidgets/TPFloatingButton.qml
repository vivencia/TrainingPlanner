import QtQuick
import QtQuick.Controls

import "../"

TPFloatingControl {
	id: button
	objName: "floatingButton"
	radius: width / 2
	height: cboSetType.height + 10;
	width: cboSetType.width + textAndImageSize + 50
	x: (windowWidth-width)/2;
	y: windowHeight * 0.5 - height;
	color: AppSettings.primaryDarkColor
	dragWidget: buttonText

	property int exerciseIdx
	property string image
	property alias text: buttonText.text
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

		Image {
			anchors.fill: parent
			source: "qrc:/images/"+AppSettings.iconFolder+"close.png";
		}
	}

	TPComboBox {
		id: cboSetType
		currentIndex: comboIndex
		z: 0

		anchors {
			left: parent.left
			verticalCenter: parent.verticalCenter
			leftMargin: 27
		}

		onActivated: { comboIndex = cboSetType.currentValue; }
	}

	Label {
		id: buttonText
		color: AppSettings.fontColor
		font.bold: true
		padding: 0
		width: 100
		minimumPointSize: 8
		fontSizeMode: Text.Fit
		z: 0

		anchors {
			left: cboSetType.right
			verticalCenter: parent.verticalCenter
			leftMargin: 5
		}
	}

	Image {
		id: buttonImage
		source: "qrc:/images/"+AppSettings.iconFolder+image;
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
