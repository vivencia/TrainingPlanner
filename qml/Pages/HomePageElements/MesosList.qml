pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

import TpQml
import TpQml.Widgets

Item {
	id: _control

//public:
	required property HomePageMesoModel mesoFilteredModel
	readonly property int bottomBarHeight: height * (Qt.platform.os !== "android" ? 0.2 : 0.25)

	TPLabel {
		id: lblTitle
		text: _control.mesoFilteredModel.viewTitle
		useBackground: true
		backgroundColor: _control.mesoFilteredModel.backgroundColor

		anchors {
			top: parent.top
			horizontalCenter: parent.horizontalCenter
			margins: 5
		}
	}

	TPListView {
		id: mesosListView
		listModel: _control.mesoFilteredModel
		parentPage:_control.mesoFilteredModel.parentPage as TPPage
		spacing: 10
		width: _control.width
		height: Math.min(minimumHeight, _control.height) - lblTitle.height - 10

		anchors {
			top: lblTitle.bottom
			left: parent.left
			right: parent.right
			margins: 5
		}

		delegate: TPListViewDelegate {
			id: delegate
			_index: index
			_itemVisible: itemVisible
			tpListView: mesosListView

			required property int index
			required property bool itemVisible
			required property bool selected

			required property string mesoName
			required property string mesoStartDate
			required property string mesoEndDate
			required property string mesoSplit
			required property string mesoCoach
			required property string mesoClient
			required property int mesoIdx
			required property bool mesoExportable
			required property bool mesoSplitsAvailable
			required property bool haveCalendar

			Rectangle {
				id: optionsRec
				color: AppSettings.primaryDarkColor
				radius: 6
				layer.enabled: true
				visible: false
				anchors.fill: parent
			}

			swipe.left: MultiEffect {
				id: optionsEffect
				anchors.fill: parent
				source: optionsRec
				shadowEnabled: true
				shadowOpacity: 0.5
				blurMax: 16
				shadowBlur: 1
				shadowHorizontalOffset: 5
				shadowVerticalOffset: 5
				shadowColor: "black"
				shadowScale: 1
				opacity: delegate.swipe.complete ? 0.8 : delegate.swipe.position
				Behavior on opacity { NumberAnimation {} }

				TPButton {
					id: btnMesoInfo
					text: qsTr("View Program")
					image: "mesocycle.png"
					textUnderIcon: true
					rounded: false
					width: parent.width / 2 - 10
					height: parent.height / 2 - 10
					z:1

					anchors {
						top: parent.top
						topMargin: 5
						left: parent.left
						leftMargin: 5
					}

					onClicked: _control.mesoFilteredModel.mesoModel().getMesocyclePage(delegate.mesoIdx, false);
				}

				TPButton {
					id: btnMesoCalendar
					text: qsTr("Calendar")
					image: "meso-calendar.png"
					rounded: false
					textUnderIcon: true
					enabled: delegate.haveCalendar
					width: parent.width / 2 - 10
					height: parent.height / 2 - 10
					z: 1

					anchors {
						top: parent.top
						topMargin: 5
						left: btnMesoInfo.right
						leftMargin: 5
					}

					onClicked: _control.mesoFilteredModel.mesoModel().getMesoCalendarPage(delegate.mesoIdx);
				}

				TPButton {
					id: btnMesoPlan
					text: qsTr("Exercises Sheet")
					image: "meso-splitplanner.png"
					rounded: false
					enabled: delegate.mesoSplitsAvailable
					textUnderIcon: true
					width: parent.width / 2 - 10
					height: parent.height / 2 - 10
					z: 1

					anchors {
						top: btnMesoInfo.bottom
						topMargin: 5
						left: parent.left
						leftMargin: 5
					}

					onClicked: _control.mesoFilteredModel.mesoModel().getExercisesPlannerPage(delegate.mesoIdx);
				}

				TPButton {
					id: btnExport
					text: qsTr("Export")
					image: "export.png"
					rounded: false
					textUnderIcon: true
					enabled: delegate.mesoExportable
					width: parent.width / 2 - 10
					height: parent.height / 2 - 10
					z: 1

					anchors {
						top: btnMesoCalendar.bottom
						topMargin: 5
						left: btnMesoPlan.right
						leftMargin: 5
					}

					onClicked: _control.mesoFilteredModel.showOptionsMenu(btnExport, delegate.mesoIdx);
				}
			} //swipe.left: Rectangle

			Rectangle {
				id: removeBackground
				anchors.fill: parent
				color: "lightgray"
				radius: 6
				layer.enabled: true
				visible: false
			}

			swipe.right: MultiEffect {
				id: removeRec
				anchors.fill: parent
				source: removeBackground
				shadowEnabled: true
				shadowOpacity: 0.5
				blurMax: 16
				shadowBlur: 1
				shadowHorizontalOffset: 5
				shadowVerticalOffset: 5
				shadowColor: "black"
				shadowScale: 1
				opacity: delegate.swipe.complete ? 0.8 : 0-delegate.swipe.position
				Behavior on opacity { NumberAnimation {} }

				TPButton {
					text: qsTr("Remove Program")
					image: "remove"
					z: 1

					anchors {
						horizontalCenter: parent.horizontalCenter
						verticalCenter: parent.verticalCenter
					}

					onClicked: {
						msgDlg.meso_name = delegate.mesoName;
						msgDlg.tpOpen();
					}
				}

				TPBalloonTip {
					id: msgDlg
					title: qsTr("Remove ") + meso_name + "?"
					message: qsTr("This action cannot be undone.")
					image: "remove"
					keepAbove: true
					parentPage: ItemManager.appPagesManager.homePage() as TPPage

					property string meso_name
					onButton1Clicked: _control.mesoFilteredModel.mesoModel().removeMesocycle(delegate.mesoIdx);
				}
			} //swipe.right

			Rectangle {
				id: backRec
				anchors.fill: parent
				radius: 8
				layer.enabled: true
				color: {
					switch (_control.mesoFilteredModel.type) {
						case MesocyclesModel.MT_MESO_FROM_COACH: return AppSettings.primaryColor;
						case MesocyclesModel.MT_MESO_FOR_SELF: return AppSettings.primaryLightColor;
						case MesocyclesModel.MT_MESO_FOR_CLIENT: return AppSettings.primaryDarkColor;
					}
				}
				border.color: delegate.index === _control.mesoFilteredModel.currentRow ? AppSettings.fontColor : "transparent"
				visible: false
			}

			background: MultiEffect {
				id: mesoEntryEffect
				visible: true
				source: backRec
				shadowEnabled: true
				shadowOpacity: 0.5
				blurMax: 16
				shadowBlur: 1
				shadowHorizontalOffset: 5
				shadowVerticalOffset: 5
				shadowColor: "black"
				shadowScale: 1
				opacity: 0.8
			}

			contentItem: ColumnLayout {
				id: mesoContent
				spacing: 2

				TPLabel {
					text: delegate.mesoName
					fontColor: AppSettings.fontColor
					horizontalAlignment: Text.AlignHCenter
					Layout.bottomMargin: 10
					Layout.maximumWidth: parent.width
				}
				TPLabel {
					text: delegate.mesoCoach
					fontColor: AppSettings.fontColor
					Layout.maximumWidth: parent.width
					visible: _control.mesoFilteredModel.type === MesocyclesModel.MT_MESO_FOR_CLIENT
				}
				TPLabel {
					text: delegate.mesoClient
					fontColor: AppSettings.fontColor
					Layout.maximumWidth: parent.width
					visible: _control.mesoFilteredModel.type === MesocyclesModel.MT_MESO_FOR_CLIENT
				}
				TPLabel {
					text: delegate.mesoStartDate
					fontColor: AppSettings.fontColor
				}
				TPLabel {
					text: delegate.mesoEndDate
					fontColor: AppSettings.fontColor
				}
				TPLabel {
					text: delegate.mesoSplit
					fontColor: AppSettings.fontColor
					Layout.maximumWidth: parent.width
				}
			}
		} //delegate
	} //ListView

	TPToolBar {
		height: _control.bottomBarHeight

		anchors {
			left: parent.left
			right: parent.right
			bottom: parent.bottom
		}

		ColumnLayout {
			spacing: 5
			anchors.fill: parent
			anchors.margins: 5

			TPButton {
				id: btnAddMeso
				text: qsTr("New Training Program")
				image: "mesocycle-add.png"
				visible: _control.mesoFilteredModel.type !== MesocyclesModel.MT_MESO_FOR_CLIENT
				Layout.preferredWidth: preferredWidth
				Layout.maximumWidth: parent.width
				Layout.maximumHeight: AppSettings.itemDefaultHeight
				Layout.alignment: Qt.AlignCenter

				onClicked: _control.mesoFilteredModel.mesoModel().startNewMesocycle(_control.mesoFilteredModel.type);
			}

			TPButton {
				id: btnImportMeso
				text: qsTr("Import program from file")
				image: "import.png"
				Layout.preferredWidth: preferredWidth
				Layout.maximumWidth: parent.width
				Layout.maximumHeight: AppSettings.itemDefaultHeight
				Layout.alignment: Qt.AlignCenter

				onClicked: ItemManager.chooseFileToImport();
			}

			TPButton {
				id: btnWorkout
				text: qsTr("Today's workout")
				image: "workout.png"
				enabled: _control.mesoFilteredModel.canHaveTodaysWorkout
				Layout.preferredWidth: preferredWidth
				Layout.maximumHeight: AppSettings.itemDefaultHeight
				Layout.alignment: Qt.AlignCenter

				onClicked: _control.mesoFilteredModel.mesoModel().startTodaysWorkout(_control.mesoFilteredModel.currentMesoIdx());
			}
		} //ColumnLayout
	}
} //ListView
