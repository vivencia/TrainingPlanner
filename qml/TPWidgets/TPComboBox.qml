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

	//Prevents the combo box from erasing the current text after the fist click on the combo box when no item is selectec
	property int _current_index;
	property bool ignore_index_change: false
	onCurrentIndexChanged: {
		if (ignore_index_change) {
			ignore_index_change = false;
			return;
		}

		if (currentIndex === -1 && _current_index !== -1)
			currentIndex = _current_index;
		else
			_current_index = currentIndex;
	}
	Component.onCompleted: _current_index = currentIndex;

	delegate: ItemDelegate {
		id: delegate
		width: control.width - 10
		enabled: model.enabled
		leftPadding: 5
		rightPadding: 5
		topPadding: 0
		bottomPadding: 0
		spacing: 0
		clip: true

		required property var model
		required property int index

		contentItem: TPLabel {
			text: model.text
			elide: Text.ElideRight
			minimumPixelSize: appSettings.smallFontSize * 0.8
			leftPadding: completeModel ? appSettings.itemDefaultHeight + 5 : 5
			enabled: model.enabled

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
		minimumPixelSize: appSettings.smallFontSize * 0.8
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
		radius: 8
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
					width: ListView.view.width - 10
					height: appSettings.itemDefaultHeight
					x: 5
					color: appSettings.primaryColor
					radius: 8
					y: ListView.view.currentItem ? ListView.view.currentItem.y + 2 : 2
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
			radius: 8
		}
	}

	function setCurIndex(new_index: int): void {
		ignore_index_change = true;
		control.currentIndex = new_index;
	}
}
