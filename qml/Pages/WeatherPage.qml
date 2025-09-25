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
		id: mainLayout
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
			Layout.minimumWidth: parent.width - 30
			Layout.maximumWidth: parent.width - 30
			Layout.maximumHeight: appSettings.itemDefaultHeight * 2

			TPButton {
				imageSource: "gps.png"
				enabled: weatherInfo.canUseGps
				width: 30
				height: 30
				onClicked: weatherInfo.requestWeatherForGpsCity();

				anchors {
					verticalCenter: parent.verticalCenter
					left: parent.right
				}
			}
		}

		Rectangle {
			id: savedCities
			radius: 8
			color: "#3F000000"
			Layout.alignment: Qt.AlignCenter
			Layout.preferredHeight: 0.2 * mainLayout.height
			Layout.fillWidth: true
			Layout.topMargin: 10

			ListView {
				id: scrollViewCities
				model: appSettings.weatherCitiesCount
				width: mainLayout.width
				height: parent.height - appSettings.itemDefaultHeight - 10
				contentHeight: model.count * appSettings.itemLargeHeight
				contentWidth: availableWidth
				spacing: 0
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

		TodayForecast {
			id: current
			height: 0.55 * mainLayout.height
			width: parent.width
			Layout.alignment: Qt.AlignHCenter
			Layout.preferredHeight: height
			Layout.preferredWidth: width

			topText: weatherInfo.city + "  " + weatherInfo.weather.coordinates + "\n" + weatherInfo.weather.temperature
			weatherIcon: weatherInfo.weather.weatherIcon
			bottomText: weatherInfo.weather.weatherDescription
			bottomBottomText: weatherInfo.weather.extraInfo
		}

		Item {
			height: 0.22 * mainLayout.height
			Layout.alignment: Qt.AlignHCenter
			Layout.preferredHeight: height
			Layout.bottomMargin: 10
			Layout.fillWidth: true

			Rectangle {
				id: forecastFrame
				color: "#3F000000"
				radius: 40
				opacity: 0.15
				visible: false
				anchors.fill: parent

				Row {
					id: iconRow
					anchors.centerIn: parent
					anchors.leftMargin: 20
					padding: 0

					property int daysCount: weatherInfo.forecast.length
					property real iconWidth: (daysCount > 0) ? ((forecastFrame.width - 20) / daysCount) : forecastFrame.width

					Repeater {
						model: weatherInfo.forecast

						NextDaysForecast {
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

			MultiEffect {
				source: forecastFrame
				anchors.fill: forecastFrame
				shadowEnabled: true
				shadowBlur: 0.5
				shadowHorizontalOffset: 0
				shadowVerticalOffset: 4
				shadowOpacity: 0.6
			}
		} //Item
	} //ColumnLayout mainLayout


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
