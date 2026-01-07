import QtQuick
import QtQuick.Controls

import "../"
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

ComboBox {
	property string textColor: appSettings.fontColor
	property string backgroundColor: appSettings.primaryDarkColor
	property bool completeModel: false
	property bool selectable: true

	id: control
	height: appSettings.itemLargeHeight
	implicitHeight: height
	implicitWidth: width
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
			leftPadding: completeModel ? appSettings.itemDefaultHeight + 5 : 5

			TPImage {
				id: lblImg
				source: completeModel ? model.icon : ""
				dropShadow: false
				visible: completeModel
				width: appSettings.itemSmallHeight
				height: width

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
		width: appSettings.itemDefaultHeight / 2
		height: width
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
		leftPadding: completeModel ? appSettings.itemDefaultHeight + 5 : 5
		minimumPixelSize: -1
		fontSizeMode: Text.FixedSize
		elide: Text.ElideRight
	}

	TPImage {
		visible: completeModel
		source: completeModel ? model.get(currentIndex).icon : ""
		dropShadow: false
		width: appSettings.itemSmallHeight
		height: width

		anchors {
			left: parent.left
			leftMargin: 5
			verticalCenter: parent.verticalCenter
		}
	}

	background: Rectangle {
		implicitWidth: control.implicitWidth
		implicitHeight: control.implicitHeight
		color: control.enabled ? backgroundColor : "transparent"
		opacity: 0.8
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
			highlight: selectable ? highlight_component : null
			highlightFollowsCurrentItem: false
			delegateModelAccess: DelegateModel.ReadOnly
			enabled: selectable

			Component {
				id:	highlight_component
				Rectangle {
					width: ListView.view.width
					height: appSettings.itemDefaultHeight
					color: appSettings.primaryColor
					radius: 8
					y: ListView.view.currentItem ? ListView.view.currentItem.y : 0
					Behavior on y {
						SpringAnimation {
							spring: 3
							damping: 0.2
						}
					}
				}
			}
		}

		background: Rectangle {
			border.color: textColor
			color: backgroundColor
			opacity: 0.9
			radius: 6
		}
	}
}
