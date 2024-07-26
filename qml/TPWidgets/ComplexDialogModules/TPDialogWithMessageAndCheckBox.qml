import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

import "../"
import "../.."

ColumnLayout {
	id: mainLayout
	spacing: 0
	height: Math.max(lblMessage.height, imgEffects.height) + checkbox.height + 15
	Layout.fillWidth: true

	required property var parentDlg

	FontMetrics {
		id: fontMetrics
		font.family: lblMessage.font.family
		font.pointSize: lblMessage.font.pointSize
	}

	RowLayout {
		Layout.leftMargin: 5
		Layout.rightMargin: 5
		Layout.topMargin: 20
		Layout.fillWidth: true
		spacing: 5

		Image {
			id: imgElement
			source: parentDlg.customStringProperty3 !== "" ? "qrc:/images/"+AppSettings.iconFolder+parentDlg.customStringProperty3 : ""
			fillMode: Image.PreserveAspectFit
			asynchronous: true
			visible: false
			layer.enabled: true
		}

		MultiEffect {
			id: imgEffects
			visible: parentDlg.customStringProperty3.length > 0
			source: imgElement
			shadowEnabled: true
			shadowOpacity: 0.5
			blurMax: 16
			shadowBlur: 1
			shadowHorizontalOffset: 5
			shadowVerticalOffset: 5
			shadowColor: "black"
			shadowScale: 1
			width: 50
			height: 50
			Layout.alignment: Qt.AlignCenter
		}

		Label {
			id: lblMessage
			text: parentDlg.customStringProperty1
			color: textColor
			wrapMode: Text.WordWrap
			horizontalAlignment: Text.AlignJustify
			font.pointSize: AppSettings.fontSizeText
			font.weight: Font.Black
			width: parent.width - imgEffects.width - 10
			height: Math.ceil(fontMetrics.boundingRect(text).width / width) * 30
			padding: 0
			Layout.fillWidth: true
		}
	}

	TPCheckBox {
		id: checkbox
		text: parentDlg.customStringProperty2
		checked: false
		width: parent.width
		Layout.leftMargin: 10
		Layout.rightMargin: 5
		Layout.topMargin: 5
		Layout.bottomMargin: 5

		onCheckedChanged: parentDlg.customBoolProperty1 = checked;
	} //TPCheckBox
} // ColumnLayout
