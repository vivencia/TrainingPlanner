import QtQuick
import QtQuick.Controls

TPTextInput {
	id: control
	heightAdjustable: false
	showClearTextButton: true
	echoMode: btnShowHidePassword.show ? TextInput.Normal : TextInput.Password
	inputMethodHints: Qt.ImhSensitiveData|Qt.ImhNoPredictiveText|Qt.ImhNoAutoUppercase
	rightPadding: defaultPadding + btnShowHidePassword.width + 5
	focus: true

	property bool inputOK: false
	property bool matchOK: true

	TPButton {
		id: btnShowHidePassword
		imageSource: show ? "hide-password.png" : "show-password.png"
		hasDropShadow: false
		width: userSettings.itemDefaultHeight
		height: width
		focus: false

		property bool show: false

		anchors {
			right: control.right
			rightMargin: control.defaultPadding + 5
			verticalCenter: control.verticalCenter
		}

		onClicked: {
			show = !show;
			control.forceActiveFocus();
		}
	}
}
