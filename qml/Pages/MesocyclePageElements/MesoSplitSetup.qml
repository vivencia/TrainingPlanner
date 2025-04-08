import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

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

			delegate: RowLayout {
				id: delegateRow
				Layout.fillWidth: true
				//objectName: "delegateRow"

				required property int index
				readonly property int delegateIndex: index

				/*Component.onCompleted: {
					for (let i = 0; i < children.length; ++i)
					{
						console.log(i, children[i].objectName);
						for (let x = 0; x < children[i].children.length; ++x)
							console.log(i,x, children[i].children[x].objectName);
					}
				}*/

				TPLabel {
					//objectName: "label"
					text: daysOfWeek[delegateIndex]
					width: col1Width
					Layout.minimumWidth: col1Width
					Layout.maximumWidth: col1Width
				}

				Item {
					//objectName: "item"
					height: 50
					Layout.minimumHeight: 50
					Layout.maximumHeight: 50
					Layout.minimumWidth: col2Width
					Layout.maximumWidth: col2Width

					TPComboBox {
						//objectName: "combo"
						// Don't allow a day to skip a letter. Letters must be added sequentially or be repeated, never skipped
						id: cboSplit
						editable: !mesoManager.ownMeso

						readonly property int nDelegateRows: 7
						readonly property int nLastDelegateIdx: 6

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
							txtMesoSplit.text = mesoSplit.substring(0,delegateRow.delegateIndex) +
													valueAt(cboindex) + mesoSplit.substring(delegateRow.delegateIndex+1);
							bMesoSplitChanged = true;
							let last_letter_idx = cboindex + 1;
							if (last_letter_idx === nDelegateRows) {
								last_letter_idx = 0;
								if (delegateRow.delegateIndex >= 1) {
									let prev_index = delegateRow.delegateIndex-1;
									let prev_item_index;
									do {
										prev_item_index = splitRepeater.itemAt(prev_index).children[1].children[0].currentIndex;
										if (prev_item_index !== nLastDelegateIdx) {
											last_letter_idx = prev_item_index + 1;
											break;
										}
									} while (--prev_index >= 0);
								}
							}

							for (let i = delegateRow.delegateIndex + 1; i < nDelegateRows; ++i) {
								const cboBox = splitRepeater.itemAt(i).children[1].children[0];
								const curIdx = cboBox.currentIndex;
								if (curIdx > last_letter_idx && curIdx !== 6)
									cboBox.currentIndex = last_letter_idx;
								for (let x = delegateRow.delegateIndex; x < nLastDelegateIdx; ++x)
									cboBox.model.get(x).enabled = x <= last_letter_idx;
							}
						}

						Component.onCompleted: {
							currentIndex = Qt.binding(function() { return indexOfValue(txtMesoSplit.text.charAt(delegateRow.delegateIndex)); });
							btnMuscularGroups.visible = Qt.binding(function() { return currentIndex !== 6; });
							let last_letter_idx = indexOfValue(currentValue);
							if (last_letter_idx === nLastDelegateIdx) {
								let prev_index = delegateRow.delegateIndex-1;
								let prev_item_index;
								do {
									prev_item_index = splitRepeater.itemAt(prev_index).children[1].children[0].currentIndex;
									if (prev_item_index !== nLastDelegateIdx) {
										last_letter_idx = prev_item_index + 1;
										break;
									}
								} while (--prev_index >= 0);
							}

							for (let x = delegateRow.delegateIndex; x < nLastDelegateIdx; ++x)
								model.get(x).enabled = x <= last_letter_idx;
						}
					}

					TPButton {
						//objectName: "button"
						id: btnMuscularGroups
						imageSource: "choose.png"
						imageSize: 25
						enabled: !mesoManager.ownMeso

						anchors {
							bottom: parent.bottom
							left: parent.left
							right: parent.right
						}

						onClicked: showMGDialog(this, splitRepeater.itemAt(index).children[2]);
					}
				} //Item

				TPTextInput {
					//objectName: "text"
					id: txtSplit
					readOnly: true
					suggestedHeight: 50
					Layout.minimumWidth: col3Width
					Layout.maximumWidth: col3Width
					Layout.minimumHeight: 50
					Layout.maximumHeight: 80

					Component.onCompleted: {
						createBindings();

						textChanged.connect(function() {
							switch (splitRepeater.itemAt(index).children[1].children[0].currentIndex) {
								case 0: mesoManager.muscularGroupA = text; break;
								case 1: mesoManager.muscularGroupB = text; break;
								case 2: mesoManager.muscularGroupC = text; break;
								case 3: mesoManager.muscularGroupD = text; break;
								case 4: mesoManager.muscularGroupE = text; break;
								case 5: mesoManager.muscularGroupF = text; break;
							}
						});
					}

					function createBindings(): void {
						text = Qt.binding(function() {
							switch (splitRepeater.itemAt(index).children[1].children[0].currentIndex) {
								case 0: return mesoManager.muscularGroupA;
								case 1: return mesoManager.muscularGroupB;
								case 2: return mesoManager.muscularGroupC;
								case 3: return mesoManager.muscularGroupD;
								case 4: return mesoManager.muscularGroupE;
								case 5: return mesoManager.muscularGroupF;
								case 6: return mesoManager.muscularGroupR;
							}
						});
					}
				} //TPTextInput
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
	property TPTextInput txtWidget: null
	function showMGDialog(button: TPButton, text_widget: TPTextInput): void {
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

		function setWidgetText(groups) {
			txtWidget.text = groups;
			bMesoSplitChanged = true;
			txtWidget.createBindings();
		}

		filterDlg.muscularGroupCreated.disconnect(setWidgetText);
		txtWidget = text_widget;

		filterDlg.muscularGroupCreated.connect(setWidgetText);
		filterDlg.show(text_widget.text, button, 3);
	}
} //Pane
