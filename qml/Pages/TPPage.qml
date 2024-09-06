import QtQuick
import QtQuick.Controls

import "../"
import com.vivenciasoftware.qmlcomponents

Page {
	id: tpPage
	width: windowWidth
	height: windowHeight

	readonly property int headerHeight: 45
	readonly property int footerHeight: 55

	signal pageActivated();
	signal pageDeActivated();

	Component.onCompleted: {
		mainwindow.pageActivated_main.connect(pageActivation);
		mainwindow.pageDeActivated_main.connect(pageDeActivation);
	}

	TPImage {
		anchors.fill: parent
		source: "app_logo"
		dropShadow: false
		opacity: 0.6
	}

	background: Rectangle {
		color: AppSettings.primaryDarkColor
		opacity: 0.7
	}

	function pageDeActivation(page: Item) {
		if (page !== null) {
			if (page.objectName === tpPage.objectName)
				pageDeActivated();
		}
	}

	function pageActivation(page: Item) {
		if (page !== null) {
			if (page.objectName === tpPage.objectName)
				pageActivated();
		}
	}
}
