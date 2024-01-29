import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia

Image {
	required property string mediaSource

	id: imagePreview
	fillMode: Image.PreserveAspectFit
	asynchronous: true
	clip: true
	source: mediaSource
	width: mainwindow.width * 0.7
	height: mainwindow.width * 0.9
	Layout.margins: 10
	Layout.alignment: Qt.AlignCenter
	Layout.maximumHeight: height
	Layout.maximumWidth: width

	MouseArea {
		anchors.fill: parent
		onDoubleClicked: pictureWindow.showFullScreen();
		onClicked: console.log("imagePreview.source:  ", imagePreview.source);
	}

	Window {
		id: pictureWindow

		Image {
			id: photoFullScreen
			anchors.fill: parent
			fillMode: Image.PreserveAspectFit
			asynchronous: true
			source: mediaSource

			MouseArea {
				anchors.fill: parent
				onClicked: pictureWindow.close();
			}
		}
	}
} // Image
