pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import QtQuick.Layouts

import TpQml
import TpQml.Widgets

import "./WeatherPageElements"

TPPage {
	id: weatherPage
	objectName: "weatherPage"
	imageSource: AppSettings.weatherBackground
	backgroundOpacity: 0.6

	WeatherInfo {
		id: weatherInfo
	}

	ColumnLayout {
		id: mainLayout
		spacing: 0

		anchors {
			fill: parent
			leftMargin: 5
			rightMargin: 5
			topMargin: 5
			bottomMargin: 5
		}

		TPLabel {
			text: weatherInfo.gpsMessage
			font: AppGlobals.smallFont
			wrapMode: Text.WordWrap
			Layout.minimumWidth: parent.width - 30
			Layout.maximumWidth: parent.width - 30
			Layout.maximumHeight: AppSettings.itemDefaultHeight * 2

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
			Layout.topMargin: 5

			TPListView {
				id: scrollViewCities
				model: weatherInfo.savedLocationsCount
				width: mainLayout.width
				height: parent.height - AppSettings.itemDefaultHeight - 10

				onCountChanged: scrollViewCities.positionViewAtIndex(weatherInfo.currentlyViewedLocationIndex, ListView.Contain);

				anchors {
					top: parent.top
					left: parent.left
					leftMargin: 5
					right: parent.right
					rightMargin: 5
				}

				delegate: ItemDelegate {
					id: delegate
					spacing: 0
					padding: 0
					width: scrollViewCities.width
					height: AppSettings.itemDefaultHeight

					required property int index

					contentItem: TPLabel {
						id: txtCity
						text: weatherInfo.savedLocationName(delegate.index)
						leftPadding: 5
					}

					TPButton {
						imageSource: "remove"
						width: AppSettings.itemDefaultHeight
						enabled: delegate.index == weatherInfo.currentlyViewedLocationIndex

						anchors {
							right: txtCity.right
							rightMargin: 5
							verticalCenter: parent.verticalCenter
						}

						onClicked: weatherInfo.removeWeatherLocation(delegate.index);
					}

					background: Rectangle {
						color: delegate.index === weatherInfo.currentlyViewedLocationIndex ?
																		AppSettings.listEntryColor1 : AppSettings.listEntryColor2
						border.color: AppSettings.fontColor
						border.width: delegate.index === weatherInfo.currentlyViewedLocationIndex ? 1 : 0
						radius: 8
					}

					onClicked: weatherInfo.currentlyViewedLocationIndex = delegate.index;
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
					Layout.preferredWidth: 0.25 * parent.width
					Layout.leftMargin: 5
				}

				TPTextInput {
					id: txtCities
					Layout.preferredWidth: 0.70 * parent.width

					onTextEdited: weatherInfo.searchForCities(text);
				}
			} //Row txtNewCity
		} // Rectangle savedCities

		TodayForecast {
			id: current
			Layout.alignment: Qt.AlignHCenter
			Layout.preferredHeight: 0.55 * mainLayout.height
			Layout.preferredWidth: parent.width

			topText: weatherInfo.city + "  " + weatherInfo.weather.coordinates + "\n" + weatherInfo.weather.temperature
			weatherIcon: weatherInfo.weather.weatherIcon
			bottomText: weatherInfo.weather.weatherDescription
			bottomBottomText: weatherInfo.weather.extraInfo
		}

		Item {
			Layout.alignment: Qt.AlignHCenter
			Layout.preferredHeight: 0.22 * mainLayout.height
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

					readonly property int daysCount: weatherInfo.forecast.length
					readonly property real iconWidth: (daysCount > 0) ? ((forecastFrame.width - 20) / daysCount) : forecastFrame.width

					Repeater {
						model: weatherInfo.forecast

						delegate: NextDaysForecast {
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

	TPPageMenu {
		id: locationsMenu
		parentPage: weatherPage
		entriesList: weatherInfo.locationList
		showIndicator: false
		width: weatherPage.width * 0.9
		onMenuEntrySelected: (id) => {
			weatherInfo.locationSelected(id);
			txtCities.clear();
		}

		Connections {
			target: weatherInfo

			function onLocationListChanged(): void {
				locationsMenu.clearEntries();
				if (weatherInfo.locationList.length === 0)
					locationsMenu.close();
				else {
					locationsMenu.createEntries(weatherInfo.locationList, [], [], []);
					locationsMenu.open();
				}
			}
		}
	}
}
