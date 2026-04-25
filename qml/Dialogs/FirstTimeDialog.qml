import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import TpQml
import TpQml.User
import TpQml.Widgets

TPPopup {
	id: _firstTimeDlg
	modal: true
	keepAbove: true
	showTitleBar: false
	closeButtonVisible: false
	width: AppSettings.pageWidth - 50
	height: stackLayout.childrenRect.height

	readonly property int minimumHeight: AppSettings.windowHeight * 0.5
	property bool nextStartsTheApp: false
	property alias parentPage: _parent_page

	StackLayout {
		id: stackLayout

		anchors {
			left: parent.left
			right: parent.right
			top: parent.top
			margins: 5
		}

		UserLanguage {
			id: language_module
			Layout.fillWidth: true
			Layout.preferredHeight: _firstTimeDlg.minimumHeight
		}

		UserWelcome {
			id: welcome_module
			Layout.fillWidth: true
			Layout.preferredHeight: _firstTimeDlg.minimumHeight
		}

		UserExistingFromNet {
			id: existing_user_module
			Layout.fillWidth: true
			Layout.minimumHeight: _firstTimeDlg.minimumHeight

			onNetConfigurationResult: (success) => _firstTimeDlg.nextStartsTheApp = success;
		}

		UserPersonalData {
			id: personal_module
			userRow: 0
			parentPage: _firstTimeDlg.parentPage
			Layout.fillWidth: true
			Layout.minimumHeight: _firstTimeDlg.minimumHeight
		}

		UserContact {
			id: contact_module
			userRow: 0
			Layout.fillWidth: true
			Layout.minimumHeight: _firstTimeDlg.minimumHeight
		}

		UserCoach {
			id: coach_module
			userRow: 0
			parentPage: _firstTimeDlg.parentPage
			Layout.fillWidth: true
			Layout.minimumHeight: _firstTimeDlg.minimumHeight
		}

		UserProfile {
			id: profile_module
			userRow: 0
			parentPage: _firstTimeDlg.parentPage
			Layout.fillWidth: true
			Layout.minimumHeight: _firstTimeDlg.minimumHeight
		}

		UserReady {
			Layout.fillWidth: true
			Layout.fillHeight: true
			Layout.preferredHeight:_firstTimeDlg. minimumHeight
		}

		Component.onCompleted: currentIndex = AppSettings.userLocale.length > 0 ? 1 : 0;
	}

	Frame {
		id: frmFooter
		height: btnPrev.height + 10
		background: Rectangle {
			border.width: 0
			radius: 8
		}

		anchors {
			left: parent.left
			right: parent.right
			top: stackLayout.bottom
		}

		TPButton {
			id: btnPrev
			text: qsTr("Previous")
			imageSource: "back.png"
			hasDropShadow: false
			iconOnTheLeft: true
			autoSize: true
			enabled: stackLayout.currentIndex > 0

			anchors {
				right: btnNext.left
				rightMargin: 20
				verticalCenter: parent.verticalCenter
			}

			onClicked: stackLayout.currentIndex--;
		}

		TPButton {
			id: btnNext
			text: stackLayout.currentIndex < stackLayout.count - 1 ? qsTr("Next") : qsTr("Conclude")
			imageSource: "next.png"
			hasDropShadow: false
			autoSize: true
			enabled: stackLayout.currentIndex < stackLayout.count ? _firstTimeDlg.isModuleReady(stackLayout.currentIndex) : false

			anchors {
				right: parent.right
				verticalCenter: parent.verticalCenter
			}

			onClicked: {
				if (stackLayout.currentIndex === stackLayout.count - 1)
					_firstTimeDlg.finish();
				if (_firstTimeDlg.nextStartsTheApp)
					_firstTimeDlg.finish();
				else {
					stackLayout.currentIndex++;
					_firstTimeDlg.focusOnNextModuleFirstField(stackLayout.currentIndex);
				}
			}
		}
	}

	function backKeyPressed(): void {
		if (stackLayout.currentIndex > 0)
			stackLayout.currentIndex--;
	}

	function finish(): void {
		AppUserModel.setMainUserConfigurationFinished();
		closePopup();
	}

	function isModuleReady(index: int): bool {
		let ready = false;
		switch (index) {
		case 0: ready = language_module.bReady; break;
		case 1: ready = welcome_module.bReady; break;
		case 2: ready = existing_user_module.bReady; break;
		case 3: ready = personal_module.bReady; break;
		case 4: ready = contact_module.bReady; break;
		case 5: ready = coach_module.bReady; break;
		case 6: ready = profile_module.bReady; break;
		default: break;
		}
		return ready;
	}

	function focusOnNextModuleFirstField(index: int): void {
		switch (index) {
		case 3: personal_module.focusOnFirstField(); break;
		case 4: contact_module.focusOnFirstField(); break;
		case 5: coach_module.focusOnFirstField(); break;
		case 6: profile_module.focusOnFirstField(); break;
		default: break;
		}
	}
}
