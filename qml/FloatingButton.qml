import QtQuick
import QtQuick.Controls

import com.vivenciasoftware.qmlcomponents

Rectangle {
	id: button
	color: paneBackgroundColor
	opacity: bHeld ? 0.7 : 1
	radius: width / 2
	parent: Overlay.overlay //global Overlay object. Assures that the dialog is always displayed in relation to global coordinates

	required property DBTrainingDayModel tDayModel
	property int exerciseIdx

	property string text
	property string image
	property bool bHasText: text.length > 1
	property bool bHasImage: image.length > 1
	property int comboIndex

	property bool bHeld: false

	signal buttonClicked(int settype, int exerciseidx)
	property bool bEmitSignal: false

	property int textAndImageSize: (bHasText ? buttonText.width + (bHasImage ? buttonImage.width: 0) : bHasImage ? buttonImage.width: 0)

	readonly property var setTypesModel: [ { text:qsTr("Regular"), value:0 }, { text:qsTr("Pyramid"), value:1 }, { text:qsTr("Drop Set"), value:2 },
							{ text:qsTr("Cluster Set"), value:3 }, { text:qsTr("Giant Set"), value:4 }, { text:qsTr("Myo Reps"), value:5 } ]

	height: Math.max(buttonText.height, buttonImage.height) + 10;
	width: cboSetType.width + textAndImageSize + 50

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
			source: "qrc:/images/"+lightIconFolder+"close.png";
		}
	}

	TPComboBox {
		id: cboSetType
		model: setTypesModel
		width: 100
		currentIndex: comboIndex
		z: 0

		anchors {
			left: parent.left
			verticalCenter: parent.verticalCenter
			leftMargin: 27
		}

		onActivated: { comboIndex = cboSetType.currentValue; }
	}

	Text {
		id: buttonText
		color: "white"
		padding: 0
		visible: bHasText
		z: 0

		anchors {
			left: cboSetType.right
			verticalCenter: parent.verticalCenter
			leftMargin: 5
		}
	}

	Image {
		id: buttonImage
		visible: bHasImage
		source: "qrc:/images/"+lightIconFolder+image;
		width: bHasText > 1 ? 20 : 20
		height: bHasText > 1 ? 20 : 20
		z: 0
		anchors {
			right: parent.right
			rightMargin: 10
			verticalCenter: parent.verticalCenter
		}
	}

	MouseArea {
		property var prevPos
		anchors {
			top: parent.top
			bottom: parent.bottom
			left: cboSetType.right
			right: parent.right
		}

		z: 1

		onClicked: (mouse) => {
			if (!mouse.wasHeld)
				bEmitSignal = true;
		}

		onReleased: {
			bHeld = false;
		}

		onPressed: (mouse) => {
			prevPos = { x: mouse.x, y: mouse.y };
			bHeld = true;
			anim.start();
		}

		onPositionChanged: {
			if (bHeld) {
				const deltaX = mouseX - prevPos.x;
				if (Math.abs(deltaX) < 10) {
					const deltaY = mouseY - prevPos.y;
					if (Math.abs(deltaY) < 10) {
						button.x += deltaX;
						button.y += deltaY;
					}
				}
				prevPos = { x: mouseX, y: mouseY };
			}
		}
	}

	SequentialAnimation {
		id: anim
		alwaysRunToEnd: true

		// Expand the button
		PropertyAnimation {
			target: button
			property: "scale"
			to: 1.5
			duration: 200
			easing.type: Easing.InOutCubic
		}

		// Shrink back to normal
		PropertyAnimation {
			target: button
			property: "scale"
			to: 1.0
			duration: 200
			easing.type: Easing.InOutCubic
		}

		onFinished: {
			if (bEmitSignal) {
				bEmitSignal = false;
				buttonClicked(comboIndex, exerciseIdx);
			}
		}
	}

	Component.onCompleted: {
		x = 10;
		y = windowHeight * 0.5 - height;
		mainwindow.mainMenuOpened.connect(hideButtons);
		mainwindow.mainMenuClosed.connect(showButtons);
	}

	function updateDisplayText() {
		buttonText.text = button.text + " #" + (tDayModel.setsNumber(exerciseIdx) + 1).toString();
	}

	function hideButtons() {
		if (button)
			button.visible = false;
	}

	function showButtons() {
		if (button)
			button.visible = true;
	}
}
