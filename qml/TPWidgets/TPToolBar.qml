import QtQuick
import QtQuick.Controls
import QtQuick.Shapes

ToolBar {
	spacing: 0
	padding: 0
	height: headerHeight
	width: appSettings.windowWidth

	background: Shape {
		preferredRendererType: Shape.CurveRenderer
		anchors.fill: parent

		ShapePath {
			startX: 0
			startY: 0

			PathLine { x: width; y: 0 }
			PathLine { x: width; y: height }
			PathLine { x: 0; y: height }
			fillGradient: LinearGradient {
				x1: 0
				y1: height / 2
				x2: width
				y2: height / 2 * 3
				GradientStop { position: 0.0; color: appSettings.primaryDarkColor }
				GradientStop { position: 1.0; color: appSettings.primaryLightColor }
			}
		}
	}
}
