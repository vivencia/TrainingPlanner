import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

TPImage {
	id: imagePreview
	clip: true
	smooth: false
	source: previewSource
	dropShadow: false
	keepAspectRatio: true
	imageSizeFollowControlSize: true
	fullWindowView: false

	required property string mediaSource
	required property string previewSource
	required property bool openExternally

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

		TPImage {
			id: largeImage
			dropShadow: false
			antialiasing: true
			imageSizeFollowControlSize: false
			fullWindowView: true
			keepAspectRatio: true
			anchors.fill: parent

			MouseArea {
				anchors.fill: parent
				onClicked: pictureWindow.close();
			}
		}
	}
} // Image
