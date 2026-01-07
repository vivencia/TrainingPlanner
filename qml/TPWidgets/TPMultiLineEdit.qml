import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
	id: mainLayout
	spacing: 0

	property bool showToolBox: false
	property bool editable: true
	property alias text: textControl.text

	signal textAltered(_text: string);
	signal enterOrReturnKeyPressed(mod_key: int);

	readonly property int _maxHeight: appSettings.pageHeight / 3
	property int _nFormatting: 0

	RowLayout {
		id: toolBoxLayout
		visible: showToolBox
		spacing: 5
		Layout.fillWidth: true

		TPButton {
			imageSource: "copy_"
			focus: false
			enabled: textControl.length > 0
			width: appSettings.itemDefaultHeight
			height: width
			onClicked: {
				appUtils.copyToClipboard(textControl.selectionStart === textControl.selectionEnd ?
						textControl.text : textControl.text.substr(textControl.selectionStart, textControl.selectionEnd));
				mainwindow.showTextCopiedMessage();
			}
		}
		TPButton {
			imageSource: "paste_"
			focus: false
			enabled: textControl.canPaste
			width: appSettings.itemDefaultHeight
			height: width
			onClicked: textControl.paste()
		}
		TPButton {
			imageSource: "undo_"
			enabled: textControl.canUndo
			focus: false
			width: appSettings.itemDefaultHeight
			height: width
			onClicked: textControl.undo();
		}
		TPButton {
			imageSource: "redo_"
			enabled: textControl.canRedo
			focus: false
			width: appSettings.itemDefaultHeight
			height: width
			onClicked: textControl.redo();
		}

		TPButton {
			id: btnItalic
			imageSource: "italic_"
			checkable: true
			focus: false
			enabled: textControl.length > 0
			width: appSettings.itemDefaultHeight
			height: width
			onCheck: {
				formatChanged(checked);
				textControl.cursorSelection.font.italic = checked;
			}
		}
		TPButton {
			id: btnUnderline
			imageSource: "underscore_"
			checkable: true
			focus: false
			enabled: textControl.length > 0
			width: appSettings.itemDefaultHeight
			height: width
			onCheck: {
				formatChanged(checked);
				textControl.cursorSelection.font.underline = checked
			}
		}
		TPButton {
			id: btnCase
			imageSource: "upperlowercase_"
			checkable: true
			focus: false
			enabled: textControl.length > 0
			width: appSettings.itemDefaultHeight
			height: width
			onCheck: {
				formatChanged(checked);
				textControl.cursorSelection.font.capitalization = checked ? Font.AllUppercase : Font.MixedCase;
			}
		}
	}

	Row {
		Layout.fillWidth: true
		spacing: 5

		Flickable {
			id: scrollArea
			contentWidth: availableWidth
			contentHeight: availableHeight
			clip: true
			height: 2 * appSettings.itemDefaultHeight
			width: parent.width - appSettings.itemDefaultHeight - 5

			ScrollBar.vertical: ScrollBar { id: vBar }

			TextArea.flickable: TextArea {
				id: textControl
				readOnly: !editable
				wrapMode: TextEdit.Wrap
				textFormat: TextEdit.RichText
				renderType: TextEdit.QtRendering
				color: appSettings.fontColor
				font.pixelSize: appSettings.fontSize
				font.preferShaping: false
				focus: true
				persistentSelection: true
				topPadding: 6
				leftPadding: 6
				rightPadding: btnClearText.width
				bottomPadding: 6
				leftInset: 0
				rightInset: 0
				topInset: 0
				bottomInset: 0

				background: Rectangle {
					color: appSettings.paneBackgroundColor
					radius: 8
					border.color: appSettings.fontColor
				}

				property bool modified: false
				property bool formatted: false

				cursorSelection.onFontChanged: {
					btnItalic.checked = cursorSelection.font.italic;
					btnUnderline.checked = cursorSelection.font.underline;
					btnCase.checked = cursorSelection.font.capitalization === Font.AllUppercase;
				}

				Keys.onPressed: (event) => {
					switch (event.key) {
						case Qt.Key_Enter:
						case Qt.Key_Return:
						{
							let mod_key = 0;
							if (event.modifiers)
							{
								if (event.modifiers & Qt.ControlModifier)
									mod_key = Qt.Key_Control;
								else if (event.modifiers & Qt.AltModifier)
									mod_key = Qt.Key_Alt;
								else if (event.modifiers & Qt.ShiftModifier)
									mod_key = Qt.Key_Shift;
							}
							if (mod_key !== 0)
								event.accepted = true;
							enterOrReturnKeyPressed(mod_key);
						}
						break;
						case Qt.Key_Left:
							event.accepted = true;
						break;
						default: return;
					}
				}

				onReadOnlyChanged: positionCaret();
				onLineCountChanged: scrollArea.calculateHeight();
				onTextEdited: modified = true;
				onEditingFinished: {
					if (modified) {
						modified = false;
						textAltered(contentsText());
					}
				}
				onActiveFocusChanged: {
					if (activeFocus)
						positionCaret();
				}

				function positionCaret(): void {
					if (readOnly) {
						vBar.setPosition(0);
						cursorPosition = 0;
					}
					else {
						vBar.setPosition(1);
						cursorPosition = length;
					}
				}
			} //TextArea

			function calculateHeight(): void {
				const new_height = (textControl.lineCount * appSettings.itemDefaultHeight) + 10;
				if (new_height <= _maxHeight) {
					if (new_height < (2 * appSettings.itemDefaultHeight))
						height = implicitHeight = 2 * appSettings.itemDefaultHeight;
					else
						height = implicitHeight = new_height;
				}
				else
					height = implicitHeight = _maxHeight;
			}
		} //ScrollView

		Item {
			visible: editable
			width: appSettings.itemDefaultHeight
			height: scrollArea.height

			TPButton {
				id: btnShowToolBox
				imageSource: "toolbox_"
				checkable: true
				width: appSettings.itemDefaultHeight
				height: width

				anchors {
					verticalCenter: parent.verticalCenter
					verticalCenterOffset: -(height / 2)
					horizontalCenter: parent.horizontalCenter
				}

				onCheck: mainLayout.showToolBox = checked;
			}

			TPButton {
				id: btnClearText
				imageSource: "edit-clear"
				hasDropShadow: false
				enabled: textControl.length > 0
				width: appSettings.itemDefaultHeight
				height: width

				anchors {
					verticalCenter: parent.verticalCenter
					verticalCenterOffset: (height / 2)
					horizontalCenter: parent.horizontalCenter
				}

				onClicked: {
					clear();
					textControl.forceActiveFocus();
				}
			}
		} //Item
	} //Row

	function clear(): void {
		textControl.clear();
		_nFormatting = 0;
	}

	function formatChanged(added_format: bool) : void {
		if (added_format)
			_nFormatting++;
		else {
			_nFormatting--;
			if (_nFormatting < 0)
				_nFormatting = 0;
		}
		textControl.modified = true;
	}

	function contentsText() : string {
		if (_nFormatting == 0)
			return textControl.getText(0, textControl.length);
		else
			return appUtils.stripInvalidCharacters(textControl.text);
	}
} //ColumnLayout
