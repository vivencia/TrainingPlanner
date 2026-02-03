import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import "../../TPWidgets"
import "../../Dialogs"
import "../.."

Pane {
	id: trainingSplitPane
	implicitHeight: mainLayout.implicitHeight + 100
	spacing: 0
	padding: 0
	Layout.topMargin: 10
	Layout.leftMargin: 0

	property alias mesoSplitText: txtMesoSplit.text
	readonly property int col1Width: width * 0.1
	readonly property int col2Width: width * 0.15
	readonly property int col3Width: appSettings.itemDefaultHeight
	readonly property int col4Width: appSettings.pageWidth * 0.6
	readonly property list<string> daysOfWeek: [qsTr("Mon"), qsTr("Tue"), qsTr("Wed"), qsTr("Thu"), qsTr("Fri"), qsTr("Sat"), qsTr("Sun")]
	property bool bMesoSplitChanged: false

	background: Rectangle {
		color: "transparent"
	}

	ColumnLayout {
		id: mainLayout

		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
		}

		TPLabel {
			id: lblMesoSplit
			text: mesoModel.splitLabel
			Layout.fillWidth: true
		}

		TPTextInput {
			id: txtMesoSplit
			text: mesoManager.split
			ToolTip.text: qsTr("On any training program, there should be at least one rest day(R) per week and one training day(A-F)")
			ToolTip.visible: !mesoManager.splitOK
			ToolTip.timeout: 5000
			readOnly: true
			width: parent.width * 0.4
			Layout.minimumWidth: width
			Layout.maximumWidth: width

			TPImage {
				source: "set-completed"
				enabled: mesoManager.splitOK
				width: appSettings.itemDefaultHeight
				height: width

				anchors {
					left: parent.right
					verticalCenter: parent.verticalCenter
				}
			}
		}

		Repeater {
			id: splitRepeater
			model: 7

			property int highest_letter_idx: 0;

			delegate: RowLayout {
				id: delegateRow
				Layout.fillWidth: true
				Layout.topMargin: 10
				//objectName: "delegateRow"

				required property int index
				readonly property int delegateIndex: index

				TPLabel {
					//objectName: "label"
					text: daysOfWeek[delegateIndex]
					Layout.preferredWidth: col1Width
					Layout.alignment: Qt.AlignCenter
				}

				TPComboBox {
					id: cboSplit
					Layout.preferredWidth: col2Width
					Layout.alignment: Qt.AlignCenter

					readonly property int nDelegateRows: 7
					readonly property int nLastDelegateIdx: 6

					model: ListModel {
						ListElement { text: "A"; value: "A"; enabled: true; }
						ListElement { text: "B"; value: "B"; enabled: true; }
						ListElement { text: "C"; value: "C"; enabled: true; }
						ListElement { text: "D"; value: "D"; enabled: true; }
						ListElement { text: "E"; value: "E"; enabled: true; }
						ListElement { text: "F"; value: "F"; enabled: true; }
						ListElement { text: "R"; value: "R"; enabled: true; }
					}

					// Don't allow a day to skip a letter. Letters must be added sequentially or be repeated, never skipped
					onActivated: (cboindex) => {
						let mesoSplit = txtMesoSplit.text;
						mesoManager.split = mesoSplit.substring(0,delegateRow.delegateIndex) +
													valueAt(cboindex) + mesoSplit.substring(delegateRow.delegateIndex+1);
						splitRepeater.itemAt(delegateRow.index).children[3].fillMuscularGroupsModel(mesoManager.muscularGroup(currentValue));

						let last_letter_idx = cboindex + 1;
						if (last_letter_idx === nDelegateRows) {
							last_letter_idx = 0;
							if (delegateRow.delegateIndex >= 1) {
								let prev_index = delegateRow.delegateIndex-1;
								let prev_item_index;
								do {
									prev_item_index = splitRepeater.itemAt(prev_index).children[1].currentIndex;
									if (prev_item_index !== nLastDelegateIdx) {
										last_letter_idx = prev_item_index + 1;
										break;
									}
								} while (--prev_index >= 0);
							}
						}

						for (let i = delegateRow.delegateIndex + 1; i < nDelegateRows; ++i) {
							const cboBox = splitRepeater.itemAt(i).children[1];
							const curIdx = cboBox.currentIndex;
							if (curIdx > last_letter_idx && curIdx !== 6)
								cboBox.currentIndex = last_letter_idx;
							for (let x = 0; x < nLastDelegateIdx; ++x)
								cboBox.model.get(x).enabled = x <= last_letter_idx;
						}
					}

					Component.onCompleted: {
						currentIndex = indexOfValue(txtMesoSplit.text.charAt(delegateRow.delegateIndex));
						splitRepeater.itemAt(delegateRow.index).children[3].fillMuscularGroupsModel(mesoManager.muscularGroup(currentValue));

						let last_letter_idx = indexOfValue(currentValue);
						if (last_letter_idx === nLastDelegateIdx) { //split is an 'R'
							let prev_index = delegateRow.delegateIndex-1;
							last_letter_idx = 0;
							if (prev_index >= 0) {
								let prev_item_index;
								do {
									prev_item_index = splitRepeater.itemAt(prev_index).children[1].currentIndex;
									if (prev_item_index !== nLastDelegateIdx) {
										last_letter_idx = prev_item_index + 1;
										break;
									}
								} while (--prev_index >= 0);
							}
						}

						for (let x = 0; x < nLastDelegateIdx; ++x)
							model.get(x).enabled = x <= last_letter_idx;
					}
				} //TPComboBox

				TPButton {
					id: btnMuscularGroups
					imageSource: "choose.png"
					enabled: cboSplit.currentIndex !== 6
					width: col3Width
					height: col3Width

					onClicked: showMGDialog(this, cboSplit.currentValue);
				}

				TPMuscularGroupsList {
					id: cboMuscularGroup
					Layout.minimumWidth: col4Width
					Layout.maximumWidth: col4Width
				} //cboMuscularGroup
			} //RowLayout
		} //Repeater
	} //GridLayout

	TPButton {
		text: qsTr("Exercises Planner")
		enabled: mesoManager.splitOK
		autoSize: true

		anchors {
			bottom: parent.bottom
			horizontalCenter: parent.horizontalCenter
		}

		onClicked: mesoManager.getExercisesPlannerPage();
	}

	function forcusOnFirstItem(): void {
		splitRepeater.itemAt(0).children[1].forceActiveFocus();
	}

	function setMuscularGroup(split: string, groups: string): void {
			mesoManager.setMuscularGroup(split, groups);
			let index = -1;
			switch (split) {
				case 'A' : index = 0; break;
				case 'B' : index = 1; break;
				case 'C' : index = 2; break;
				case 'D' : index = 3; break;
				case 'E' : index = 4; break;
				case 'F' : index = 5; break;
			}
			splitRepeater.itemAt(index).children[3].fillMuscularGroupsModel(groups);
		}

	property MuscularGroupPicker filterDlg: null
	function showMGDialog(button: TPButton, split: string): void {
		if (filterDlg === null) {
			let component = Qt.createComponent("qrc:/qml/Dialogs/MuscularGroupPicker.qml", Qt.Asynchronous);

			function finishCreation() {
				filterDlg = component.createObject(mainwindow, {
										parentPage: mesoPropertiesPage, buttonLabel: qsTr("Define") });
				filterDlg.muscularGroupsCreated.connect(function(groups) { setMuscularGroup(split, groups) });
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}
		filterDlg.show(mesoManager.muscularGroup(split), button, 3);
	}
} //Pane
