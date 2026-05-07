pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Pdf

import TpQml

Item {
	id: _control
	width: minimumWidth
	height: minimumHeight

//public:
	required property string mediaSource
	property bool canAddFile: false
	readonly property int minimumWidth: _file_ops.controlSize.width
	readonly property int minimumHeight: minimumWidth

	signal removalRequested()
	signal fileAdded(filepath: string)

//private:
	enum WindowStates { WS_UNDEFINED, WS_NORMAL, WS_FULLSCREEN }

	property string _preview_source
	property int _window_state: TPFileViewer.WindowStates.WS_UNDEFINED
	property FileOperations _file_ops
	property TPMediaPlayer _media_player
	property Item _full_screen_widget

	Component.onCompleted: {
		mediaSource = AppUtils.getCorrectPath(mediaSource);
		_window_state = TPFileViewer.WindowStates.WS_NORMAL;
	}

	states: [
		State {
			when: _control._window_state === TPFileViewer.WindowStates.WS_FULLSCREEN

			ParentChange {
				target: fileOpsRec
				parent: _control._full_screen_widget
			}
			ParentChange {
				target: mediaPlayerLoader
				parent: _control._full_screen_widget
			}
		},
		State {
			when: _control._window_state === TPFileViewer.WindowStates.WS_NORMAL

			ParentChange {
				target: fileOpsRec
				parent: _control
			}
			ParentChange {
				target: mediaPlayerLoader
				parent: _control
			}
		}
	]

	Loader {
		asynchronous: true
		active: _control._file_ops.fileType === AppUtils.FT_TEXT
		anchors.fill: parent

		sourceComponent: Label {
			text: _control._file_ops.getFileText(true);
			font.pixelSize: 0.05 * _control.height
			color: "black"
			padding: 10
			wrapMode: Text.Wrap

			background: Rectangle { color: "white"; border.color: "black"; }
		}
	}

	Loader {
		asynchronous: true
		active: _control._file_ops.fileType !== AppUtils.FT_TEXT
		anchors.fill: parent

		sourceComponent: TPImage {
			id: _imagePreview
			smooth: false
			source: _control._preview_source
			dropShadow: false
			keepAspectRatio: true
			imageSizeFollowControlSize: _control._file_ops.fileType !== AppUtils.FT_IMAGE
			fullWindowView: false
		}
	}

	Rectangle {
		id: fileOpsRec
		radius: 8
		color: AppSettings.paneBackgroundColor
		border.color: AppSettings.fontColor
		opacity: 0.8
		width: file_ops.controlSize.width
		height: file_ops.controlSize.height
		z: 1

		anchors {
			horizontalCenter: parent.horizontalCenter
			bottom: parent.bottom
			bottomMargin: 10
		}

		FileOperations {
			id: file_ops
			fileName: _control.mediaSource
			canAddFile: _control.canAddFile

			anchors {
				horizontalCenter: parent.horizontalCenter
				verticalCenter: parent.verticalCenter
			}

			Component.onCompleted: _control._file_ops = this;
			onShowFullScreen: fullScreenLoader.showFullScreen();
			onFileRemovalRequested: _control.removalRequested();
			onFileNameChanged: if (canAddFile) _control.fileAdded(fileName);
			onFileTypeChanged: _control._preview_source = getFileTypeIcon(fileName, Qt.size(0, 0), true);
		}
	}

	Loader {
		id: mediaPlayerLoader
		asynchronous: true
		active: _control._file_ops.fileType === AppUtils.FT_VIDEO
		anchors.fill: parent

		sourceComponent: TPMediaPlayer {
			mediaUrl: _control._file_ops.fileURL
			fileOps: _control._file_ops
			windowState: _control._window_state
			Component.onCompleted: _control._media_player = this;

		} //sourceCompoent: TPMediaPlayer
	} //mediaPlayerLoader

	Loader {
		id: fullScreenLoader
		asynchronous: true
		active: false

		Component.onCompleted: loaded.connect(showFullScreen);
		property Window _window

		function showFullScreen() : void {
			if (!active) {
				active = true;
				return;
			}

			if (_control._window_state === TPFileViewer.WS_NORMAL) {
				_window.showFullScreen();
				_control._window_state = TPFileViewer.WS_FULLSCREEN;
				_control._file_ops.repaintControls();
			}
			else {
				_window.close();
				fullScreenLoader.active = false;
				_control._window_state = TPFileViewer.WindowStates.WS_NORMAL;
				_control._file_ops.repaintControls();
			}

			if (_control._media_player)
				_control._media_player.changeState(_control._window_state);
		}

		sourceComponent: Window {
			contentOrientation: Qt.LandscapeOrientation
			width: 640
			height: 480
			Component.onCompleted: {
				fullScreenLoader._window = this;
				_control._full_screen_widget = contentItem;
			}

			Rectangle {
				color: "#000000"
				anchors.fill: parent
			}

			Loader {
				asynchronous: true
				active: _control._file_ops.fileType === AppUtils.FT_IMAGE
				anchors.fill: parent

				sourceComponent: TPImage {
					source: _control.mediaSource
					dropShadow: false
					antialiasing: true
					imageSizeFollowControlSize: false
					fullWindowView: true
					keepAspectRatio: true
				}
			} //Loader : TPImage

			Loader {
				asynchronous: true
				active: _control._file_ops.fileType === AppUtils.FT_PDF
				anchors.fill: parent

				sourceComponent: PdfMultiPageView {
					id: pdfViewer
					document: PdfDocument {
						source: _control._file_ops.fileURL
					}

					Connections {
						target: _control._file_ops
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
				active: _control._file_ops.fileType < AppUtils.FT_IMAGE
				anchors.fill: parent

				sourceComponent: TPAppFileViewer {
					fileOps: _control._file_ops;
				}
			} //Loader : TPAppFileViewer

			Loader {
				asynchronous: true
				active: _control._file_ops.fileType === AppUtils.FT_TEXT
				anchors.fill: parent

				sourceComponent: TPMultiLineEdit {
					id: _edit
					text: _control._file_ops.getFileText(false)
					editable: false
					maxHeight: -1
					minHeight: height

					Connections {
						target: _control
						function onFileAdded(filepath: string): void {
							_edit.text = _control._file_ops.getFileText(false);
						}
					}
				}
			} //Loader : TPAppFileViewer
		} //Window fullViewWindow
	} //Loader fullScreenLoader
} //Item
