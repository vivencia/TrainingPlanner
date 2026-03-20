import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import TpQml

ColumnLayout {
	id: _control
	spacing: 0

//public:
	property bool editable: true
	property int minHeight: 2 * AppSettings.itemDefaultHeight
	property int maxHeight: AppSettings.pageHeight / 3
	property TextArea textControl
	property alias text: _textControl.text
	property alias modified: _textControl.modified
	signal textAltered(_text: string);
	signal enterOrReturnKeyPressed(mod_key: int);

//private:
	property bool _show_toolbox: false
	property int _nFormatting: 0

	Row {
		id: toolBoxLayout
		visible: _control._show_toolbox
		spacing: 5
		Layout.fillWidth: true
		Layout.preferredHeight: _control._show_toolbox ? AppSettings.itemDefaultHeight : 0

		TPButton {
			imageSource: "copy_"
			focus: false
			enabled: _control.textControl.length > 0
			width: AppSettings.itemDefaultHeight
			height: width
			onClicked: {
				AppUtils.copyToClipboard(_control.getControlText(_control.textControl.selectionStart, _control.textControl.selectionEnd));
				ItemManager.showTextCopiedMessage();
			}
		}
		TPButton {
			imageSource: "paste_"
			focus: false
			enabled: _control.textControl.canPaste
			width: AppSettings.itemDefaultHeight
			height: width
			onClicked: _control.textControl.paste()
		}
		TPButton {
			imageSource: "undo_"
			enabled: _control.textControl.canUndo
			focus: false
			width: AppSettings.itemDefaultHeight
			height: width
			onClicked: _control.textControl.undo();
		}
		TPButton {
			imageSource: "redo_"
			enabled: _control.textControl.canRedo
			focus: false
			width: AppSettings.itemDefaultHeight
			height: width
			onClicked: _control.textControl.redo();
		}

		TPButton {
			id: btnItalic
			imageSource: "italic_"
			checkable: true
			focus: false
			enabled: _control.textControl.length > 0
			width: AppSettings.itemDefaultHeight
			height: width
			onCheck: {
				_control.formatChanged(checked);
				_control.textControl.cursorSelection.font.italic = checked;
			}
		}
		TPButton {
			id: btnUnderline
			imageSource: "underscore_"
			checkable: true
			focus: false
			enabled: _control.textControl.length > 0
			width: AppSettings.itemDefaultHeight
			height: width
			onCheck: {
				_control.formatChanged(checked);
				_control.textControl.cursorSelection.font.underline = checked
			}
		}
		TPButton {
			id: btnCase
			imageSource: "upperlowercase_"
			checkable: true
			focus: false
			enabled: _control.textControl.length > 0
			width: AppSettings.itemDefaultHeight
			height: width
			onCheck: {
				_control.formatChanged(checked);
				_control.textControl.cursorSelection.font.capitalization = checked ? Font.AllUppercase : Font.MixedCase;
			}
		}
	}

	Row {
		Layout.fillWidth: true
		spacing: 5

		Flickable {
			id: scrollArea
			clip: true
			height: _control.minHeight - toolBoxLayout.height
			width: parent.width - AppSettings.itemDefaultHeight - 5

			ScrollBar.vertical: ScrollBar { id: vBar }

			TextArea.flickable: TextArea {
				id: _textControl
				readOnly: !_control.editable
				wrapMode: TextEdit.Wrap
				textFormat: TextEdit.RichText
				renderType: TextEdit.QtRendering
				color: AppSettings.fontColor
				font.pixelSize: AppSettings.fontSize
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
					color: AppSettings.paneBackgroundColor
					radius: 8
					border.color: AppSettings.fontColor
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
							if (event.modifiers) {
								if (event.modifiers & Qt.ControlModifier)
									mod_key = Qt.Key_Control;
								else if (event.modifiers & Qt.AltModifier)
									mod_key = Qt.Key_Alt;
								else if (event.modifiers & Qt.ShiftModifier)
									mod_key = Qt.Key_Shift;
							}
							if (mod_key !== 0)
								event.accepted = true;
							_control.enterOrReturnKeyPressed(mod_key);
						}
						break;
					case Qt.Key_Left:
						event.accepted = true;
						break;
					default: return;
					}
				}

				onReadOnlyChanged: positionCaret();
				onLineCountChanged: if (_control.maxHeight > 0) scrollArea.calculateHeight();
				onTextEdited: modified = true;
				onEditingFinished: {
					if (modified) {
						modified = false;
						_control.textAltered(_control.contentsText());
					}
				}
				onActiveFocusChanged: {
					if (activeFocus)
						positionCaret();
				}

				Component.onCompleted: _control.textControl = this;

				function positionCaret(): void {
					if (readOnly) {
						vBar.setPosition(0);
						cursorPosition = 0;
					}
					else
						vBar.setPosition(Math.floor(cursorPosition/length));
				}
			} //TextArea

			function calculateHeight(): void {
				const new_height = (_control.textControl.lineCount * AppSettings.itemDefaultHeight) + 10;
				if (new_height <= _control.maxHeight) {
					if (new_height < (2 * AppSettings.itemDefaultHeight))
						height = implicitHeight = 2 * AppSettings.itemDefaultHeight;
					else
						height = implicitHeight = new_height;
				}
				else
					height = implicitHeight = _control.maxHeight;
			}
		} //ScrollView

		Item {
			enabled: _control.editable
			width: AppSettings.itemDefaultHeight
			height: scrollArea.height

			TPButton {
				id: btnShowToolBox
				imageSource: "toolbox_"
				checkable: true
				width: AppSettings.itemDefaultHeight
				height: width

				anchors {
					verticalCenter: parent.verticalCenter
					verticalCenterOffset: -(height / 2)
					horizontalCenter: parent.horizontalCenter
				}

				onCheck: _control._show_toolbox = checked;
			}

			TPButton {
				id: btnClearText
				imageSource: "edit-clear"
				hasDropShadow: false
				enabled: _control.textControl.length > 0
				width: AppSettings.itemDefaultHeight
				height: width

				anchors {
					verticalCenter: parent.verticalCenter
					verticalCenterOffset: (height / 2)
					horizontalCenter: parent.horizontalCenter
				}

				onClicked: {
					_control.clear();
					_control.textControl.forceActiveFocus();
				}
			}
		} //Item
	} //Row

	function clear() : void {
		_control.textControl.clear();
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
		_control.textControl.modified = true;
	}

	function contentsText() : string {
		return getControlText(0, _control.textControl.length);
	}

	function getControlText(start: int, end: int) : string {
		if (_nFormatting == 0)
			return start !== end ? _control.textControl.getText(start, end) : _control.textControl.getText(0, textControl.length);
		else
			return AppUtils.stripInvalidCharacters(start === end ? _control.textControl.text : _control.textControl.selectedText);
	}
} //ColumnLayout
