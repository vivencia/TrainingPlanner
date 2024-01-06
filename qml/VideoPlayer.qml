import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia

VideoOutput {
	//required property url videoSource

	id: videoPreview
	fillMode: VideoOutput.PreserveAspectCrop
	clip: true
	focus: true
	width: mainwindow.width * 0.7
	height: mainwindow.width * 0.9
	Layout.margins: 10
	Layout.alignment: Qt.AlignCenter
	Layout.maximumHeight: height
	Layout.maximumWidth: width

	property alias duration: mediaPlayer.duration
	property alias mediaSource: mediaPlayer.source
	property alias metaData: mediaPlayer.metaData
	property alias playbackRate: mediaPlayer.playbackRate
	property alias position: mediaPlayer.position
	property alias seekable: mediaPlayer.seekable
	property alias volume: audioOutput.volume

	MediaPlayer{
		id: mediaPlayer
		videoOutput: videoPreview;
		audioOutput: AudioOutput {
			id: audioOutput
			volume: 0.0
		}

		onSourceChanged: {
			stop();
			play();
		}

		onErrorOccurred: (error,errorString) => {
			console.log("Video error:")
			console.log(error + ": " + errorString);
		}
	}

	MouseArea {
		anchors.fill: parent
		onClicked: mediaPlayer.playbackState === Video.PlayingState ? mediaPlayer.pause() : mediaPlayer.play();
		/*onDoubleClicked: {
			mediaPlayer.stop();
			videoWindow.showFullScreen();
			videoFullScreen.play();
			videoFullScreen.focus = true;
		}*/
	}

	/*Window {
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
	}*/
} // Video
