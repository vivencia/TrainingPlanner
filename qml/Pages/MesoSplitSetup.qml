import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../TPWidgets"
import ".."

Pane {
	id: trainingSplitPane
	implicitHeight: mainLayout.implicitHeight + 100
	spacing: 0
	padding: 0
	Layout.topMargin: 10
	Layout.leftMargin: 0

	property alias mesoSplitText: txtMesoSplit.text
	readonly property int col1Width: width*0.15
	readonly property int col2Width: width*0.15
	readonly property int col3Width: width*0.6
	property bool bMesoSplitTextOK: true

	background: Rectangle {
		color: "transparent"
	}

	TPLabel {
		id: lblMesoSplit
		text: mesoManager.splitLabel
		widthAvailable: parent.width*0.7

		anchors {
			top: parent.top
			left: parent.left
		}
	}

	TPTextInput {
		id: txtMesoSplit
		text: mesoManager.split
		ToolTip.text: qsTr("On any training program, there should be at least one rest day(R) per week")
		ToolTip.visible: !bMesoSplitTextOK
		readOnly: true
		width: parent.width*0.3

		onTextChanged: bMesoSplitTextOK = text.indexOf('R') !== -1

		anchors {
			top: parent.top
			right: parent.right
			rightMargin: 20
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
					text: qsTr("Day ") + parseInt(index+1) + ":"
					width: col1Width
					Layout.minimumWidth: col1Width
					Layout.maximumWidth: col1Width
				}

				TPComboBox {
					id: cboSplit
					model: AppGlobals.splitModel
					implicitWidth: col2Width

					onActivated: (cboindex) => {
						var mesoSplit = txtMesoSplit.text;
						txtMesoSplit.text = mesoSplit.substring(0,index) + valueAt(cboindex) + mesoSplit.substring(index+1);
						txtSplit.readOnly = cboindex === 6;
						if (cboindex !== 6)
							txtSplit.forceActiveFocus();
					}

					Component.onCompleted: currentIndex = Qt.binding(function() { return indexOfValue(txtMesoSplit.text.charAt(index)); });
				}

				TPTextInput {
					id: txtSplit
					implicitWidth: col3Width

					onEnterOrReturnKeyPressed: {
						if (index < 6)
							splitRepeater.itemAt(index+1).children[1].forceActiveFocus();
					}

					Component.onCompleted: {
						text = Qt.binding(function() {
							switch (cboSplit.currentIndex) {
								case 0: return mesoManager.muscularGroupA;
								case 1: return mesoManager.muscularGroupB;
								case 2: return mesoManager.muscularGroupC;
								case 3: return mesoManager.muscularGroupD;
								case 4: return mesoManager.muscularGroupD;
								case 5: return mesoManager.muscularGroupF;
								case 6: return mesoManager.muscularGroupR;
							}
						});

						editingFinished.connect(function() {
							switch (cboSplit.currentIndex) {
								case 0: mesoManager.muscularGroupA = text; break;
								case 1: mesoManager.muscularGroupB = text; break;
								case 2: mesoManager.muscularGroupC = text; break;
								case 3: mesoManager.muscularGroupD = text; break;
								case 4: mesoManager.muscularGroupD = text; break;
								case 5: mesoManager.muscularGroupF = text; break;
							}
						});
					}
				}
			} //RowLayout
		} //Repeater
	} //GridLayout

	TPButton {
		id: btnAcceptSplit
		text: qsTr("Accept changes")
		imageSource: "set-completed"
		imageSize: 15
		flat: false
		checkable: true
		enabled: bMesoSplitTextOK

		anchors {
			top: mainLayout.bottom
			topMargin: 5
			horizontalCenter: parent.horizontalCenter
		}

		onCheck: {
			mainLayout.enabled = !checked;
			if (checked) {
				lblNewMesoRequiredFieldsCounter.decreaseCounter();
				mesoManager.split = txtMesoSplit.text;
			}
			else
				lblNewMesoRequiredFieldsCounter.increaseCounter();
		}
	}

	TPButton {
		id: btnCreateExercisePlan
		text: qsTr("Exercises Planner")
		enabled: bMesoSplitTextOK
		flat: false

		anchors {
			top: btnAcceptSplit.bottom
			topMargin: 5
			horizontalCenter: parent.horizontalCenter
		}

		onClicked: mesoManager.getExercisesPlannerPage();
	}

	function forcusOnFirstItem() {
		splitRepeater.itemAt(0).children[1].forceActiveFocus();
	}
} //Pane
