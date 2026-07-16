import QtQuick
import QtQuick.Pdf

import TpQml

Item {
	id: _viewPort

	PdfMultiPageView {
		id: pdfViewer
		searchModel.searchString: search_term
		x: 0
		y: 0
		width: parent.width
		height: parent.height

		property string search_term
		property PdfStyle style

		document: PdfDocument {
			source: _control.fileOps.fileURL
		}

		Component.onCompleted: {
			// Access the internal PdfStyle via resources[1] or data[1]
			style = resources[1];
			// Customize search result appearance
			style.pageSearchResultsColor = "#80ffff00";
			style.currentSearchResultStrokeColor = "#80ff0000";
			style.selectionColor = "#88006eb1";
		}

		Connections {
			target: _control.fileOps
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

		TPMouseArea {
			movableWidget: pdfViewer
			movingWidget: pdfViewer
			viewPort: _viewPort
			onWheel: (wheel) => {
				wheel.accepted = true;
				pdfViewer.zoom(wheel.angleDelta.y > 0 ? 1 : -1);
			}
		}

		Loader {
			active: Qt.platform.os !== "android"
			asynchronous: true
			anchors.fill: parent

			sourceComponent: PinchArea {
				pinch.target: pdfViewer
				onPinchUpdated: (pinch) => {
					pinch.accepted = true;
					pdfViewer.zoom(pinch.scale > pinch.previewScale ? 1 : -1);
				}
			}
		}

		function zoom(value: int): void {
			if (value === 1) {
				pdfViewer.renderScale *= 1.1;
			} else {
				let new_scale = pdfViewer.renderScale * 0.91;
				if (new_scale < 0)
					new_scale = 1;
				pdfViewer.renderScale = new_scale;
			}
		}
	} //PdfMultiPageView

	TPBackRec {
		useShape: true
		x: (parent.width - width) / 2
		y: 0
		width: Qt.platform.os !== "android" ? parent.width / 2 : parent.width
		height: txtSearch.height + 4

		Row {
			id: _row
			padding: 2
			spacing: 5
			anchors.fill: parent

			TPTextInput {
				id: txtSearch
				showClearTextButton: true
				width: parent.width - 3 * (AppSettings.itemDefaultHeight + _row.spacing + _row.padding)
				onTextEdited: search_enabled
				onEnterOrReturnKeyPressed: if (search_enabled) btnSearch.clicked(0);

				readonly property bool search_enabled: btnSearch.enabled = text.length > 0 && text !== pdfViewer.search_term;
			} // txtSearch

			TPButton {
				id: btnSearch
				imageSource: "search.png"
				enabled: false
				width: AppSettings.itemDefaultHeight
				height: width
				onClicked: pdfViewer.search_term = txtSearch.text;
			}

			TPButton {
				id: btnNextResult
				imageSource: "down.png"
				enabled: pdfViewer.searchModel.currentResult < pdfViewer.searchModel.count
				width: AppSettings.itemDefaultHeight
				height: width
				onClicked: pdfViewer.searchForward();
			}
			TPButton {
				id: btnPrevResult
				imageSource: "up.png"
				enabled: pdfViewer.searchModel.currentResult >= 1
				width: AppSettings.itemDefaultHeight
				height: width
				onClicked: pdfViewer.searchBack();
			}
		} //Row (search row)
	} //TPBackRec
} //Item (_viewPort)
