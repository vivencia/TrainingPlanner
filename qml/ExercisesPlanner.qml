import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Page {
	id: pagePlanner
	required property int mesoId
	property string splitAText

	property bool bCanEdit: false
	property int currentModelIndex: 0
	property var activeSplitObject: null

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

	ScrollView {
		id: scrollMain
		anchors.fill: parent
		ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
		ScrollBar.vertical.policy: ScrollBar.AsNeeded
		ScrollBar.vertical.active: true
		padding: 2

		ColumnLayout {
			id: layoutMain
			width: scrollMain.availableWidth

			Frame {
				id: paneSplitA
				property bool shown: true
				visible: height > 0
				height: shown ? implicitHeight : lblMain.height
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

				implicitHeight: splitALayout.implicitHeight + 10
				implicitWidth: pagePlanner.width
				Layout.leftMargin: 5

				ColumnLayout {
					id: splitALayout
					anchors.fill: parent

					Label {
						id: lblMain
						text: qsTr("Training Division A")
						Layout.fillWidth: true
						Layout.alignment: Qt.AlignCenter
						Layout.topMargin: 10
						Layout.maximumWidth: parent.width - 20
						z: 1

						Image {
							anchors.left: lblMain.right
							anchors.leftMargin: -20
							anchors.verticalCenter: lblMain.verticalCenter
							source: paneSplitA.shown ? "qrc:/images/"+darkIconFolder+"fold-up.png" : "qrc:/images/"+darkIconFolder+"fold-down.png"
							height: 20
							width: 20
							z: 0
						}

						MouseArea {
							anchors.fill: parent
							onClicked: paneSplitA.shown = !paneSplitA.shown;
							z:2
						}
					}// Label lblMain

					Label {
						text: qsTr("Muscle groups trained in this division:")
						Layout.leftMargin: 5
						Layout.topMargin: 10
					}
					TextField {
						id: txtSplitA
						text: splitAText
						readOnly: !bCanEdit
						font.italic: bCanEdit
						font.pixelSize: AppSettings.fontSizeText
						Layout.fillWidth: true
						Layout.leftMargin: 10
						Layout.rightMargin: 20
						font.bold: true
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
							anchors {
								left: parent.right
								verticalCenter: parent.verticalCenter
								leftMargin: 10
							}

							Image {
								source: "qrc:/images/"+darkIconFolder+"add-new.png"
								anchors.verticalCenter: parent.verticalCenter
								anchors.horizontalCenter: parent.horizontalCenter
								height: 20
								width: 20
							}

							onClicked : {
								bottomPane.shown = true;
								appendNewExerciseToDivision();
								//activeSplitObject = this;
							}
						}
					}

					ListView {
						id: lstSplitAExercises
						boundsBehavior: Flickable.StopAtBounds
						flickableDirection: Flickable.VerticalFlick
						Layout.fillWidth: true
						contentHeight: totalHeight * 1.1 + 20//contentHeight: Essencial for the ScrollBars to work.
						contentWidth: totalWidth //contentWidth: Essencial for the ScrollBars to work
						property int totalHeight
						property int totalWidth

						ScrollBar.horizontal: ScrollBar {
							id: hBar
							policy: ScrollBar.AsNeeded
							active: true; visible: lstSplitAExercises.totalWidth > lstSplitAExercises.width
						}
						ScrollBar.vertical: ScrollBar {
							id: vBar
							policy: ScrollBar.AsNeeded
							active: true; visible: lstSplitAExercises.totalHeight > lstSplitAExercises.height
						}

						model: ListModel {
							id: exercisesListModelA

							Component.onCompleted: {
								/*if (count === 0) {
									let exercises = Database.getExercises();
									for (let exercise of exercises)
									append(exercise);
								}
								if (count > 0) {
									curIndex = 0;
									displaySelectedExercise(curIndex);
								}*/
							}
						} //model

						delegate: SwipeDelegate {
							id: delegate
							spacing: 0
							padding: 0
							width: Math.max(lstSplitAExercises.width, listItem.implicitWidth)
							height: listItem.implicitHeight
							clip: false

							contentItem: GroupBox {
								id: listItem
								padding: 0
								label: Label {
									text: "<b>" + exerciseName +"</b> - " + exerciseDescription
									width: parent.width
									wrapMode: Text.WordWrap
								}
								GridLayout {
									anchors.fill: parent
									rows: 4
									columns: 2

									Label {
										text: qsTr("Set Type:")
										Layout.row: 0
										Layout.column: 0
									}
									ComboBox {
										id: cboSetType
										model: setTypes
										Layout.minimumWidth: 120
										currentIndex: setType
										textRole: "key"
										valueRole: "idx"
										Layout.row: 1
										Layout.column: 0

										onActivated: (index) => {
											txtNSets.forceActiveFocus();
										}
									}

									Label {
										text: qsTr("Number of Sets:")
										Layout.row: 0
										Layout.column: 1
									}
									SetInputField {
										id: txtNSets
										text: setsNumber
										type: SetInputField.Type.SetType
										nSetNbr: 0
										availableWidth: listItem.width / 2
										Layout.row: 1
										Layout.column: 1

										onEnterOrReturnKeyPressed: {
											txtNReps.forceActiveFocus();
										}
									}

									Label {
										text: qsTr("Baseline number of reps:")
										Layout.row: 2
										Layout.column: 0
									}
									SetInputField {
										id: txtNReps
										text: repsNumber
										type: SetInputField.Type.RepType
										nSetNbr: 0
										availableWidth: listItem.width / 2
										Layout.row: 2
										Layout.column: 1

										onEnterOrReturnKeyPressed: {
											txtNWeight.forceActiveFocus();
										}
									}

									Label {
										text: qsTr("Baseline weight:")
										Layout.row: 3
										Layout.column: 0
									}
									SetInputField {
										id: txtNWeight
										text: weightValue
										type: SetInputField.Type.WeightType
										nSetNbr: 0
										availableWidth: listItem.width / 2
										Layout.row: 3
										Layout.column: 1

										onEnterOrReturnKeyPressed: {
											//save?
										}
									}
								}
							} //contentItem

							background: Rectangle {
								id:	backgroundColor
								radius: 5
								color: index % 2 === 0 ? "#dce3f0" : "#c3cad5"
							}

							Component.onCompleted: {
								if ( lstSplitAExercises.totalWidth < width )
									lstSplitAExercises.totalWidth = width;
								lstSplitAExercises.totalHeight += height;
							}

							onClicked: currentModelIndex = index;

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
						} // SwipeDelegate
					} //ListView

					ButtonFlat {
						id: btnSaveSplit
						text: qsTr("Accept Plan")
					}

				} //ColumnLayout
			} //Frame
		}//ColumnLayout layoutMain
	}//ScrollView scrollMain

	footer: ToolBar {
		id: bottomPane
		width: parent.width
		height: shown ? parent.height * 0.5 : btnShowHideList.height
		visible: height >= btnShowHideList.height
		clip: true
		spacing: 0
		padding: 0
		property bool shown: true

		Behavior on height {
			NumberAnimation {
				easing.type: Easing.InOutQuad
			}
		}

		background: Rectangle {
			opacity: 0.3
			color: paneBackgroundColor
		}

		ColumnLayout {
			width: parent.width
			height: parent.height
			spacing: 0

			ButtonFlat {
				id: btnShowHideList
				imageSource: bottomPane.shown ? "qrc:/images/"+darkIconFolder+"fold-up.png" : "qrc:/images/"+darkIconFolder+"fold-down.png"
				onClicked: bottomPane.shown = !bottomPane.shown;
				Layout.fillWidth: true
				Layout.topMargin: 0
				height: 20
				width: bottomPane.width
			}

			ExercisesListView {
				id: exercisesList
				Layout.fillWidth: true
				Layout.topMargin: 5
				Layout.alignment: Qt.AlignTop
				Layout.rightMargin: 5
				Layout.maximumHeight: parent.height * 0.8
				Layout.leftMargin: 5

				onExerciseEntrySelected:(exerciseName, subName, muscularGroup, sets, reps, weight, mediaPath) => {
					changeModel(exerciseName, subName, sets, reps, weight);
				}
			}
		}
	} //footer: ToolBar

	function appendNewExerciseToDivision() {
		currentModelIndex = exercisesListModelA.count;
		exercisesListModelA.append ( {"exerciseName":qsTr("Choose exercise..."), "exerciseDescription":" ",
				"setType":0, "setsNumber":0, "repsNumber":12, "weightValue": 20 } );
	}

	function changeModel(name1, name2, nsets, nreps, nweight) {
		exercisesListModelA.setProperty(currentModelIndex, "exerciseName", name1);
		exercisesListModelA.setProperty(currentModelIndex, "exerciseDescription", name2);
		exercisesListModelA.setProperty(currentModelIndex, "setsNumber", nsets);
		exercisesListModelA.setProperty(currentModelIndex, "repsNumber", nreps);
		exercisesListModelA.setProperty(currentModelIndex, "weightValue", nweight);
	}
} //Page
