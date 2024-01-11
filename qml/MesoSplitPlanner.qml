import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Frame {
	id: paneSplit
	required property int divisionId
	required property int mesoId
	required property string splitLetter
	required property string splitText
	property string splitExercises
	property string splitSetTypes
	property string splitNSets
	property string splitNReps
	property string splitNWeight
	property bool bCurrentItem: false

	property int currentModelIndex: 0
	property int seconds: 0
	property bool bModified: false

	signal currentSplitObjectChanged(var splitItem)

	ListModel {
		id: exercisesListModel
	}

	Timer {
		id: undoTimer
		interval: 1000
		property int idxToRemove

		onTriggered: {
			if ( seconds === 0 ) {
				undoTimer.stop();
				removeExercise(idxToRemove);
			}
			else {
				seconds = seconds - 1000;
				start();
			}
		}

		function init(idxtoremove) {
			idxToRemove = idxtoremove;
			start();
		}
	} //Timer


	property bool shown: true
	visible: height > 0
	height: shown ? implicitHeight : lblMain.height * 3
	Behavior on height {
		NumberAnimation {
			easing.type: Easing.InOutQuad
		}
	}
	clip: true
	padding: 0
	z: 0

	background: Rectangle {
		border.color: "transparent"
		radius: 5
	}

	implicitHeight: splitLayout.implicitHeight + 10
	implicitWidth: pagePlanner.width
	Layout.leftMargin: 5

	ColumnLayout {
		id: splitLayout
		anchors.fill: parent

		Label {
			id: lblMain
			text: qsTr("Training Division ") + splitLetter
			Layout.alignment: Qt.AlignHCenter|Qt.AlignTop
			Layout.topMargin: 10
			Layout.maximumWidth: parent.width - 20
			font.bold: true
			z: 1

			Image {
				anchors.left: lblMain.right
				anchors.verticalCenter: lblMain.verticalCenter
				source: paneSplit.shown ? "qrc:/images/"+darkIconFolder+"fold-up.png" : "qrc:/images/"+darkIconFolder+"fold-down.png"
				height: 20
				width: 20
				z: 0
			}

			MouseArea {
				anchors.fill: parent
				onClicked: paneSplit.shown = !paneSplit.shown;
				z:2
			}
		}// Label lblMain

		Label {
			text: qsTr("Muscle groups trained in this division:")
			Layout.leftMargin: 5
			Layout.topMargin: 10
		}

		TextField {
			id: txtSplit
			text: splitText
			font.pixelSize: AppSettings.fontSizeText
			Layout.fillWidth: true
			Layout.leftMargin: 5
			Layout.rightMargin: 20
			font.bold: true

			onTextEdited: {
				splitText = text;
				bModified = true;
			}
		}

		Label {
			text: qsTr("Exercises:")
			Layout.leftMargin: 5
			width: pagePlanner.width / 2

			ToolButton {
				id: btnAddExercise
				height: 25
				width: 25
				text: qsTr("Add exercise")
				font.capitalization: Font.MixedCase
				display: AbstractButton.TextBesideIcon

				anchors {
					left: parent.right
					verticalCenter: parent.verticalCenter
					leftMargin: 10
				}

				Image {
					source: "qrc:/images/"+darkIconFolder+"add-new.png"
					height: 20
					width: 20
				}

				onClicked: {
					bottomPane.shown = true;
					appendNewExerciseToDivision();
					//activeSplitObject = this;
				}
			} //btnAddExercise
		} //Label

		ListView {
			id: lstSplitAExercises
			boundsBehavior: Flickable.StopAtBounds
			flickableDirection: Flickable.VerticalFlick
			contentHeight: totalHeight * 1.1 + 20//contentHeight: Essencial for the ScrollBars to work.
			contentWidth: totalWidth //contentWidth: Essencial for the ScrollBars to work
			property int totalHeight
			property int totalWidth

			Layout.fillWidth: true
			Layout.rightMargin: 10
			Layout.leftMargin: 5
			Layout.minimumHeight: 200

			function setModel(newmodel) {
				model = newmodel;
			}

			delegate: SwipeDelegate {
				id: delegate
				spacing: 0
				padding: 0
				implicitWidth: Math.max(lstSplitAExercises.width, listItem.implicitWidth)
				implicitHeight: listItem.height
				clip: false
				property bool bEditName: false

				contentItem: Rectangle {
					id: listItem
					width: lstSplitAExercises.width - 10
					height: (contentsLayout.rows + 1) * 40
					border.color: "transparent"
					color: "transparent"
					radius: 5
				}

				GridLayout {
					id: contentsLayout
					anchors.fill: parent
					rows: 5
					columns: 2
					rowSpacing: 2
					columnSpacing: 2

					TextField {
						id: txtExerciseName
						text: exerciseName
						wrapMode: Text.WordWrap
						readOnly: !delegate.bEditName
						Layout.row: 0
						Layout.column: 0
						Layout.columnSpan: 2
						Layout.minimumWidth: parent.width - 40
						Layout.maximumWidth: parent.width - 40
						width: parent.width - 40

							background: Rectangle {
							color: txtExerciseName.readOnly ? "transparent" : "white"
							border.color: txtExerciseName.readOnly ? "transparent" : "black"
							radius: 5
						}

						Keys.onReturnPressed: { //Alphanumeric keyboard
							delegate.bEditName = false;
							cboSetType.forceActiveFocus();
						}

						onPressed: (mouse) => { //relay the signal to the delegate
							mouse.accepted = false;
						}

						onEditingFinished: {
							exercisesListModel.setProperty(currentModelIndex, "exerciseName", text);
						}

						ToolButton {
							id: btnEditExercise
							padding: 10
							anchors.left: txtExerciseName.right
							anchors.top: txtExerciseName.top
							height: 25
							width: 25
							z: 2
							Image {
								source: "qrc:/images/"+darkIconFolder+"edit.png"
								anchors.verticalCenter: parent.verticalCenter
								anchors.horizontalCenter: parent.horizontalCenter
								height: 20
								width: 20
							}

							onClicked: {
								delegate.bEditName = !delegate.bEditName;
								txtExerciseName.forceActiveFocus();
							}
						} //ToolButton btnEditExercise
					} //TextField

					Label {
						text: qsTr("Set Type:")
						Layout.row: 1
						Layout.column: 0
						Layout.leftMargin: 5
					}
					ComboBox {
						id: cboSetType
						model: setTypes
						Layout.minimumWidth: 120
						currentIndex: parseInt(setType);
						textRole: "key"
						valueRole: "idx"
						Layout.row: 1
						Layout.column: 1
						Layout.rightMargin: 5

						onActivated: (index) => {
							exercisesListModel.setProperty(currentModelIndex, "setType", currentValue.toString());
							txtNSets.forceActiveFocus();
						}
					}

					Label {
						text: qsTr("Number of Sets:")
						Layout.row: 2
						Layout.column: 0
						Layout.leftMargin: 5
					}
					SetInputField {
						id: txtNSets
						text: setsNumber
						type: SetInputField.Type.SetType
						nSetNbr: 0
						availableWidth: listItem.width / 3
						showLabel: false
						Layout.row: 2
						Layout.column: 1
						Layout.rightMargin: 5

						onEnterOrReturnKeyPressed: {
							txtNReps.forceActiveFocus();
						}
					}

					Label {
						text: qsTr("Baseline number of reps:")
						Layout.row: 3
						Layout.column: 0
						Layout.leftMargin: 5
					}
					SetInputField {
						id: txtNReps
						text: repsNumber
						type: SetInputField.Type.RepType
						nSetNbr: 0
						availableWidth: listItem.width / 3
						showLabel: false
						Layout.row: 3
						Layout.column: 1
						Layout.rightMargin: 5

						onEnterOrReturnKeyPressed: {
							txtNWeight.forceActiveFocus();
						}
					}

					Label {
						text: qsTr("Baseline weight:")
						Layout.row: 4
						Layout.column: 0
						Layout.leftMargin: 5
					}
					SetInputField {
						id: txtNWeight
						text: weightValue
						type: SetInputField.Type.WeightType
						nSetNbr: 0
						availableWidth: listItem.width / 3
						showLabel: false
						Layout.row: 4
						Layout.column: 1
						Layout.rightMargin: 5

						onEnterOrReturnKeyPressed: {
							//save?
						}
					}
				} //GridLayout

				background: Rectangle {
					id:	backgroundColor
					radius: 5
					color: bCurrentItem ? currentModelIndex === index? "lightcoral" : index % 2 === 0 ? "#dce3f0" : "#c3cad5" : index % 2 === 0 ? "#dce3f0" : "#c3cad5"
				}

				Component.onCompleted: {
					if ( lstSplitAExercises.totalWidth < width )
						lstSplitAExercises.totalWidth = width;
					lstSplitAExercises.totalHeight += height;
				}

				onClicked: {
					currentModelIndex = index;
					currentSplitObjectChanged(paneSplit);
				}

				swipe.right: Rectangle {
					width: parent.width
					height: parent.height
					clip: false
					color: SwipeDelegate.pressed ? "#555" : "#666"
					radius: 5

					Image {
						source: "qrc:/images/"+lightIconFolder+"remove.png"
						anchors.left: parent.left
						anchors.leftMargin: 10
						anchors.verticalCenter: parent.verticalCenter
						width: 20
						height: 20
						opacity: 2 * -delegate.swipe.position
						z:2
					}

					Label {
						text: qsTr("Removing in " + seconds/1000 + "s")
						color: "white"
						padding: 5
						anchors.fill: parent
						anchors.leftMargin: 40
						horizontalAlignment: Qt.AlignLeft
						verticalAlignment: Qt.AlignVCenter
						opacity: delegate.swipe.complete ? 1 : 0
						Behavior on opacity { NumberAnimation { } }
						z:0
					}

					SwipeDelegate.onClicked: delegate.swipe.close();
					SwipeDelegate.onPressedChanged: undoTimer.stop();
				} //swipe.right

				swipe.onCompleted: {
					seconds = 4000;
					undoTimer.init(index);
				}
			} //delegate: SwipeDelegate
		} //ListView

		ButtonFlat {
			id: btnSave
			text: qsTr("Save Plan for division ") + splitLetter
			Layout.alignment: Qt.AlignCenter
			Layout.topMargin: 10
			enabled: bModified

			onClicked: saveMesoDivisionPlan();
		}
	} //ColumnLayout

	Component.onCompleted: {
		lstSplitAExercises.setModel(exercisesListModel);
		const exercises = splitExercises.split('|');
		const nExercises = exercises.length;
		if (nExercises === 0) {
			appendNewExerciseToDivision();
			return;
		}

		var i = 0;
		const types = splitSetTypes.split('|');
		const nsets = splitNSets.split('|');
		const nreps = splitNReps.split('|');
		const nweights = splitNWeight.split('|');
		while (i < nExercises) {
			exercisesListModel.append ({ "exerciseName":exercises[i], "setType":types[i],
			"setsNumber":nsets[i], "repsNumber":nreps[i], "weightValue":nweights[i] });
			i++;
		}
		currentModelIndex = exercisesListModel.count - 1;
	}

	function appendNewExerciseToDivision() {
		currentModelIndex = exercisesListModel.count;
		exercisesListModel.append ( {"exerciseName":qsTr("Choose exercise..."), "setType":"0",
			"setsNumber":"0", "repsNumber":"12", "weightValue":"20" } );
		currentSplitObjectChanged(paneSplit);
	}

	function changeModel(name1, name2, nsets, nreps, nweight) {
		if (exercisesListModel.count > 0) {
			exercisesListModel.setProperty(currentModelIndex, "exerciseName", name1 + " - " + name2);
			exercisesListModel.setProperty(currentModelIndex, "setsNumber", nsets.toString());
			exercisesListModel.setProperty(currentModelIndex, "repsNumber", nreps.toString());
			exercisesListModel.setProperty(currentModelIndex, "weightValue", nweight.toString());
		}
	}

	function saveMesoDivisionPlan() {
		for (var i = 0; i < exercisesListModel.count; ++i) {
			var exercises = exercisesListModel.get(i).exerciseName + '|';
			var types = exercisesListModel.get(i).setType + '|';
			var nsets = exercisesListModel.get(i).setsNumber + '|';
			var nreps = exercisesListModel.get(i).repsNumber + '|';
			var nweights = exercisesListModel.get(i).weightValue + '|';
		}
		Database.updateMesoDivisionComplete(divisionId, exercisesListModel.get(0).splitText , exercises.slice(0, -1), types.slice(0, -1),
			nsets.slice(0, -1), nreps.slice(0, -1), nweights.slice(0, -1));
	}
} //Page
