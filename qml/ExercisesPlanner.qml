import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Page {
	id: pagePlanner
	required property int mesoId
	required property string mesoSplit
	property var currentSplitItem: null
	property var navButtons: null

	ScrollView {
		id: scrollMain
		anchors.fill: parent
		ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
		ScrollBar.vertical.policy: ScrollBar.AsNeeded
		ScrollBar.vertical.active: true
		padding: 2

		ScrollBar.vertical.onPositionChanged: {
			if (navButtons) {
				if (contentItem.contentY <= 50) {
					navButtons.showUpButton = false;
					navButtons.showDownButton = true;
				}
				else if (contentItem.contentY >= height + navBar.height) {
					navButtons.showUpButton = true;
					navButtons.showDownButton = false;
				}
				else {
					navButtons.showUpButton = true;
					navButtons.showDownButton = true;
				}
			}
		}

		ColumnLayout {
			id: layoutMain
			width: scrollMain.availableWidth
		} //ColumnLayout layoutMain
	}//ScrollView scrollMain

	footer: ToolBar {
		id: bottomPane
		width: parent.width
		height: shown ? parent.height * 0.5 : btnShowHideList.height
		visible: height >= btnShowHideList.height
		clip: true
		spacing: 0
		padding: 0
		property bool shown: false

		Behavior on height {
			NumberAnimation {
				easing.type: Easing.InOutQuad
			}
		}

		background: Rectangle {
			opacity: 0.3
			color: paneBackgroundColor
		}

		ColumnLayout {
			width: parent.width
			height: parent.height
			spacing: 0

			ButtonFlat {
				id: btnShowHideList
				imageSource: bottomPane.shown ? "qrc:/images/"+lightIconFolder+"fold-down.png" : "qrc:/images/"+lightIconFolder+"fold-up.png"
				onClicked: bottomPane.shown = !bottomPane.shown;
				Layout.fillWidth: true
				Layout.topMargin: 0
				height: 10
				width: bottomPane.width
			}

			ExercisesListView {
				id: exercisesList
				Layout.fillWidth: true
				Layout.topMargin: 5
				Layout.alignment: Qt.AlignTop
				Layout.rightMargin: 5
				Layout.maximumHeight: parent.height * 0.8
				Layout.leftMargin: 5

				onExerciseEntrySelected:(exerciseName, subName, muscularGroup, sets, reps, weight, mediaPath) => {
					if (currentSplitItem !== null)
						currentSplitItem.changeModel(exerciseName, subName, sets, reps, weight);
				}
			}
		}
	} //footer: ToolBar

	Component.onCompleted: {
		var idx = 0;
		let results = [];
		do {
			switch (mesoSplit.charAt(idx)) {
				case 'A':
					results = Database.getCompleteDivisionAForMeso(mesoId);
				break;
				case 'B':
					results = Database.getCompleteDivisionBForMeso(mesoId);
				break;
				case 'C':
					results = Database.getCompleteDivisionCForMeso(mesoId);
				break;
				case 'D':
					results = Database.getCompleteDivisionDForMeso(mesoId);
				break;
				case 'E':
					results = Database.getCompleteDivisionEForMeso(mesoId);
				break;
				case 'F':
					results = Database.getCompleteDivisionFForMeso(mesoId);
				break;
				default: continue;
			}
			var component = Qt.createComponent("MesoSplitPlanner.qml");
			if (component.status === Component.Ready) {
				var object = component.createObject(layoutMain, { divisionId:results[0].divisionId,
						mesoId:mesoId, splitLetter:results[0].splitLetter, splitText:results[0].splitText,
						splitExercises:results[0].splitExercises, splitSetTypes:results[0].splitSetTypes,
						splitNSets:results[0].splitNSets, splitNReps:results[0].splitNReps,
						splitNWeight:results[0].splitNWeight
				});
				object.selectedSplitObjectChanged.connect(selectSplitItem);
				object.scrollPage.connect(scrollDown);
				if (idx === 0)
					selectSplitItem(object, "");
			}
		} while (++idx < mesoSplit.length);
		if (idx > 0)
			createNavButtons();
	}

	function selectSplitItem(splitObjectItem, strFilter) {
		if (currentSplitItem !== null)
			currentSplitItem.bCurrentItem = false;
		currentSplitItem = splitObjectItem;
		currentSplitItem.bCurrentItem = true;
		exercisesList.setFilter(strFilter);
	}

	function createNavButtons() {
		if (navButtons === null) {
			var component = Qt.createComponent("PageScrollButtons.qml");
			navButtons = component.createObject(this, {});
			navButtons.scrollTo.connect(setScrollBarPosition);
			navButtons.backButtonWasPressed.connect(maybeShowNavButtons);
		}
		navButtons.visible = true;
	}

	function maybeShowNavButtons() {
		navButtons.showButtons();
	}

	function setScrollBarPosition(pos) {
		if (pos === 0)
			scrollMain.ScrollBar.vertical.setPosition(0);
		else
			scrollMain.ScrollBar.vertical.setPosition(pos - scrollMain.ScrollBar.vertical.size/2);
	}

	function scrollDown(npixels) {
		scrollMain.contentItem.contentY += npixels/2;
	}
} //Page
