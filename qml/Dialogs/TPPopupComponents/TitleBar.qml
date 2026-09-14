pragma ComponentBehavior: Bound

import QtQuick

import TpQml
import TpQml.Widgets

TPBackRec {
	id: _control
	topLeftRadius: 8
	topRightRadius: 8
	bottomLeftRadius: 0
	bottomRightRadius: 0
	opacity: 0.8
	height: parentPopup.titleBarHeight

//public:
	required property TPPopup parentPopup
	property Item titleBarButtons: null

//private:
	property real _w_ratio
	property real _h_ratio
	property int _start_width
	property int _start_height
	readonly property int _min_width: _control.parentPopup.minimum_size.width
	readonly property int _min_height: _control.parentPopup.minimum_size.height

	TPButton {
		id: btnClose
		imageSource: "close.png"
		hasDropShadow: false
		visible: _control.parentPopup.showCloseButton
		width: AppSettings.itemSmallHeight
		height: width
		z: 2

		anchors {
			verticalCenter: parent.verticalCenter
			right: parent.right
			rightMargin: 5
		}

		onClicked: _control.parentPopup.closePopup(-1);
		Component.onCompleted: {
			if (!_control.parentPopup.resizeable)
				_control.titleBarButtons = this;
		}
	}

	Loader {
		id: windowBarControlsLoader
		asynchronous: true
		active: _control.parentPopup.resizeable
		z: 2
		anchors {
			verticalCenter: parent.verticalCenter
			right: btnClose.visible ? btnClose.left : parent.right
		}

		sourceComponent: Item {
			id: _titleBarButtons
			width: btnMaxRestoreWindow.width + btnMinimizeWindow.width + 10
			height: AppSettings.itemDefaultHeight
			property int finalWidth
			property int finalHeight

			ParallelAnimation {
				id: shrink
				alwaysRunToEnd: true

				PropertyAnimation {
					target: _control.parentPopup
					property: "width"
					to: _control.parentPopup.minimized_size.width
					duration: 200
					easing.type: Easing.OutQuad
				}

				PropertyAnimation {
					target: _control.parentPopup
					property: "height"
					to: _control.parentPopup.minimized_size.height
					duration: 200
					easing.type: Easing.OutQuad
				}

				onFinished: {
					_control.parentPopup.x = _control.parentPopup._end_x_pos;
					_control.parentPopup.y = _control.parentPopup._end_y_pos;
					if (_control.parentPopup.savePopupState) {
						AppSettings.setCustomValue(_control.parentPopup.configFieldName + ".size",
											Qt.size(_control.parentPopup.width, _control.parentPopup.height));
						AppSettings.setCustomValue(_control.parentPopup.configFieldName + ".pos",
											Qt.point(_control.parentPopup.x, _control.parentPopup.y));
					}
				}
			}

			ParallelAnimation {
				id: expand
				alwaysRunToEnd: true

				PropertyAnimation {
					target: _control.parentPopup
					property: "width"
					to: _titleBarButtons.finalWidth
					duration: 200
					easing.type: Easing.InQuad
				}

				PropertyAnimation {
					target: _control.parentPopup
					property: "height"
					to: _titleBarButtons.finalHeight
					duration: 200
					easing.type: Easing.InQuad
				}

				PropertyAnimation {
					target: _control.parentPopup
					property: "x"
					to: _control.parentPopup._end_x_pos
					duration: 200
					easing.type: Easing.InQuad
				}

				PropertyAnimation {
					target: _control.parentPopup
					property: "y"
					to: _control.parentPopup._end_y_pos
					duration: 200
					easing.type: Easing.InQuad
				}

				onFinished: {
					const size_changed = !_control.parentPopup._minimized;
					if (size_changed) {
						_control._w_ratio = _titleBarButtons.finalWidth / _control._start_width;
						_control._h_ratio = _titleBarButtons.finalHeight / _control._start_height;
					}
					const is_maximized = _titleBarButtons.finalHeight >= AppSettings.pageHeight
													&& _titleBarButtons.finalWidth >= AppSettings.pageWidth;
					if (is_maximized) {
						_control.parentPopup.x = 0;
						_control.parentPopup.y = _control.parentPopup.realY;
					}
					_control.afterResize(size_changed);
				}
			}

			TPButton {
				id: btnMaxRestoreWindow
				imageSource: _control.parentPopup._maximized ? "restore.png" : "maximize.png"
				hasDropShadow: false
				visible: _control.parentPopup.show_maximize_button
				width: AppSettings.itemSmallHeight
				height: width
				z: 2

				anchors {
					verticalCenter: parent.verticalCenter
					right: parent.right
					margins: 5
				}

				onClicked: _control.restore();
			}

			TPButton {
				id: btnMinimizeWindow
				imageSource: "minimize.png"
				hasDropShadow: false
				visible: _control.parentPopup.show_minimize_button
				enabled: !_control.parentPopup._minimized
				width: AppSettings.itemSmallHeight
				height: width
				z: 2

				anchors {
					verticalCenter: btnMaxRestoreWindow.verticalCenter
					right: btnMaxRestoreWindow.left
				}

				onClicked: _control.minimize();
			}

			TPImage {
				id: imgResize
				source: "resize-window.png"
				dropShadow: false
				width: AppSettings.itemSmallHeight * 0.6
				height: width
				visible: !(_control.parentPopup._maximized || _control.parentPopup._minimized)
				z: 1

				anchors {
					right: parent.right
					bottom: parent.bottom
				}

				Component.onCompleted: parent = _control.parentPopup.contentItem;

				MouseArea {
					enabled: imgResize.visible
					anchors.fill: parent

					property bool _can_resize: false
					property int _start_x
					property int _start_y

					onPressed: (mouse) => {
						_can_resize = true;
						_start_x = mouse.x;
						_start_y = mouse.y;
						_control._start_width = _control.parentPopup.width;
						_control._start_height = _control.parentPopup.height;
					}

					onReleased: (mouse) => {
						_can_resize = false;
						if (_control._start_width !== _control.parentPopup.width ||
										_control._start_height !== _control.parentPopup.height) {
							_control._w_ratio = _control.parentPopup.width / _control._start_width;
							_control._h_ratio = _control.parentPopup.height / _control._start_height;
							_control.afterResize(true);
						}
					}

					onPositionChanged: (mouse) => {
						if (_can_resize) {
							const deltaX = mouse.x - _start_x;
							let new_width = _control.parentPopup.width + deltaX;
							if (new_width >= _control._min_width) {
								if (new_width >= AppSettings.pageWidth - 10)
									new_width = AppSettings.pageWidth - 10;
								_control.parentPopup.width = new_width;
							}

							const deltaY = mouse.y - _start_y;
							let new_height = _control.parentPopup.height + deltaY;
							if (new_height >= _control._min_height) {
								if (new_height >= AppSettings.pageHeight - 10)
									new_height = AppSettings.pageHeight - 10;
								_control.parentPopup.height = new_height;
							}
						}
					}
				} //MouseArea
			} //imgResize

			Component.onCompleted: _control.titleBarButtons = this;
			function startExpand(): void {
				_control._start_width = _control.parentPopup.width;
				_control._start_height = _control.parentPopup.width;
				expand.start();
			}
			function startShrink(): void {
				_control._start_width = _control.parentPopup.width;
				_control._start_height = _control.parentPopup.width;
				shrink.start();
			}
		} //sourceComponent: Item
	} //windowBarControlsLoader

	function restore(): void {
		if (parentPopup._maximized) { //Restore popup state to previous values
			_control.titleBarButtons.finalWidth = parentPopup._normal_width;
			_control.titleBarButtons.finalHeight = parentPopup._normal_height;
		} else if (parentPopup._minimized) { //Restore popup state to previous values
			if (parentPopup._normal_width + parentPopup.x > AppSettings.pageWidth) {
				//reverse expand so that popup ends up inside the page
				parentPopup._end_x_pos = parentPopup.x - parentPopup._normal_width;
				if (parentPopup._end_x_pos < 0)
					parentPopup._end_x_pos = 0;
			} else {
				parentPopup._end_x_pos = parentPopup.x; //popup, lengthwise, after expansion will be is inside page, do nothing
			}
			if (parentPopup._normal_height + parentPopup.y > AppSettings.pageHeight) {
				//reverse expand so that popup ends up inside the page
				parentPopup._end_y_pos = parentPopup.y - parentPopup._normal_height;
				if (parentPopup._end_y_pos < 0)
					parentPopup._end_y_pos = 0;
			} else {
				parentPopup._end_y_pos = parentPopup.y; //popup, heightwise, after expansion will be is inside page, do nothing
			}
			_control.titleBarButtons.finalWidth = parentPopup._normal_width;
			_control.titleBarButtons.finalHeight = parentPopup._normal_height;
		} else {
			//Save current popup state
			parentPopup._end_x_pos = parentPopup.x;
			parentPopup._end_y_pos = parentPopup.y;
			parentPopup._normal_width = parentPopup.width;
			parentPopup._normal_height = parentPopup.height;
			//Maximize popup
			_control.titleBarButtons.finalWidth = AppSettings.pageWidth;
			_control.titleBarButtons.finalHeight = AppSettings.pageHeight;
		}
		_control.titleBarButtons.startExpand();
	}

	function minimize(): void {
		_control.titleBarButtons.startShrink();
	}

	function afterResize(size_changed: bool): void {
		if (size_changed)
			_control.parentPopup.popupSizeChanged(_w_ratio, _h_ratio);
		if (_control.parentPopup.savePopupState) {
			AppSettings.setCustomValue(_control.parentPopup.configFieldName + ".size",
													Qt.size(_control.parentPopup.width, _control.parentPopup.height));
			AppSettings.setCustomValue(_control.parentPopup.configFieldName + ".pos",
													Qt.point(_control.parentPopup.x, _control.parentPopup.y));
		}
	}
} //ToolBar
