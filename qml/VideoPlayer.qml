import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia

Video {
	required property url videoSource

	id: videoPreview
	fillMode: VideoOutput.PreserveAspectCrop
	clip: true
	source: videoSource
	volume: 0.0
	focus: true
	width: mainwindow.width * 0.7
	height: mainwindow.width * 0.9
	Layout.margins: 10
	Layout.alignment: Qt.AlignCenter
	Layout.maximumHeight: height
	Layout.maximumWidth: width

	MouseArea {
		anchors.fill: parent
		onClicked: videoPreview.playbackState === Video.PlayingState ? videoPreview.pause() : videoPreview.play();
		onDoubleClicked: {
			videoPreview.stop();
			videoWindow.showFullScreen();
			videoFullScreen.play();
			videoFullScreen.focus = true;
		}
	}

	onErrorOccurred: (error,errorString) => {
		console.log("Video error:")
		console.log(error + ": " + errorString);
	}

	Window {
		id: videoWindow

		Video {
			id: videoFullScreen
			fillMode: VideoOutput.PreserveAspectCrop
			clip: true
			anchors.fill: parent
			source: videoSource
			volume: 0.5
			focus: true

			MouseArea {
				anchors.fill: parent
				onClicked: videoFullScreen.playbackState === Video.PlayingState ? videoFullScreen.pause() : videoFullScreen.play();
				onDoubleClicked: {
					videoFullScreen.stop();
					videoWindow.close();
					videoPreview.play();
					videoPreview.focus = true;
				}
			}

			Keys.onSpacePressed: {
				videoFullScreen.playbackState === Video.PlayingState ? videoFullScreen.pause() : videoFullScreen.play();
			}
			Keys.onLeftPressed: videoFullScreen.position = videoFullScreen.position - 5000
			Keys.onRightPressed: videoFullScreen.position = videoFullScreen.position + 5000
			Keys.onUpPressed: videoFullScreen.volume + 0.1
			Keys.onDownPressed: videoFullScreen.volume - 0.1

			onErrorOccurred: (error,errorString) => {
				console.log("Fullscreen video error:")
				console.log(error + ": " + errorString);
			}
		} // Video
	}
} // Video
