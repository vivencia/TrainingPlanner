import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

Drawer {
	id: drawer
	height: mainwindow.height
	implicitWidth: mainwindow.width * 0.7
	spacing: 0
	padding: 0
	edge: Qt.LeftEdge

	Connections {
		target: userModel
		function onUserModified(row: int, field: int): void {
			if (row === 0) {
				switch (field) {
					case 1: lblAvatar.text = userModel.userName(0); break;
					case 20: imgAvatar.source = userModel.avatar(0, false); break;
					default: break;
				}
			}
		}
	}

	background: Rectangle {
		id: backgrundRec
		gradient: Gradient {
			orientation: Gradient.Horizontal
			GradientStop { position: 0.0; color: appSettings.paneBackgroundColor; }
			GradientStop { position: 0.25; color: appSettings.primaryLightColor; }
			GradientStop { position: 0.50; color: appSettings.primaryColor; }
			GradientStop { position: 0.75; color: appSettings.primaryDarkColor; }
		}
		opacity: 0.8
	}

	contentItem {
		Keys.onPressed: (event) => {
			if (event.key === mainwindow.backKey) {
				event.accepted = true;
				close();
			}
		}
	}

	Column {
		id: drawerLayout
		objectName: "drawerLayout"
		spacing: 5
		opacity: parent.opacity
		height: drawer.height*0.65

		anchors {
			left: parent.left
			right: parent.right
			top: parent.top
			leftMargin: 5
			rightMargin: 5
			topMargin: 0
			bottomMargin: 5
		}

		TPImage {
			id: imgLogo
			source: "app_icon"
			dropShadow: false
			width: parent.height*0.25
			height: width
			x: (parent.width-width)/2
		}

		TPLabel {
			text: "TrainingPlanner by VivenciaSoftware - " + appSettings.appVersion
			wrapMode: Text.WordWrap
			font: AppGlobals.smallFont
			horizontalAlignment: Text.AlignHCenter
			width: drawer.width - 10
		}

		TPImage {
			id: imgAvatar
			dropShadow: true
			source: userModel.avatar(0)
			width: parent.height*0.25
			height: width
			x: (parent.width-width)/2

			MouseArea {
				anchors.fill: parent
				enabled: { // Force the binding to re-evaluate so that the check is run each time the page changes.
					stackView.currentItem
					!stackView.find((item, index) => { return item.objectName === "configurationPage"; })
				}

				onClicked: {
					itemManager.getSettingsPage(1);
					close();
				}
			}
		}

		TPLabel {
			id: lblAvatar
			text: userModel.userName(0);
			horizontalAlignment: Text.AlignHCenter
			width: parent.width
		}

		Rectangle {
			color: appSettings.fontColor
			height: 3
			width: parent.width
		}

		TPButton {
			id: btnExercises
			text: qsTr("Exercises Database")
			width: parent.width

			enabled: { // Force the binding to re-evaluate so that the objectName check is run each time the page changes.
				stackView.currentItem
				!stackView.find((item, index) => { return item.objectName === "exercisesPage"; })
			}

			onClicked: {
				itemManager.getExercisesPage();
				close();
			}
		}

		TPButton {
			id: btnSettings
			text: qsTr("Settings")
			width: parent.width

			enabled: { // Force the binding to re-evaluate so that the check is run each time the page changes.
				stackView.currentItem
				!stackView.find((item, index) => { return item.objectName === "configurationPage"; })
			}

			onClicked: {
				itemManager.getSettingsPage(0);
				close();
			}
		}

		TPButton {
			id: btnAllWorkouts
			text: qsTr("All Workouts")
			width: parent.width

			enabled: { // Force the binding to re-evaluate so that the check is run each time the page changes.
				stackView.currentItem
				!stackView.find((item, index) => { return item.objectName === "allWorkoutsPage"; })
			}

			onClicked: {
				itemManager.getAllWorkoutsPage();
				close();
			}
		}

		Rectangle {
			color: appSettings.fontColor
			height: 3
			width: parent.width
			Layout.topMargin: 10
			Layout.bottomMargin: 10
		}
	} //ColumnLayout

	ListView {
		id: pagesList
		model: pagesListModel
		clip: true
		boundsBehavior: Flickable.StopAtBounds
		height: drawer.height*0.35
		contentHeight: availableHeight
		contentWidth: availableWidth

		ScrollBar.vertical: ScrollBar {
			policy: ScrollBar.AsNeeded
			active: true
		}

		anchors {
			top: drawerLayout.bottom
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}

		delegate: SwipeDelegate {
			id: delegate
			spacing: 10
			padding: 5
			width: pagesList.width
			height: 35

			contentItem: TPLabel {
				id: listItem
				text: displayText
				horizontalAlignment: Text.AlignHCenter
			}

			background: Rectangle {
				id:	backgroundColor
				color: appSettings.primaryDarkColor
				radius: 6
				opacity: 1
			}

			onClicked: pagesListModel.openMainMenuShortCut(index);

			swipe.right: Rectangle {
				width: parent.width
				height: parent.height
				clip: false
				color: SwipeDelegate.pressed ? "#555" : "#666"
				radius: 5

				TPImage {
					source: "remove"
					width: 30
					height: 30
					opacity: 2 * -delegate.swipe.position
					z:2

					anchors {
						right: parent.right
						rightMargin: 20
						verticalCenter: parent.verticalCenter
					}
				}
			} //swipe.right

			swipe.onCompleted: pagesListModel.removeMainMenuShortCut(index);
		} //delegate: SwipeDelegate
	} //ListView

	TPButton {
		id: btnExit
		text: qsTr("Exit")
		imageSource: "application-exit.png"
		iconOnTheLeft: true
		rounded: false

		anchors {
			left: parent.left
			right: parent.right
			bottom: parent.bottom
		}

		onClicked: itemManager.exitApp();
	}
} //Drawer
