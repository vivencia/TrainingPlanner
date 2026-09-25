import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import TpQml

Item {
	id: _control

//public:
	property bool editable: true
	property string text
	readonly property int preferredHeight: toolBoxLayout.height + 4 * _margins + textControl.contentHeight
	readonly property TextArea textControl: _textControl

	signal textEdited()
	signal editingFinished(_text: string)
	signal enterOrReturnKeyPressed(mod_key: int)

//private:
	property bool _show_toolbox: false
	property int _nFormatting: 0
	property bool _modified
	property int _margins: 2

	Row {
		id: toolBoxLayout
		visible: _control._show_toolbox
		spacing: 5
		height: _control._show_toolbox ? AppSettings.itemDefaultHeight : 0

		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
			margins: _control._margins
		}

		TPButton {
			image: "copy_"
			focus: false
			enabled: _control.textControl.length > 0
			width: AppSettings.itemDefaultHeight
			height: width
			onClicked: AppUtils.copyToClipboard(_control.getControlText(_control.textControl.selectionStart,
																					_control.textControl.selectionEnd));
		}
		TPButton {
			image: "paste_"
			focus: false
			enabled: _control.textControl.canPaste
			width: AppSettings.itemDefaultHeight
			height: width
			onClicked: _control.textControl.paste()
		}
		TPButton {
			image: "undo_"
			enabled: _control.textControl.canUndo
			focus: false
			width: AppSettings.itemDefaultHeight
			height: width
			onClicked: _control.textControl.undo();
		}
		TPButton {
			image: "redo_"
			enabled: _control.textControl.canRedo
			focus: false
			width: AppSettings.itemDefaultHeight
			height: width
			onClicked: _control.textControl.redo();
		}

		TPButton {
			id: btnItalic
			image: "italic_"
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
			image: "underscore_"
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
			image: "upperlowercase_"
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
	} //toolBoxLayout

	Flickable {
		id: scrollArea
		clip: true

		anchors {
			top: toolBoxLayout.bottom
			left: parent.left
			right: parent.right
			bottom: parent.bottom
			margins: _control._margins
		}

		ScrollBar.vertical: ScrollBar { id: vBar }

		TextArea.flickable: TextArea {
			id: _textControl
			text: _control.text
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

			property bool formatted: false

			cursorSelection.onFontChanged: {
				btnItalic.checked = cursorSelection.font.italic;
				btnUnderline.checked = cursorSelection.font.underline;
				btnCase.checked = cursorSelection.font.capitalization === Font.AllUppercase;
			}

			Keys.onPressed: (event) => {
				switch (event.key) {
				case Qt.Key_Enter:
				case Qt.Key_Return: {
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

			onTextEdited: {
				_control._modified = true;
				_control.textEdited();
			}
			onEditingFinished: {
				if (_control._modified) {
					_control._modified = false;
					_control.editingFinished(_control.contentsText());
				}
			}
			onActiveFocusChanged: {
				if (activeFocus)
					positionCaret();
			}
			onReadOnlyChanged: positionCaret();

			function positionCaret(): void {
				if (readOnly) {
					vBar.setPosition(0);
					cursorPosition = 0;
				} else {
					vBar.setPosition(Math.floor(cursorPosition/length));
				}
			}
		} //TextArea
	} //ScrollView

	TPButton {
		id: btnClearText
		image: "edit-clear"
		enabled: _control.textControl.length > 0
		width: AppSettings.itemDefaultHeight
		height: width

		anchors {
			right: scrollArea.right
			bottom: scrollArea.bottom
			margins: 10
		}

		onClicked: {
			_control.clear();
			_control.textControl.forceActiveFocus();
		}
	}

	TPButton {
		id: btnShowToolBox
		image: "toolbox_"
		checkable: true
		width: AppSettings.itemDefaultHeight
		height: width

		anchors {
			right: btnClearText.left
			bottom: scrollArea.bottom
			margins: 10
		}

		onCheck: _control._show_toolbox = checked;
	}

	function clear() : void {
		_control.textControl.clear();
		_nFormatting = 0;
	}

	function formatChanged(added_format: bool) : void {
		if (added_format) {
			_nFormatting++;
		} else {
			_nFormatting--;
			if (_nFormatting < 0)
				_nFormatting = 0;
		}
		_control._modified = true;
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
} //Item
