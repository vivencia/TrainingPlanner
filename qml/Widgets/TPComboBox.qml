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
	currentIndex: -1

//public:
	property string textColor: AppSettings.fontColor
	property string backgroundColor: AppSettings.primaryDarkColor
	property bool completeModel: false
	property bool selectable: true
	//When set to a value >= 0 and < model.count, when the item at the specified index is activated, instead of activated
	//reflect the item's actual index, it will return -100. Also, the activated index of all the other items after specialIndex
	//will be as if specialIndex did not exist
	property int specialIndex: -1

	signal itemActivated(int real_index, int index, string value)

//private:
	property bool _ignore_index_change: false

	onCurrentIndexChanged: {
		if (_ignore_index_change) {
			_ignore_index_change = false;
			return;
		}
		if (currentIndex < 0 && specialIndex >= 0)
			setCurIndex(specialIndex);
	}

	Component.onCompleted: {
		if (currentIndex < 0 && specialIndex >= 0)
			setCurIndex(specialIndex);
		else
			setCurIndex(currentIndex);
	}

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
		x: _control.width - width - 5
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
		id: _popup
		y: _control.height - 1
		width: _control.width
		padding: 5
		spacing: 0
		clip: true

		property int preferredHeight: 0

		enter: Transition {
			PropertyAnimation {
				target: _popup
				property: "height"
				from: 0
				to: Math.min(AppSettings.pageHeight / 3, _popup.preferredHeight)
				duration: 300
				easing.type: Easing.InCubic
			}
		}

		exit: Transition {
			PropertyAnimation {
				target: _popup
				property: "height"
				from: _popup.height
				to: 0
				duration: 300
				easing.type: Easing.InCubic
			}
		}

		contentItem: Flickable {
			contentWidth: width
			contentHeight: itemsRepeater.height

			ScrollBar.vertical: ScrollBar {
				id: vBar
				policy: ScrollBar.AsNeeded
				interactive: Qt.platform.os !== "android"

				anchors {
					top: parent.top
					bottom: parent.bottom
					right: parent.right
				}
			}

			Repeater {
				id: itemsRepeater
				model: _control.modelSize()
				delegateModelAccess: DelegateModel.ReadOnly
				enabled: _control.selectable

				anchors {
					left: parent.left
					right: parent.right
					top: parent.top
					margins: 3
				}

				property list<int> separator_indices
				property int last_actual_pos: -1
				property int n_skipped: 0
				property bool first_item: false
				readonly property int spacing: 5

				function positionItem(index: int, item: Item): void {
					item.y = itemsRepeater.height;
					itemsRepeater.height += item.height + spacing;
					_popup.preferredHeight += item.height + spacing;
				}

				onItemAdded: (index, item) => {
					if (!first_item) {
						if (index !== 0) {
							n_skipped++;
							return;
						} else
							first_item = true;
					}
					if (n_skipped === 0) {
						positionItem(index, item);
					} else {
						for (let i = 0; i < _control.modelSize(); ++i)
							positionItem(i, itemAt(i));
					}
				}

				delegate: Item {
					id: popupDelegate
					height: !separator ? label.height : AppSettings.itemSmallHeight
					width: parent.width

					required property int index
					readonly property bool separator: _control.isSeparator(index)
					property int real_index
					property int actual_pos

					Component.onCompleted: {
						if (!separator) {
							actual_pos = itemsRepeater.last_actual_pos + 1;
							itemsRepeater.last_actual_pos++;
							if (index !== _control.specialIndex) {
								let _real_index = _control.specialIndex < 0 ? index : index < _control.specialIndex ? index : index - 1;
								for (let i = 0; i < itemsRepeater.separator_indices.length; ++i) {
									if (index > itemsRepeater.separator_indices[i])
										--_real_index;
									else
										break;
								}
								real_index = _real_index;
							} else {
								real_index = _control.specialIndex;
							}
						} else {
							itemsRepeater.separator_indices.push(index);
							real_index = -1000;
						}
					}

					Rectangle {
						id: line
						color: AppSettings.fontColor
						visible: popupDelegate.separator
						height: 2
						width: parent.width * 0.8

						anchors {
							verticalCenter: parent.verticalCenter
							horizontalCenter: parent.horizontalCenter
						}
					}

					TPLabel {
						id: label
						text: _control.getText(popupDelegate.index)
						useBackground: true
						backgroundColor: popupDelegate.actual_pos % 2 === 0 ? AppSettings.listEntryColor1 : AppSettings.listEntryColor2
						enabled: _control.isEnabled(popupDelegate.index)
						visible: !popupDelegate.separator
						x: popupDelegate.index !== _control.currentIndex ? 0 : -itemsRepeater.spacing
						y: popupDelegate.index !== _control.currentIndex ? 0 : -2*itemsRepeater.spacing
						width: popupDelegate.index !== _control.currentIndex ? parent.width : parent.width + _popup.width
						height: popupDelegate.index !== _control.currentIndex ? preferredHeight() : preferredHeight() + 4*itemsRepeater.spacing

						Behavior on y {
							SpringAnimation {
								spring: 3
								damping: 0.2
							}
						}
						Behavior on height {
							NumberAnimation {
								easing.type: Easing.InOutBack
							}
						}
						Behavior on width {
							NumberAnimation {
								easing.type: Easing.InOutBack
							}
						}

						MouseArea {
							anchors.fill: parent
							hoverEnabled: true
							onClicked: (mouse) => {
								mouse.accepted = true;
								_control.currentIndex = popupDelegate.index;
								_control.displayText = label.text;
								_control.itemActivated(popupDelegate.real_index, popupDelegate.index, _control.getValue(popupDelegate.index));
								_popup.close();
							}
							onEntered: label.y += 3
							onExited: label.y -= 3
						}
					}
				} //Repeater
			} //Flickable
		}

		background: TPBackRec {
			useShape: true
			showBorder: true
		}
	}

	function setCurIndex(new_index: int): void {
		displayText = getText(new_index);
		_control._ignore_index_change = true;
		_control.currentIndex = new_index;
	}

	function modelSize(): int {
		if (model) {
			if (model instanceof ListModel)
				return model.count;
			else
				return model.length;
		}
		return 0;
	}

	function getText(index: int): string {
		if (model) {
			if (model instanceof ListModel)
				return model.get(index).text;
			else
				return model[index];
		}
		return "";
	}

	function isSeparator(index: int): bool {
		return getText(index) === "--";
	}

	function getValue(index: int): string {
		if (model instanceof ListModel)
			return model.get(index).value;
		else
			return model[index];
	}

	function isEnabled(index: int): bool {
		if (model instanceof ListModel)
			return model.get(index).enabled;
		else
			return true;
	}
}
