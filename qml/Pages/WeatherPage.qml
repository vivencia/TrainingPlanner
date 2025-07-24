import QtQuick
import QtQuick.Controls
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

		TPLabel {
			text: weatherInfo.gpsMessage
			wrapMode: Text.WordWrap
			font: AppGlobals.smallFont
			leftPadding: appSettings.itemDefaultHeight + 5
			horizontalAlignment: Text.AlignJustify
			Layout.maximumWidth: main.width - leftPadding
			Layout.maximumHeight: 0.1 * main.height

			TPImage {
				source: "gps.png"
				enabled: weatherInfo.canUseGps
				width: appSettings.itemDefaultHeight
				height: width

				anchors {
					left: parent.left
					verticalCenter: parent.verticalCenter
				}
			}
		}

		Rectangle {
			id: savedCities
			radius: 8
			color: "#3F000000"
			Layout.alignment: Qt.AlignCenter
			Layout.preferredHeight: 0.2 * main.height
			Layout.fillWidth: true
			Layout.topMargin: 10

			ListView {
				id: scrollViewCities
				model: appSettings.weatherCitiesCount
				width: main.width
				height: parent.height - appSettings.itemDefaultHeight - 10
				contentHeight: model.count * appSettings.itemDefaultHeight * 1.1
				contentWidth: availableWidth
				spacing: 10
				clip: true

				anchors {
					top: parent.top
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
					height: appSettings.itemDefaultHeight

					contentItem: TPLabel {
						id: txtCity
						text: appSettings.weatherCity(index)
						leftPadding: 10

						TPButton {
							imageSource: "remove"
							width: appSettings.itemDefaultHeight

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

				anchors {
					bottom: parent.bottom
					bottomMargin: 5
					left: parent.left
					right: parent.right
				}

				TPLabel {
					text: qsTr("Search:")
					width: 0.25 * parent.width
					Layout.leftMargin: 5
					Layout.preferredWidth: width
				}

				TPTextInput {
					id: txtCities
					width: 0.70 * parent.width
					Layout.preferredWidth: width

					onTextEdited: weatherInfo.searchForCities(text);
					Component.onCompleted: weatherInfo.locationListChanged.connect(showLocationsList);
				}
			} //Row txtNewCity
		} // Rectangle savedCities

		BigForecastIcon {
			id: current
			Layout.fillWidth: true
			//Layout.fillHeight: true
			Layout.maximumHeight: 0.45 * main.height

			topText: weatherInfo.city + "  " + weatherInfo.weather.coordinates + "\n" + weatherInfo.weather.temperature
			weatherIcon: weatherInfo.weather.weatherIcon
			bottomText: weatherInfo.weather.weatherDescription
			bottomBottomText: weatherInfo.weather.extraInfo
		}

		TPButton {
			text: qsTr("Update")
			imageSource: "reload"
			autoSize: true
			flat: false
			Layout.preferredWidth: width
			Layout.alignment: Qt.AlignCenter
			Layout.topMargin: 10

			onClicked: weatherInfo.refreshWeather();
		}

		Rectangle {
			id: forecastFrame
			color: "#3F000000"
			radius: 40
			opacity: 0.9
			height: 0.2 * main.height
			Layout.alignment: Qt.AlignHCenter
			Layout.minimumHeight: height
			Layout.fillWidth: true

			Row {
				id: iconRow
				padding: 0
				spacing: 0
				anchors.centerIn: parent

				readonly property int daysCount: weatherInfo.forecast.length
				readonly property real iconWidth: (daysCount > 0) ? ((forecastFrame.width - 20) / daysCount) : forecastFrame.width

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
						Layout.alignment: Qt.AlignCenter
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
	}

	property TPFloatingMenuBar locationsMenu: null
	function showLocationsList() {
		const list_len = weatherInfo.locationList.length;
		if (list_len > 0) {
			if (locationsMenu === null) {
				let locationsMenuComponent = Qt.createComponent("qrc:/qml/TPWidgets/TPFloatingMenuBar.qml");
				locationsMenu = locationsMenuComponent.createObject(weatherPage, { parentPage: weatherPage, width: weatherPage.width*0.9 });
				locationsMenu.menuEntrySelected.connect(function(id) { weatherInfo.locationSelected(id);  txtCities.clear(); });
			}
			for(let i = 0; i < list_len; ++i)
				locationsMenu.addEntry(weatherInfo.locationList[i], "", i, true);
			locationsMenu.show2(savedCities, 3);
		}
		else {
			if (locationsMenu)
				locationsMenu.clear();
		}
	}
}
