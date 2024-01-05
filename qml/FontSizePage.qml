import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
	RowLayout {
		id: control
		width: parent.width - 60
		anchors.centerIn: parent
		spacing: 12

		Label {
			text: "A"
			font.pixelSize: 10
			font.weight: 400
		}

		Slider {
			snapMode: Slider.SnapAlways
			stepSize: 1
			from: 10
			value: AppSettings.fontSize
			to: 30
			Layout.fillWidth: true
			onMoved:  {
				AppSettings.fontSize = value;
				AppSettings.fontSizeTitle = value * 1.5
				AppSettings.fontSizeLists = value * 0.7
				AppSettings.fontSizeText = value * 0.9
			}
		}

		Label {
			text: "A"
			font.pixelSize: 30
			font.weight: 400
		}
	} //RowLayout

	Label {
		id: lblExample
		text: qsTr("Example text")
		font.pointSize: AppSettings.fontSize
		anchors.bottom: control.top
		anchors.bottomMargin: 10
		anchors.horizontalCenter: control.horizontalCenter
	}

	Label {
		id: lblTitleFont
		text: qsTr("Font used in titles")
		font.pointSize: AppSettings.fontSizeTitle
		anchors.top: control.bottom
		anchors.topMargin: 30
		anchors.left: parent.left
		anchors.leftMargin: 20
	}
	Label {
		id: lblListsFont
		text: qsTr("Font used in lists")
		font.pointSize: AppSettings.fontSizeLists
		anchors.top: lblTitleFont.bottom
		anchors.topMargin: 5
		anchors.left: parent.left
		anchors.leftMargin: 20
	}
	Label {
		id: lblTextFont
		text: qsTr("Font used on text input fields")
		font.pointSize: AppSettings.fontSizeText
		anchors.top: lblListsFont.bottom
		anchors.topMargin: 5
		anchors.left: parent.left
		anchors.leftMargin: 20
	}
}
