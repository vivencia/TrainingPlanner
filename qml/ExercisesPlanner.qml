import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Page {
	id: pagePlanner
	required property int mesoId
	required property int mesoIdx
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

				Component.onCompleted: setModel(exercisesListModel);
			}
		}
	} //footer: ToolBar

	Component.onCompleted: {
		var idx = 0;
		var splitLetter;
		do {
			splitLetter = mesoSplit.charAt(idx);
			if (splitLetter === 'R') continue;

			var component = Qt.createComponent("MesoSplitPlanner.qml", Qt.Asynchronous);

			function finishCreation(Component) {
				var splitletter = Component.objectName;
				console.log("finishCreation   ", splitletter)
				var object = Component.createObject(splitView, { mesoId:mesoId, mesoIdx:mesoIdx, splitLetter:splitletter });
				splitView.addItem(object);
			}

			function checkStatusA() {
				console.log("checkStatusA: component", component.objectName);
				finishCreation(component);
			}
			function checkStatusB() {
				console.log("checkStatusB: component", component.objectName);
				finishCreation(component);
			}
			function checkStatusC() {
				console.log("checkStatusC: component", component.objectName);
				finishCreation(component);
			}
			function checkStatusD() {
				console.log("checkStatusD: component", component.objectName);
				finishCreation(component);
			}
			function checkStatusE() {
				console.log("checkStatusE: component", component.objectName);
				finishCreation(component);
			}
			function checkStatusF() {
				console.log("checkStatusF: component", component.objectName);
				finishCreation(component);
			}

			appDB.pass_object(mesoSplitModel);
			appDB.getCompleteMesoSplit(mesoId, splitLetter);
			component.objectName = splitLetter;
			if (component.status === Component.Ready)
				finishCreation(component);
			else {
				switch (splitLetter) {
					case 'A': component.statusChanged.connect(checkStatusA); break;
					case 'B': component.statusChanged.connect(checkStatusB); break;
					case 'C': component.statusChanged.connect(checkStatusC); break;
					case 'D': component.statusChanged.connect(checkStatusD); break;
					case 'E': component.statusChanged.connect(checkStatusE); break;
					case 'F': component.statusChanged.connect(checkStatusF); break;
				}
			}
		} while (++idx < mesoSplit.length);
	}

	function closeSimpleExerciseList() {
		bottomPane.shown = false;
	}
} //Page
