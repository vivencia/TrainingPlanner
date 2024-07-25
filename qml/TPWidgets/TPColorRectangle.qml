import QtQuick

Rectangle {
	property string darkColor
	property string midColor
	property string lightColor

	id: darkRec
	width: windowWidth/3
	height: width
	border.color: "transparent"
	color: darkColor

	Rectangle {
		id: midRec
		width: darkRec.width*0.6
		height: width
		border.color: "transparent"
		color: midColor
		anchors.verticalCenter: parent.verticalCenter
		anchors.horizontalCenter: parent.horizontalCenter

		Rectangle {
			id: lightRec
			width: midRec.width* 0.5
			height: width
			border.color: "transparent"
			color: lightColor
			anchors.verticalCenter: parent.verticalCenter
			anchors.horizontalCenter: parent.horizontalCenter
		}
	}
}
