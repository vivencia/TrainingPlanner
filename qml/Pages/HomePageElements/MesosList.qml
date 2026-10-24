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

			onClicked: _control.mesoFilteredModel.mesoModel().getMesocyclePage(delegate.mesoIdx, false);

			Rectangle {
				id: optionsRec
				color: AppSettings.primaryDarkColor
				radius: 6
				layer.enabled: true
				visible: false
				anchors.fill: parent
			}

			Rectangle {
				id: removeBackground
				anchors.fill: parent
				color: "lightgray"
				radius: 6
				layer.enabled: true
				visible: false
			}

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

			itemContent: Item {
				id: mesoContent
				height: (lblMesoName.height + lblCoach.height + lblClient.height + lblStartDate.height
																	+ lblEndDate.height + lblSplit.height) + 20
				width: mesosListView.width

				TPLabel {
					id: lblMesoName
					text: delegate.mesoName
					horizontalAlignment: Text.AlignHCenter
					width: parent.width
					y: 0
				}
				TPLabel {
					id: lblCoach
					text: delegate.mesoCoach
					visible: _control.mesoFilteredModel.type !== MesocyclesModel.MT_MESO_FOR_SELF
					width: parent.width
					y: lblMesoName.height
				}
				TPLabel {
					id: lblClient
					text: delegate.mesoClient
					visible: _control.mesoFilteredModel.type === MesocyclesModel.MT_MESO_FOR_CLIENT
					width: parent.width
					y: lblCoach.visible ? lblCoach.y + lblCoach.height: lblMesoName.height
				}
				TPLabel {
					id: lblStartDate
					text: delegate.mesoStartDate
					width: parent.width
					y: lblClient.visible ? lblClient.y + lblClient.height : lblCoach.visible
						? lblCoach.y + lblCoach.height : lblMesoName.height
				}
				TPLabel {
					id: lblEndDate
					text: delegate.mesoEndDate
					width: parent.width
					y: lblStartDate.y + lblStartDate.height
				}
				TPLabel {
					id: lblSplit
					text: delegate.mesoSplit
					width: parent.width
					y: lblEndDate.y + lblEndDate.height
				}
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
				visible: _control.mesoFilteredModel.type !== MesocyclesModel.MT_MESO_FROM_COACH
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

				onClicked: _control.mesoFilteredModel.mesoModel().importProgramFromFile();
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
