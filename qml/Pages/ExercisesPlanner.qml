import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import QtCore

import "../"
import "../TPWidgets"
import "../ExercisesAndSets"

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

TPPage {
	id: pagePlanner
	imageSource: ":/images/backgrounds/backimage-splits.jpg"
	backgroundOpacity: 0.6
	objectName: "exercisesPlanner"

	required property SplitManager splitManager
	property WorkoutOrSplitExercisesList currentSplitPage: null
	readonly property int splitPageHeight: appSettings.pageHeight - topToolBar.height - bottomToolBar.height

	Keys.onPressed: (event) => {
		if (event.key === mainwindow.backKey) {
			if (swipeView.currentIndex !== 0) {
				event.accepted = true;
				swipeView.decrementCurrentIndex();
			}
		}
	}

	header: TPToolBar {
		id: topToolBar
		height: appSettings.pageHeight * 0.3

		ColumnLayout {
			id: toolbarLayout
			spacing: 5
			anchors {
				fill: parent
				margins: 5
			}

			Row {
				Layout.fillWidth: true
				spacing: 2
				padding: 0

				TPButton {
					imageSource: "prev"
					enabled: swipeView.currentIndex > 0
					width: appSettings.itemDefaultHeight
					height: width

					onClicked: swipeView.decrementCurrentIndex();
				}

				TPLabel {
					id: lblMain
					text: currentSplitPage && currentSplitPage.exercisesModel && qsTr("Training Division ") + currentSplitPage.exercisesModel.splitLetter
					font: AppGlobals.largeFont
					width: parent.width - appSettings.itemDefaultHeight * 2 - 5
					horizontalAlignment: Text.AlignHCenter
				}

				TPButton {
					imageSource: "next"
					enabled: swipeView.count > 0 && swipeView.currentIndex < swipeView.count - 1
					width: appSettings.itemDefaultHeight
					height: width

					onClicked: swipeView.incrementCurrentIndex();
				}
			} //Row

			TPLabel {
				id: lblGroups
				text: qsTr("Muscle groups trained in this division:")
				Layout.fillWidth: true
			}

			TPMuscularGroupsList {
				id: cboGroups
				Layout.fillWidth: true
			}

			Row {
				enabled: currentSplitPage && currentSplitPage.exercisesModel && currentSplitPage.exercisesModel.exerciseCount > 1
				height: appSettings.itemDefaultHeight
				Layout.fillWidth: true
				spacing: 0

				TPLabel {
					text: qsTr("Go to exercise: ")
					width: parent.width * 0.4
				}

				TPComboBox {
					id: cboGoToExercise
					width: parent.width * 0.6

					property int current_exercise

					model: ListModel {
						id: cboModel
					}

					onActivated: (cboIndex) => {
						currentSplitPage.positionViewAtIndex(cboIndex, ListView.Contain);
						currentSplitPage.exercisesModel.workingExercise = cboIndex;
					}

					function addExerciseToCombo(exercise_number: int): void {
						cboModel.append({ text: String(exercise_number+1) + ": " + currentSplitPage.exercisesModel.exerciseName(exercise_number, 0),
							value: exercise_number, enabled: exercise_number !== currentSplitPage.exercisesModel.workingExercise});
					}

					function delExerciseFromCombo(exercise_number: int): void {
						cboModel.remove(exercise_number);
					}

					function manageComboItems(exercise_number: int): void {
						if (cboModel.count > 0) {
							cboModel.get(exercise_number).enabled = false;
							if (cboGoToExercise.current_exercise >= 0 && cboGoToExercise.current_exercise !== exercise_number)
								cboModel.get(cboGoToExercise.current_exercise).enabled = true;
							cboGoToExercise.current_exercise = exercise_number;
							cboGoToExercise.currentIndex = exercise_number;
						}
					}

					function populateComboModel(): void {
						for (let i = 0; i < currentSplitPage.exercisesModel.exerciseCount; ++i)
							addExerciseToCombo(i);
						manageComboItems(currentSplitPage.exercisesModel.workingExercise);
					}

					Connections {
						target: currentSplitPage ? currentSplitPage.exercisesModel : null

						function onWorkingExerciseChanged(exercise_number: int): void {
							cboGoToExercise.manageComboItems(exercise_number);
						}

						function onExerciseCountChanged(): void {
							if (currentSplitPage.exercisesModel.exerciseCount > cboModel.count)
								cboGoToExercise.addExerciseToCombo(currentSplitPage.exercisesModel.exerciseCount - 1);
							else if (currentSplitPage.exercisesModel.exerciseCount < cboModel.count)
								cboGoToExercise.delExerciseFromCombo(currentSplitPage.exercisesModel.workingExercise);
						}
					}
				}
			}
		}
	}

	SwipeView {
		id: swipeView
		objectName: "swipeView"
		interactive: true
		anchors.fill: parent

		onCurrentIndexChanged: {
			currentSplitPage = splitManager.setCurrentPage(currentIndex);
			cboModel.clear();
			cboGroups.fillMuscularGroupsModel(currentSplitPage.exercisesModel.muscularGroup);
			cboGoToExercise.populateComboModel();
			if (!navButtons)
				createNavButtons();
		}
	} //SwipeView

	PageIndicator {
		id: indicator
		count: swipeView.count
		currentIndex: swipeView.currentIndex
		height: 20

		delegate: Label {
			text: swipeView.itemAt(index).exercisesModel.splitLetter
			color: appSettings.fontColor
			font.bold: true
			fontSizeMode: Text.Fit
			width: 25
			height: 25
			horizontalAlignment: Text.AlignHCenter
			verticalAlignment: Text.AlignVCenter

			required property int index

			background: Rectangle {
				radius: width/2
				opacity: index === indicator.currentIndex ? 0.95 : pressed ? 0.7 : 0.45
				color: appSettings.paneBackgroundColor
			}

			MouseArea {
				anchors.fill: parent
				onClicked: swipeView.setCurrentIndex(index);
			}

			Behavior on opacity {
				OpacityAnimator {
					duration: 200
				}
			}
		}

		anchors {
			bottom: parent.bottom
			bottomMargin: 10
			horizontalCenter: parent.horizontalCenter
		}
	}

	footer: TPToolBar {
		id: bottomToolBar
		height: footerHeight

		readonly property int buttonWidth: width * 0.22

		TPButton {
			id: btnClearPlan
			text: qsTr("Clear")
			imageSource: "clear.png"
			textUnderIcon: true
			rounded: false
			enabled: splitManager.haveExercises
			width: bottomToolBar.buttonWidth
			height: footerHeight - 4

			anchors {
				left: parent.left
				leftMargin: 5
				verticalCenter: parent.verticalCenter
			}

			onClicked: splitManager.currentSplitModel.clearExercises();
		}

		TPButton {
			id: btnSwapPlan
			text: splitManager.currentSplitLetter + " <-> " + splitManager.currentSwappableLetter
			imageSource: "swap.png"
			textUnderIcon: true
			visible: splitManager.canSwapExercises
			rounded: false
			width: bottomToolBar.buttonWidth
			height: footerHeight - 4

			anchors {
				left: btnClearPlan.right
				leftMargin: 3
				verticalCenter: parent.verticalCenter
			}

			onClicked: splitManager.swapMesoPlans();
		}

		TPButton {
			id: btnImExport
			text: qsTr("In/Ex")
			imageSource: "import-export.png"
			textUnderIcon: true
			rounded: false
			width: bottomToolBar.buttonWidth
			height: footerHeight - 4

			anchors {
				left: btnSwapPlan.right
				leftMargin: 3
				verticalCenter: parent.verticalCenter
			}

			onClicked: showInExMenu();
		}

		TPButton {
			text: qsTr("+Exercise")
			imageSource: "exercises-add.png"
			textUnderIcon: true
			rounded: false
			width: bottomToolBar.buttonWidth*1.3
			height: footerHeight - 4

			anchors {
				right: parent.right
				rightMargin: 5
				verticalCenter: parent.verticalCenter
			}

			onClicked: {
				cboGroups.forceActiveFocus(); //Force triggering of onEditFinished of the last text control to receive input
				currentSplitPage.appendNewExerciseToDivision();
			}
		}
	}

	property PageScrollButtons navButtons: null
	function createNavButtons(): void {
		let component = Qt.createComponent("qrc:/qml/ExercisesAndSets/PageScrollButtons.qml", Qt.Asynchronous);

		function finishCreation() {
			navButtons = component.createObject(pagePlanner, { ownerPage: pagePlanner });
			navButtons.scrollTo.connect(setScrollBarPosition);
			navButtons.visible = Qt.binding(function() { return splitManager.currentSplitModel.exerciseCount > 1; });
			navButtons.showUpButton = Qt.binding(function() { return !currentSplitPage.viewPositionAtBeginning; });
			navButtons.showDownButton = Qt.binding(function() { return !currentSplitPage.viewPositionAtEnd; });
		}

		if (component.status === Component.Ready)
			finishCreation();
		else
			component.statusChanged.connect(finishCreation);
	}

	function setScrollBarPosition(pos): void {
		if (pos === 0)
			currentSplitPage.positionViewAtBeginning();
		else
			currentSplitPage.positionViewAtEnd();
	}

	readonly property bool bExportEnabled: splitManager.haveExercises
	onBExportEnabledChanged: {
		if (imExportMenu) {
			imExportMenu.enableMenuEntry(1, bExportEnabled);
			if (Qt.platform.os === "android")
				imExportMenu.enableMenuEntry(2, bExportEnabled);
		}
	}

	property TPFloatingMenuBar imExportMenu: null
	function showInExMenu(): void {
		if (imExportMenu === null) {
			let imExportMenuComponent = Qt.createComponent("qrc:/qml/TPWidgets/TPFloatingMenuBar.qml");
			imExportMenu = imExportMenuComponent.createObject(pagePlanner, { parentPage: pagePlanner });
			imExportMenu.addEntry(qsTr("Import"), "import.png", 0, true);
			if (splitManager.prevMesoName().length > 0)
				imExportMenu.addEntry(qsTr("Import from ") + splitManager.prevMesoName(), "import.png", 1, true);
			imExportMenu.addEntry(qsTr("Export"), "export.png", 2, true);
			if (Qt.platform.os === "android")
				imExportMenu.addEntry(qsTr("Share"), "export.png", 3, true);
			imExportMenu.menuEntrySelected.connect(selectedMenuOption);
		}
		imExportMenu.show2(btnImExport, 0);
	}

	function selectedMenuOption(menuid): void {
		switch (menuid) {
			case 0: splitManager.importMesoSplit(); break;
			case 1: msgDlgImport.show(-1); break;
			case 2: exportTypeTip.init(false); break;
			case 3: exportTypeTip.init(true); break;
		}
	}

	TPBalloonTip {
		id: msgDlgImport
		title: qsTr("Import Exercises Plan?")
		message: qsTr("Import the exercises plan for training division <b>") + splitManager.currentSplitLetter +
						 qsTr("</b> from <b>") + splitManager.prevMesoName() + "</b>?"
		imageSource: "import"
		parentPage: pagePlanner

		onButton1Clicked: splitManager.loadSplitFromPreviousMeso();
	} //TPBalloonTip

	TPBalloonTip {
		id: exportTypeTip
		title: bShare ? qsTr("What do you want to share?") : qsTr("What to you want to export?")
		imageSource: "export"
		closeButtonVisible: true
		button1Text: qsTr("Entire plan")
		button2Text: qsTr("Just this split")
		parentPage: pagePlanner

		onButton1Clicked: splitManager.exportMesoSplit(bShare, "X");
		onButton2Clicked: splitManager.exportMesoSplit(bShare, currentSplitPage.exercisesModel.splitLetter());

		property bool bShare: false

		function init(share: bool): void {
			bShare = share;
			show(-1);
		}
	}

	function getExerciseNameFieldYPos(): int {
		return currentSplitPage.exerciseNameFieldYPosition();
	}
} //Page
