pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import QtQuick.Pdf

import TpQml

TPImage {
	id: _fileViewer
	smooth: false
	source: _preview_source
	dropShadow: false
	keepAspectRatio: true
	imageSizeFollowControlSize: _file_ops.fileType !== AppUtils.FT_IMAGE
	fullWindowView: false

//public:
	required property string mediaSource

	signal removalRequested();

//private:
	enum WindowStates { WS_UNDEFINED, WS_NORMAL, WS_FULLSCREEN }

	property string _preview_source
	property int _window_state: TPFileViewer.WindowStates.WS_UNDEFINED
	property FileOperations _file_ops
	readonly property string _media_uri: "file://" + _fileViewer.mediaSource

	Component.onCompleted: {
		mediaSource = AppUtils.getCorrectPath(mediaSource);
		_window_state = TPFileViewer.WindowStates.WS_NORMAL;
	}

	property Rectangle fileOpsRec: Rectangle {
		radius: 8
		color: AppSettings.paneBackgroundColor
		border.color: AppSettings.fontColor
		opacity: 0.8
		height: 0
		width: 0
		z: 1

		states: [
			State {
				when: _fileViewer._window_state === TPFileViewer.WindowStates.WS_FULLSCREEN

				ParentChange {
					target: _fileViewer.fileOpsRec
					parent: fullScreenLoader.fullScreenWidget.contentItem
					height: AppSettings.itemLargeHeight
					width: (FileOperations.OT_TypeCount * (height + 5))
					x: (fullScreenLoader.fullScreenWidget.contentItem.width - fileOpsRec.width) / 2
					y: fullScreenLoader.fullScreenWidget.contentItem.height - fileOpsRec.height - 10
				}
			},
			State {
				when: _fileViewer._window_state === TPFileViewer.WindowStates.WS_NORMAL

				ParentChange {
					target: _fileViewer.fileOpsRec
					parent: _fileViewer
					height: AppSettings.itemDefaultHeight
					width: (FileOperations.OT_TypeCount * (height + 5))
					x: (_fileViewer.width - fileOpsRec.width) / 2
					y: _fileViewer.height - fileOpsRec.height - 10
				}
			}
		]

		FileOperations {
			fileName: _fileViewer.mediaSource
			anchors.fill: parent

			anchors {
				horizontalCenter: parent.horizontalCenter
				verticalCenter: parent.verticalCenter
			}

			Component.onCompleted: {
				_fileViewer._file_ops = this;
				_fileViewer._preview_source = getFileTypeIcon(_fileViewer.mediaSource, Qt.size(0, 0), true);
			}
			onShowFullScreen: fullScreenLoader.showFullScreen();
			onFileRemovalRequested: _fileViewer.removalRequested();
		}
	}

	property Loader mediaControlsLoader: Loader {
		asynchronous: true
		active: _fileViewer._file_ops.fileType === AppUtils.FT_VIDEO
		height: 0
		width: media_controls ? media_controls.availableControls.length * (height + 10) : 0
		z: 1

		readonly property list<int> previewControls: [MediaControls.CT_Play, MediaControls.CT_Stop, MediaControls.CT_Mute]
		readonly property list<int> fullScreenControls: [MediaControls.CT_Play, MediaControls.CT_Stop, MediaControls.CT_Rewind,
								MediaControls.CT_FastForward, MediaControls.CT_VolumeUp, MediaControls.CT_VolumeDown, MediaControls.CT_Mute]
		property MediaControls media_controls

		states: [
			State {
				when: _fileViewer._window_state === TPFileViewer.WindowStates.WS_UNDEFINED

				ParentChange {
					target: _fileViewer.mediaControlsLoader
					parent: null
					height: 0
					x: 0
					y: 0
				}
			},
			State {
				when: _fileViewer._window_state === TPFileViewer.WindowStates.WS_NORMAL

				ParentChange {
					target: _fileViewer.mediaControlsLoader
					parent: _fileViewer
					height: AppSettings.itemDefaultHeight
					x: (_fileViewer.width - mediaControlsLoader.width) / 2
					y: _fileViewer.height - mediaControlsLoader.height - fileOpsRec.height - 15
				}
			},
			State {
				when: _fileViewer._window_state === TPFileViewer.WindowStates.WS_FULLSCREEN

				ParentChange {
					target: _fileViewer.mediaControlsLoader
					parent: fullScreenLoader.fullScreenWidget.contentItem
					height: AppSettings.itemLargeHeight
					x: (fullScreenLoader.fullScreenWidget.contentItem.width - mediaControlsLoader.width) / 2
					y: fullScreenLoader.fullScreenWidget.contentItem.height - mediaControlsLoader.height - fileOpsRec.height - 15
				}
			}
		]

		sourceComponent: Rectangle {
			radius: 8
			color: AppSettings.paneBackgroundColor
			border.color: AppSettings.fontColor
			opacity: 0.8

			MediaControls {
				id: mediaControls
				fileOps: _fileViewer._file_ops
				anchors.fill: parent

				states: [
					State {
						when: _fileViewer._window_state === TPFileViewer.WindowStates.WS_UNDEFINED

						PropertyChanges {
							explicit: true
							availableControls: []
						}
					},
					State {
						when: _fileViewer._window_state === TPFileViewer.WindowStates.WS_NORMAL

						PropertyChanges {
							explicit: true
							availableControls: mediaControlsLoader.previewControls
						}
					},
					State {
						when: _fileViewer._window_state === TPFileViewer.WindowStates.WS_FULLSCREEN

						PropertyChanges {
							explicit: true
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
					setEnabled(MediaControls.CT_Mute, false);
				}
			}
		}
	} //mediaControlsLoader

	property Loader mediaPlayerLoader: Loader {
		asynchronous: true
		active: false
		anchors.fill: parent

		property string remainingTime
		property string mediaVolume
		property MediaPlayer mediaPlayer: null
		property AudioOutput audioOutput: null

		sourceComponent: VideoOutput {
			id: videoOutput
			clip: true

			MediaPlayer {
				autoPlay: true
				videoOutput: videoOutput
				source: _fileViewer._media_uri

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
						mediaControlsLoader.media_controls.emulateControlClick(MediaControls.CT_Stop);
				}
				onPositionChanged: mediaPlayerLoader.remainingTime = AppUtils.formatTime(mediaPlayer.duration - mediaPlayer.position);
			} //MediaPlayer

			TPLabel {
				id: lblTime
				text: mediaPlayerLoader.remainingTime
				useBackground: true
				horizontalAlignment: Text.AlignHCenter
				x: 10
				y: 10
				states: [
					State {
						when: _fileViewer._window_state === TPFileViewer.WindowStates.WS_NORMAL
						PropertyChanges {
							explicit: true
							font: AppGlobals.regularFont
							height: AppSettings.itemDefaultHeight
						}
					},
					State {
						when: _fileViewer._window_state === TPFileViewer.WindowStates.WS_FULLSCREEN
						PropertyChanges {
							explicit: true
							font: AppGlobals.largeFont
							height: AppSettings.itemLargeHeight
						}
					}
				]
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
					when: _fileViewer._window_state === TPFileViewer.WindowStates.WS_UNDEFINED

					ParentChange {
						target: _fileViewer.mediaPlayerLoader
						parent: null
					}
				},
				State {
					when: _fileViewer._window_state === TPFileViewer.WindowStates.WS_NORMAL

					ParentChange {
						target: _fileViewer.mediaPlayerLoader
						parent: _fileViewer
						width: _fileViewer.width
						height: _fileViewer.height
					}
					PropertyChanges {
						explicit: true
						fillMode: VideoOutput.Stretch
					}
				},
				State {
					when: _fileViewer._window_state === TPFileViewer.WindowStates.WS_FULLSCREEN

					ParentChange {
						target: _fileViewer.mediaPlayerLoader
						parent: fullScreenLoader.fullScreenWidget.contentItem
						width: fullScreenLoader.fullScreenWidget.contentItem.width
						height: fullScreenLoader.fullScreenWidget.contentItem.height
					}
					PropertyChanges {
						explicit: true
						fillMode: VideoOutput.PreserveAspectFit
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

		Component.onCompleted: loaded.connect(showFullScreen);
		property VideoOutput videoOutput
		property Window fullScreenWidget: active ? item : null

		function showFullScreen() : void {
			if (!active) {
				active = true;
				return;
			}
			const playing = mediaPlayerLoader.mediaPlayer ? mediaPlayerLoader.mediaPlayer.playing : false;
			if (playing)
				mediaControlsLoader.media_controls.emulateControlClick(MediaControls.CT_Play);

			if (_fileViewer._window_state === TPFileViewer.WindowStates.WS_NORMAL) {
				_fileViewer._window_state = TPFileViewer.WindowStates.WS_UNDEFINED;
				item.showFullScreen();
				_fileViewer._window_state = TPFileViewer.WindowStates.WS_FULLSCREEN;
			}
			else
				item.close();

			if (playing)
				mediaControlsLoader.media_controls.emulateControlClick(MediaControls.CT_Play);
		}

		sourceComponent: Window {
			contentOrientation: Qt.LandscapeOrientation
			width: 640
			height: 480
			onClosing: (close_event) => {
				_fileViewer._window_state = TPFileViewer.WindowStates.WS_UNDEFINED;
				if (mediaControlsLoader.active)
					mediaControlsLoader.media_controls.emulateControlClick(MediaControls.CT_Stop);
				fullScreenLoader.active = false;
				_fileViewer._window_state = TPFileViewer.WindowStates.WS_NORMAL;
			}

			Rectangle {
				color: "#000000"
				anchors.fill: parent
			}

			Loader {
				asynchronous: true
				active: _fileViewer._file_ops.fileType === AppUtils.FT_IMAGE
				anchors.fill: parent

				sourceComponent: TPImage {
					source: _fileViewer.mediaSource
					dropShadow: false
					antialiasing: true
					imageSizeFollowControlSize: false
					fullWindowView: true
					keepAspectRatio: true
				}
			} //Loader : TPImage

			Loader {
				asynchronous: true
				active: _fileViewer._file_ops.fileType === AppUtils.FT_PDF
				anchors.fill: parent

				sourceComponent: PdfMultiPageView {
					id: pdfViewer
					document: PdfDocument {
						source: _fileViewer._media_uri
					}

					Connections {
						target: _fileViewer._file_ops
						function onMultimediaKeyPressed(key: int): void {
							switch (key) {
							case Qt.Key_Left:
								if (pdfViewer.forwardEnabled)
									pdfViewer.forward();
								break;
							case Qt.Key_Right:
								if (pdfViewer.backEnabled)
									pdfViewer.back();
								break;
							case Qt.Key_Up:
								pdfViewer.goToPage(0);
								break;
							case Qt.Key_Down:
								pdfViewer.goToPage(pdfViewer.document.pageCount - 1);
								break;
							}
						}
					}
				}
			} //Loader : PdfMultiPageView

			Loader {
				asynchronous: true
				active: _fileViewer._file_ops.fileType < AppUtils.FT_IMAGE
				anchors.fill: parent

				sourceComponent: Item {
					id: _tpFileItem
					TabBar {
						id: tabSections
						height: AppSettings.itemLargeHeight
						clip: true
						currentIndex: sectionsLayout.currentIndex

						anchors {
							top: _tpFileItem.top
							topMargin: 10
							horizontalCenter: _tpFileItem.horizontalCenter
						}

						Repeater {
							id: tabSectionsRepeater
							model: _fileViewer._file_ops.tpFileSectionCount

							TPTabButton {
								text: _fileViewer._file_ops.tpFileSectionTitle(index);
								parentTab: tabSections

								onClicked: sectionsLayout.currentIndex = index;
							} //TPTabButton
						} //Repeater: tabSectionsRepeater
					} //TabBar: tabSections

					StackLayout {
						id: sectionsLayout

						anchors {
							fill: parent
							topMargin: AppSettings.itemLargeHeight + 20
						}

						Repeater {
							id: sectionsRepeater
							model: _fileViewer._file_ops.tpFileSectionCount

							TPMultiLineEdit {
								id: _multiline_edit
								text: _fileViewer._file_ops.tpFileSection(index);
								maxHeight: -1
								minHeight: height
								width: sectionsLayout.width
								height: sectionsLayout.height

								required property int index

								onTextControlChanged: {
									textControl.cursorPositionChanged.connect(function () {
																	_file_ops.setWorkingDocumentCursorPosition(textControl.cursorPosition); });
								}

								Connections {
									target: _fileViewer._file_ops
									function onSetCursorPorsition(cursor_pos: int) : void {
										if (sectionsLayout.currentIndex === _multiline_edit.index)
											_multiline_edit.textControl.cursorPosition = cursor_pos;
									}
									function onInsertString(str: string, pos: int): void {
										_multiline_edit.textControl.insert(pos, str);
									}
								}
								Connections {
									target: sectionsLayout
									function onCurrentIndexChanged(): void {
										if (sectionsLayout.currentIndex === _multiline_edit.index)
											_fileViewer._file_ops.setWorkingTextDocument(_multiline_edit.textControl.textDocument);
									}
								}
							} //TPMultiLineEdit
						} //Repeater: tabSectionsRepeater
					} //StackLayout: sectionsLayout
				} //Item
			} //Loader : TPMultiLineEdit
		} //Window fullViewWindow
	} //Loader fullScreenLoader
} // TPImage
