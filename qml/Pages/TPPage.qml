import QtQuick
import QtQuick.Controls

import "../"
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

Page {
	id: tpPage
	width: appSettings.pageWidth
	height: appSettings.pageHeight

	readonly property int headerHeight: 0.08*appSettings.pageHeight
	readonly property int footerHeight: 0.10*appSettings.pageHeight

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
		color: appSettings.primaryDarkColor
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
