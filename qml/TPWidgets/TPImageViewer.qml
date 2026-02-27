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
	imageSizeFollowControlSize: mediaType !== TPUtils.FT_Image
	fullWindowView: false

	required property string mediaSource
	required property string previewSource
	property int mediaType: TPUtils.FT_UNKNOWN

	Component.onCompleted: {
		if (mediaType === TPUtils.FT_UNKNOWN)
			mediaType = appUtils.getFileType(mediaSource);
	}

	Loader {
		id: videoPreviewControlsLoader
		asynchronous: true
		active: mediaType === TPUtils.FT_VIDEO;
		height: appSettings.itemDefaultHeight
		width: (height * 2) + 30
		z: 2

		anchors {
			horizontalCenter: parent.horizontalCenter
			bottom: parent.bottom
			bottomMargin: 10
		}

		sourceComponent: Rectangle {
			radius: 8
			color: appSettings.paneBackgroundColor
			border.color: appSettings.fontColor
			opacity: 0.8

			MediaControls {
				availableControls: [ MediaControls.CT_Play, MediaControls.CT_Mute]
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
						case MediaControls.CT_Mute:
							previewMediaPlayerLoader.sound();
						break;
					}
				}

				Component.onCompleted: setEnabled(MediaControls.CT_Mute, false);
			}
		}
	}

	Loader {
		id: previewMediaPlayerLoader
		asynchronous: true
		active: false
		anchors.fill: parent

		sourceComponent: VideoOutput {
			id: videoPreview
			fillMode: VideoOutput.Stretch
			clip: true
			focus: false

			property string mediaSource: imagePreview.mediaSource
			property alias playing: mediaPlayer.playing
			property alias position: mediaPlayer.position
			property alias seekable: mediaPlayer.seekable
			property alias volume: mediaPlayer.audioOutput.volume

			MediaPlayer {
				id: mediaPlayer
				videoOutput: videoPreview
				audioOutput: AudioOutput {
					id: audioOutput
					volume: 0.0
				}
			}

			function play(): void {
				mediaPlayer.play();
			}
			function pause() : void {
				mediaPlayer.pause();
			}

			function setVolume(vol: real): void {
				mediaPlayer.audioOutput.volume = vol;
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

		function sound(): void {
			if (item.volume === 0.0)
				item.setVolume(0.5);
			else
				item.setVolume(0.0);
		}
	}

	MouseArea {
		anchors.fill: parent
		onClicked: {
			switch (mediaType) {
				case TPUtils.FT_UNKNOWN:
				break;
				case TPUtils.FT_IMAGE:
					fullScreenLoader.active = true;
				break;
				case TPUtils.FT_VIDEO:
					//fullScreenLoader.active = true;
				break;
				default:
					osInterface.openURL(imagePreview.mediaSource);
				break;
			}
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
} // Image
