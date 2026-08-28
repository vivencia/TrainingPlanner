import QtQuick
import QtQuick.Controls

import TpQml

FocusScope {
	id: _control
	height: defaultHeight
	implicitHeight: height

	property string label
	property string matchAgainst: ""
	property bool showNotAllowableChars: false
	readonly property bool passwordOK: txtPassword.input_ok && txtPassword.match_oK
	readonly property int defaultHeight: 2 * AppSettings.itemDefaultHeight

	signal passwordEntered()

//private:
	readonly property string notAllowableChars: "# &?=\'\""

	TPLabel {
		id: lblPassword
		text: _control.label + (_control.showNotAllowableChars
													? "(" + _control.notAllowableChars + qsTr(" not allowed)") : "")
		visible: _control.label.length > 0 || _control.showNotAllowableChars
		height: visible ? AppSettings.itemDefaultHeight : 0

		anchors {
			top: parent.top
			topMargin: -5
			left: parent.left
			right: parent.right
		}
	}

	TPPasswordInput {
		id: txtPassword
		validator: RegularExpressionValidator { regularExpression: /^[^# &?="']*$/ }
		ToolTip.visible: !input_ok || !match_oK
		ToolTip.text: !input_ok ? AppUserModel.invalidPasswordLabel : !match_oK
																	? ToolTip.text = qsTr("Passwords do not match") : ""

		property bool match_oK: _control.matchAgainst.length === 0
		readonly property bool input_ok: text.length >= 6

		anchors {
			top: lblPassword.bottom
			left: parent.left
			right: parent.right
			margins: 5
		}

		onEnterOrReturnKeyPressed: {
			if (_control.passwordOK)
				_control.passwordEntered();
		}

		onTextEdited: {
			if (acceptableInput) {
				if (text.length >= 6) {
					if (_control.matchAgainst.length > 0)
						txtPassword.match_oK = (text === _control.matchAgainst);
				}
			}
		}
	}

	function reset(): void {
		txtPassword.clear();
		txtPassword.match_oK = Qt.binding(function() { return _control.matchAgainst.length === 0; });
	}

	function setPasswordText(passwd: string): void {
		if (passwd.length >= 6 || passwd.length === 0)
			txtPassword.text = passwd;
	}

	function getPassword(): string {
		return passwordOK ? txtPassword.text.trim() : "";
	}
}
