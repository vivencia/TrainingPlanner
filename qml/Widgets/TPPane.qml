import QtQuick
import QtQuick.Controls

import TpQml

//For a vertically expandable control:
	//set maxHeight to limit the height of the expansion if necessary
	//set a fixed width with width or shownWidth
	//Only child: ColumnLayout(Item).anchors { top: pane.headerWidget.bottom; left: parent.left; topMargin: 10 }
		//Only child: max width: pane.width, any height < 1000

//For a horizontally expandable control:
	//set maxWidth to limit the width of the expansion if necessary
	//set a fixed height with height or shownHeight
	//Only child: RowLayout(Item).anchors { top: parent.top; left: pane.headerWidget.right; leftMargin: 10 }
		//Only child: max height: pane.height, any width < 1000

Flickable {
	id: _control
	clip: true
	contentWidth: children[0].childrenRect.width
	contentHeight: children[0].childrenRect.height
	flickableDirection: vExpandable ? Flickable.VerticalFlick : Flickable.HorizontalFlick
	boundsMovement: Flickable.StopAtBounds
	width: Math.min(hExpandable ? (expanded ? shownWidth : lblHeader.height + 15)
								: shownWidth, maxWidth < lblHeader.height ? 1000 : maxWidth)
	height: Math.min(vExpandable ? (expanded ? shownHeight : lblHeader.height + 15)
								 : shownHeight, maxHeight < lblHeader.height ? 1000 : maxHeight)

//public:
	property string header
	property string icon
	property int shownWidth: hExpandable ? lblHeader.height + contentChild.width : -1
	property int shownHeight: vExpandable ? lblHeader.height + contentChild.height : -1
	property int maxHeight: -1
	property int maxWidth: -1
	property bool vExpandable: true
	property bool hExpandable: false
	property bool expanded: false
	readonly property TPLabel headerWidget: lblHeader

//private:
	readonly property Item contentChild: children[0].children[1]

	ScrollBar.horizontal: ScrollBar {
		id: hBar
		policy: _control.expanded ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
		visible: _control.maxWidth > lblHeader.height ? _control.shownWidth + _control.maxWidth : false
		interactive: Qt.platform.os !== "android"
		x: lblHeader.height + 10
	}
	ScrollBar.vertical: ScrollBar {
		id: vBar
		policy: _control.expanded ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
		visible: _control.maxHeight > lblHeader.height ? _control.shownHeight + _control.maxHeight : false
		interactive: Qt.platform.os !== "android"
		y: lblHeader.height + 10
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

	TPLabel {
		id: lblHeader
		text: _control.header
		width: (_control.vExpandable ? _control.width : _control.height) - imgIcon.width - imgExpand.width
		//transform: [ Rotation	{ origin.x: 0; origin.y: 0; angle: _control.vExpandable ? 0 : 270},
		//			 Translate	{y: _control.vExpandable ? 0 : height}
		//			]
		transform: Rotation	{ origin.x: 0; origin.y: 0; angle: _control.vExpandable ? 0 : 270}

		anchors {
			left: parent.left
			margins: 5
			leftMargin: _control.vExpandable ? imgExpand.width + 10 : 5
			bottomMargin: _control.vExpandable ? 10 : 2
		}
		Component.onCompleted: {
			if (_control.vExpandable)
				anchors.top = parent.top;
			else
				anchors.bottom = parent.bottom;
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
				leftMargin: -width
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
				rightMargin: -width
				verticalCenter: parent.verticalCenter
			}
		}

		MouseArea {
			enabled: parent.enabled
			anchors {
				fill: parent
				leftMargin: -imgIcon.width
				rightMargin: -imgExpand.width
			}

			onClicked: _control.expanded = !_control.expanded;
		}
	}
	/*Component.onCompleted: {
		for (let i = 0; i < children.length; ++i) {
			console.log("children[", i, "] = ", children[i].objectName)
			for (let x = 0; x < children[i].children.length; ++x)
				console.log("	children[", i, "][", x, "] = ", children[i].children[x].objectName)
		}
	}*/
}
