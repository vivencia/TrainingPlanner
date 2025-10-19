import QtQuick
import QtQuick.Controls
import QtQuick.Shapes

import "../"
import "../TPWidgets"

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

Page {
	id: tpPage
	width: appSettings.pageWidth
	height: appSettings.pageHeight

	property color colorLight: appSettings.primaryLightColor
	property color colorDark: appSettings.primaryDarkColor
	readonly property int headerHeight: 0.08 * height
	readonly property int footerHeight: 0.10 * height

	signal pageActivated();
	signal pageDeActivated();

	Component.onCompleted: {
		mainwindow.pageActivated_main.connect(pageActivation);
		mainwindow.pageDeActivated_main.connect(pageDeActivation);
	}

	background: TPBackRec {
		useImage: true
		widthScale: 1.05
		heightScale: 1.3
		sourceImage: ":/images/backgrounds/backimage4.jpg"
	}

	function pageDeActivation(page: Item): void {
		if (page !== null) {
			if (page.objectName === tpPage.objectName)
				pageDeActivated();
		}
	}

	function pageActivation(page: Item): void {
		if (page !== null) {
			if (page.objectName === tpPage.objectName)
				pageActivated();
		}
	}
}
