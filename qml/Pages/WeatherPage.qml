import QtQuick
import QtQuick.Controls
import QtQuick.Shapes
import QtQuick.Effects
import QtQuick.Layouts

import org.vivenciasoftware.TrainingPlanner.qmlcomponents
import "./WeatherPageElements"
import "../TPWidgets"
import ".."

TPPage {
	id: weatherPage
	objectName: "weatherPage"

	//Shape because Rectangle does not support diagonal gradient
	Shape {
		preferredRendererType: Shape.CurveRenderer

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
				GradientStop { position: 0.0; color: "#31adff" }
				GradientStop { position: 1.0; color: "#d0e3ff" }
			}
		}
	}

	Rectangle {
		color: "#000000"
		opacity: 0.15
		radius: width / 2
		width: appSettings.heightToWidthRatio > 1.5 ? appSettings.pageHeight : appSettings.pageWidth * 1.5
		height: width

		anchors {
			horizontalCenter: parent.horizontalCenter
			top: parent.top
			topMargin: parent.width / 2
		}
	}

	Item {
		id: statesItem
		visible: false
		state: "loading"
		states: [
			State {
				name: "loading"
				PropertyChanges { main.opacity: 0 }
				PropertyChanges { wait.opacity: 1 }
			},
			State {
				name: "ready"
				PropertyChanges { main.opacity: 1 }
				PropertyChanges { wait.opacity: 0 }
			}
		]
	}

	WeatherInfo {
		id: weatherInfo
		onReadyChanged: {
			if (weatherInfo.ready)
				statesItem.state = "ready"
			else
				statesItem.state = "loading"
		}
	}

	Item {
		id: wait
		anchors.fill: parent

		Text {
			text: weatherInfo.loadMessage
			anchors.centerIn: parent
			font.pixelSize: 18
		}
	}

	ColumnLayout {
		id: main
		spacing: 5

		anchors {
			fill: parent
			leftMargin: 5
			rightMargin: 5
			topMargin: 5
			bottomMargin: 5
		}

		Rectangle {
			id: savedCities
			radius: 8
			color: "#3F000000"
			Layout.alignment: Qt.AlignCenter
			Layout.preferredHeight: 0.25*main.height
			Layout.preferredWidth: main.width

			TPButton {
				id: btnGPS
				text: weatherInfo.canUseGps ? (weatherInfo.useGps ?
							(weatherInfo.hasValidCity ? weatherInfo.gpsCity : qsTr("Unknown location")) : qsTr("Not using GPS now")) :
							qsTr("Cannot use GPS on this device")

				textColor: "white"
				imageSource: "gps.png"
				enabled: weatherInfo.canUseGps
				width: scrollViewCities.width
				height: 25
				fixedSize: true
				Layout.alignment: Qt.AlignCenter
				Layout.minimumWidth: width
				Layout.maximumWidth: width

				onClicked: weatherInfo.useGps = true;

				anchors {
					top: parent.top
					horizontalCenter: parent.horizontalCenter
				}
			} //TPLabel gpsCity

			ListView {
				id: scrollViewCities
				width: main.width
				height: 0.7*parent.height
				contentHeight: model.count*25*1.1
				contentWidth: availableWidth
				spacing: 0
				clip: true
				model: appSettings.weatherCitiesCount

				anchors {
					top: btnGPS.bottom
					left: parent.left
					right: parent.right
				}

				ScrollBar.vertical: ScrollBar {
					policy: ScrollBar.AsNeeded
					active: true; visible: scrollViewCities.contentHeight > scrollViewCities.height
				}

				delegate: ItemDelegate {
					id: delegate
					spacing: 0
					padding: 0
					width: parent.width
					height: 25

					contentItem: Text {
						text: appSettings.weatherCity(index)
						font.pixelSize: appSettings.fontSize
						fontSizeMode: Text.Fit

						TPButton {
							imageSource: "remove"
							fixedSize: true
							imageSize: 20
							height: 25
							width: 25

							anchors {
								right: parent.right
								rightMargin: 10
								verticalCenter: parent.verticalCenter
							}

							onClicked: appSettings.removeWeatherCity(index);
						}
					} //contentItem

					background: Rectangle {
						color: index % 2 === 0 ? appSettings.listEntryColor1 : appSettings.listEntryColor2
					}
					onClicked: weatherInfo.city = cityName.text;
				} //ItemDelegate
			} //ListView

			RowLayout {
				spacing: 0
				height: 25

				anchors {
					top: scrollViewCities.bottom
					topMargin: -10
					left: parent.left
					right: parent.right
				}

				TPLabel {
					text: qsTr("Search:")
					width: 0.25*parent.width
					height: 25
					Layout.leftMargin: 5
					Layout.minimumWidth: width
					Layout.maximumWidth: width
					Layout.preferredHeight: height
				}

				TPTextInput {
					id: txtCities
					width: 0.70*parent.width
					height: 25
					Layout.minimumWidth: width
					Layout.maximumWidth: width
					Layout.preferredHeight: height

					onTextEdited: weatherInfo.placeLookUp(text);

					ListView {
						id: citiesFound
						model: weatherInfo.locationList
						boundsBehavior: Flickable.StopAtBounds
						visible: model.length > 0
						highlight: Rectangle { color: "lightsteelblue"; radius: 5 }
						focus: true
						height: contentHeight
						z: 3

						delegate: ItemDelegate
						{
							spacing: 0
							padding: 0
							width: parent.width
							height: 25

							required property string modelData
							required property int index

							background: Rectangle {
								color: index % 2 === 0 ? appSettings.listEntryColor1 : appSettings.listEntryColor2
							}

							contentItem: TPLabel {
								text: parent.modelData
								width: parent.width
								height: parent.height
							}

							onClicked: {
								weatherInfo.locationSelected(index);
								citiesFound.parent.clear();
							}
						}

						anchors {
							top: txtCities.bottom
							left: txtCities.left
							right: txtCities.right
						}
					}
				}
			} //Row txtNewCity
		} // Rectangle savedCities

		BigForecastIcon {
			id: current
			height: 0.5*main.height
			width: parent.width
			Layout.alignment: Qt.AlignHCenter
			Layout.minimumHeight: height
			Layout.maximumHeight: height
			Layout.minimumWidth: width
			Layout.maximumWidth: width

			topText: weatherInfo.hasValidWeather ? (weatherInfo.city + "  " + weatherInfo.weather.coordinates + "\n" + weatherInfo.weather.temperature) : "??"
			weatherIcon: weatherInfo.hasValidWeather ? weatherInfo.weather.weatherIcon : "sunny"
			bottomText: weatherInfo.hasValidWeather ? weatherInfo.weather.weatherDescription : qsTr("No weather data")
			bottomBottomText: weatherInfo.hasValidWeather ? weatherInfo.weather.extraInfo : ""
		}

		Item {
			width: parent.width
			height: 0.25*main.height
			Layout.topMargin: -15
			Layout.alignment: Qt.AlignHCenter
			Layout.minimumHeight: height
			Layout.maximumHeight: height
			Layout.minimumWidth: width
			Layout.maximumWidth: width

			Rectangle {
				id: forecastFrame
				color: "#3F000000"
				radius: 40
				opacity: 0.15
				anchors.fill: parent
				visible: false

				Row {
					id: iconRow
					anchors.centerIn: parent

					property int daysCount: weatherInfo.forecast.length
					property real iconWidth: (daysCount > 0) ? ((forecastFrame.width - 20) / daysCount) : forecastFrame.width

					Repeater {
						model: weatherInfo.forecast

						ForecastIcon {
							required property string dayOfWeek
							required property string minMaxTemperatures
							required property string weatherIcon

							width: iconRow.iconWidth
							topText: (weatherInfo.hasValidWeather ? dayOfWeek : "??")
							middleIcon: (weatherInfo.hasValidWeather ? weatherIcon : "sunny")
							bottomText: (weatherInfo.hasValidWeather ? minMaxTemperatures : ("??/??"))
						}
					}
				}

				Label {
					text: weatherInfo.weather.provider
					color: "#ffffff"
					font: AppGlobals.smallFont

					anchors {
						bottom: parent.bottom
						horizontalCenter: parent.horizontalCenter
					}
				}

			} //Rectangle forecastFrame
			MultiEffect {
				source: forecastFrame
				anchors.fill: forecastFrame
				shadowEnabled: true
				shadowBlur: 0.5
				shadowHorizontalOffset: 0
				shadowVerticalOffset: 4
				shadowOpacity: 0.6
			}
		}
	}
}
