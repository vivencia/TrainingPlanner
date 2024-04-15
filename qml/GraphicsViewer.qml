import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Graph

Page {
	id: pageGraphViewer

	/*Image { //Avoid painting the same area several times. Use Item as root element rather than Rectangle to avoid painting the background several times.
		anchors.fill: parent
		source: "qrc:/images/app_logo.png"
		fillMode: Image.PreserveAspectFit
		asynchronous: true
		opacity: 0.6
	}
	background: Rectangle {
		color: AppSettings.primaryDarkColor
		opacity: 0.7
	}*/

	Graph {
		id: graph
		anchors.centerIn: parent
		anchors.margins: 5
		color: "red"
		width: 100; height: 100
		name: "A simple pie chart"
	}
}
