import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../TPWidgets"
import ".."

Pane {
	id: trainingSplitPane
	implicitWidth: windowWidth
	implicitHeight: mainLayout.implicitHeight + 100

	readonly property var splitModel: [ { value:'A', text:'A' }, { value:'B', text:'B' }, { value:'C', text:'C' },
							{ value:'D', text:'D' }, { value:'E', text:'E' }, { value:'F', text:'F' }, { value:'R', text:'R' } ]
	readonly property int col1Width: width*0.15
	readonly property int col2Width: width*0.15
	readonly property int col3Width: width*0.6

	background: Rectangle {
		color: "transparent"
	}

	Label {
		id: lblMesoSplit
		text: mesocyclesModel.columnLabel(6)
		font.bold: true
		color: AppSettings.fontColor
		width: parent.width*0.6

		anchors {
			top: parent.top
			left: parent.left
		}
	}

	TPTextInput {
		id: txtMesoSplit
		text: mesocyclesModel.get(mesoIdx, 6)
		ToolTip.text: qsTr("On any training program, there should be at least one rest day(R) per week")
		readOnly: true
		width: parent.width*0.3

		anchors {
			top: parent.top
			left: lblMesoSplit.right
			rightMargin: 5
		}
	}

	ColumnLayout {
		id: mainLayout

		anchors {
			top: lblMesoSplit.bottom
			topMargin: 15
			left: parent.left
			right: parent.right
		}

		Repeater {
			id: splitRepeater
			model: 7

			RowLayout {
				Layout.fillWidth: true

				Label {
					text: qsTr("Day ") + parseInt(index) + ":"
					font.bold: true
					color: AppSettings.fontColor
					width: col1Width
					Layout.minimumWidth: col1Width
					Layout.maximumWidth: col1Width
				}

				TPComboBox {
					id: cboSplit
					model: splitModel
					implicitWidth: col2Width

					onActivated: (cboindex) => {
						txtSplit.text = mesocyclesModel.getMuscularGroup(mesoIdx, currentValue);
						var mesoSplit = txtMesoSplit.text;
						txtMesoSplit.text = mesoSplit.substring(0,index) + valueAt(cboindex) + mesoSplit.substring(index+1);
						txtSplit.forceActiveFocus();
					}

					Component.onCompleted: {
						currentIndex = indexOfValue(mesocyclesModel.getSplitLetter(mesoIdx, index));
						txtSplit.text = mesocyclesModel.getMuscularGroup(mesoIdx, valueAt(index));
					}
				}

				TPTextInput {
					id: txtSplit
					implicitWidth: col3Width

					onEditingFinished: mesocyclesModel.setMuscularGroup(mesoIdx, cboSplit.currentText, text);

					onEnterOrReturnKeyPressed: {
						if (index < 6)
							itemAt(index+1).cboSplit.forceActiveFocus();
					}
				}
			} //RowLayout

			Component.onCompleted: mesocyclesModel.muscularGroupChanged.connect(updateMuscularGroup);

			function updateMuscularGroup(splitindex: int, splitletter: string) {
				const musculargroup = mesocyclesModel.getMuscularGroup(mesoIdx, splitletter);
				for (var i = 0; i < 7; ++i) {
					if (itemAt(i).cboSplit.currentIndex === splitindex)
						itemAt(splitindex).txtSplit.text = musculargroup;
				}
			}

		} //Repeater
	} //GridLayout

	TPButton {
		id: btnAcceptSplit
		text: qsTr("Accept changes")
		flat: false

		anchors {
			top: mainLayout.bottom
			topMargin: 5
			horizontalCenter: parent.horizontalCenter
		}

		onClicked: {
			const ok = txtMesoSplit.text.indexOf("R") !== -1;
			btnCreateExercisePlan.enabled = ok;
			txtMesoSplit.ToolTip.visible = !ok;
			if (ok)
				mesocyclesModel.setMesoSplit(mesoIdx, txtMesoSplit.text);
		}
	}

	TPButton {
		id: btnCreateExercisePlan
		text: qsTr("Exercises Planner")
		flat: false

		anchors {
			top: btnAcceptSplit.bottom
			topMargin: 5
			horizontalCenter: parent.horizontalCenter
		}

		onClicked: appDB.getExercisesPlannerPage(mesoIdx);
	}

	function forcusOnFirstItem() {
		splitRepeater.itemAt(0).cboSplit.forceActiveFocus();
	}
} //Pane
