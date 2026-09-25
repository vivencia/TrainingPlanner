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
	required property HomePageMesoModel mesoSubModel
	property int index
	readonly property int bottomBarHeight: height * (Qt.platform.os !== "android" ? 0.2 : 0.25)

	TPLabel {
		id: lblTitle
		text: _control.mesoSubModel.viewTitle
		useBackground: true
		backgroundColor: _control.mesoSubModel.backgroundColor

		anchors {
			top: parent.top
			horizontalCenter: parent.horizontalCenter
			margins: 5
		}
	}

	TPListView {
		id: mesosListView
		model: _control.mesoSubModel
		spacing: 10
		width: _control.width
		height: calculatePreferredHeight()

		anchors {
			top: lblTitle.bottom
			left: parent.left
			right: parent.right
			margins: 5
		}

		readonly property int maxHeight: _control.height * 0.8 - lblTitle.height - 10

		/** When homePage is loaded, mesoSubModel is probably loaded so no countChanged signal. When mesosListView is
		  * completed, the delegates are not. So, the only way to resize the list upon start up is to wait for the appropriate
		  * delegates to load (itemAtIndex() will not be null) before calculating the height. All of this is that so TPSwipeView
		  * (or, indeed SwipeView for that matter) will have more space to swipe (other than the title label and the left/right
		  * margins) when the list is not accupying the entire screen;
		  **/
		Timer {
			id: waitForDelegatesTimer
			interval: 100
			onTriggered: mesosListView.height = mesosListView.calculatePreferredHeight();
		}

		Connections {
			target: _control.mesoSubModel
			function onCountChanged(): void {
				mesosListView.height = mesosListView.calculatePreferredHeight();
			}
		}

		function calculatePreferredHeight(): int {
			if (_control.mesoSubModel.count > 0) {
				let children_height = 0;
				for (let i = 0; i < _control.mesoSubModel.count; ++i) {
					let _item = itemAtIndex(i);
					if (!_item) { //we know there has to be a delegate(model.count > 0), so we wait for it
						waitForDelegatesTimer.start();
						return 0;
					} else {
						if (children_height + _item.implicitHeight > maxHeight) {
							children_height = maxHeight;
							break;
						} else {
							children_height += _item.implicitHeight + 10;
						}
					}
				}
				return children_height;
			}
			return 0;
		}

		delegate: SwipeDelegate {
			id: delegate
			width: parent.width
			onClicked: _control.mesoSubModel.mesoModel().getMesocyclePage(mesoIdx, false);

			required property int index
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

					onClicked: _control.mesoSubModel.mesoModel().getMesocyclePage(delegate.mesoIdx, false);
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

					onClicked: _control.mesoSubModel.mesoModel().getMesoCalendarPage(delegate.mesoIdx);
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

					onClicked: _control.mesoSubModel.mesoModel().getExercisesPlannerPage(delegate.mesoIdx);
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

					onClicked: _control.mesoSubModel.showOptionsMenu(btnExport, delegate.mesoIdx);
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
					onButton1Clicked: _control.mesoSubModel.mesoModel().removeMesocycle(delegate.mesoIdx);
				}
			} //swipe.right

			Rectangle {
				id: backRec
				anchors.fill: parent
				radius: 8
				layer.enabled: true
				color: _control.mesoSubModel.ownMesosModel ? AppSettings.primaryColor : AppSettings.primaryDarkColor
				border.color: delegate.index === _control.mesoSubModel.currentIndex ? AppSettings.fontColor : "transparent"
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
					visible: _control.mesoSubModel.type === MesocyclesModel.MT_MESO_FOR_CLIENT
				}
				TPLabel {
					text: delegate.mesoClient
					fontColor: AppSettings.fontColor
					Layout.maximumWidth: parent.width
					visible: _control.mesoSubModel.type === MesocyclesModel.MT_MESO_FOR_CLIENT
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
				visible: _control.mesoSubModel.type !== MesocyclesModel.MT_MESO_FOR_CLIENT
				Layout.preferredWidth: preferredWidth
				Layout.maximumWidth: parent.width
				Layout.maximumHeight: AppSettings.itemDefaultHeight
				Layout.alignment: Qt.AlignCenter

				onClicked: _control.mesoSubModel.mesoModel().startNewMesocycle(_control.mesoSubModel.ownMesosModel);
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
				enabled: _control.mesoSubModel.canHaveTodaysWorkout
				Layout.preferredWidth: preferredWidth
				Layout.maximumHeight: AppSettings.itemDefaultHeight
				Layout.alignment: Qt.AlignCenter

				onClicked: _control.mesoSubModel.mesoModel().startTodaysWorkout(_control.mesoSubModel.currentMesoIdx());
			}
		} //ColumnLayout
	}
} //ListView
