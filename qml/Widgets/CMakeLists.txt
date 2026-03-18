set( QML_WIDGETS
	"TPBackRec.qml"
	"TPBalloonTip.qml"
	"TPButton.qml"
	"TPButtonGroup.qml"
	"TPFloatingControl.qml"
	"TPColorRectangle.qml"
	"TPComboBox.qml"
	"TPDatePicker.qml"
	"TPDigitalClock.qml"
	"TPFileViewer.qml"
	"TPFloatingMenuBar.qml"
	"TPLabel.qml"
	"TPListView.qml"
	"TPMouseArea.qml"
	"TPMultiLineEdit.qml"
	"TPMuscularGroupsList.qml"
	"TPPassword.qml"
	"TPPasswordInput.qml"
	"TPPopup.qml"
	"TPPhoneNumberInput.qml"
	"TPRadioButtonOrCheckBox.qml"
	"TPScrollView.qml"
	"TPTabButton.qml"
	"TPTextInput.qml"
	"TPToolBar.qml"
	"TPVideoPlayer.qml"
)

qt_add_library(tp_widgets_module STATIC)
qt_add_qml_module(tp_widgets_module
	URI TpQml.Widgets
	VERSION 1.0
	QML_FILES
		${QML_WIDGETS}
	RESOURCE_PREFIX
		"/"
)
