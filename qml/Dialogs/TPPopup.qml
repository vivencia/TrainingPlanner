pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import TpQml
import TpQml.Widgets
import TpQml.Pages

import "./TPPopupComponents"

Popup {
	id: _control
	closePolicy: keepAbove ? Popup.NoAutoClose : Popup.CloseOnPressOutside
	parent: ItemManager.popupsVisualParent
	spacing: 0
	padding: showBorder ? 2 : 0
	topInset: 0

//public:
	property TPPage parentPage
	property bool keepAbove: false

	signal popupClosed(popup: QtObject);
	signal keyboardNumberPressed(key1: int, key2: int);
	signal keyboardEnterPressed();
	signal closeActionExeced(btn_id: int);
	signal mouseItemClicked(mouse: MouseEvent);
	signal popupSizeChanged(w_ratio: real, h_ratio: real);

//protected:
	property bool showTitleBar: false
	property bool showCloseButton: showTitleBar
	property bool resizeable: false
	property bool enableEffects: false
	property bool lockMovingToYAxis: false
	property bool showBorder: false
	property bool useGradient: false
	property bool useShape: false
	property bool canSlideToClose: false
	property bool useAlternateBackground: false
	property bool visibilityCondition: true
	property bool savePopupState: false
	property bool open_in_window: false
	property bool show_minimize_button: true
	property bool show_maximize_button: true
	property int showBehavior: TPPopup.PARENT_PAGE_ACTIVE
	property int backgroundRotation: 0
	property int show_position: 0 //Use Qt.AlignBaseline when position(x,y) is retrieved from a config file.
	property point defaultCoordinates
	property size minimized_size: Qt.size(titleBar !== null ? titleBar.width : AppSettings.itemDefaultHeight, titleBarHeight)
	property size normal_size
	property size minimum_size: Qt.size(normal_size.width / 2, normal_size.height / 2)
	property string backGroundImage
	property string defaultBackgroundColor: AppSettings.paneBackgroundColor
	property string configFieldName: objectName
	property Item mouseItem
	property Item reference_widget: null
	property TPBackRec backgroundRec
	property TitleBar titleBar: null
	property TPMouseArea mouse_area: null

	enum ShowBehavior { PARENT_PAGE_ACTIVE, ALWAYS_VISIBLE }
	enum CloseActionType { DEFAULT_ACTION = 0, BTN_CLOSE = -1, SWIPE = -2, BACK_KEY = -3 }
	readonly property int realY: AppSettings.windowHeight - AppSettings.pageHeight;

//private:
	property bool _use_burst_transition: true
	property bool _use_alternate_transition: false
	property bool _hidden: false
	property bool _reopen: false
	property bool _can_reopen: false
	//not maximized is de default state, i. e. normal widget size. The default is also not minimized. Popup's
	//width and height are uninitialized until open() is called
	readonly property bool _maximized: height >= AppSettings.pageHeight && width >= AppSettings.pageWidth
	readonly property bool _minimized: height <= minimized_size.height && width <= minimized_size.width
	property int _start_y_pos; property int _end_y_pos
	property int _start_x_pos; property int _end_x_pos
	property int _key_pressed
	property int _normal_width: 0
	property int _normal_height: 0
	property int _close_action_type: TPPopup.DEFAULT_ACTION

	readonly property Transition _transition_in: !_use_alternate_transition ? (_use_burst_transition ? burstOutTransition : slideInTransition) : alternateCloseTransition
	readonly property Transition _transition_out: !_use_alternate_transition ? (_use_burst_transition ? burstInTransition : slideOutTransition) : alternateCloseTransition
	readonly property int titleBarHeight: AppSettings.itemDefaultHeight + 5
	readonly property int titleBarWidth: showTitleBar ? (showCloseButton ? AppSettings.itemSmallHeight : 0) + (resizeable ? 2*AppSettings.itemSmallHeight : 0) : 0

	enter: _transition_in
	exit: _close_action_type !== TPPopup.SWIPE ? _transition_out : null

	onClosed: {
		if (!_hidden && (modal || keepAbove))
			popupClosed(this);
	}

	onResizeableChanged: {
		if (showTitleBar !== resizeable)
			showTitleBar = resizeable;
	}

	onMouseItemChanged: createMouseArea();
	onVisibilityConditionChanged: {
		if (visible !== visibilityCondition) {
			if (keepAbove) {
				if (visibilityCondition)
					tpQmlOpen(parentPage);
				else
					close();
			} else {
				visible = visibilityCondition;
			}
		}
	}

	contentItem {
		Keys.onPressed: (event) => {
			switch (event.key) {
			case Qt.Key_Enter:
			case Qt.Key_Return:
				keyboardEnterPressed();
				break;
			default:
				if (event.key >= Qt.Key_0 && event.key <= Qt.Key_9) {
					if (keyPressTimer.running) {
						keyPressTimer.stop();
						keyboardNumberPressed(event.key, _key_pressed);
					} else {
						_key_pressed = event.key;
						keyboardNumberPressed(event.key, -1);
						keyPressTimer.start();
					}
				}
				break;
			}
		}
	}

	Loader {
		active: !_control.useAlternateBackground
		asynchronous: true

		TPBackRec {
			useGradient: _control.useGradient
			useShape: _control.useShape
			useImage: _control.backGroundImage.length > 0
			sourceImage: _control.backGroundImage
			backColor: _control.defaultBackgroundColor
			showBorder: _control.showBorder
			enableShadow: _control.enableEffects
			rotate_angle: _control.backgroundRotation
			implicitWidth: _control.width
			implicitHeight: _control.height
			radius: 8
			Component.onCompleted: {_control.backgroundRec = this; }
		}
	}

	background: backgroundRec

	Timer {
		id: keyPressTimer
		interval: 800
	}

	Loader {
		id: titleBarLoader
		asynchronous: false
		active: _control.showTitleBar

		anchors {
			top: parent.top
			topMargin: 2
			left: parent.left
			right: parent.right
		}

		sourceComponent: TitleBar {
			parentPopup: _control as TPPopup
			Component.onCompleted: {
				_control.titleBar = this;
				if (!_control.mouseItem)
					_control.mouseItem = this;
			}
		}
	} //titleBarLoader

	Transition {
		id: burstOutTransition

		ParallelAnimation {
			alwaysRunToEnd: true

			NumberAnimation {
				property: "opacity"
				from: 0.0
				to: 1.0
				duration: 300
			}
			// Optional: Scale up
			NumberAnimation {
				property: "scale"
				from: 0.4
				to: 1.0
				duration: 300
				easing.type: Easing.OutBack
			}
		}
	}

	Transition {
		id: burstInTransition

		ParallelAnimation {
			alwaysRunToEnd: true

			NumberAnimation {
				property: "opacity"
				from: 1.0
				to: 0.0
				duration: 300
			}
			// Optional: Scale up
			NumberAnimation {
				property: "scale"
				from: 1.0
				to: 0.4
				duration: 300
				easing.type: Easing.OutBack
			}
		}
	}

	Transition {
		id: slideInTransition

		ParallelAnimation {
			alwaysRunToEnd: true

			PropertyAnimation {
				property: "x"
				from: _control._start_x_pos
				to: _control._end_x_pos
				duration: 300
				easing.type: Easing.InCubic
			}

			PropertyAnimation {
				property: "y"
				from: _control._start_y_pos
				to: _control._end_y_pos
				duration: 300
				easing.type: Easing.InCubic
			}
		}
	}

	Transition {
		id: slideOutTransition

		ParallelAnimation {
			alwaysRunToEnd: true

			PropertyAnimation {
				property: "x"
				from: _control.x
				to: _control._start_x_pos
				duration: 300
				easing.type: Easing.InCubic
			}

			PropertyAnimation {
				property: "y"
				from: _control.y
				to: _control._start_y_pos
				duration: 300
				easing.type: Easing.InCubic
			}
		}
	}

	Transition {
		id: alternateCloseTransition
		property int finalPos
		property string property_name

		NumberAnimation {
			alwaysRunToEnd: true
			running: false
			property: alternateCloseTransition.property_name
			to: alternateCloseTransition.finalPos
			duration: 300
			easing.type: Easing.OutQuad
		}
	}

	function createMouseArea(): void {
		if (mouseItem && !mouse_area) {
			let component = Qt.createComponent("TpQml.Widgets", TPMouseArea, Qt.Asynchronous);
			function finishCreation() {
				mouse_area = component.createObject(_control.mouseItem, { enabled: _control.enabled,
						movableWidget: _control, canSlide: _control.canSlideToClose,
						movingWidget: _control.mouseItem, lockMovingToYAxis: _control.lockMovingToYAxis,
						movableWidgetCanGoOutsideBounds: canSlideToClose });
				mouse_area.mousePressed.connect(mouseAreaPressed);
				mouse_area.movingFinished.connect(mouseAreaMovingFinished);
				mouse_area.mouseClicked.connect(mouseItemClicked);
				if (canSlideToClose) {
					mouse_area.slideOutToSide.connect(mouseAreaSlide);
					mouse_area.widgetOutOfBounds.connect(function() { closePopup(TPPopup.SWIPE); });
				}
			}
			function checkComponentStatus() {
				switch (component.status) {
				case Component.Ready:
					component.statusChanged.disconnect(checkComponentStatus);
					finishCreation();
					break;
				case Component.Loading:
					break;
				case Component.Null:
				case Component.Error:
					component.statusChanged.disconnect(checkComponentStatus);
					console.error(component.errorString());
					break;
				}
			}
			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(checkComponentStatus);
		} else if (mouseItem && mouse_area) {
			if (mouse_area.movingWidget !== mouseItem) {
				mouse_area.parent = mouseItem;
				mouse_area.movingWidget = mouseItem;
			}
		}
	}

	function mouseAreaMovingFinished(x: int, y: int): void {
		if (savePopupState > 0)
			AppSettings.setCustomValue(configFieldName + ".pos", Qt.point(x, y));
	}

	function mouseAreaPressed(mouse: MouseEvent): void {
		ItemManager.appPagesManager.raisePopup(_control);
	}

	function mouseAreaSlide(side: int): void {
		_use_alternate_transition = true;
		switch (side) {
		case TPMouseArea.MA_LEFT:
			alternateCloseTransition.finalPos = -width;
			alternateCloseTransition.property_name = "x";
			break;
		case TPMouseArea.MA_RIGHT:
			alternateCloseTransition.finalPos = AppSettings.windowWidth;
			alternateCloseTransition.property_name = "x";
			break;
		case TPMouseArea.MA_TOP:
			alternateCloseTransition.finalPos = 0;
			alternateCloseTransition.property_name = "y";
			break;
		case TPMouseArea.MA_BOTTOM:
			alternateCloseTransition.finalPos = AppSettings.windowHeight;
			alternateCloseTransition.property_name = "y";
			break;
		}
		closePopup(TPPopup.SWIPE);
		_use_alternate_transition = false;
	}

	function showInWindow(): void {
		if (savePopupState) {
			const saved_pos = AppSettings.getCustomValue(configFieldName + ".pos", defaultCoordinates);
			x = _end_x_pos = saved_pos.x;
			y = _end_y_pos = saved_pos.y;
		} else {
			x = _end_x_pos = defaultCoordinates.x;
			y = _end_y_pos = defaultCoordinates.y;
		}
		open();
	}

	function showAlignedInWindow(): void {
		if (show_position & Qt.AlignTop) {
			_start_y_pos = -height;
			_end_y_pos = 0;
			_start_x_pos = _end_x_pos = (AppSettings.pageWidth - width) / 2;
		} else if (show_position & Qt.AlignVCenter) {
			_start_y_pos = _end_y_pos = (AppSettings.windowHeight - height) / 2 - AppSettings.itemDefaultHeight;
		} else if (show_position & Qt.AlignBottom) {
			_start_y_pos = AppSettings.windowHeight + height;
			_end_y_pos = parentPage.height - height;
			_start_x_pos = _end_x_pos = (AppSettings.pageWidth - width) / 2;
		}
		if (show_position & Qt.AlignHCenter) {
			_start_x_pos = _end_x_pos = (AppSettings.pageWidth - width) / 2;
		} else if (show_position & Qt.AlignLeft) {
			_start_x_pos = -width;
			_end_x_pos = 0;
			_start_y_pos = _end_y_pos = (AppSettings.windowHeight - height) / 2;
		} else if (show_position & Qt.AlignRight) {
			_start_x_pos = AppSettings.windowWidth;
			_end_x_pos = AppSettings.windowWidth - width;
			_start_y_pos = _end_y_pos = (AppSettings.windowHeight - height) / 2;
		}

		if (_use_burst_transition) {
			x = _end_x_pos;
			y = _end_y_pos;
		}
		open();
	}

	function showByWidget(): void {
		const point = reference_widget.parent.mapToItem(parentPage, reference_widget.x, reference_widget.y);

		switch (show_position) {
		case Qt.AlignTop:
			_start_x_pos = point.x + reference_widget.width / 2;
			_end_x_pos = _start_x_pos + (reference_widget.width <= width ? - width : width) / 2;
			_end_y_pos = point.y - height;
			_start_y_pos = point.y;
			break;
		case Qt.AlignLeft:
			_start_x_pos = point.x;
			_end_x_pos = point.x - width;
			_start_y_pos = _end_y_pos = (point.y + reference_widget.height / 2) - height / 2;
			break;
		case Qt.AlignRight:
			_start_x_pos = point.x + reference_widget.width;
			_end_x_pos = _start_x_pos + width;
			_start_y_pos = _end_y_pos = (point.y + reference_widget.height / 2) - height / 2;
			break;
		case Qt.AlignBottom:
			_start_x_pos = point.x + reference_widget.width / 2;
			_end_x_pos = _start_x_pos + (reference_widget.width <= width ? - width : width) / 2;
			_end_y_pos = point.y + reference_widget.height;
			_start_y_pos = _end_y_pos + height / 2;
			break;
		}

		if (_end_x_pos < 0)
			_end_x_pos = 0;
		else if (_end_x_pos + width > AppSettings.windowWidth)
			_end_x_pos = AppSettings.pageWidth - width;
		if (_end_y_pos < realY)
			_end_y_pos = realY;
		else if (_end_y_pos + height > 0 + parentPage.height)
			_end_y_pos = parentPage.height - height;

		if (_use_burst_transition) {
			x = _end_x_pos;
			y = _end_y_pos;
		}
		open();
	}

	function tpopen__(): void {
		if (savePopupState) {
			const saved_size = AppSettings.getCustomValue(configFieldName + ".size", normal_size);
			width = saved_size.width;
			height = saved_size.height;
			if (!_minimized && !_maximized) {
				_normal_width = width;
				_normal_height = height;
			} else {
				_normal_width = normal_size.width;
				_normal_height = normal_size.height;
			}
		}
		_can_reopen = false;
		if (show_position === Qt.AlignBaseline) {
			showInWindow();
		} else {
			if (open_in_window)
				showAlignedInWindow();
			else
				showByWidget();
		}
	}

	function tpQmlOpen(parent_page: TPPage): void {
		ItemManager.appPagesManager.openPopup(this, parent_page, show_position);
	}

	function tpOpen(): void {
		tpopen__();
	}

	Timer {
		id: waitForSwipeTimer
		interval: 500
		onTriggered: _control.closeActionExeced(TPPopup.SWIPE);
	}

	function closePopup(action_type: int): void {
		_close_action_type = action_type;
		close();
		if (action_type !== TPPopup.SWIPE)
			closeActionExeced(action_type);
		else
			waitForSwipeTimer.start();
		//when a action button is clicked, the dialog may be immediately reopened for a follow up.
		//When it's closed via btnClose or swipe or backkey (btn_id = -1, and -2, and -3 respectively), no
		_can_reopen = action_type >= TPPopup.DEFAULT_ACTION;
	}

	//This function can be overridden in a derived QML object to perform other actions
	function backKeyPressed(): void {
		closePopup(TPPopup.BACK_KEY);
	}

	function hide(): void {
		if (showBehavior === TPPopup.PARENT_PAGE_ACTIVE) {
			_hidden = true;
			visible = false;
		}
	}

	function restore(): void {
		if (showBehavior === TPPopup.PARENT_PAGE_ACTIVE) {
			_hidden = false;
			visible = true;
		}
	}
}
