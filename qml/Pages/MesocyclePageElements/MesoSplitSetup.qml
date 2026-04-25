pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import TpQml
import TpQml.Widgets
import TpQml.Dialogs

Pane {
	id: _control
	implicitHeight: mainLayout.implicitHeight + 100
	spacing: 0
	padding: 0
	Layout.topMargin: 10
	Layout.leftMargin: 0

//public:
	required property MesoManager mesoManager
	required property MesocyclesModel mesoModel

//private:
	property alias mesoSplitText: txtMesoSplit.text
	readonly property int col1Width: width * 0.1
	readonly property int col2Width: width * 0.15
	readonly property int col3Width: AppSettings.itemDefaultHeight
	readonly property int col4Width: AppSettings.pageWidth * 0.6
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
			text: _control.mesoModel.splitLabel
			Layout.fillWidth: true
		}

		TPTextInput {
			id: txtMesoSplit
			text: _control.mesoManager.split
			ToolTip.text: qsTr("On any training program, there should be at least one rest day(R) per week and one training day(A-F)")
			ToolTip.visible: !_control.mesoManager.splitOK
			ToolTip.timeout: 5000
			readOnly: true
			Layout.minimumWidth: parent.width * 0.4
			Layout.maximumWidth: parent.width * 0.4

			TPImage {
				source: "set-completed"
				enabled: _control.mesoManager.splitOK
				width: AppSettings.itemDefaultHeight
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
				id: delegate
				Layout.fillWidth: true
				Layout.topMargin: 10

				required property int index

				TPLabel {
					text: AppUtils.shortDayName(delegate.index)
					Layout.preferredWidth: _control.col1Width
					Layout.alignment: Qt.AlignCenter
				}

				TPComboBox {
					id: cboSplit
					Layout.preferredWidth: _control.col2Width
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
						_control.mesoManager.split = mesoSplit.substring(0, delegate.index) + valueAt(cboindex) +
																								mesoSplit.substring(delegate.index + 1);
						splitRepeater.itemAt(delegate.index).children[3].fillMuscularGroupsModel(_control.mesoManager.muscularGroup(currentValue));

						let last_letter_idx = cboindex + 1;
						if (last_letter_idx === nDelegateRows) {
							last_letter_idx = 0;
							if (delegate.index >= 1) {
								let prev_index = delegate.index-1;
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

						for (let i = delegate.index + 1; i < nDelegateRows; ++i) {
							const cboBox = splitRepeater.itemAt(i).children[1];
							const curIdx = cboBox.currentIndex;
							if (curIdx > last_letter_idx && curIdx !== 6)
								cboBox.currentIndex = last_letter_idx;
							for (let x = 0; x < nLastDelegateIdx; ++x)
								cboBox.model.get(x).enabled = x <= last_letter_idx;
						}
					}

					Component.onCompleted: {
						currentIndex = indexOfValue(txtMesoSplit.text.charAt(delegate.index));
						splitRepeater.itemAt(delegate.index).children[3].fillMuscularGroupsModel(_control.mesoManager.muscularGroup(currentValue));

						let last_letter_idx = indexOfValue(currentValue);
						if (last_letter_idx === nLastDelegateIdx) { //split is an 'R'
							let prev_index = delegate.index - 1;
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
					Layout.preferredWidth: _control.col3Width
					Layout.preferredHeight: _control.col3Width

					onClicked: _control.showMGDialog(this, cboSplit.currentValue);
				}

				TPMuscularGroupsList {
					id: cboMuscularGroup
					Layout.minimumWidth: _control.col4Width
					Layout.maximumWidth: _control.col4Width
				} //cboMuscularGroup
			} //RowLayout
		} //Repeater
	} //GridLayout

	TPButton {
		text: qsTr("Exercises Planner")
		enabled: _control.mesoManager.splitOK
		autoSize: true

		anchors {
			bottom: parent.bottom
			horizontalCenter: parent.horizontalCenter
		}

		onClicked: _control.mesoManager.getExercisesPlannerPage();
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

	Loader {
		id: mGDialogLoader
		asynchronous: true
		active: false

		property MuscularGroupPicker _mg_picker
		property TPButton _button
		property string _split

		sourceComponent: MuscularGroupPicker {
			parentPage: _control.mesoManager.qmlPage() as TPPage
			buttonLabel: qsTr("Define")
			initialGroups: _control.mesoManager.muscularGroup(mGDialogLoader._split)
			onMuscularGroupsCreated: (groups) => _control.setMuscularGroup(groups);
			onClosed: mGDialogLoader.active = false;
			Component.onCompleted: mGDialogLoader._mg_picker = this;
		}

		onLoaded: _mg_picker.showByWidget(Qt.AlignRight, _button);
	}

	function showMGDialog(button: TPButton, split: string): void {
		mGDialogLoader._button = button;
		mGDialogLoader._split = split;
		mGDialogLoader.active = true;
	}
} //Pane
