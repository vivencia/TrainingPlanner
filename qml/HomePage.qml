import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
	id: homePage
	property date minimumStartDate;

	Image {
		anchors.fill: parent
		source: "qrc:/images/app_logo.png"
		fillMode: Image.PreserveAspectFit
		asynchronous: true
		opacity: 0.6
	}
	background: Rectangle {
		color: AppSettings.primaryDarkColor
		opacity: 0.7
	}

	header: ToolBar {
		topPadding: 5
		bottomPadding: 20
		leftPadding: 10
		rightPadding: 10
		height: 60

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

			Image {
				source: "qrc:/images/"+AppSettings.iconFolder+"mesocycle.png"
				width: 50
				height: 50
				Layout.alignment: Qt.AlignCenter
			}

			Label {
				text: qsTr("Training Program")
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
		anchors.fill: parent
		anchors.leftMargin: 10
		anchors.rightMargin: 10
		anchors.topMargin: 40
		spacing: 10

		ScrollBar.vertical: ScrollBar {
			policy: ScrollBar.AsNeeded
			active: ScrollBar.AsNeeded
		}

		delegate: SwipeDelegate {
			id: mesoDelegate
			width: ListView.view.width
			height: 110

			onClicked: appDB.getMesocycle(index);

			swipe.left: Rectangle {
				id: optionsRec
				width: parent.width
				height: parent.height
				clip: false
				color: "steelblue"
				radius: 6
				z: 0
				opacity: mesoDelegate.swipe.complete ? 0.7 : mesoDelegate.swipe.position
				Behavior on opacity { NumberAnimation { } }

				TransparentButton {
					id: btnMesoInfo
					text: qsTr("Mesocycle Info")
					imageSource: "qrc:/images/"+AppSettings.iconFolder+"mesocycle.png"
					width: parent.width*0.65
					z: 1

					anchors {
						left: parent.left
						leftMargin: 2
						top: parent.top
						topMargin: 2
					}

					onClicked: appDB.getMesocycle(index);
				}
				TransparentButton {
					id: btnMesoCalendar
					text: qsTr("View Calendar")
					imageSource: "qrc:/images/"+AppSettings.iconFolder+"edit-mesocycle.png"
					width: parent.width*0.65
					z: 1

					anchors {
						top: btnMesoInfo.bottom
						topMargin: 2
						left: parent.left
						leftMargin: 2
					}

					onClicked: {
						appDB.setWorkingMeso(-1, index);
						appDB.getMesoCalendar(true);
					}
				}
				TransparentButton {
					id: btnMesoPlan
					text: qsTr("Exercises Plan")
					imageSource: "qrc:/images/"+AppSettings.iconFolder+"exercises.png"
					width: parent.width*0.65
					z: 1

					anchors {
						top: btnMesoCalendar.bottom
						topMargin: 2
						left: parent.left
						leftMargin: 2
					}

					onClicked: {
						appDB.setWorkingMeso(-1, index);
						appDB.createExercisesPlannerPage();
					}
				}
				TPRadioButton {
					id: optCurrentMeso
					text: qsTr("Current mesocycle")
					checked: mesocyclesModel.currentRow === index;
					width: parent.width*0.35
					height: 35
					z: 1

					anchors {
						verticalCenter: parent.verticalCenter
						left: btnMesoInfo.right
					}

					onClicked: mesocyclesModel.currentRow = index;
				}
			} //swipe.left: Rectangle

			swipe.right: Rectangle {
				id: removeRec
				width: parent.width
				height: parent.height
				clip: false
				color: "red"
				radius: 6
				z: 0
				opacity: mesoDelegate.swipe.complete ? 0.8 : 0-mesoDelegate.swipe.position
				Behavior on opacity { NumberAnimation { } }

				TransparentButton {
					text: qsTr("Remove Mesocycle")
					imageSource: "qrc:/images/"+AppSettings.iconFolder+"remove.png"
					z: 1

					anchors {
						horizontalCenter: parent.horizontalCenter
						verticalCenter: parent.verticalCenter
					}

					onClicked: msgDlg.show(parent.y + parent.height);
				}

				TPBalloonTip {
					id: msgDlg
					title: qsTr("Remove Mesocycle?")
					message: qsTr("This action cannot be undone. Note: removing a Mesocycle does not remove the records of the days within it.")
					button1Text: qsTr("Yes")
					button2Text: qsTr("No")
					imageSource: "qrc:/images/"+darkIconFolder+"remove.png"

					onButton1Clicked: appDB.removeMesocycle();
				}
			} //swipe.right: Rectangle

			background: Rectangle {
				id: backRec
				radius: 6
				opacity: index === mesocyclesModel.currentRow ? 0.8 : 0.6
				color: index === mesocyclesModel.currentRow ? AppSettings.entrySelectedColor : listEntryColor2
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
			id: btnAddOpenSchedule
			text: qsTr("New open-ended schedule")
			textUnderIcon: true
			imageSource: "qrc:/images/"+AppSettings.iconFolder+"open-schedule.png"
			anchors.left: parent.left
			anchors.verticalCenter: parent.verticalCenter
			anchors.leftMargin: 5

			onClicked: tertetre.init(); //newAction(0);
		}

		TPBalloonTip {
			id: tertetre

			function init() {
				message = appDB.appArgs();
				show(-1);
			}
		}

		TPButton {
			id: btnAddMeso
			text: qsTr("New Mesocycle")
			imageSource: "qrc:/images/"+AppSettings.iconFolder+"mesocycle-add.png"
			textUnderIcon: true
			anchors.right: parent.right
			anchors.verticalCenter: parent.verticalCenter
			anchors.rightMargin: 5

			onClicked: newAction(1);
		}
	} // footer

	function newAction(opt) {
		appDB.createNewMesocycle(opt, opt === 1 ? qsTr("New Mesocycle") : qsTr("New Training Plan"));
	}

	function setViewModel() {
		mesosListView.model = mesocyclesModel;
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
