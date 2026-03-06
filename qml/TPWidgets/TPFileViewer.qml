import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import "../"

TPImage {
	id: fileViewer
	clip: true
	smooth: false
	source: previewSource
	dropShadow: false
	keepAspectRatio: true
	imageSizeFollowControlSize: _media_type !== TPUtils.FT_Image
	fullWindowView: false

	enum WindowStates { WS_UNDEFINED, WS_NORMAL, WS_FULLSCREEN }

	required property string mediaSource
	required property string previewSource

	property int _media_type: TPUtils.FT_UNKNOWN
	property int _window_state: TPFileViewer.WindowStates.WS_UNDEFINED
	property FileOperations _file_ops

	Component.onCompleted: {
		if (_media_type === TPUtils.FT_UNKNOWN)
			_media_type = appUtils.getFileType(mediaSource);
		_window_state = TPFileViewer.WindowStates.WS_NORMAL;
	}

	property Rectangle fileOpsRec: Rectangle {
		radius: 8
		color: appSettings.paneBackgroundColor
		border.color: appSettings.fontColor
		opacity: 0.8
		height: 0
		width: 0
		z: 1

		states: [
			State {
				when: fileViewer._window_state === TPFileViewer.WindowStates.WS_FULLSCREEN

				PropertyChanges {
					target: fileOpsRec
					parent: fullScreenLoader.fullScreenWidget.contentItem
					height: appSettings.itemLargeHeight
					width: (FileOperations.OT_TypeCount * (height + 5))
					x: (fullScreenLoader.fullScreenWidget.contentItem.width - fileOpsRec.width) / 2
					y: fullScreenLoader.fullScreenWidget.contentItem.height - fileOpsRec.height - 10
				}
			},
			State {
				when: fileViewer._window_state === TPFileViewer.WindowStates.WS_NORMAL

				PropertyChanges {
					target: fileOpsRec
					parent: fileViewer
					height: appSettings.itemDefaultHeight
					width: (FileOperations.OT_TypeCount * (height + 5))
					x: (fileViewer.width - fileOpsRec.width) / 2
					y: fileViewer.height - fileOpsRec.height - 10
				}
			}
		]

		FileOperations {
			fileName: mediaSource
			fileType: _media_type
			anchors.fill: parent

			anchors {
				horizontalCenter: parent.horizontalCenter
				verticalCenter: parent.verticalCenter
			}

			Component.onCompleted: fileViewer._file_ops = this;
			onShowFullScreen: fullScreenLoader.showFullScreen();
		}
	}

	property Loader mediaControlsLoader: Loader {
		asynchronous: true
		active: _media_type === TPUtils.FT_VIDEO;
		height: 0
		width: media_controls ? media_controls.availableControls.length * (height + 10) : 0
		z: 1

		readonly property list<int> previewControls: [MediaControls.CT_Play, MediaControls.CT_Stop, MediaControls.CT_Mute]
		readonly property list<int> fullScreenControls: [MediaControls.CT_Play, MediaControls.CT_Stop, MediaControls.CT_Rewind,
								MediaControls.CT_FastForward, MediaControls.CT_VolumeUp, MediaControls.CT_VolumeDown, MediaControls.CT_Mute]
		property MediaControls media_controls

		states: [
			State {
				when: fileViewer._window_state === TPFileViewer.WindowStates.WS_UNDEFINED

				PropertyChanges {
					target: mediaControlsLoader
					parent: null
					height: 0
					x: 0
					y: 0
				}
			},
			State {
				when: fileViewer._window_state === TPFileViewer.WindowStates.WS_NORMAL

				PropertyChanges {
					target: mediaControlsLoader
					parent: fileViewer
					height: appSettings.itemDefaultHeight
					x: (fileViewer.width - mediaControlsLoader.width) / 2
					y: fileViewer.height - mediaControlsLoader.height - fileOpsRec.height - 15
				}
			},
			State {
				when: fileViewer._window_state === TPFileViewer.WindowStates.WS_FULLSCREEN

				PropertyChanges {
					target: mediaControlsLoader
					parent: fullScreenLoader.fullScreenWidget.contentItem
					height: appSettings.itemLargeHeight
					x: (fullScreenLoader.fullScreenWidget.contentItem.width - mediaControlsLoader.width) / 2
					y: fullScreenLoader.fullScreenWidget.contentItem.height - mediaControlsLoader.height - fileOpsRec.height - 15
				}
			}
		]

		sourceComponent: Rectangle {
			radius: 8
			color: appSettings.paneBackgroundColor
			border.color: appSettings.fontColor
			opacity: 0.8

			MediaControls {
				id: mediaControls
				fileOps: fileViewer._file_ops
				anchors.fill: parent

				states: [
					State {
						when: fileViewer._window_state === TPFileViewer.WindowStates.WS_UNDEFINED

						PropertyChanges {
							target: mediaControls
							availableControls: []
						}
					},
					State {
						when: fileViewer._window_state === TPFileViewer.WindowStates.WS_NORMAL

						PropertyChanges {
							target: mediaControls
							availableControls: mediaControlsLoader.previewControls
						}
					},
					State {
						when: fileViewer._window_state === TPFileViewer.WindowStates.WS_FULLSCREEN

						PropertyChanges {
							target: mediaControls
							availableControls: mediaControlsLoader.fullScreenControls
						}
					}
				]

				onControlClicked: (type) => {
					switch (type) {
						case MediaControls.CT_Play:
							mediaPlayerLoader.play();
							setEnabled(MediaControls.CT_Mute, true);
						break;
						case MediaControls.CT_Stop:
							mediaPlayerLoader.stop();
						break;
						case MediaControls.CT_Mute:
							mediaPlayerLoader.mute();
						break;
					}
				}

				onControlPressed: (type) => mediaPlayerLoader.pressed_operations(type, true);
				onControlReleased: (type) => mediaPlayerLoader.pressed_operations(type, false);

				Component.onCompleted: {
					mediaControlsLoader.media_controls = this;
					setEnabled(MediaControls.CT_Mute, false, false);
				}
			}
		}
	} //mediaControlsLoader

	property Loader mediaPlayerLoader: Loader {
		asynchronous: true
		active: false
		x: 0
		y: 0

		property string remainingTime
		property string mediaVolume
		property MediaPlayer mediaPlayer: null
		property AudioOutput audioOutput: null

		sourceComponent: VideoOutput {
			id: videoOutput
			clip: true
			anchors.fill: parent

			MediaPlayer {
				autoPlay: true
				videoOutput: videoOutput
				source: fileViewer.mediaSource

				Component.onCompleted: mediaPlayerLoader.mediaPlayer = this;

				audioOutput: AudioOutput {
					id: audioOutput
					volume: 0.5
					muted: true

					Component.onCompleted: {
						mediaPlayerLoader.audioOutput = this;
						mediaPlayerLoader.mediaVolume = Qt.binding(function() { return audioOutput.muted ?
																				qsTr("Muted") : parseInt(audioOutput.volume * 100, 10); });
					}
				}

				onMediaStatusChanged: {
					if (mediaStatus === MediaPlayer.EndOfMedia)
						mediaControlsLoader.emulateControlClick(MediaControls.CT_Stop);
				}
				onPositionChanged: mediaPlayerLoader.remainingTime = appUtils.formatTime(mediaPlayer.duration - mediaPlayer.position);
			} //MediaPlayer

			TPLabel {
				id: lblTime
				text: mediaPlayerLoader.remainingTime
				useBackground: true
				horizontalAlignment: Text.AlignHCenter
				x: 10
				y: 10
			}
			TPLabel {
				id: lblVolume
				text: qsTr("Volume: ") + mediaPlayerLoader.mediaVolume
				useBackground: true
				horizontalAlignment: Text.AlignHCenter
				x: 10
				y: 15 + lblTime.height
			}

			states: [
				State {
					when: fileViewer._window_state === TPFileViewer.WindowStates.WS_UNDEFINED

					PropertyChanges {
						target: mediaPlayerLoader
						parent: null
					}
				},
				State {
					when: fileViewer._window_state === TPFileViewer.WindowStates.WS_NORMAL

					PropertyChanges {
						target: mediaPlayerLoader
						parent: fileViewer
						width: fileViewer.width
						height: fileViewer.height
					}
					PropertyChanges {
						target: videoOutput
						fillMode: VideoOutput.Stretch
					}
					PropertyChanges {
						target: lblTime
						font: AppGlobals.regularFont
						height: appSettings.itemDefaultHeight
					}
				},
				State {
					when: fileViewer._window_state === TPFileViewer.WindowStates.WS_FULLSCREEN

					PropertyChanges {
						target: mediaPlayerLoader
						parent: fullScreenLoader.fullScreenWidget.contentItem
						width: fullScreenLoader.fullScreenWidget.contentItem.width
						height: fullScreenLoader.fullScreenWidget.contentItem.height
					}
					PropertyChanges {
						target: videoOutput
						fillMode: VideoOutput.PreserveAspectFit
					}
					PropertyChanges {
						target: lblTime
						font: AppGlobals.largeFont
						height: appSettings.itemLargeHeight
					}
				}
			]
		} //sourceCompoent: VideoOutput

		Timer {
			id: pressedTimer
			interval: 500
			repeat: true
			triggeredOnStart: true
		}

		function play(): void {
			if (!active) {
				loaded.connect(play);
				active = true;
			}
			else {
				if (!mediaPlayer.playing)
					mediaPlayer.play();
				else
					mediaPlayer.pause();
			}
		}

		function stop(): void {
			mediaPlayer.stop();
			active = false;
		}

		function mute(): void {
			audioOutput.muted = !audioOutput.muted;
		}


		function pressed_operations(op: int, begin: bool) : void {
			let pressed_function;
			switch (op) {
				case MediaControls.CT_FastForward:
					pressed_function = function () {
						mediaPlayer.position += 5000;
						if (mediaPlayer.position >= mediaPlayer.duration)
							mediaControlsLoader.media_controls.controlReachedLimit(MediaControls.CT_FastForward);
					};
				break;
				case MediaControls.CT_Rewind:
					pressed_function = function () {
						mediaPlayer.position -= 5000;
						if (mediaPlayer.position <= 0)
							mediaControlsLoader.media_controls.controlReachedLimit(MediaControls.CT_Rewind);
					};
				break;
				case MediaControls.CT_VolumeUp:
					pressed_function = function () {
						audioOutput.volume += 0.1;
						if (audioOutput.volume >= 1.0)
							mediaControlsLoader.media_controls.controlReachedLimit(MediaControls.CT_VolumeUp);
					};
				break;
				case MediaControls.CT_VolumeDown:
					pressed_function = function () {
						audioOutput.volume -= 0.1;
						if (audioOutput.volume === 0.0)
							mediaControlsLoader.media_controls.controlReachedLimit(MediaControls.CT_VolumeDown);
					};
				break;
				default: return;
			}

			if (begin) {
				pressedTimer.triggered.connect(pressed_function);
				pressedTimer.start();
			}
			else {
				pressedTimer.stop();
				pressedTimer.triggered.disconnect(pressed_function);
			}
		}
	} //mediaPlayerLoader

	Loader {
		id: fullScreenLoader
		asynchronous: true
		active: false

		property VideoOutput videoOutput
		property Window fullScreenWidget: active ? item : null

		function showFullScreen() : void {
			if (!active) {
				loaded.connect(showFullScreen);
				active = true;
				return;
			}
			const playing = mediaPlayerLoader.mediaPlayer ? mediaPlayerLoader.mediaPlayer.playing : false;
			if (playing)
				mediaControlsLoader.media_controls.emulateControlEvent(MediaControls.CT_Play, false);

			if (fileViewer._window_state === TPFileViewer.WindowStates.WS_NORMAL) {
				fileViewer._window_state = TPFileViewer.WindowStates.WS_UNDEFINED;
				item.showNormal();
				fileViewer._window_state = TPFileViewer.WindowStates.WS_FULLSCREEN;
			}
			else {
				fileViewer._window_state = TPFileViewer.WindowStates.WS_UNDEFINED;
				item.close();
				fileViewer._window_state = TPFileViewer.WindowStates.WS_NORMAL;
				active = false;
			}
			if (playing)
				mediaControlsLoader.media_controls.emulateControlEvent(MediaControls.CT_Play, true);
		}

		sourceComponent: Window {
			contentOrientation: Qt.LandscapeOrientation
			width: 640
			height: 480

			Rectangle {
				color: "#000000"
				anchors.fill: parent
			}

			Loader {
				asynchronous: true
				active: _media_type === TPUtils.FT_IMAGE;

				TPImage {
					source: fileViewer.mediaSource
					dropShadow: false
					antialiasing: true
					imageSizeFollowControlSize: false
					fullWindowView: true
					keepAspectRatio: true
					width: preferredWidth
					height: preferredHeight
					x: (parent.width - width)/2
					y: (parent.height - height)/2
				}
			} //Loader
		} //Window fullViewWindow
	} //Loader fullScreenLoader
} // TPImage
