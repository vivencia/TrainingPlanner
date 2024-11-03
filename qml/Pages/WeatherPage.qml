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
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.top: parent.top
		anchors.topMargin: parent.width / 2
		width: appSettings.pageHeight > appSettings.pageWidth * 1.5 ? appSettings.pageHeight : appSettings.pageWidth * 1.5
		height: width
		radius: width / 2
		color: "#000000"
		opacity: 0.15
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
		anchors.fill: parent
		spacing: 0

		Rectangle {
			id: savedCities
			radius: 8
			color: "#3F000000"
			Layout.alignment: Qt.AlignCenter
			Layout.preferredWidth: parent.width
			Layout.preferredHeight: scrollViewCities.height

			ScrollView {
				id: scrollViewCities
				width: parent.width
				contentHeight: citiesLayout.implicitHeight
				contentWidth: width
				Layout.minimumHeight: 0.1*appSettings.pageHeight
				Layout.maximumHeight: 0.25*appSettings.pageHeight
				Layout.topMargin: 10
				Layout.bottomMargin: 10

				ScrollBar.vertical: ScrollBar {
					policy: ScrollBar.AsNeeded
					active: true; visible: citiesLayout.totalHeight > height
				}

				ColumnLayout {
					id: citiesLayout
					anchors.fill: parent
					spacing: 0

					TPLabel {
						id: gpsCity
						text: weatherInfo.canUseGps ? ( (weatherInfo.hasValidCity ? weatherInfo.gpsCity : qsTr("Unknown location"))
							  + (weatherInfo.useGps ? " (GPS)" : "") ) : qsTr("Cannot use GPS on this device")
						font: AppGlobals.largeFont
						fontColor: "white"
						enabled: weatherInfo.canUseGps
						width: 0.8*parent.width
						Layout.alignment: Qt.AlignCenter
						Layout.minimumWidth: width
						Layout.maximumWidth: width
						Layout.topMargin: 5

						TPImage {
							id: gpsIcon
							source: "appSettings.iconFolder/gps"
							enabled: parent.enabled
							height: 25
							width: 25

							anchors {
								left: gpsCity.right
								verticalCenter: gpsCity.verticalCenter
								verticalCenterOffset: -2
							}
						}

						MouseArea {
							anchors.fill: parent
							onClicked: weatherInfo.useGps = true;
						}
					} //TPLabel gpsCity

					Repeater {
						model: appSettings.weatherCitiesCount
						Row {
							spacing: 5
							width: 0.7*parent.width
							height: 30
							Layout.fillWidth: true
							Layout.alignment: Qt.AlignCenter
							Layout.minimumWidth: width
							Layout.maximumWidth: width

							TPButton {
								id: cityName
								text: appSettings.weatherCity(index)
								textColor: "white"
								backgroundColor: "transparent"

								onClicked: weatherInfo.city = cityName.text;
							}

							TPButton {
								imageSource: "remove"
								fixedSize: true
								imageSize: 20
								height: 25
								width: 25
								Layout.topMargin: 5

								onClicked: appSettings.removeWeatherCity(index);
							}
						} //Text cityName
					} //Repeater

					Row {
						Layout.fillWidth: true
						Layout.leftMargin: 5
						Layout.rightMargin: 5
						Layout.bottomMargin: 5
						spacing: 0
						padding: 0

						TPLabel {
							text: qsTr("Search:")
							width: 0.2*parent.width
							Layout.minimumWidth: width
							Layout.maximumWidth: width
						}

						TPTextInput {
							id: txtNewCity
							width: 0.7*parent.width
							Layout.minimumWidth: width
							Layout.maximumWidth: width
							property bool textOK: text.length > 5

							onTextEdited: {
								if (textOK)
									weatherInfo.city = txtNewCity.text;
							}
						}
					} //Row txtNewCity
				} //ColumnLayout citiesLayout
			} //ScrollView
		} // Rectangle savedCities

		BigForecastIcon {
			id: current
			Layout.fillWidth: true
			Layout.fillHeight: true
			Layout.alignment: Qt.AlignHCenter

			topText: weatherInfo.hasValidWeather ? (weatherInfo.city + "  " + weatherInfo.weather.coordinates + "\n" + weatherInfo.weather.temperature) : "??"
			weatherIcon: weatherInfo.hasValidWeather ? weatherInfo.weather.weatherIcon : "sunny"
			bottomText: weatherInfo.hasValidWeather ? weatherInfo.weather.weatherDescription : qsTr("No weather data")
			bottomBottomText: weatherInfo.hasValidWeather ? weatherInfo.weather.extraInfo : ""
		}

		Item {
			implicitWidth: iconRow.implicitWidth
			implicitHeight: iconRow.implicitHeight
			Layout.fillWidth: true

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
