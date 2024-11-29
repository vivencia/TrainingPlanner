import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import QtQuick.Layouts

import org.vivenciasoftware.TrainingPlanner.qmlcomponents
import "./WeatherPageElements"
import "../TPWidgets"
import ".."

TPPage {
	id: weatherPage
	objectName: "weatherPage"

	WeatherInfo {
		id: weatherInfo
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
				text: weatherInfo.gpsCity

				textColor: "white"
				imageSource: "gps.png"
				enabled: weatherInfo.canUseGps
				width: scrollViewCities.width
				height: 25
				fixedSize: true
				Layout.alignment: Qt.AlignCenter
				Layout.minimumWidth: width
				Layout.maximumWidth: width

				onClicked: weatherInfo.requestWeatherForGpsCity();

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
						id: txtCity
						text: appSettings.weatherCity(index)
						font.pixelSize: appSettings.fontSize
						fontSizeMode: Text.Fit
						leftPadding: 5
						bottomPadding: 2

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
					onClicked: weatherInfo.requestWeatherForSavedCity(index);
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

					onTextEdited: weatherInfo.searchForCities(text);

					Component.onCompleted: weatherInfo.locationListChanged.connect(showLocationsList);
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

			topText: weatherInfo.city + "  " + weatherInfo.weather.coordinates + "\n" + weatherInfo.weather.temperature
			weatherIcon: weatherInfo.weather.weatherIcon
			bottomText: weatherInfo.weather.weatherDescription
			bottomBottomText: weatherInfo.weather.extraInfo
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
							topText: dayOfWeek
							middleIcon: weatherIcon
							bottomText: minMaxTemperatures
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

	property TPFloatingMenuBar locationsMenu: null
	function showLocationsList() {
		const list_len = weatherInfo.locationList.length;
		if (list_len > 0) {
			if (locationsMenu === null) {
				var locationsMenuComponent = Qt.createComponent("qrc:/qml/TPWidgets/TPFloatingMenuBar.qml");
				locationsMenu = locationsMenuComponent.createObject(weatherPage, { parentPage: weatherPage, width: weatherPage.width*0.9 });
				locationsMenu.menuEntrySelected.connect(function(id) { weatherInfo.locationSelected(id);  txtCities.clear(); });
			}
			for(var i = 0; i < list_len; ++i)
				locationsMenu.addEntry(weatherInfo.locationList[i], "", i, true);
			locationsMenu.show(savedCities, 3);
		}
		else {
			if (locationsMenu)
				locationsMenu.clear();
		}
	}
}
