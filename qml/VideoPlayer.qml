import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia

VideoOutput {
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

	property alias mediaSource: mediaPlayer.source
	property alias volume: audioOutput.volume
	property alias position: mediaPlayer.position
	property alias seekable: mediaPlayer.seekable


	MediaPlayer {
		id: mediaPlayer
		videoOutput: videoPreview
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
		onDoubleClicked: {
			mediaPlayer.stop();
			videoWindow.showFullScreen();
			videoFullScreen.play();
			videoFullScreen.focus = true;
		}
	}

	Window {
		id: videoWindow

		VideoOutput {
			id: videoFullScreen
			fillMode: VideoOutput.PreserveAspectCrop
			clip: true
			anchors.fill: parent
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
				mediaPlayer2.playbackState === Video.PlayingState ? mediaPlayer2.pause() : mediaPlayer2.play();
			}
			Keys.onLeftPressed: mediaPlayer2.position = mediaPlayer2.position - 5000
			Keys.onRightPressed: mediaPlayer2.position = mediaPlayer2.position + 5000
			Keys.onUpPressed: audioOutput2.volume + 0.1
			Keys.onDownPressed: audioOutput2.volume - 0.1


			MediaPlayer {
				id: mediaPlayer2
				videoOutput: videoFullScreen
				source: mediaSource
				audioOutput: AudioOutput {
					id: audioOutput2
					volume: 0.5
				}

				onSourceChanged: {
					if (videoWindow.visible) {
						stop();
						play();
					}
				}

				onErrorOccurred: (error,errorString) => {
					console.log("Fullscreen video error:")
					console.log(error + ": " + errorString);
				}

			}
		}
	}
} // Video
