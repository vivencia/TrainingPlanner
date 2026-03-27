pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

import TpQml
import TpQml.Widgets

Item {
	id: _control

	required property HomePageMesoModel mesoSubModel

	TPLabel {
		id: lblTitle
		text: _control.mesoSubModel.ownMesosModel ? qsTr("My Programs") : qsTr("Clients' Programs")
		useBackground: true
		backgroundColor: _control.mesoSubModel.ownMesosModel ? AppSettings.primaryLightColor : AppSettings.primaryColor

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
		height: _control.height * 0.8 - lblTitle.height - 10

		anchors {
			top: lblTitle.bottom
			left: parent.left
			right: parent.right
			margins: 5
		}

		delegate: SwipeDelegate {
			id: delegate
			width: parent.width

			onClicked: _control.mesoSubModel.mesoModel().getMesocyclePage(mesoIdx, false);
			onPressAndHold: _control.mesoSubModel.currentIndex = index;
			swipe.onCompleted: _control.mesoSubModel.currentIndex = index;

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
					imageSource: "mesocycle.png"
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
					imageSource: "meso-calendar.png"
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
					imageSource: "meso-splitplanner.png"
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
					imageSource: "export.png"
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

					onClicked: _control.showExportMenu(this);
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
					imageSource: "remove"
					hasDropShadow: false
					autoSize: true
					z: 1

					anchors {
						horizontalCenter: parent.horizontalCenter
						verticalCenter: parent.verticalCenter
					}

					onClicked: {
						msgDlg.meso_name = delegate.mesoName;
						msgDlg.showInWindow(-Qt.AlignCenter);
					}
				}

				TPBalloonTip {
					id: msgDlg
					title: qsTr("Remove ") + meso_name + "?"
					message: qsTr("This action cannot be undone.")
					imageSource: "remove"
					keepAbove: true
					parentPage: ItemManager.AppPagesManager.homePage() as TPPage

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
					visible: _control.mesoSubModel.ownMesosModel
				}
				TPLabel {
					text: delegate.mesoClient
					fontColor: AppSettings.fontColor
					Layout.maximumWidth: parent.width
					visible: !_control.mesoSubModel.ownMesosModel
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
		height: parent.height * (Qt.platform.os !== "android" ? 0.2 : 0.25)

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
				imageSource: "mesocycle-add.png"
				Layout.preferredWidth: preferredWidth
				Layout.maximumWidth: parent.width
				Layout.maximumHeight: AppSettings.itemDefaultHeight
				Layout.alignment: Qt.AlignCenter

				onClicked: _control.mesoSubModel.mesoModel().startNewMesocycle(_control.mesoSubModel.ownMesosModel);
			}

			TPButton {
				id: btnImportMeso
				text: qsTr("Import program from file")
				imageSource: "import.png"
				Layout.preferredWidth: preferredWidth
				Layout.maximumWidth: parent.width
				Layout.maximumHeight: AppSettings.itemDefaultHeight
				Layout.alignment: Qt.AlignCenter

				onClicked: ItemManager.chooseFileToImport();
			}

			TPButton {
				id: btnWorkout
				text: qsTr("Today's workout")
				imageSource: "workout.png"
				visible: _control.mesoSubModel.ownMesosModel
				enabled: _control.mesoSubModel.canHaveTodaysWorkout
				Layout.preferredWidth: preferredWidth
				Layout.maximumHeight: AppSettings.itemDefaultHeight
				Layout.alignment: Qt.AlignCenter

				onClicked: _control.mesoSubModel.mesoModel().startTodaysWorkout(_control.mesoSubModel.currentMesoIdx());
			}
		} //ColumnLayout
	}

	FileOperations {
		id: fileOps
		fileType: AppUtils.FT_TP_PROGRAM
	}

	Loader {
		id: exportMenuLoader
		asynchronous: true
		active: false

		property TPFloatingMenuBar _export_menu;
		property TPButton exportButton

		sourceComponent: TPFloatingMenuBar {
			parentPage: ItemManager.AppPagesManager.homePage() as TPPage
			titleHeader: qsTr("Export options")
			entriesList: fileOps.operationsList
			onMenuEntrySelected: (id) => fileOps.doFileOperation(id);
			onClosed: exportMenuLoader.active = false;
			Component.onCompleted: exportMenuLoader._export_menu = this;
		}

		onLoaded: _export_menu.showByWidget(exportButton, Qt.AlignTop);
	}

	property TPFloatingMenuBar exportMenu: null
	function showExportMenu(button: TPButton): void {
		exportMenuLoader.exportButton = button;
		exportMenuLoader.active = true;
	}
} //ListView
