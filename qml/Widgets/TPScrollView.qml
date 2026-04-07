pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import TpQml.Pages

ScrollView {
	id: _control
	contentWidth: availableWidth //stops bouncing to the sides

//public:
	required property TPPage parentPage
	property bool navButtonsVisible

//private:
	property TPPageScrollButtons _navButtons

	anchors.margins: 5

	ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
	ScrollBar.vertical: ScrollBar {
		id: vBar
		policy: ScrollBar.AsNeeded

		anchors {
			top: _control.top
			bottom: _control.bottom
			right: _control.right
		}

		onPositionChanged: {
			if (vBar.position < 0.06) {
				_control._navButtons.showUpButton = false;
				_control._navButtons.showDownButton = true;
			}
			else if (vBar.position - (1 - vBar.size) > -0.029) {
				_control._navButtons.showUpButton = true;
				_control._navButtons.showDownButton = false;
			}
			else {
				_control._navButtons.showUpButton = true;
				_control._navButtons.showDownButton = true;
			}
		}
	}

	function setScrollBarPosition(pos: int): void {
		if (pos === 0)
			vBar.setPosition(0);
		else
			vBar.setPosition(pos - vBar.size);
	}

	Loader {
		id: navButtonsLoader
		asynchronous: true
		active: _control.contentHeight > _control.height

		readonly property point coord: _control.parentPage.mapToItem(Overlay.overlay, _control.parentPage.x,
																									_control.parentPage.y + _control.height);
		sourceComponent: TPPageScrollButtons {
			ownerPage: _control.parentPage
			showUpButton: false
			position: navButtonsLoader.coord
			visible: _control.navButtonsVisible
			onScrollTo: (pos) => _control.setScrollBarPosition(pos);
			Component.onCompleted: _control._navButtons = this;
		}
	}
}
