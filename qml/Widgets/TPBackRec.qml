pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Shapes
import QtQuick.Effects

import TpQml

Rectangle {
	id: _control
	property bool useShape: false
	property bool useGradient: false
	property bool useImage: false
	property bool showBorder: false
	property bool enableShadow: false

	property string sourceImage
	property string backColor
	property double widthScale: 1.0
	property double heightScale: 1.0
	property double overlayOpacity: 0
	property int rotate_angle: 0
	property int x_translation: 0
	property int y_translation: 0
	property size image_size: Qt.size(0, 0)

	property color lightColor: AppSettings.primaryLightColor
	property color darkColor: AppSettings.primaryDarkColor
	property color midColor: AppSettings.primaryColor
	property color paneColor: AppSettings.paneBackgroundColor

	property Gradient _gradient

	gradient: useGradient ? _gradient : null
	color: backColor
	border.color: showBorder ? AppSettings.fontColor : "transparent"

	Loader {
		active: _control.useImage
		asynchronous: true
		anchors.fill: parent

		sourceComponent: TPImage {
			id: _image
			imageSizeFollowControlSize: true
			keepAspectRatio: false
			fullWindowView: false
			wScale: _control.widthScale
			hScale: _control.heightScale
			source: _control.sourceImage

			readonly property Translate translate_viewport: _control.x_translation !== 0 ? trlt_view : null
			readonly property Rotation rotate_viewport: _control.rotate_angle !== 0 ? rtt_view: null

			Translate {
				id: trlt_view
				x: _control.x_translation == -1 ? - _image.width / 2 : _control.x_translation
				y: _control.y_translation == -1 ? - _image.height / 2 : _control.y_translation
			}

			Rotation {
				id: rtt_view
				angle: _control.rotate_angle
				origin.x: _image.width / 2
				origin.y: _image.height / 2
			}
			transform: [ rotate_viewport, translate_viewport ]

			Rectangle {
				color: AppSettings.primaryColor
				opacity: _control.overlayOpacity
				anchors.fill: parent
				radius: _control.radius
			}
		}
	}

	Loader {
		active: _control.useShape
		asynchronous: true
		anchors.fill: parent

		sourceComponent: Shape {
			id: _shape
			preferredRendererType: Shape.CurveRenderer

			ShapePath {
				startX: 0
				startY: 0

				PathLine { x: _shape.width; y: 0 }
				PathLine { x: _shape.width; y: _shape.height }
				PathLine { x: 0; y: _shape.height }
				fillGradient: LinearGradient {
					x1: 0
					y1: _shape.height / 2
					x2: _shape.width
					y2: _shape.height / 2 * 3
					GradientStop { position: 0.0; color: _control.darkColor }
					GradientStop { position: 1.0; color: _control.lightColor }
				}
			}
		}
	}

	Loader {
		asynchronous: false
		active: _control.enableShadow
		anchors.fill: _control
		anchors.leftMargin: -8
		anchors.topMargin: -8

		sourceComponent: RectangularShadow {
			offset.x: 8
			offset.y: 8
			radius: _control.radius
			spread: 8
			color: Qt.darker(_control.color, 1.6)
		}
	}

	Loader {
		active: _control.useGradient
		asynchronous: true

		Gradient {
			id: _gradient
			orientation: Gradient.Horizontal
			GradientStop { position: 0.0; color: _control.paneColor; }
			GradientStop { position: 0.25; color: _control.lightColor; }
			GradientStop { position: 0.50; color: _control.midColor; }
			GradientStop { position: 0.75; color: _control.darkColor; }

			Component.onCompleted: _control._gradient = this;
		}
	}
}
