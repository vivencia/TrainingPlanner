import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

import "../"
import "../inexportMethods.js" as INEX
import "../TPWidgets"
import com.vivenciasoftware.qmlcomponents

TPPage {
	id: homePage
	objectName: "homePage"
	property date minimumStartDate;
	property TPFloatingMenuBar newMesoMenu: null
	property TPFloatingMenuBar imexportMenu: null
	property TPButton btnImExport: null //INEX cannot see inside the nested tree of mesosListView objects. Make btnImExport a global symbol
	property bool bExportEnabled: true

	header: ToolBar {
		topPadding: 5
		bottomPadding: 20
		leftPadding: 10
		rightPadding: 10
		height: headerHeight

		background: Rectangle {
			gradient: Gradient {
				orientation: Gradient.Horizontal
				GradientStop { position: 0.0; color: AppSettings.paneBackgroundColor; }
				GradientStop { position: 0.25; color: AppSettings.primaryLightColor; }
				GradientStop { position: 0.50; color: AppSettings.primaryColor; }
				GradientStop { position: 0.75; color: AppSettings.primaryDarkColor; }
			}
			opacity: 0.8
		}


		RowLayout {
			anchors.fill: parent

			TPImage {
				source: "app_icon"
				dropShadow: false
				width: 50
				height: 50
				Layout.alignment: Qt.AlignCenter
			}

			Label {
				text: qsTr("Training Organizer")
				color: AppSettings.fontColor
				font.weight: Font.ExtraBold
				font.pointSize: AppSettings.fontSizeTitle
				wrapMode: Text.WordWrap
				Layout.alignment: Qt.AlignCenter
			}
		}
	}

	ListView {
		id: mesosListView
		spacing: 10

		anchors {
			fill: parent
			leftMargin: 10
			rightMargin: 10
			topMargin: 40
		}

		ScrollBar.vertical: ScrollBar {
			policy: ScrollBar.AsNeeded
			active: ScrollBar.AsNeeded
		}

		delegate: SwipeDelegate {
			id: mesoDelegate

			property bool realMeso: mesocyclesModel.isRealMeso(index)

			width: ListView.view.width
			height: mesoContent.childrenRect.height + 20

			onClicked: appDB.getMesocycle(index);

			Rectangle {
				id: optionsRec
				color: "lightgray"
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
				opacity: mesoDelegate.swipe.complete ? 1.0 : mesoDelegate.swipe.position
				Behavior on opacity { NumberAnimation { } }

				TPRadioButton {
					id: optCurrentMeso
					text: qsTr("Current plan")
					checked: mesocyclesModel.currentRow === index;
					width: parent.width/3 - 3
					height: parent.height/2 - 3
					z:1

					anchors {
						top: parent.top
						topMargin: 3
						left: parent.left
						leftMargin: 5
					}

					onClicked: appDB.setWorkingMeso(index);
				}

				TPButton {
					id: btnMesoInfo
					text: qsTr("View Plan")
					textUnderIcon: true
					rounded: false
					imageSource: "mesocycle.png"
					fixedSize: true
					width: parent.width/3 - 3
					height: parent.height/2 - 3
					z:1

					anchors {
						left: parent.left
						leftMargin: 5
						top: optCurrentMeso.bottom
					}

					onClicked: {
						appDB.setWorkingMeso(index);
						appDB.getMesocycle(index);
					}
				}
				TPButton {
					id: btnMesoCalendar
					text: qsTr("Calendar")
					rounded: false
					textUnderIcon: true
					imageSource: "meso-calendar.png"
					fixedSize: true
					width: parent.width/3 - 3
					height: parent.height/2 - 3
					z:1

					anchors {
						top: parent.top
						topMargin: 3
						left: optCurrentMeso.right
					}

					onClicked: {
						appDB.setWorkingMeso(index);
						appDB.getMesoCalendar(true);
					}

					Component.onCompleted: {
						enableButton(index);
						userModel.appUseModeChanged.connect(function(userrow) { enableButton(index); });
						mesocyclesModel.realMesoChanged.connect(enableButton);
					}

					function enableButton(mesoidx) {
						var bEnabled = userModel.appUseMode(0) !== 2;
						if (bEnabled) {
							if (mesoidx === index)
								bEnabled = mesocyclesModel.get(index, 8) === userModel.userName(0);
						}
						enabled = bEnabled;
					}
				}

				TPButton {
					id: btnMesoPlan
					text: qsTr("Exercises Plan")
					rounded: false
					textUnderIcon: true
					imageSource: "meso-splitplanner.png"
					fixedSize: true
					width: parent.width/3 - 3
					height: parent.height/2 - 3
					z:1

					anchors {
						top: btnMesoCalendar.bottom
						left: btnMesoInfo.right
					}

					onClicked: {
						appDB.setWorkingMeso(index);
						appDB.createExercisesPlannerPage();
					}
				}
				TPButton {
					id: btnImport
					text: qsTr("Import")
					rounded: false
					textUnderIcon: true
					imageSource: "import.png"
					fixedSize: true
					width: parent.width/3 - 3
					height: parent.height/2 - 3
					z:1

					anchors {
						top: parent.top
						topMargin: 3
						left: btnMesoPlan.right
					}

					onClicked: {
						appDB.setWorkingMeso(index);
						mainwindow.chooseFileToImport();
					}
				}
				TPButton {
					id: btnExport
					text: qsTr("Export")
					rounded: false
					textUnderIcon: true
					imageSource: "export.png"
					fixedSize: true
					width: parent.width/3 - 3
					height: parent.height/2 - 3
					z:1

					anchors {
						top: btnImport.bottom
						left: btnMesoCalendar.right
					}

					onClicked: {
						appDB.setWorkingMeso(index);
						if (Qt.platform.os === "android")
						{
							btnImExport = this;
							INEX.showInExMenu(homePage, false);
						}
						else
							exportTypeTip.init(false);
					}
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
				opacity: mesoDelegate.swipe.complete ? 0.8 : 0-mesoDelegate.swipe.position
				Behavior on opacity { NumberAnimation { } }

				TPButton {
					text: qsTr("Remove Plan")
					imageSource: "remove"
					hasDropShadow: false
					z:2

					anchors {
						horizontalCenter: parent.horizontalCenter
						verticalCenter: parent.verticalCenter
					}

					onClicked: msgDlg.init(index);
				}

				TPBalloonTip {
					id: msgDlg
					title: qsTr("Remove Plan?")
					message: qsTr("This action cannot be undone. Note: removing a Plan does not remove the records of the days within it.")
					button1Text: qsTr("Yes")
					button2Text: qsTr("No")
					imageSource: "remove"
					parentPage: homePage

					property int mesoidx
					onButton1Clicked: appDB.removeMesocycle(mesoidx);

					function init(meso_idx: int) {
						mesoidx = meso_idx;
						show(-1);
					}
				}
			} //swipe.right

			Rectangle {
				id: backRec
				anchors.fill: parent
				radius: 6
				layer.enabled: true
				color: index === mesocyclesModel.currentRow ? AppSettings.entrySelectedColor : listEntryColor2
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
				opacity: index === mesocyclesModel.currentRow ? 0.8 : 0.6
			}

			contentItem: Column {
				id: mesoContent
				padding: 0
				spacing: 2

				Label {
					text: mesocyclesModel.columnLabel(1) + "<b>" + mesoName + "</b>"
					color: AppSettings.fontColor
					width: mesoDelegate.width
					elide: Text.ElideRight
					opacity: backRec.opacity
				}
				Label {
					text: realMeso ?
							mesocyclesModel.columnLabel(2) + "<b>" + runCmd.formatDate(mesoStartDate) + "</b>" :
							qsTr("Program start date: ") + "<b>" + runCmd.formatDate(mesoStartDate) + "</b>"
					color: AppSettings.fontColor
					opacity: backRec.opacity
				}
				Label {
					text: realMeso ?
							mesocyclesModel.columnLabel(3) + "<b>" + runCmd.formatDate(mesoEndDate) + "</b>" :
							qsTr("Open-ended program - no end date set")
					color: AppSettings.fontColor
					opacity: backRec.opacity
				}
				Label {
					text: mesocyclesModel.columnLabel(5) + "<b>" + mesoWeeks + "</b>"
					color: AppSettings.fontColor
					visible: realMeso
					opacity: backRec.opacity
				}
				Label {
					text: mesocyclesModel.columnLabel(6) + "<b>" + mesoSplit + "</b>"
					color: AppSettings.fontColor
					opacity: backRec.opacity
				}
			}

			Component.onCompleted: {
				mesocyclesModel.realMesoChanged.connect(function (mesoidx) {
					if (mesoidx === index)
						realMeso = mesocyclesModel.isRealMeso(index);
				});
			}
		} //delegate
	} //ListView

	footer: ToolBar {
		id: homePageToolBar
		width: parent.width
		height: 55

		background: Rectangle {
			gradient: Gradient {
				orientation: Gradient.Horizontal
				GradientStop { position: 0.0; color: AppSettings.paneBackgroundColor; }
				GradientStop { position: 0.25; color: AppSettings.primaryLightColor; }
				GradientStop { position: 0.50; color: AppSettings.primaryColor; }
				GradientStop { position: 0.75; color: AppSettings.primaryDarkColor; }
			}
			opacity: 0.8
		}

		TPButton {
			id: btnAddMeso
			text: qsTr("New Training Plan")
			imageSource: "mesocycle-add.png"
			textUnderIcon: true
			rounded: false
			flat: false

			anchors {
				left: parent.left
				verticalCenter: parent.verticalCenter
				leftMargin: 5
			}

			onClicked: {
				if (mesocyclesModel.count > 0)
					appDB.createNewMesocycle(true);
				else
					showEmptyDatabaseMenu();
			}
		}

		TPButton {
			id: btnWorkout
			text: qsTr("Today's workout")
			imageSource: "workout.png"
			textUnderIcon: true
			rounded: false
			flat: false

			anchors {
				right: parent.right
				verticalCenter: parent.verticalCenter
				rightMargin: 5
			}

			onClicked: appDB.getTrainingDay(new Date());
		}
	} // footer

	function setViewModel() {
		mesosListView.model = mesocyclesModel;
	}

	function showEmptyDatabaseMenu() {
		if (newMesoMenu === null) {
			var newMesoMenuMenuComponent = Qt.createComponent("qrc:/qml/TPWidgets/TPFloatingMenuBar.qml");
			newMesoMenu = newMesoMenuMenuComponent.createObject(homePage, { parentPage: homePage });
			newMesoMenu.addEntry(qsTr("Create new plan"), "mesocycle-add.png", 0, true);
			newMesoMenu.addEntry(qsTr("Import plan from file"), "import.png", 1, true);
			newMesoMenu.menuEntrySelected.connect(selectedNewMesoMenuOption);
		}
		newMesoMenu.show(btnAddMeso, 0);
	}

	function selectedNewMesoMenuOption(menuid) {
		switch (menuid) {
			case 0: newAction(1); break;
			case 1: mainwindow.chooseFileToImport(); break;
		}
	}

	function btnWorkoutEnabled() {
		var bMesoOK = mesocyclesModel.count > 0;
		var bHasWorkOuts = false;
		if (bMesoOK) {
			bHasWorkOuts = userModel.appUseMode(0) !== 2;
			if (bHasWorkOuts) {
				if (stackView.depth === 1)
					btnWorkout.enabled = mesocyclesModel.isDateWithinCurrentMeso(new Date());
				else
					btnWorkout.enabled = false;
			}
		}
		btnWorkout.visible = bHasWorkOuts;
	}

	TPComplexDialog {
		id: exportTypeTip
		customStringProperty1: bShare ? qsTr("Share complete plan?") : qsTr("Export complete plan to file?")
		customStringProperty2: qsTr("Include Coach data?")
		customStringProperty3: "export.png"
		button1Text: qsTr("Yes")
		button2Text: qsTr("No")
		customItemSource: "TPDialogWithMessageAndCheckBox.qml"
		parentPage: homePage

		onButton1Clicked: appDB.exportMeso(bShare, customBoolProperty1);

		property bool bShare: false

		function init(share: bool) {
			bShare = share;
			show(-1);
		}
	}
} //Page


/*MouseArea {
				id: swipeDetector
				anchors.fill: parent
				preventStealing: true
				pressAndHoldInterval: 300
				property int xPrev: 0
				property bool tracing: false

				onClicked: {
					if (!recRemoveMeso.visible)
						appDB.getMesocycle(index);
					else
						recRemoveMeso.visible = false;
				}

				onPressAndHold: (mouse) => {
					xPrev = mouse.x;
					if (!recRemoveMeso.visible) {
						if (xPrev >= width/3) {
							tracing = true;
							recRemoveMeso.width = width - xPrev;
							recRemoveMeso.visible = true;
						}
					}
				}

				onPositionChanged: (mouse) => {
					if (!tracing) return;
					if (mouse.x <= 0)
						recRemoveMeso.width = mesosListView.width;
					else {
						recRemoveMeso.width += (xPrev - mouse.x);
						if (mouse.x > xPrev) {
							if (recRemoveMeso.width <= 30)
								recRemoveMeso.visible = false;
						}
						xPrev = mouse.x;
					}
				}

				onReleased: (mouse) => {
					if (tracing)
						tracing = false;
				}
			} //MouseArea */
