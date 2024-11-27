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
	height: lblTitle.height + mesosListView.height + btnSelect.height + 20

	property string mesosList: []
	property int idxsList: []
	property string message

	TPLabel {
		id: lblTitle
		text: message
		wrapMode: Text.WordWrap
		fontSizeMode: Text.FixedSize
		horizontalAlignment: Text.AlignHCenter

		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
		}
	}

	TPImage {
		id: importImg
		source: appSettings.iconFolder+"mesocycle.png"
		width: 50
		height: 50

		anchors {
			top: parent.top
			topMargin: lblTitle.height
			left: parent.left
			leftMargin: 5
		}
	}

	ListView {
		id: mesosListView
		contentHeight: model.count*50*1.1
		contentWidth: availableWidth
		height: 0.4*appSettings.pageHeight
		spacing: 0
		clip: true
		model: mesosList

		anchors {
			top: lblTitle.bottom
			left: importImg.right
			right: parent.right
		}

		ScrollBar.vertical: ScrollBar {
			policy: ScrollBar.AsNeeded
			active: true; visible: scrollViewCities.contentHeight > scrollViewCities.height
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
	} //ListView

	RowLayout
	{
		anchors {
			left: parent.left
			right: parent.right
			bottom: parent.bottom
			bottomMargin: 10
		}

		TPButton {
			id: btnSelect
			text: qsTr("Select")
			flat: false
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
	}

	function show(ypos) {
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
