import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

TPPopup {
	id: selectMesoDlg
	modal: true
	width: appSettings.pageWidth * 0.9
	height: totalHeight*1.3

	property list<string> mesosList
	property list<int> idxsList
	property string message
	property int totalHeight: 0

	TPImage {
		id: importImg
		source: appSettings.iconFolder+"mesocycle.png"
		width: 50
		height: 50

		anchors {
			top: parent.top
			topMargin: 5
			left: parent.left
			leftMargin: 5
		}
	}

	TPLabel {
		id: lblTitle
		text: message
		wrapMode: Text.WordWrap
		horizontalAlignment: Text.AlignHCenter

		anchors {
			top: parent.top
			topMargin: lblTitle.lineCount*5
			left: importImg.right
			right: parent.right
			rightMargin: 15
		}

		Component.onCompleted: totalHeight += Math.max(height, importImg.height);
	}

	ListView {
		id: mesosListView
		contentHeight: model.count*50*1.1
		contentWidth: availableWidth
		height: 0.15*appSettings.pageHeight
		spacing: 0
		clip: true
		model: mesosList

		anchors {
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
			bottom: buttonsRow.top
			bottomMargin: 5
		}

		ScrollBar.vertical: ScrollBar {
			policy: ScrollBar.AsNeeded
			active: true; visible: mesosListView.contentHeight > mesosListView.height
		}

		delegate: ItemDelegate {
			id: delegate
			spacing: 0
			padding: 0
			width: parent.width
			height: 50

			contentItem: Text {
				id: txtCity
				text: mesosList[index]
				font.pixelSize: appSettings.fontSize
				fontSizeMode: Text.Fit
				leftPadding: 5
				bottomPadding: 2
			}

			background: Rectangle {
				color: index == mesosListView.currentIndex ? appSettings.entrySelectedColor :
						(index % 2 === 0 ? appSettings.listEntryColor1 : appSettings.listEntryColor2)
			}

			onClicked: mesosListView.currentIndex = index;
		} //ItemDelegate

		Component.onCompleted: totalHeight += height;
	} //ListView

	RowLayout
	{
		id: buttonsRow
		uniformCellSizes: true
		height: 30

		anchors {
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
			bottom: parent.bottom
			bottomMargin: 5
		}

		TPButton {
			id: btnSelect
			text: qsTr("Select")
			flat: false
			enabled: mesosListView.currentIndex >= 0
			Layout.alignment: Qt.AlignCenter

			onClicked: {
				itemManager.displayImportDialogMessageAfterMesoSelection(idxsList[mesosListView.currentIndex]);
				selectMesoDlg.close();
			}
		}

		TPButton {
			id: btnCancel
			text: qsTr("Cancel")
			flat: false
			Layout.alignment: Qt.AlignCenter

			onClicked: selectMesoDlg.close();
		}

		Component.onCompleted: totalHeight += height;
	}

	function show(ypos) {
		mesosListView.currentIndex = -1;
		selectMesoDlg.x = (appSettings.pageWidth - selectMesoDlg.width)/2;

		if (ypos < 0)
			ypos = (appSettings.pageHeight-selectMesoDlg.height)/2;

		finalYPos = ypos;
		if (ypos <= appSettings.pageHeight/2)
			startYPos = -300;
		else
			startYPos = appSettings.pageHeight + 300;

		selectMesoDlg.open();
	}
}
