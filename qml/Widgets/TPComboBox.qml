pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import TpQml

ComboBox {
	id: _control
	height: AppSettings.itemLargeHeight
	implicitHeight: height
	implicitWidth: width
	textRole: "text"
	valueRole: "value"
	padding: 0
	spacing: 0

//public:
	property string textColor: AppSettings.fontColor
	property string backgroundColor: AppSettings.primaryDarkColor
	property bool completeModel: false
	property bool selectable: true

//private:
	//Prevents the combo box from erasing the current text after the fist click on the combo box when no item is selected
	property int _current_index;
	property bool _ignore_index_change: false

	onCurrentIndexChanged: {
		if (_ignore_index_change) {
			_ignore_index_change = false;
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
		width: _control.width - 10
		enabled: enabled
		leftPadding: 5
		rightPadding: 5
		topPadding: 0
		bottomPadding: 0
		spacing: 0
		clip: true

		required property int index
		required property var model

		contentItem: TPLabel {
			text: delegate.model.text
			elide: Text.ElideRight
			minimumPixelSize: AppSettings.smallFontSize * 0.8
			leftPadding: _control.completeModel ? AppSettings.itemDefaultHeight + 5 : 5
			enabled: delegate.model.enabled

			TPImage {
				id: lblImg
				source: _control.completeModel ? delegate.model.icon : ""
				dropShadow: false
				visible: _control.completeModel
				width: AppSettings.itemSmallHeight
				height: width

				anchors {
					left: parent.left
					leftMargin: 5
					verticalCenter: parent.verticalCenter
				}
			}
		}
		highlighted: _control.highlightedIndex === delegate.index
	} //ItemDelegate

	indicator: Canvas {
		id: canvas
		x: _control.width - width - _control.rightPadding
		y: _control.topPadding + (_control.availableHeight - height) / 2
		width: AppSettings.itemDefaultHeight / 2
		height: width
		contextType: "2d"

		Connections {
			target: _control
			function onEnabledChanged() { canvas.requestPaint(); }
		}

		onPaint: {
			if (context) {
				context.reset();
				context.moveTo(0, 0);
				context.lineTo(width, 0);
				context.lineTo(width / 2, height);
				context.closePath();
				context.fillStyle = _control.enabled ? _control.textColor : AppSettings.disabledFontColor
				context.fill();
			}
		}
	}

	contentItem: TPLabel {
		text: _control.displayText
		leftPadding: _control.completeModel ? AppSettings.itemDefaultHeight + 5 : 5
		minimumPixelSize: AppSettings.smallFontSize * 0.8
		elide: Text.ElideRight
	}

	TPImage {
		visible: _control.completeModel
		source: _control.completeModel ? _control.model.get(_control.currentIndex).icon : ""
		dropShadow: false
		width: AppSettings.itemSmallHeight
		height: width

		anchors {
			left: parent.left
			leftMargin: 5
			verticalCenter: parent.verticalCenter
		}
	}

	background: Rectangle {
		implicitWidth: _control.implicitWidth
		implicitHeight: _control.implicitHeight
		color: _control.enabled ? _control.backgroundColor : "transparent"
		opacity: 0.8
		border.width: _control.visualFocus ? 2 : 1
		border.color: _control.textColor
		radius: 8
	}

	popup: Popup {
		y: _control.height - 1
		width: _control.width
		implicitHeight: contentItem.implicitHeight
		padding: 0
		spacing: 0

		contentItem: ListView {
			implicitHeight: contentHeight
			model: _control.popup.visible ? _control.delegateModel : null
			currentIndex: _control.highlightedIndex
			highlight: _control.selectable ? highlight_component : null
			highlightFollowsCurrentItem: false
			delegateModelAccess: DelegateModel.ReadOnly
			enabled: _control.selectable

			Component {
				id:	highlight_component
				Rectangle {
					width: ListView.view.width - 10
					height: AppSettings.itemDefaultHeight
					x: 5
					color: AppSettings.primaryColor
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
			border.color: _control.textColor
			color: _control.backgroundColor
			opacity: 0.9
			radius: 8
		}
	}

	function setCurIndex(new_index: int): void {
		_control._ignore_index_change = true;
		_control.currentIndex = new_index;
	}
}
