import QtQuick
import QtQuick.Shapes
import QtQuick.Effects
import QtQuick.Layouts

import org.vivenciasoftware.TrainingPlanner.qmlcomponents
import "./WeatherPageElements"

TPPage {
	id: weatherPage
	objectName: "weatherPage"
	width: appSettings.pageWidth
	height: appSettings.pageHeight

	Shape {//Shape because Rectangle does not support diagonal gradient
        preferredRendererType: Shape.CurveRenderer

        ShapePath {
            strokeWidth: 0
            startX: 0; startY: 0

            PathLine { x: appSettings.pageWidth; y: 0 }
            PathLine { x: appSettings.pageWidth; y: appSettings.pageHeight }
            PathLine { x: 0; y: appSettings.pageHeight }
            fillGradient: LinearGradient {
                x1: 0; y1: appSettings.pageHeight / 4
                x2: appSettings.pageWidth; y2:  appSettings.pageHeight / 4 * 3
                GradientStop { position: 0.0; color: "#2CDE85" }
                GradientStop { position: 1.0; color: "#9747FF" }
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

//! [1]
    WeatherInfo {
        id: weatherInfo
        onReadyChanged: {
            if (weatherInfo.ready)
                statesItem.state = "ready"
            else
                statesItem.state = "loading"
        }
    }
//! [1]
    Item {
        id: wait
        anchors.fill: parent

        Text {
            text: "Loading weather data..."
            anchors.centerIn: parent
            font.pointSize: 18
        }
    }

    Item {
        id: main
        anchors.fill: parent

        ColumnLayout {
            id: layout
            spacing: 4

            anchors {
                fill: parent
                topMargin: 6; bottomMargin: 6; leftMargin: 6; rightMargin: 6
            }

            Item {
                Layout.preferredHeight: cityButton.height
                Layout.fillWidth: true

                Rectangle {
                    id: cityButton
                    property int margins: 10
                    anchors.centerIn: parent
                    width: cityName.contentWidth + cityIcon.width + 3 * margins
                    height: cityName.contentHeight + 2 * margins
                    radius: 8
                    color: "#3F000000"
                    visible: false

                    Text {
                        id: cityName
                        text: (weatherInfo.hasValidCity ? weatherInfo.city : qsTr("Unknown location"))
                              + (weatherInfo.useGps ? " (GPS)" : "")
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.margins: parent.margins
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: 24
                        color: "white"
                    }

                    TPImage {
                        id: cityIcon
                        source: "weather/waypoint.svg"
                        height: cityName.font.pixelSize
                        width: implicitWidth * height / implicitHeight
                        anchors.left: cityName.right
                        anchors.margins: parent.margins
                        anchors.verticalCenter: cityName.verticalCenter
                        anchors.verticalCenterOffset: 2
                        visible: false
                    }
                    MultiEffect {
                        source: cityIcon
                        anchors.fill: cityIcon
                        brightness: 1 // make icons white, remove for dark icons
                    }

                }

                MultiEffect {
                    source: cityButton
                    anchors.fill: cityButton
                    shadowEnabled: true
                    shadowBlur: 0.5
                    shadowHorizontalOffset: 0
                    shadowVerticalOffset: 2
                }

                MouseArea {
                    anchors.fill: cityButton
                    onClicked: {
                        if (weatherInfo.useGps) {
                            weatherInfo.useGps = false
                            weatherInfo.city = "Brisbane"
                        } else {
                            switch (weatherInfo.city) {
                            case "Brisbane":
                                weatherInfo.city = "Oslo"
                                break
                            case "Oslo":
                                weatherInfo.city = "Helsinki"
                                break
                            case "Helsinki":
                                weatherInfo.city = "New York"
                                break
                            case "New York":
                                weatherInfo.useGps = true
                                break
                            }
                        }
                    }
                }
            }

            BigForecastIcon {
                id: current
                Layout.fillWidth: true
                Layout.fillHeight: true

                weatherIcon: (weatherInfo.hasValidWeather
                              ? weatherInfo.weather.weatherIcon
                              : "sunny")

                topText: (weatherInfo.hasValidWeather
                          ? weatherInfo.weather.temperature
                          : "??")
                bottomText: (weatherInfo.hasValidWeather
                             ? weatherInfo.weather.weatherDescription
                             : qsTr("No weather data"))
            }

            Item {
                implicitWidth: iconRow.implicitWidth + 40
                implicitHeight: iconRow.implicitHeight + 40

                Layout.fillWidth: true

                Rectangle {
                    id: forcastFrame

                    anchors.fill: parent
                    color: "#3F000000"
                    radius: 40

                    Row {
                        id: iconRow
                        anchors.centerIn: parent

                        property int daysCount: weatherInfo.forecast.length
                        property real iconWidth: (daysCount > 0) ? ((forcastFrame.width - 20) / daysCount)
                                                                 : forcastFrame.width

                        Repeater {
                            model: weatherInfo.forecast
                            ForecastIcon {
                                required property string dayOfWeek
                                required property string temperature
                                required property string weatherIcon
                                id: forecast1
                                width: iconRow.iconWidth
                                topText: (weatherInfo.hasValidWeather ? dayOfWeek : "??")
                                bottomText: (weatherInfo.hasValidWeather ? temperature : ("??" + "/??"))
                                middleIcon: (weatherInfo.hasValidWeather ? weatherIcon : "sunny")
                            }
                        }
                    }
                    visible: false
                }

                MultiEffect {
                    source: forcastFrame
                    anchors.fill: forcastFrame
                    shadowEnabled: true
                    shadowBlur: 0.5
                    shadowHorizontalOffset: 0
                    shadowVerticalOffset: 4
                    shadowOpacity: 0.6
                }
            }
        }
    }
}
