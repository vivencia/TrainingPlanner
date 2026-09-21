import QtQuick
import QtQuick.Controls

import TpQml

//For a vertically expandable control:
	//set maxHeight to limit the height of the expansion if necessary
	//set a fixed width with width or shownWidth
	//Only child: ColumnLayout(Item).anchors { top: pane.headerWidget.bottom; left: parent.left; topMargin: 10 }
		//Only child: width: pane.width, any height < 1000

//For a horizontally expandable control:
	//set maxWidth to limit the width of the expansion if necessary
	//set a fixed height with height or shownHeight
	//Only child: RowLayout(Item).anchors { top: parent.top; left: pane.headerWidget.left; leftMargin: pane.headerWidget.height }
		//Only child: height: pane.height, any width < 1000

//customHeaderWidget: set width: pane.headerWidth

Flickable {
	id: _control
	clip: true
	contentWidth: vExpandable ? width : children[0].childrenRect.width
	contentHeight: vExpandable ? children[0].childrenRect.height : height
	flickableDirection: vExpandable ? Flickable.VerticalFlick : Flickable.HorizontalFlick
	boundsMovement: Flickable.StopAtBounds
	width: Math.min(hExpandable ? (expanded ? shownWidth : _headerWidget.height): shownWidth, maxWidth)
	height: Math.min(vExpandable ? (expanded ? shownHeight : _headerWidget.height) : shownHeight, maxHeight)

//public:
	property string header
	property string icon
	property int shownWidth: hExpandable ? _headerWidget.height + contentChild.width : -1
	property int shownHeight: vExpandable ? _headerWidget.height + contentChild.height : -1
	property int maxHeight: -1
	property int maxWidth: -1
	property bool vExpandable: true
	property bool hExpandable: false
	property bool expanded: false
	property bool customHeaderOwnsMouse: customHeaderWidget !== placeholder
	property Item customHeaderWidget: placeholder
	readonly property Item headerWidget: _headerWidget
	readonly property int headerWidth: (vExpandable ? width : height) - imgIcon.width - imgExpand.width
	// Expose placecholder's children as the default property
	//default property alias placeholderContent: placeholder.children

	Timer {
		id: bufferTimer
		interval: 500
		onTriggered: customHeaderWidget.width = headerWidth;
	}

	onHeaderWidthChanged: {
		if (customHeaderWidget !== placeholder) {
			if (bufferTimer.running)
				return;
			else
				bufferTimer.start();
		}
	}

//private:
	readonly property Item contentChild: children[0].children[1]

	ScrollBar.horizontal: ScrollBar {
		id: hBar
		policy: _control.expanded ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
		visible: _control.maxWidth > _headerWidget.height ? _control.shownWidth + _control.maxWidth : false
		interactive: Qt.platform.os !== "android"
		x: _headerWidget.height + 10
	}
	ScrollBar.vertical: ScrollBar {
		id: vBar
		policy: _control.expanded ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
		visible: _control.maxHeight > _headerWidget.height ? _control.shownHeight + _control.maxHeight : false
		interactive: Qt.platform.os !== "android"
		y: _headerWidget.height + 10
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

	Item {
		id: _headerWidget
		width: _control.vExpandable ? _control.width : _control.height
		height: placeholder.height
		transform: [ Rotation	{ origin.x: 0; origin.y: 0; angle: _control.vExpandable ? 0 : 270},
					 Translate	{y: _control.vExpandable ? 0 : height}
					]

		MouseArea {
			z: _control.customHeaderOwnsMouse ? -1 : 1
			anchors.fill: parent
			onClicked: _control.expanded = !_control.expanded;
		}

		Control {
			id: placeholder
			width: _control.headerWidth

			anchors {
				top: _headerWidget.top
				left: imgIcon.right
				right: imgExpand.left
			}

			Loader {
				id: loader
				//set to true because if not, the sourceComponent will be instantiated before the anchorage takes place,
				//even before onCompleted is issued, which will make the source not able to calculate its size correctly
				asynchronous: true
				active: _control.customHeaderWidget === placeholder

				anchors {
					top: parent.top
					left: parent.left
					right: parent.right
				}

				sourceComponent: TPLabel {
					text: _control.header
					Component.onCompleted: placeholder.height = preferredHeight();
				}
			}
		}

		TPImage {
			id: imgIcon
			source: _control.icon
			dropShadow: false
			width: visible ? AppSettings.itemDefaultHeight : 0
			height: width
			visible: _control.icon.length > 0

			anchors {
				left: parent.left
				verticalCenter: parent.verticalCenter
			}
		}
		TPImage {
			id: imgExpand
			source: _control.expanded ? "fold-up.png" : "fold-down.png"
			width: AppSettings.itemSmallHeight
			height: width

			anchors {
				right: parent.right
				verticalCenter: parent.verticalCenter
			}
		}
	}

	Component.onCompleted: {
		if (customHeaderWidget !== placeholder) {
			customHeaderWidget.parent = placeholder;
			customHeaderWidget.anchors.top = placeholder.top;
			customHeaderWidget.anchors.left = placeholder.left;
			customHeaderWidget.anchors.right = placeholder.right;
			placeholder.height = customHeaderWidget.height;
		}

		/*for (let i = 0; i < children.length; ++i) {
			console.log("children[", i, "] = ", children[i].objectName)
			for (let x = 0; x < children[i].children.length; ++x)
				console.log("	children[", i, "][", x, "] = ", children[i].children[x].objectName)
		}*/
	}
}
