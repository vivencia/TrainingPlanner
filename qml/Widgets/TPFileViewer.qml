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
	required property string fileName
	property bool canAddFile: false
	property bool useBackground: false
	property bool attemptToGetFile: true //means: attempt to download, copy or generate fileName
	readonly property int minimumWidth: file_ops.controlSize.width
	readonly property int minimumHeight: minimumWidth

	signal removalRequested()
	signal fileAdded(filepath: string)

//private:
	enum WindowStates { WS_UNDEFINED, WS_NORMAL, WS_FULLSCREEN }

	property string _preview_source
	property int _window_state: TPFileViewer.WindowStates.WS_NORMAL
	property TPMediaPlayer _media_player
	property Item _full_screen_widget

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

	TPBackRec {
		visible: _control.useBackground
		backColor: AppSettings.paneBackgroundColor
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
		active: file_ops.fileType === AppUtils.FT_TEXT
		anchors.fill: parent

		sourceComponent: Label {
			text: file_ops.getFileText(true);
			font.pixelSize: 0.05 * _control.height
			color: "black"
			padding: 10
			wrapMode: Text.Wrap

			background: Rectangle { color: "white"; border.color: "black"; }
		}
	}

	Loader {
		asynchronous: true
		active: file_ops.fileType !== AppUtils.FT_TEXT
		anchors.centerIn: parent
		width: parent.width
		height: parent.height

		sourceComponent: TPImage {
			id: _imagePreview
			smooth: false
			source: _control._preview_source
			dropShadow: false
			keepAspectRatio: true
			imageSizeFollowControlSize: file_ops.fileType !== AppUtils.FT_IMAGE
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
			fileName: _control.fileName
			canAddFile: _control.canAddFile
			canDownloadOrGenerate: _control.attemptToGetFile
			useControls: true

			anchors {
				horizontalCenter: parent.horizontalCenter
				verticalCenter: parent.verticalCenter
			}

			onShowFullScreen: fullScreenLoader.showFullScreen();
			onFileRemovalRequested: _control.removalRequested();
			onFileAdded: (filepath) => _control.fileAdded(fileName);
			onFileTypeChanged: _control._preview_source = getFileTypeIcon(fileName, Qt.size(0, 0), true);
		}
	}

	Loader {
		id: mediaPlayerLoader
		asynchronous: true
		active: file_ops.fileType === AppUtils.FT_VIDEO
		anchors.fill: parent

		sourceComponent: TPMediaPlayer {
			mediaUrl: file_ops.fileURL
			fileOps: file_ops
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
				file_ops.repaintControls();
			}
			else {
				_window.close();
				fullScreenLoader.active = false;
				_control._window_state = TPFileViewer.WindowStates.WS_NORMAL;
				file_ops.repaintControls();
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
				active: file_ops.fileType === AppUtils.FT_IMAGE
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
				active: file_ops.fileType === AppUtils.FT_PDF
				anchors.fill: parent

				sourceComponent: PdfMultiPageView {
					id: pdfViewer
					document: PdfDocument {
						source: file_ops.fileURL
					}

					Connections {
						target: file_ops
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
				active: file_ops.fileType < AppUtils.FT_IMAGE
				anchors.fill: parent

				sourceComponent: TPAppFileViewer {
					fileOps: file_ops;
				}
			} //Loader : TPAppFileViewer

			Loader {
				asynchronous: true
				active: file_ops.fileType === AppUtils.FT_TEXT
				anchors.fill: parent

				sourceComponent: TPMultiLineEdit {
					id: _edit
					text: file_ops.getFileText(false)
					editable: false
					maxHeight: -1
					minHeight: height

					Connections {
						target: _control
						function onFileAdded(filepath: string): void {
							_edit.text = file_ops.getFileText(false);
						}
					}
				}
			} //Loader : TPAppFileViewer
		} //Window fullViewWindow
	} //Loader fullScreenLoader
} //Item
