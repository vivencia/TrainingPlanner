pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import TpQml

Item {
	id: _control

	required property FileOperations fileOps

	TabBar {
		id: tabSections
		height: AppSettings.itemLargeHeight
		clip: true
		currentIndex: sectionsLayout.currentIndex

		anchors {
			top: _control.top
			topMargin: 10
			horizontalCenter: _control.horizontalCenter
		}

		Repeater {
			id: tabSectionsRepeater
			model: _control.fileOps.tpFileSectionCount

			delegate: TPTabButton {
				text: _control.fileOps.tpFileSectionTitle(index);
				parentTab: tabSections

				required property int index

				onClicked: sectionsLayout.currentIndex = index;
			} //TPTabButton
		} //Repeater: tabSectionsRepeater
	} //TabBar: tabSections

	StackLayout {
		id: sectionsLayout

		anchors {
			fill: parent
			topMargin: AppSettings.itemLargeHeight + 20
		}

		Repeater {
			id: sectionsRepeater
			model: _control.fileOps.tpFileSectionCount

			delegate: TPMultiLineEdit {
				id: _multiline_edit
				text: _control.fileOps.tpFileSection(index);
				maxHeight: -1
				minHeight: height
				width: sectionsLayout.width
				height: sectionsLayout.height

				required property int index

				onTextControlChanged: {
					textControl.cursorPositionChanged.connect(function () {
									_control.fileOps.setWorkingDocumentCursorPosition(textControl.cursorPosition); });
				}

				Connections {
					target: _control.fileOps
					function onSetCursorPorsition(cursor_pos: int) : void {
						if (sectionsLayout.currentIndex === _multiline_edit.index)
							_multiline_edit.textControl.cursorPosition = cursor_pos;
					}
					function onInsertString(str: string, pos: int): void {
						_multiline_edit.textControl.insert(pos, str);
					}
				}
				Connections {
					target: sectionsLayout
					function onCurrentIndexChanged(): void {
						if (sectionsLayout.currentIndex === _multiline_edit.index)
							_control.fileOps.setWorkingTextDocument(_multiline_edit.textControl.textDocument);
					}
				}
			} //TPMultiLineEdit
		} //Repeater: tabSectionsRepeater
	} //StackLayout: sectionsLayout
} //Item
