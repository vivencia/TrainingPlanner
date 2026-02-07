import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia

Image {
	property string mediaSource
	property string previewSource
	property bool openExternally

	id: imagePreview
	fillMode: Image.PreserveAspectFit
	asynchronous: true
	clip: true
	source: previewSource

	MouseArea {
		anchors.fill: parent
		onClicked: {
			if (mediaSource.length > 0) {
				if (!openExternally) {
					largeImage.source = mediaSource;
					pictureWindow.showFullScreen();
				}
				else
					osInterface.openURL(mediaSource);
			}
		}
	}

	Window {
		id: pictureWindow

		Image {
			id: largeImage
			anchors.fill: parent
			fillMode: Image.PreserveAspectFit
			asynchronous: true

			MouseArea {
				anchors.fill: parent
				onClicked: pictureWindow.close();
			}
		}
	}
} // Image
