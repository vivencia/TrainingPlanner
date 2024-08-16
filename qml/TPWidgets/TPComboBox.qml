import QtQuick
import QtQuick.Controls

import "../"

ComboBox {
	id: control
	implicitWidth: 120//fontMetrics.boundingRect("LorenIpsuM").width + 15
	implicitHeight: 25
	textRole: "text"
	valueRole: "value"

	property string textColor: AppSettings.fontColor
	property string backgroundColor: AppSettings.primaryDarkColor
	property bool completeModel: false

	FontMetrics {
		id: fontMetrics
		font.family: control.font.family
		font.pointSize: AppSettings.fontSizeText
	}

	delegate: ItemDelegate {
		id: delegate
		width: control.width
		required property var model
		required property int index

		contentItem: Label {
			//text: control.textRole ? (Array.isArray(control.model) ? modelData[control.textRole] : model[control.textRole]) : modelData
			text: delegate.model[control.textRole]
			color: textColor
			elide: Text.ElideRight
			minimumPointSize: 8
			fontSizeMode: Text.Fit
			font.weight: Font.ExtraBold
			verticalAlignment: Text.AlignVCenter
			leftPadding: completeModel ? 10 : 0

			Image {
				id: lblImg
				visible: completeModel
				fillMode: Image.PreserveAspectFit
				asynchronous: true
				source: completeModel ? model.icon : ""
				width: 20
				height: 20

				anchors {
					left: parent.left
					leftMargin: -12
					verticalCenter: parent.verticalCenter
				}
			}
		}
		highlighted: control.highlightedIndex === index
	}

	indicator: Canvas {
		id: canvas
		x: control.width - width - control.rightPadding
		y: control.topPadding + (control.availableHeight - height) / 2
		width: 12
		height: 8
		contextType: "2d"

		Connections {
			target: control
			function onPressedChanged() { canvas.requestPaint(); }
		}

		onPaint: {
			if (context) {
				context.reset();
				context.moveTo(0, 0);
				context.lineTo(width, 0);
				context.lineTo(width / 2, height);
				context.closePath();
				context.fillStyle = textColor
				context.fill();
			}
		}
	}

	contentItem: Label {
		text: control.displayText
		rightPadding: control.indicator.width + control.spacing
		color: control.enabled ? textColor : "gray"
		font.pointSize: AppSettings.fontSizeText
		font.weight: Font.ExtraBold
		verticalAlignment: Text.AlignVCenter
		elide: Text.ElideRight
		leftPadding: completeModel ? 30 : 5
	}

	Image {
		visible: completeModel
		fillMode: Image.PreserveAspectFit
		asynchronous: true
		source: completeModel ? control.model.get(control.currentIndex).icon : ""
		width: 18
		height: 18

		anchors {
			left: parent.left
			leftMargin: 5
			verticalCenter: parent.verticalCenter
		}
	}

	background: Rectangle {
		implicitWidth: control.implicitWidth
		implicitHeight: control.implicitHeight
		color: backgroundColor
		opacity: 0.5
		border.width: control.visualFocus ? 2 : 1
		border.color: textColor
		radius: 6
	}

	popup: Popup {
		y: control.height - 1
		width: control.width
		implicitHeight: contentItem.implicitHeight
		padding: 1

		contentItem: ListView {
			implicitHeight: contentHeight
			model: control.popup.visible ? control.delegateModel : null
			currentIndex: control.highlightedIndex

			delegate: Text {
				text: model.key
				color: control.highlighted ? AppSettings.primaryLightColor : textColor
				minimumPointSize: 8
				fontSizeMode: Text.Fit
				font.weight: Font.ExtraBold
				elide: Text.ElideRight
				padding: 0
			}
			ScrollIndicator.vertical: ScrollIndicator { }
		}

		background: Rectangle {
			border.color: textColor
			color: backgroundColor
			opacity: 0.9
			radius: 6
		}
	}
}
