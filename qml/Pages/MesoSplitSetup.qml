import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../TPWidgets"
import "../Dialogs"
import ".."

Pane {
	id: trainingSplitPane
	implicitHeight: mainLayout.implicitHeight + 100
	spacing: 0
	padding: 0
	Layout.topMargin: 10
	Layout.leftMargin: 0

	property alias mesoSplitText: txtMesoSplit.text
	readonly property int col1Width: width*0.10
	readonly property int col2Width: width*0.15
	readonly property int col3Width: appSettings.pageWidth*0.65
	readonly property list<string> daysOfWeek: [qsTr("Mon"), qsTr("Tue"), qsTr("Wed"), qsTr("Thu"), qsTr("Fri"), qsTr("Sat"), qsTr("Sun")]

	readonly property bool bMesoSplitTextOK: mesoSplitText.indexOf('R') !== -1
	property bool bMesoSplitChanged: false

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
		ToolTip.visible: !bMesoSplitTextOK
		readOnly: true
		width: parent.width*0.3

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

			property int highest_letter_idx: 0;

			RowLayout {
				Layout.fillWidth: true

				TPLabel {
					text: daysOfWeek[index]
					width: col1Width
					Layout.minimumWidth: col1Width
					Layout.maximumWidth: col1Width
				}

				Item {
					height: 50
					Layout.minimumHeight: 50
					Layout.maximumHeight: 50
					Layout.minimumWidth: col2Width
					Layout.maximumWidth: col2Width

					TPComboBox {
						// Don't allow a day to skip a letter. Letters must be added sequentially or be repeated, never skipped
						id: cboSplit

						anchors {
							top: parent.top
							left: parent.left
							right: parent.right
						}

						model: ListModel {
							ListElement { text: "A"; value: "A"; enabled: true; }
							ListElement { text: "B"; value: "B"; enabled: true; }
							ListElement { text: "C"; value: "C"; enabled: true; }
							ListElement { text: "D"; value: "D"; enabled: true; }
							ListElement { text: "E"; value: "E"; enabled: true; }
							ListElement { text: "F"; value: "F"; enabled: true; }
							ListElement { text: "R"; value: "R"; enabled: true; }
						}

						onActivated: (cboindex) => {
							let mesoSplit = txtMesoSplit.text;
							txtMesoSplit.text = mesoSplit.substring(0,index) + valueAt(cboindex) + mesoSplit.substring(index+1);
							bMesoSplitChanged = true;
							let last_letter_idx = cboindex + 1;
							if (last_letter_idx === 7) {
								last_letter_idx = 0;
								if (index >= 1) {
									let prev_index = index-1;
									let prev_item_index;
									do {
										prev_item_index = splitRepeater.itemAt(prev_index).children[1].currentIndex;
										if (prev_item_index !== 6) {
											last_letter_idx = prev_item_index + 1;
											break;
										}
									} while (--prev_index >= 0);
								}
							}

							for (let i = index + 1; i < 7; ++i) {
								const cboBox = splitRepeater.itemAt(i).children[1];
								const curIdx = cboBox.currentIndex;
								if (curIdx > last_letter_idx && curIdx !== 6)
									cboBox.currentIndex = last_letter_idx;
								let cboModel = cboBox.model;
								for (let x = 0; x < 6; ++x)
									cboModel.get(x).enabled = x <= last_letter_idx;
							}
						}

						Component.onCompleted: {
							currentIndex = Qt.binding(function() { return indexOfValue(txtMesoSplit.text.charAt(index)); });
							btnMuscularGroups.visible = Qt.binding(function() { return currentIndex !== 6; });
							if (index >=1)
							{
								let last_letter_idx = indexOfValue(currentValue);
								if (last_letter_idx === 6) {
									let prev_index = index-1;
									let prev_item_index;
									do {
										prev_item_index = splitRepeater.itemAt(prev_index).children[1].currentIndex;
										if (prev_item_index !== 6) {
											last_letter_idx = prev_item_index + 1;
											break;
										}
									} while (--prev_index >= 0);
								}

								for (let x = 0; x < 6; ++x)
									model.get(x).enabled = x <= last_letter_idx;
							}
							else {
								for (let y = 1; y < 6; ++y)
									model.get(y).enabled = false;
							}
						}
					}

					TPButton {
						id: btnMuscularGroups
						imageSource: "choose.png"
						imageSize: 25

						anchors {
							bottom: parent.bottom
							left: parent.left
							right: parent.right
						}

						onClicked: showMGDialog(this, index, txtSplit);
					}
				} //Item

				TPTextInput {
					id: txtSplit
					readOnly: true
					suggestedHeight: 50
					Layout.minimumWidth: col3Width
					Layout.maximumWidth: col3Width
					Layout.minimumHeight: 50
					Layout.maximumHeight: 80

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

						textChanged.connect(function() {
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
		hasDropShadow: false
		imageSize: 20
		flat: false
		checkable: true
		enabled: bMesoSplitChanged && bMesoSplitTextOK
		anchors {
			top: mainLayout.bottom
			topMargin: 5
			horizontalCenter: parent.horizontalCenter
		}

		onCheck: {
			bMesoSplitChanged = !checked;
			if (checked)
				mesoManager.split = txtMesoSplit.text;
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

	function forcusOnFirstItem(): void {
		splitRepeater.itemAt(0).children[1].forceActiveFocus();
	}

	property MuscularGroupPicker filterDlg: null
	property TPLabel curLabel: null
	function showMGDialog(button: TPButton, letter_index: int, label: TPLabel): void {
		if (filterDlg === null) {
			let component = Qt.createComponent("qrc:/qml/Dialogs/MuscularGroupPicker.qml", Qt.Asynchronous);

			function finishCreation() {
				filterDlg = component.createObject(mainwindow, { parentPage: mesoPropertiesPage, buttonLabel: qsTr("Define"), useFancyNames: true });
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}

		function setLabelText(groups) {
			curLabel.text = groups;
			bMesoSplitChanged = true;
		}

		filterDlg.muscularGroupCreated.disconnect(setLabelText);
		curLabel = label;

		filterDlg.initialGroups = label.text;
		filterDlg.muscularGroupCreated.connect(setLabelText);
		filterDlg.show(button, 3);
	}
} //Pane
