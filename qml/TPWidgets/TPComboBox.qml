import QtQuick
import QtQuick.Controls

import "../"
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

ComboBox {
	property string textColor: appSettings.fontColor
	property string backgroundColor: appSettings.primaryDarkColor
	property bool completeModel: false

	id: control
	implicitWidth: 120
	implicitHeight: 25
	textRole: "text"
	valueRole: "value"
	padding: 0
	spacing: 0

	delegate: ItemDelegate {
		id: delegate
		width: control.width
		enabled: model.enabled
		padding: 0
		spacing: 0

		required property var model
		required property int index

		contentItem: TPLabel {
			text: model.text
			enabled: model.enabled
			leftPadding: completeModel ? 30 : 5

			TPImage {
				id: lblImg
				source: completeModel ? model.icon : ""
				dropShadow: false
				visible: completeModel
				width: 20
				height: 20

				anchors {
					left: parent.left
					leftMargin: 5
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
			function onEnabledChanged() { canvas.requestPaint(); }
		}

		onPaint: {
			if (context) {
				context.reset();
				context.moveTo(0, 0);
				context.lineTo(width, 0);
				context.lineTo(width / 2, height);
				context.closePath();
				context.fillStyle = control.enabled ? textColor : appSettings.disabledFontColor
				context.fill();
			}
		}
	}

	contentItem: TPLabel {
		text: control.displayText
		//widthAvailable: control.width
		leftPadding: completeModel ? 30 : 5
	}

	TPImage {
		visible: completeModel
		source: completeModel ? control.model.get(control.currentIndex).icon : ""
		dropShadow: false
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
		padding: 0
		spacing: 0

		contentItem: ListView {
			implicitHeight: contentHeight
			model: control.popup.visible ? control.delegateModel : null
			currentIndex: control.highlightedIndex
		}

		background: Rectangle {
			border.color: textColor
			color: backgroundColor
			opacity: 0.9
			radius: 6
		}
	}
}
