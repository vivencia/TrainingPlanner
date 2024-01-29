import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Page {
	id: pagePlanner
	required property int mesoId
	required property string mesoSplit

	contentItem {
		Keys.onPressed: (event) => {
			switch (event.key) {
				case Qt.Key_Back:
					if (bottomPane.shown) {
						event.accepted = true;
						bottomPane.shown = false;
					}
				break;
			}
		}
	}

	SwipeView {
		id: splitView
		currentIndex: 0
		anchors.fill: parent
		interactive: !bottomPane.shown
	} //SwipeView

	PageIndicator {
		count: splitView.count
		currentIndex: splitView.currentIndex
		anchors.bottom: parent.bottom
		anchors.horizontalCenter: parent.horizontalCenter
		visible: !bottomPane.shown
	}

	footer: ToolBar {
		id: bottomPane
		width: parent.width
		height: shown ? parent.height * 0.5 : btnShowHideList.height
		visible: height >= btnShowHideList.height
		spacing: 0
		padding: 0
		property bool shown: false

		Behavior on height {
			NumberAnimation {
				easing.type: Easing.InOutBack
			}
		}

		background: Rectangle {
			opacity: 0.3
			color: paneBackgroundColor
		}

		onShownChanged: {
			if (shown)
				exercisesList.setFilter(splitView.currentItem.filterString);
		}

		ColumnLayout {
			width: parent.width
			height: parent.height
			spacing: 0

			ButtonFlat {
				id: btnShowHideList
				imageSource: bottomPane.shown ? "qrc:/images/"+darkIconFolder+"fold-down.png" : "qrc:/images/"+darkIconFolder+"fold-up.png"
				imageSize: 60
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
				canDoMultipleSelection: true

				onExerciseEntrySelected:(exerciseName, subName, muscularGroup, sets, reps, weight, mediaPath, multipleSelection_option) => {
					splitView.currentItem.changeModel(exerciseName, subName, sets, reps, weight, multipleSelection_option);
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
				var object = component.createObject(splitView, { divisionId:results[0].divisionId,
						mesoId:mesoId, splitLetter:results[0].splitLetter, splitText:results[0].splitText,
						splitExercises:results[0].splitExercises, splitSetTypes:results[0].splitSetTypes,
						splitNSets:results[0].splitNSets, splitNReps:results[0].splitNReps,
						splitNWeight:results[0].splitNWeight
				});
				splitView.addItem(object);
			}
		} while (++idx < mesoSplit.length);
	}

	function closeSimpleExerciseList() {
		bottomPane.shown = false;
	}
} //Page
