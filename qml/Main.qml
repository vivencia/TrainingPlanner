pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Window
import QtQuick.Controls

import TpQml
import TpQml.Widgets
import TpQml.Dialogs
import TpQml.Pages

ApplicationWindow {
	id: mainwindow
	objectName: "mainWindow"
	width: AppSettings.windowWidth
	height: AppSettings.windowHeight
	visible: true
	title: "Training Planner"
	flags: Qt.platform.os === "android" ? Qt.Window | Qt.FramelessWindowHint : Qt.Window | Qt.CustomizeWindowHint & ~Qt.WindowMaximizeButtonHint

	signal saveFileChosen(filepath: string);
	signal tpFileOpenInquiryResult(do_import: bool);

	header: Loader {
		id: navBar
		active: AppUserModel.mainUserConfigured
		asynchronous: true
		sourceComponent: NavBar {
			pagesModel: ItemManager.AppPagesManager
		}
	}

	Loader {
		id: mainMenuLoader
		asynchronous: true
		active: AppUserModel.mainUserConfigured

		property MainMenu mainMenu

		sourceComponent: MainMenu {
			Component.onCompleted: mainMenuLoader.mainMenu = this;
		}
	}
	function openMainMenu(): void {
		if (!mainMenuLoader.mainMenu.visible)
			mainMenuLoader.mainMenu.open();
		else
			mainMenuLoader.mainMenu.close();
	}

	HomePage {
		id: homePage
		objectName: "homePage"
	}

	StackView {
		id: stackView
		objectName: "appStackView"
		initialItem: homePage
		anchors.fill: parent
	}

	function popFromStack(page: Item): void {
		if (page) {
			if (stackView.currentItem !== page) {
				let items = [];
				for (let i = 0; i < stackView.depth; ++i) {
					if (stackView.get(i) !== page)
						items.push(stackView.get(i));
				}
				stackView.clear();
				for (let x = 0; x < items.length; ++x)
					stackView.push(items[x], {}, StackView.Immediate);
			}
			else
				stackView.pop();
		}
		else
			stackView.pop();
	}

	function pushOntoStack(page: Item): void {
		if (stackView.currentItem === page)
			return;
		if (stackView.find((item, index) => { return item === page; }))
			stackView.popToItem(page);
		else
			stackView.push(page);
	}

	function clearWindowsStack(): void {
		stackView.clear();
	}

	Loader {
		id: importLoader
		asynchronous: true
		active: false

		property TPFileDialog _file_dialog

		sourceComponent: TPFileDialog {
			includeTextFilter: true

			onDialogClosed: (result) => {
				if (result === 0)
					AppUtils.viewOrOpenFile(AppUtils.getCorrectPath(currentFile));
				importLoader.active = false;
			}
			Component.onCompleted: importLoader._file_dialog = this;
		}

		onLoaded: _file_dialog.show();
	}
	function chooseFileToImport(): void {
		importLoader.active = true;
	}

	Loader {
		id: saveDialogLoader
		asynchronous: true
		active: false

		property string suggestedFileName;
		property TPFileDialog _file_dialog

		sourceComponent: TPFileDialog {
			saveDialog: true
			chooseDialog: false
			includeTextFilter: true

			onDialogClosed: (result) => {
				if (result === 0)
					mainwindow.saveFileChosen(AppUtils.getCorrectPath(currentFile));
				else
					mainwindow.saveFileChosen("");
				saveDialogLoader.active = false;
			}
			Component.onCompleted: saveDialogLoader._file_dialog = this;
		}

		onLoaded: {
			_file_dialog.suggestedName = suggestedFileName;
			_file_dialog.show();
		}
	}
	function chooseFolderToSave(filename: string): void {
		saveDialogLoader.suggestedFileName = filename;
		saveDialogLoader.active = true;
	}

	Loader {
		id: tpFileLoader
		asynchronous: true
		active: false

		property TPBalloonTip _dialog

		sourceComponent: TPBalloonTip {
			modal: true
			keepAbove: true
			parentPage: homePage

			onButton1Clicked: {
				mainwindow.tpFileOpenInquiryResult(true);
				tpFileLoader.active = false;
			}
			onButton2Clicked: {
				mainwindow.tpFileOpenInquiryResult(false);
				tpFileLoader.active = false;
			}
			Component.onCompleted: tpFileLoader._dialog = this;
		}

		onLoaded: _dialog.tpOpen();
	}
	function confirmTPFileOpening(type: string, details: string, image: string): void {
		tpFileLoader._dialog.title = qsTr("Import ") + type;
		tpFileLoader._dialog.message = details;
		tpFileLoader._dialog.imageSource = image;
		tpFileLoader.active = true;
	}
} //ApplicationWindow
