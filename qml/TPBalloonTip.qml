import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
	property string message: ""
	property string title: ""
	property string button1Text: ""
	property string button2Text: ""
	property string imageSource: ""

	id: balloon
	closePolicy: Popup.NoAutoClose
	modal: false
	parent: Overlay.overlay //global Overlay object. Assures that the dialog is always displayed in relation to global coordinates
	spacing: 0
	padding: 0
	width: mainwindow.width * 0.7
	height: mainwindow.height * 0.4

	background: Rectangle {
		border.color: "black"
		color: "white"
		opacity: 0.7
		radius: 7
	}

	enter: Transition {
		NumberAnimation { properties: "x,y"; from: -300; to: 150 }
	}

	FontMetrics {
		id: fontMetrics
		font.family: textPart.font.family
		font.pixelSize: AppSettings.titleFontSizePixelSize
	}

	Label {
		id: lblTitle
		text: title
		wrapMode: Text.WordWrap
		elide: Text.ElideRight
		font.pixelSize: AppSettings.titleFontSizePixelSize
		font.weight: Font.Black
		width: parent.width - 20
		height: 30
		visible: title.length > 0
		padding: 0

		anchors {
			left: parent.left
			top: parent.top
			leftMargin: 10
			topMargin: 5
		}
	}

	Image {
		id: imgElement
		source: imageSource
		fillMode: Image.PreserveAspectFit
		asynchronous: true
		visible: imageSource.length > 0
		width: 100
		height: 100

		anchors {
			left: parent.left
			top: lblTitle.visible ? lblTitle.bottom : parent.top
			leftMargin: 10
			topMargin: 5
		}
	}

	Label {
		id: lblMessage
		text: message
		wrapMode: Text.WordWrap
		elide: Text.ElideRight
		font.pixelSize: AppSettings.titleFontSizePixelSize
		font.weight: Font.Medium
		width: imgElement.visible ? parent.width - imgElement.width - 20 : parent.width - 20
		height: fontMetrics.boundingRect(message).width > width ? 60 : 30
		visible: message.length > 0
		padding: 0

		anchors {
			left: imgElement.visible ? imgElement.right : parent.left
			top: lblTitle.visible ? lblTitle.bottom : parent.top
			leftMargin: 10
			topMargin: 5
		}
	}

	ButtonFlat {
		id: btn1
		text: button1Text
		visible: button1Text.length > 0

		Component.onCompleted: {
			if (lblMessage.visible)
				anchors.verticalCenter = parent.verticalCenter;
			else
				anchors.top = lblMessage.top;

			if (btn2.visible) {
				anchors.left = parent.left;
				anchors.leftMargin = (parent.width - width - btn2.width) / 3;
			}
			else
				anchors.horizontalCenter = parent.horizontalCenter;
		}
	}

	ButtonFlat {
		id: btn2
		text: button2Text
		visible: button2Text.length > 0

		Component.onCompleted: {
			if (lblMessage.visible)
				anchors.verticalCenter = parent.verticalCenter;
			else
				anchors.top = lblMessage.top;

			if (btn1.visible) {
				anchors.right = parent.right;
				anchors.rightMargin = (parent.width - width - btn1.width) / 3;
			}
			else
				anchors.horizontalCenter = parent.horizontalCenter;
		}
	}
}
