import QtQuick
import QtQuick.Controls
import QtQuick.Shapes

import "../"
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

Page {
	id: tpPage
	width: appSettings.pageWidth
	height: appSettings.pageHeight

	property color colorLight: appSettings.primaryLightColor
	property color colorDark: appSettings.primaryDarkColor
	readonly property int headerHeight: 0.08 * height
	readonly property int footerHeight: 0.10 * height

	signal pageActivated();
	signal pageDeActivated();

	Component.onCompleted: {
		mainwindow.pageActivated_main.connect(pageActivation);
		mainwindow.pageDeActivated_main.connect(pageDeActivation);
	}

	background: Shape {
		preferredRendererType: Shape.CurveRenderer
		anchors.fill: parent

		ShapePath {
			strokeWidth: 0
			startX: 0
			startY: 0

			PathLine { x: appSettings.pageWidth; y: 0 }
			PathLine { x: appSettings.pageWidth; y: appSettings.pageHeight }
			PathLine { x: 0; y: appSettings.pageHeight }
			fillGradient: LinearGradient {
				x1: 0
				y1: appSettings.pageHeight / 4
				x2: appSettings.pageWidth
				y2:  appSettings.pageHeight / 4 * 3
				GradientStop { position: 0.0; color: appSettings.primaryLightColor }
				GradientStop { position: 1.0; color: appSettings.primaryDarkColor }
			}
		}
	}

	function pageDeActivation(page: Item): void {
		if (page !== null) {
			if (page.objectName === tpPage.objectName)
				pageDeActivated();
		}
	}

	function pageActivation(page: Item): void {
		if (page !== null) {
			if (page.objectName === tpPage.objectName)
				pageActivated();
		}
	}
}
