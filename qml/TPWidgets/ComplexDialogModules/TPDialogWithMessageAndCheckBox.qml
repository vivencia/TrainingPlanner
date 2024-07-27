import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

import "../"
import "../.."

Column {
	id: mainLayout
	padding: 0
	spacing: 10

	required property var parentDlg

	Component.onCompleted: {
		parentDlg.bAdjustHeightEveryOpen = true;
		parentDlg.dialogOpened.connect(resize);
	}

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
			visible: parentDlg.customStringProperty3 !== ""
			source: imgElement
			shadowEnabled: true
			shadowOpacity: 0.5
			blurMax: 16
			shadowBlur: 1
			shadowHorizontalOffset: 5
			shadowVerticalOffset: 5
			shadowColor: "black"
			shadowScale: 1
			width: parentDlg.customStringProperty3 !== "" ? 50 : 0
			height: width
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
			width: mainLayout.width - imgEffects.width - 10
			height: Math.ceil(fontMetrics.boundingRect(text).width / width) * 30
			Layout.maximumWidth: width
			Layout.minimumWidth: width
			padding: 0
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

	function resize() {
		mainLayout.height = Math.max(lblMessage.height, imgEffects.height) + checkbox.implicitHeight
	}
} // ColumnLayout
