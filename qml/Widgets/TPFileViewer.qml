pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import TpQml
import TpQml.Pages

Item {
	id: _control
	width: minimumWidth
	height: minimumHeight

//public:
	required property FileOperations fileOps
	property string missingFileInfo
	property bool useBackground: false
	readonly property int minimumWidth: Math.max(200, fileOps.controlSize.width)
	readonly property int minimumHeight: minimumWidth * 1.4

	signal removalRequested()
	signal fileAdded(string filepath)
	signal windowStateChanged(int window_state)

//private:
	enum WindowStates { WS_UNDEFINED, WS_NORMAL, WS_FULLSCREEN }

	property string _preview_source: fileOps.getFileTypeIcon(Qt.size(0,0), true);
	property int _window_state: TPFileViewer.WindowStates.WS_NORMAL
	property TPMediaPlayer _media_player
	property Item _full_screen_widget

	onFileOpsChanged: {
		fileOps.parent = fileOpsRec;
		fileOps.anchors.fill = fileOpsRec;
		fileOps.previewSize = Qt.binding(function() { return Qt.size(width, height); });
		fileOps.showFullScreen.connect(fullScreenLoader.showFullScreen);
		fileOps.fileRemovalRequested.connect(_control.removalRequested);
		fileOps.fileAdded.connect(function(filepath) { _control.fileAdded(filepath); });
		fileOps.fileTypeChanged.connect(function() {
			_control._preview_source = _control.fileOps.getFileTypeIcon(Qt.size(_control.width, _control.height), true);
		});
		fileOps.previewSizeChanged.connect(function () {
			_control._preview_source = _control.fileOps.getFileTypeIcon(Qt.size(_control.width, _control.height), true);
		});
		fileOps.pdfDocumentChanged.connect(function () {
			_control._preview_source = _control.fileOps.getFileTypeIcon(Qt.size(_control.width, _control.height), true);
		});
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
			ParentChange {
				target: missingFileLoader
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
			ParentChange {
				target: missingFileLoader
				parent: _control
			}
		}
	]

	TPBackRec {
		visible: _control.useBackground
		//backColor: AppSettings.paneBackgroundColor
		showBorder: true
		opacity: 0.8
		radius: 8
		anchors {
			fill: parent
			margins: -2
		}
	}

	Loader {
		asynchronous: true
		active: _control.fileOps.fileType === AppUtils.FT_TEXT
		anchors.fill: parent

		sourceComponent: Label {
			text: _control.fileOps.getFileText(true);
			font.pixelSize: 0.05 * _control.height
			color: "black"
			padding: 10
			wrapMode: Text.Wrap

			background: Rectangle { color: "white"; border.color: "black"; }
		}
	}

	Loader {
		asynchronous: true
		active: _control.fileOps.fileType !== AppUtils.FT_TEXT
		anchors.centerIn: parent
		width: parent.width
		height: parent.height - fileOpsRec.height

		sourceComponent: TPImage {
			id: _imagePreview
			smooth: false
			source: _control._preview_source
			dropShadow: false
			keepAspectRatio: true
			imageSizeFollowControlSize: _control.fileOps.fileType !== AppUtils.FT_IMAGE
			fullWindowView: false
		}
	}

	Loader {
		id: missingFileLoader
		active: !_control.fileOps.isKnownFile
		asynchronous: true
		z: 1
		anchors.fill: parent

		sourceComponent: TPLabel {
			text: _control.missingFileInfo
			singleLine: false
			horizontalAlignment: Text.AlignHCenter
		}
	}

	Rectangle {
		id: fileOpsRec
		radius: 8
		color: AppSettings.paneBackgroundColor
		border.color: AppSettings.fontColor
		opacity: 0.8
		width: _control.fileOps.controlSize.width
		height: _control.fileOps.controlSize.height
		z: 1

		anchors {
			horizontalCenter: parent.horizontalCenter
			bottom: parent.bottom
			bottomMargin: 10
		}
	}

	Loader {
		id: mediaPlayerLoader
		asynchronous: true
		active: _control.fileOps.fileType === AppUtils.FT_VIDEO
		anchors.fill: parent

		sourceComponent: TPMediaPlayer {
			mediaUrl: _control.fileOps.fileURL
			fileOps: _control.fileOps
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
				_control.fileOps.repaintControls();
			}
			else {
				_window.close();
				fullScreenLoader.active = false;
				_control._window_state = TPFileViewer.WindowStates.WS_NORMAL;
				_control.fileOps.repaintControls();
			}

			if (_control._media_player)
				_control._media_player.changeState(_control._window_state);
			_control.windowStateChanged(_control._window_state);
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
				active: _control.fileOps.fileType === AppUtils.FT_IMAGE
				anchors.fill: parent

				sourceComponent: TPImage {
					source: _control.fileName
					dropShadow: false
					antialiasing: true
					imageSizeFollowControlSize: false
					fullWindowView: true
					keepAspectRatio: true
				}
			} //Loader : TPImage

			Loader {
				asynchronous: true
				active: _control.fileOps.fileType === AppUtils.FT_PDF
				anchors.fill: parent

				sourceComponent: TPPdfViewer {
					pdfDoc: _control.fileOps.pdfDocument
				}
			} //Loader : PdfMultiPageView

			Loader {
				asynchronous: true
				active: _control.fileOps.isTPFile
				anchors.fill: parent

				sourceComponent: TPAppFileViewer {
					fileOps: _control.fileOps;
				}
			} //Loader : TPAppFileViewer

			Loader {
				asynchronous: true
				active: _control.fileOps.fileType === AppUtils.FT_TEXT
				anchors.fill: parent

				sourceComponent: TPMultiLineEdit {
					id: _edit
					text: _control.fileOps.getFileText(false)
					editable: false
					maxHeight: -1
					minHeight: height

					Connections {
						target: _control
						function onFileAdded(filepath: string): void {
							_edit.text = _control.fileOps.getFileText(false);
						}
					}
				}
			} //Loader : TPAppFileViewer
		} //Window fullViewWindow
	} //Loader fullScreenLoader

	function startFullScreen(): void {
		fullScreenLoader.showFullScreen();
	}
} //Item
