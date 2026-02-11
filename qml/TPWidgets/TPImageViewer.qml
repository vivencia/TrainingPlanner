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
	imageSizeFollowControlSize: openExternally
	fullWindowView: false

	required property string mediaSource
	required property string previewSource
	required property bool openExternally

	MouseArea {
		anchors.fill: parent
		onClicked: {
			if (mediaSource.length > 0) {
				if (!openExternally)
					fullScreenLoader.active = true;
				else
					osInterface.openURL(mediaSource);
			}
		}
	}

	Loader
	{
		id: fullScreenLoader
		asynchronous: true
		active: false

		sourceComponent: Window {
			id: pictureWindow

			property TPImage viewer: largeImage

			TPImage {
				id: largeImage
				source: mediaSource
				dropShadow: false
				antialiasing: true
				imageSizeFollowControlSize: false
				fullWindowView: true
				keepAspectRatio: true
				width: preferredWidth
				height: preferredHeight
				x: (parent.width - width)/2
				y: (parent.height - height)/2

				MouseArea {
					anchors.fill: parent
					onClicked: {
						pictureWindow.close();
						fullScreenLoader.active = false;
					}
				}
			}
		}

		onLoaded: item.showFullScreen();
	}
} // Image
