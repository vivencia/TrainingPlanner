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
	readonly property int col3Width: appSettings.pageWidth * 0.6
	readonly property int col4Width: appSettings.itemDefaultHeight
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
			text: appMesoModel().splitLabel
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
					Layout.preferredWidth: col1Width
					Layout.alignment: Qt.AlignCenter
				}

				TPComboBox {
					//objectName: "combo"
					// Don't allow a day to skip a letter. Letters must be added sequentially or be repeated, never skipped
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

					onActivated: (cboindex) => {
						let mesoSplit = txtMesoSplit.text;
						mesoManager.split = mesoSplit.substring(0,delegateRow.delegateIndex) +
													valueAt(cboindex) + mesoSplit.substring(delegateRow.delegateIndex+1);
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

				TPTextInput {
					//objectName: "text"
					id: txtSplit
					readOnly: true
					suggestedHeight: 30
					Layout.minimumWidth: col3Width
					Layout.maximumWidth: col3Width
					Layout.minimumHeight: 50
					Layout.maximumHeight: 80

					Component.onCompleted: {
						createBindings();

						textChanged.connect(function() {
							switch (cboSplit.currentIndex) {
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
							switch (cboSplit.currentIndex) {
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

				TPButton {
					//objectName: "button"
					id: btnMuscularGroups
					imageSource: "choose.png"
					enabled: cboSplit.currentIndex !== 6
					width: col4Width
					height: col4Width
					Layout.minimumWidth: col4Width
					Layout.maximumWidth: col4Width
					Layout.preferredHeight: col4Width

					onClicked: showMGDialog(this, splitRepeater.itemAt(index).children[2]);
				}
			} //RowLayout
		} //Repeater
	} //GridLayout

	TPButton {
		text: qsTr("Exercises Planner")
		enabled: !mesoManager.isNewMeso && mesoManager.splitOK
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

	property MuscularGroupPicker filterDlg: null
	property TPTextInput txtWidget: null
	function showMGDialog(button: TPButton, text_widget: TPTextInput): void {
		if (filterDlg === null) {
			let component = Qt.createComponent("qrc:/qml/Dialogs/MuscularGroupPicker.qml", Qt.Asynchronous);

			function finishCreation() { //use fancy_record_separator1 as groupsSeparator
				filterDlg = component.createObject(mainwindow, { parentPage: mesoPropertiesPage, groupsSeparator: '|',
									buttonLabel: qsTr("Define"), useFancyNames: true });
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}

		function setWidgetText(groups) {
			txtWidget.text = groups;
			txtWidget.createBindings();
		}

		filterDlg.muscularGroupCreated.disconnect(setWidgetText);
		txtWidget = text_widget;

		filterDlg.muscularGroupCreated.connect(setWidgetText);
		filterDlg.show(text_widget.text, button, 3);
	}
} //Pane
