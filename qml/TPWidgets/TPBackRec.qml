import QtQuick
import QtQuick.Controls
import QtQuick.Shapes

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

Rectangle {
	id: control
	property bool useShape: false
	property bool useGradient: false
	property bool useImage: false
	property bool scaleImageToControlSize: true

	property string sourceImage
	property double widthScale: 1.0
	property double heightScale: 1.0
	property double overlayOpacity: 0
	property int rotate_angle: 0
	property int x_translation: 0
	property int y_translation: 0
	property size image_size: Qt.size(0, 0)

	property color lightColor: appSettings.primaryLightColor
	property color darkColor: appSettings.primaryDarkColor
	property color midColor: appSettings.primaryColor
	property color paneColor: appSettings.paneBackgroundColor

	gradient: useGradient ? _gradient : null
	color: appSettings.paneBackgroundColor

	Loader {
		active: useImage
		asynchronous: true
		anchors.fill: parent

		sourceComponent: TPImage {
			id: _image
			imageSizeFollowControlSize: scaleImageToControlSize
			wScale: widthScale
			hScale: heightScale
			source: sourceImage

			readonly property Translate translate_viewport: x_translation !== 0 ? trlt_view : null
			readonly property Rotation rotate_viewport: rotate_angle !== 0 ? rtt_view: null

			Translate {
				id: trlt_view
				x: x_translation == -1 ? - _image.width / 2 : x_translation
				y: y_translation == -1 ? - _image.height / 2 : y_translation
			}

			Rotation {
				id: rtt_view
				angle: rotate_angle
				origin.x: _image.width / 2
				origin.y: _image.height / 2
			}
			transform: [ rotate_viewport, translate_viewport ]

			Rectangle {
				color: appSettings.primaryColor
				opacity: overlayOpacity
				anchors.fill: parent
				radius: control.radius
			}
		}
	}

	Loader {
		active: useShape
		asynchronous: true

		sourceComponent: Shape {
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
					GradientStop { position: 0.0; color: darkColor }
					GradientStop { position: 1.0; color: lightColor }
				}
			}
		}
	}

	Gradient {
		id: _gradient
		orientation: Gradient.Horizontal
		GradientStop { position: 0.0; color: paneColor; }
		GradientStop { position: 0.25; color: lightColor; }
		GradientStop { position: 0.50; color: midColor; }
		GradientStop { position: 0.75; color: darkColor; }
	}
}
