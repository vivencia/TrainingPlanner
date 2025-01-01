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
	readonly property int col3Width: appSettings.pageWidth*0.60
	readonly property list<string> daysOfWeek: [qsTr("Mon"), qsTr("Tue"), qsTr("Wed"), qsTr("Thu"), qsTr("Fri"), qsTr("Sat"), qsTr("Sun")]

	property bool bMesoSplitTextOK: true
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

			property int highest_letter_idx: 0;

			RowLayout {
				Layout.fillWidth: true

				TPLabel {
					text: daysOfWeek[index]
					width: col1Width
					Layout.minimumWidth: col1Width
					Layout.maximumWidth: col1Width
				}

				TPComboBox {
					// Don't allow a day to skip a letter. Letters must be added sequentially or be repeated, never skipped
					id: cboSplit
					implicitWidth: col2Width

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
						var mesoSplit = txtMesoSplit.text;
						txtMesoSplit.text = mesoSplit.substring(0,index) + valueAt(cboindex) + mesoSplit.substring(index+1);
						bMesoSplitChanged = true;
						if (cboindex === 6)
							return;

						let last_letter_idx;
						switch(valueAt(cboindex)) {
							case "A": last_letter_idx = 0; break;
							case "B": last_letter_idx = 1; break;
							case "C": last_letter_idx = 2; break;
							case "D": last_letter_idx = 3; break;
							case "E": last_letter_idx = 4; break;
							case "F": last_letter_idx = 5; break;
						}
						++last_letter_idx;
						if (splitRepeater.highest_letter_idx < last_letter_idx)
							splitRepeater.highest_letter_idx = last_letter_idx;

						for (let i = index + 1; i < 7; ++i) {
							let cboModel = splitRepeater.itemAt(i).children[1].model;
							for (let x = 0; x < 6; ++x)
								cboModel.get(x).enabled = x <= splitRepeater.highest_letter_idx;
						}
					}

					Component.onCompleted: {
						currentIndex = Qt.binding(function() { return indexOfValue(txtMesoSplit.text.charAt(index)); });
						btnMuscularGroups.visible = Qt.binding(function() { return currentIndex !== 6; });
						if (index >=1)
						{
							let last_letter_idx;
							switch(splitRepeater.itemAt(index-1).children[1].currentValue) {
								case "A": last_letter_idx = 0; break;
								case "B": last_letter_idx = 1; break;
								case "C": last_letter_idx = 2; break;
								case "D": last_letter_idx = 3; break;
								case "E": last_letter_idx = 4; break;
								case "F": last_letter_idx = 5; break;
								case "R": last_letter_idx = -1; break;
							}
							++last_letter_idx;
							if (splitRepeater.highest_letter_idx < last_letter_idx)
								splitRepeater.highest_letter_idx = last_letter_idx;

							for (let x = 0; x < 6; ++x)
								model.get(x).enabled = x <= splitRepeater.highest_letter_idx;
						}
						else {
							for (let y = 1; y < 6; ++y)
								model.get(y).enabled = false;
						}
					}
				}

				ScrollView {
					Layout.minimumWidth: col3Width
					Layout.maximumWidth: col3Width
					Layout.preferredHeight: 35

					ScrollBar.horizontal: ScrollBar {
						policy: ScrollBar.AsNeeded
						active: true
						visible: lblSplit._textWidth >= col3Width
						interactive: true
					}

					background: Rectangle {
						border.color: activeFocus ? appSettings.entrySelectedColor : appSettings.fontColor
						color: "transparent"
						radius: 6
					}

					TPLabel {
						id: lblSplit
						widthAvailable: _textWidth
						width: _textWidth

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
				}

				TPButton {
					id: btnMuscularGroups
					imageSource: "choose.png"
					imageSize: 25
					Layout.preferredWidth: 25
					Layout.preferredHeight: 25

					onClicked: showMGDialog(this, index, lblSplit);
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
		enabled: bMesoSplitChanged

		anchors {
			top: mainLayout.bottom
			topMargin: 5
			horizontalCenter: parent.horizontalCenter
		}

		onCheck: {
			bMesoSplitChanged = !checked;
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
