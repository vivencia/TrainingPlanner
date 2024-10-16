import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../TPWidgets"
import ".."

Pane {
	id: trainingSplitPane
	implicitWidth: appSettings.pageWidth - 20
	implicitHeight: mainLayout.implicitHeight + 100

	property alias mesoSplitText: txtMesoSplit.text

	readonly property int col1Width: width*0.15
	readonly property int col2Width: width*0.15
	readonly property int col3Width: width*0.6

	background: Rectangle {
		color: "transparent"
	}

	TPLabel {
		id: lblMesoSplit
		text: mesoManager.splitLabel
		widthAvailable: parent.width*0.6

		anchors {
			top: parent.top
			left: parent.left
		}
	}

	TPTextInput {
		id: txtMesoSplit
		text: mesoManager.split
		ToolTip.text: qsTr("On any training program, there should be at least one rest day(R) per week")
		readOnly: true
		width: parent.width*0.4

		anchors {
			top: parent.top
			right: parent.right
			rightMargin: 5
		}
	}

	ColumnLayout {
		id: mainLayout

		anchors {
			top: txtMesoSplit.bottom
			topMargin: 15
			left: parent.left
			right: parent.right
		}

		Repeater {
			id: splitRepeater
			model: 7

			RowLayout {
				Layout.fillWidth: true

				TPLabel {
					text: qsTr("Day ") + parseInt(index) + ":"
					width: col1Width
					Layout.minimumWidth: col1Width
					Layout.maximumWidth: col1Width
				}

				TPComboBox {
					id: cboSplit
					model: AppGlobals.splitModel
					implicitWidth: col2Width

					onActivated: (cboindex) => {
						txtSplit.text = mesocyclesModel.muscularGroup(itemManager.mesoIdx, currentValue);
						var mesoSplit = txtMesoSplit.text;
						txtMesoSplit.text = mesoSplit.substring(0,index) + valueAt(cboindex) + mesoSplit.substring(index+1);
						txtSplit.forceActiveFocus();
					}

					Component.onCompleted: {
						currentIndex = indexOfValue(mesocyclesModel.splitLetter(itemManager.mesoIdx, index));
						txtSplit.text = mesocyclesModel.muscularGroup(itemManager.mesoIdx, valueAt(index));
					}
				}

				TPTextInput {
					id: txtSplit
					implicitWidth: col3Width

					onEditingFinished: mesocyclesModel.setMuscularGroup(itemManager.mesoIdx, cboSplit.currentText, text, muscularGroupId);

					onEnterOrReturnKeyPressed: {
						if (index < 6)
							splitRepeater.itemAt(index+1).children[1].forceActiveFocus();
					}
				}
			} //RowLayout
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
			const ok = mesocyclesModel.setSplit(itemManager.mesoIdx, txtMesoSplit.text);
			btnCreateExercisePlan.enabled = ok;
			txtMesoSplit.ToolTip.visible = !ok;
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

		onClicked: itemManager.getExercisesPlannerPage();
	}

	function forcusOnFirstItem() {
		splitRepeater.itemAt(0).children[1].forceActiveFocus();
	}

	function updateMuscularGroup(splitindex: int, splitletter: string) {
		const musculargroup = mesocyclesModel.muscularGroup(itemManager.mesoIdx, splitletter);
		for (var i = 0; i < 7; ++i) {
			if (splitRepeater.itemAt(i).children[1].currentIndex === splitindex)
				itemAt(splitindex).children[2].text = musculargroup;
		}

	}
} //Pane
