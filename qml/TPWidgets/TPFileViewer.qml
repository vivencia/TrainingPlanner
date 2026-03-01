import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import "../"

TPImage {
	id: imagePreview
	clip: true
	smooth: false
	source: previewSource
	dropShadow: false
	keepAspectRatio: true
	imageSizeFollowControlSize: mediaType !== TPUtils.FT_Image
	fullWindowView: false

	required property string mediaSource
	required property string previewSource
	property int mediaType: TPUtils.FT_UNKNOWN

	Component.onCompleted: {
		if (mediaType === TPUtils.FT_UNKNOWN)
			mediaType = appUtils.getFileType(mediaSource);
	}

	Rectangle {
		id: fileOpsRec
		radius: 8
		color: appSettings.paneBackgroundColor
		border.color: appSettings.fontColor
		opacity: 0.8
		height: appSettings.itemDefaultHeight
		width: appSettings.itemDefaultHeight * 4 + 20

		anchors {
			horizontalCenter: parent.horizontalCenter
			bottom: parent.bottom
			bottomMargin: 10
		}

		FileOperations {
			fileName: mediaSource
			fileType: mediaType
			anchors.fill: parent

			anchors {
				horizontalCenter: parent.horizontalCenter
				verticalCenter: parent.verticalCenter
			}
		}
	}

	Loader {
		id: videoPreviewControlsLoader
		asynchronous: true
		active: mediaType === TPUtils.FT_VIDEO;
		height: appSettings.itemDefaultHeight
		width: parent.width / 2
		z: 1

		anchors {
			horizontalCenter: parent.horizontalCenter
			bottom: fileOpsRec.top
			bottomMargin: 5
		}

		sourceComponent: Rectangle {
			radius: 8
			color: appSettings.paneBackgroundColor
			border.color: appSettings.fontColor
			opacity: 0.8

			MediaControls {
				availableControls: [ MediaControls.CT_Play, MediaControls.CT_Stop, MediaControls.CT_Mute]
				anchors.fill: parent

				anchors {
					horizontalCenter: parent.horizontalCenter
					verticalCenter: parent.verticalCenter
				}

				onControlClicked: (type) => {
					switch (type) {
						case MediaControls.CT_Play:
							previewMediaPlayerLoader.play();
							setEnabled(MediaControls.CT_Mute, true);
						break;
						case MediaControls.CT_Stop:
							previewMediaPlayerLoader.stop();
						break;
						case MediaControls.CT_Mute:
							previewMediaPlayerLoader.sound();
						break;
					}
				}

				Component.onCompleted: setEnabled(MediaControls.CT_Mute, false, false);
			}
		}
	} //videoPreviewControlerLoader

	Loader {
		id: previewMediaPlayerLoader
		asynchronous: true
		active: false
		anchors.fill: parent

		property string remainingTime
		property real media_volume: 0.0

		sourceComponent: VideoOutput {
			id: videoPreview
			fillMode: VideoOutput.Stretch
			clip: true
			focus: false

			property alias playing: mediaPlayer.playing
			property alias seekable: mediaPlayer.seekable

			MediaPlayer {
				id: mediaPlayer
				videoOutput: videoPreview
				autoPlay: true
				source: imagePreview.mediaSource

				audioOutput: AudioOutput {
					id: audioOutput
					volume: previewMediaPlayerLoader.media_volume
				}
				onMediaStatusChanged: {
					if (mediaStatus === MediaPlayer.EndOfMedia)
						imagePreview.setControlEnabled(MediaControls.CT_Stop, false);
				}
				onPositionChanged: {
					previewMediaPlayerLoader.remainingTime = appUtils.formatTime(mediaPlayer.duration - mediaPlayer.position);
				}
			}

			function play(): void {
				mediaPlayer.play();
			}
			function pause() : void {
				mediaPlayer.pause();
			}
			function stop() : void {
				mediaPlayer.stop();
			}
		}

		function play(): void {
			if (!active) {
				loaded.connect(play);
				active = true;
			}
			else {
				if (!item.playing)
					item.play();
				else
					item.pause();
			}
		}

		function stop(): void {
			item.stop();
			active = false;
		}

		function sound(): void {
			if (media_volume == 0.0)
				media_volume = 0.5;
			else
				media_volume = 0.0;
		}
	} //previewMediaPlayerLoader

	Loader {
		id: videoLengthLoader
		asynchronous: true
		active: previewMediaPlayerLoader.active
		height: appSettings.itemDefaultHeight
		width: parent.width / 3
		x: 10
		y: 10

		sourceComponent: TPLabel {
			text: previewMediaPlayerLoader.remainingTime
			font: AppGlobals.smallFont
			horizontalAlignment: Text.AlignHCenter
		}
	}

	Loader
	{
		id: fullScreenLoader
		asynchronous: true
		active: false

		sourceComponent: Window {
			id: fullViewWindow

			Loader {
				id: fullImageLoader
				asynchronous: true
				active: mediaType === TPUtils.FT_IMAGE;

				TPImage {
					id: largeImage
					source: imagePreview.mediaSource
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
							fullViewWindow.close();
							fullScreenLoader.active = false;
						}
					}
				}
			}

			Loader {
				id: fullVideoLoader
				asynchronous: true
				active: mediaType === TPUtils.FT_IMAGE;

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
							fullViewWindow.close();
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
						source: imagePreview.mediaSource
						audioOutput: AudioOutput {
							id: audioOutput2
							volume: 0.5
						}

						onSourceChanged: {
							if (fullViewWindow.visible) {
								stop();
								play();
							}
						}

						onErrorOccurred: (error,errorString) => {
							console.log("Fullscreen video error:")
							console.log(error + ": " + errorString);
						}
					} //mediaPlayer2
				} //videoFullScreen
			} //fullVideoLoader
		}

		onLoaded: item.showFullScreen();
	}

	function setControlEnabled(control_type: int, enabled: bool) : void {

	}
} // Image
