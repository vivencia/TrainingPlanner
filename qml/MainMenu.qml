import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Drawer {
	id: drawer
	height: mainwindow.height
	implicitWidth: mainwindow.width * 0.6
	spacing: 0
	padding: 0
	edge: Qt.LeftEdge
	property bool bMenuClicked: false

	//background: Rectangle {
	//	color: "#b9ecff"
	//}

	onClosed: {
		if (!bMenuClicked)
			mainMenuClosed();
	}

	onOpened: {
		mainMenuOpened();
	}

	ColumnLayout {
		id: drawerLayout
		spacing: 0

		anchors {
			fill: parent
			leftMargin: 5
			rightMargin: 5
			topMargin: 10
		}

		Label {
			id: label1
			Layout.maximumWidth: parent.width - 10
			Layout.leftMargin: 5
			topPadding: 20
			bottomPadding: 20
			horizontalAlignment: Qt.AlignHCenter
			wrapMode: Text.WordWrap
			//text: qsTr("Trainning day <b>#" + mesoTDay + "</b> of meso cycle <b>" + mesoName +
			//		"</b>: <b>" + mainDate.toDateString() + "</b> Division: <b>" + mesoSplitLetter + "</b>")
			font.pixelSize: AppSettings.fontSizeText
			visible: btnTrainingDay.visible
		}

		Rectangle {
			height: 3
			width: parent.width
			color: "black"
		}

		Label {
			text: qsTr("SETTINGS")
		}

		ButtonFlat {
			id: btnSettingsExDB
			text: qsTr("Exercises Database")
			font.pixelSize: AppSettings.fontSizeText
			imageSource: "qrc:/images/"+lightIconFolder+"back.png"
			imageMirror: true
			onClicked: { stackView.push("ExercisesDatabase.qml"); menuClicked(); }
			transform: Translate {
				x: drawer.position * 0.2
			}
		}

		ButtonFlat {
			id: btnSettingsTheme
			text: qsTr("Theme")
			font.pixelSize: AppSettings.fontSizeText
			imageSource: "qrc:/images/"+lightIconFolder+"back.png"
			imageMirror: true
			onClicked: { stackView.push("ThemeSettingsPage.qml"); menuClicked(); }
			transform: Translate {
				x: drawer.position * 0.2
			}
		}

		ButtonFlat {
			id: btnSettingsLanguage
			text: qsTr("Language")
			font.pixelSize: AppSettings.fontSizeText
			imageSource: "qrc:/images/"+lightIconFolder+"back.png"
			imageMirror: true
			onClicked: { stackView.push("LanguageSettingsPage.qml", {} ); menuClicked(); }
			transform: Translate {
				x: drawer.position * 0.2
			}
		}

		ButtonFlat {
			id: btnSettingsFont
			text: qsTr("Fonts")
			font.pixelSize: AppSettings.fontSizeText
			imageSource: "qrc:/images/"+lightIconFolder+"back.png"
			imageMirror: true
			onClicked: { stackView.push("FontSizePage.qml"); menuClicked(); }
			transform: Translate {
				x: drawer.position * 0.2
			}
		}

		ButtonFlat {
			id: btnSettingsDev
			text: qsTr("Developer Options")
			font.pixelSize: AppSettings.fontSizeText
			imageSource: "qrc:/images/"+lightIconFolder+"back.png"
			imageMirror: true
			onClicked: { stackView.push("DevSettingsPage.qml"); menuClicked(); }
			transform: Translate {
				x: drawer.position * 0.2
			}
		}

		Rectangle {
			height: 3
			width: parent.width
			color: "black"
		}

		ButtonFlat {
			id: btnTrainingDay
			text: qsTr("Training")
			font.pixelSize: AppSettings.fontSizeText
			imageSource: "qrc:/images/"+lightIconFolder+"back.png"
			imageMirror: true
			onClicked: { dayStack.currentIndex = 0; }
			transform: Translate {
				x: drawer.position * 0.2
			}

			visible: {
				// Force the binding to re-evaluate so that the title check is run each time the page changes.
				stackView.currentItem
				stackView.find((item, index) => { return item.title === "mealsPage"; })
			}
		}

		ButtonFlat {
			text: qsTr("Meals")
			font.pixelSize: AppSettings.fontSizeText
			imageSource: "qrc:/images/"+lightIconFolder+"back.png"
			imageMirror: true
			onClicked: { dayStack.currentIndex = 1; }
			transform: Translate {
				x: drawer.position * 0.2
			}
			visible: {
				// Force the binding to re-evaluate so that the title check is run each time the page changes.
				stackView.currentItem
				stackView.find((item, index) => { return item.title === "trainingPage"; })
			}
		}

		Item { // spacer item
			Layout.fillWidth: true
			Layout.fillHeight: true
		}
	} //ColumnLayout

	Component.onCompleted: {
		mainwindow.backButtonPressed.connect(maybeRestore);
	}

	function menuClicked() {
		bMenuClicked = true;
		close ();
	}

	function maybeRestore() {
		if (!drawer.visible && bMenuClicked) {
			drawer.open();
			bMenuClicked = false;
		}
	}
} //Drawer
