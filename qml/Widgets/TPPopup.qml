pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import TpQml
import TpQml.Pages

Popup {
	id: _control
	closePolicy: keepAbove ? Popup.NoAutoClose : Popup.CloseOnPressOutside
	parent: Overlay.overlay //global Overlay object. Assures that the dialog is always displayed in relation to global coordinates
	spacing: 0
	topPadding: 0
	topInset: 0

//public:
	property TPPage parentPage
	property bool keepAbove: false
	property bool showTitleBar: false
	property bool closeButtonVisible: showTitleBar
	property bool enableEffects: false
	property bool lockMovingToYAxis: false
	property bool showBorder: false
	property bool useGradient: false
	property bool useShape: false
	property bool canSlideToClose: false
	property bool useAlternateBackground: false
	property string backGroundImage
	property string configFieldName
	property string defaultBackgroundColor: AppSettings.paneBackgroundColor
	property point defaultCoordinates
	property double titleBarOpacity: 0.8
	property TPBackRec backgroundRec
	property Item mouseItem

	signal popupClosed(popup: QtObject);
	signal keyboardNumberPressed(key1: int, key2: int);
	signal keyboardEnterPressed();
	signal closeActionExeced();
	signal mouseItemClicked(mouse: MouseEvent);

//protected:
	property TPButton btnClose
	property TPBackRec titleBar
	property bool global_popup: false
	property bool open_in_window: false
	property Item reference_widget: null
	property int show_position: Qt.AlignCenter
	property TPMouseArea mouse_area: null

//private:
	property bool _use_burst_transition: true
	property bool _use_alternate_transition: false
	property int _start_y_pos; property int _end_y_pos
	property int _start_x_pos; property int _end_x_pos
	property int _key_pressed
	readonly property Transition _transition_in: !_use_alternate_transition ? (_use_burst_transition ? burstOutTransition : slideInTransition) : alternateCloseTransition
	readonly property Transition _transition_out: !_use_alternate_transition ? (_use_burst_transition ? burstInTransition : slideOutTransition) : alternateCloseTransition
	readonly property int titleBarHeight: AppSettings.itemDefaultHeight + 5

	enter: _transition_in
	exit: _transition_out

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
					}
					else {
						_key_pressed = event.key;
						keyboardNumberPressed(event.key, -1);
						keyPressTimer.start();
					}
				}
				break;
			}
		}
	}

	onClosed: {
		if (modal || keepAbove)
			popupClosed(this);
	}

	onParentPageChanged: global_popup = parentPage ? parentPage === ItemManager.AppHomePage() : false;

	onMouseItemChanged: createMouseArea();
	Component.onCompleted: createMouseArea();

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
			implicitWidth: _control.width
			implicitHeight: _control.height
			radius: 8

			Component.onCompleted: {_control.backgroundRec = this; console.log(_control.objectName)}
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

		sourceComponent: TPBackRec {
			radius: 8
			backColor: AppSettings.paneBackgroundColor
			opacity: _control.titleBarOpacity
			height: _control.titleBarHeight
			visible: _control.showTitleBar

			Component.onCompleted: {
				_control.titleBar = this;
				_control.mouseItem = this;
			}

			TPButton {
				imageSource: "close.png"
				hasDropShadow: false
				visible: _control.closeButtonVisible
				width: AppSettings.itemDefaultHeight
				height: width
				z: 2

				anchors {
					verticalCenter: parent.verticalCenter
					right: parent.right
					rightMargin: 5
				}

				onClicked: _control.closePopup();
				Component.onCompleted: _control.btnClose = this;
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

	function realPageY(): int {
		return global_popup ? 0 : parentPage ? parentPage.mapToGlobal(Qt.point(parentPage.y, 0)).y : 0;
	}

	function closePopup(): void {
		closeActionExeced();
		close();
	}

	function backKeyPressed(): void {
		closePopup();
	}

	property bool _creating: false
	function createMouseArea(): void {
		if (_control._creating)
			return;
		if (mouseItem && !mouse_area) {
			_control._creating = true;
			let component = Qt.createComponent("TpQml.Widgets", TPMouseArea, Qt.Asynchronous);

			function finishCreation() {
				mouse_area = component.createObject(_control.mouseItem, { enabled: _control.enabled, movableWidget: _control,
														slideToClose: _control.canSlideToClose, movingWidget: _control.mouseItem,
																					lockMovingToYAxis: _control.lockMovingToYAxis });
				mouse_area.mousePressed.connect(mouseAreaPressed);
				mouse_area.movingFinished.connect(mouseAreaMovingFinished);
				mouse_area.mouseClicked.connect(mouseItemClicked);
				if (canSlideToClose)
					mouse_area.slideOutToSide.connect(mouseAreaSlide);
				_control._creating = false;
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
						console.log(component.errorString());
						break;
				}
			}
			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(checkComponentStatus);
		}
		else if (mouseItem && mouse_area) {
			mouse_area.parent = mouseItem;
			mouse_area.movingWidget = mouseItem;
		}
	}

	function mouseAreaMovingFinished(x: int, y: int): void {
		if (_control.configFieldName !== "")
			AppSettings.setCustomValue(_control.configFieldName, Qt.point(x, y));
	}

	function mouseAreaPressed(mouse: MouseEvent): void {
		ItemManager.AppPagesManager.raisePopup(_control);
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
		closePopup();
		_use_alternate_transition = false;
	}

	function tpopen__(): void {
		if (show_position === Qt.AlignBaseline)
			showInWindow();
		else {
			if (open_in_window)
				showAlignedInWindow();
			else
				showByWidget();
		}
	}

	function tpOpen(): void {
		tpopen__();
	}

	function showInWindow(): void {
		const saved_pos = AppSettings.getCustomValue(configFieldName, defaultCoordinates);
		x = saved_pos.x;
		y = saved_pos.y;
		open();
	}

	function showAlignedInWindow(): void {
		if (show_position & Qt.AlignTop) {
			_start_y_pos = -height;
			_end_y_pos = realPageY();
			_start_x_pos = _end_x_pos = (AppSettings.pageWidth - width) / 2;
		}
		else if (show_position & Qt.AlignVCenter)
			_start_y_pos = _end_y_pos = (AppSettings.windowHeight - height) / 2;
		else if (show_position & Qt.AlignBottom) {
			_start_y_pos = AppSettings.windowHeight + height;
			_end_y_pos = realPageY() + parentPage.height - height;
			_start_x_pos = _end_x_pos = (AppSettings.pageWidth - width) / 2;
		}
		if (show_position & Qt.AlignHCenter)
			_start_x_pos = _end_x_pos = (AppSettings.pageWidth - width) / 2;
		else if (show_position & Qt.AlignLeft) {
			_start_x_pos = -width;
			_end_x_pos = 0;
			_start_y_pos = _end_y_pos = (AppSettings.windowHeight - height) / 2;
		}
		else if (show_position & Qt.AlignRight) {
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
		const point = reference_widget.parent.mapToItem(parent, reference_widget.x, reference_widget.y);

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
		if (_end_y_pos < 0)
			_end_y_pos = 0;
		else if (_end_y_pos + height > realPageY() + parentPage.height)
			_end_y_pos = realPageY() + parentPage.height - height;

		if (_use_burst_transition) {
			x = _end_x_pos;
			y = _end_y_pos;
		}
		open();
	}
}
