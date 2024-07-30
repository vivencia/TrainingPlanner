import QtQuick
import QtQuick.Controls

import "../"

Page {
	id: tpPage
	width: windowWidth
	height: windowHeight

	readonly property int headerHeight: 45
	readonly property int footerHeight: 55

	signal pageActivated();
	signal pageDeActivated();

	Component.onCompleted: {
		tpPage.StackView.onDeactivating.connect(pageDeActivation);
		tpPage.StackView.activating.connect(pageActivation);
	}

	Image {
		anchors.fill: parent
		source: "qrc:/images/app_logo.png"
		fillMode: Image.PreserveAspectFit
		asynchronous: true
		opacity: 0.6
	}

	background: Rectangle {
		color: AppSettings.primaryDarkColor
		opacity: 0.7
	}

	function pageDeActivation() {
		pageDeActivated();
	}

	function pageActivation() {
		pageActivated();
	}
}
