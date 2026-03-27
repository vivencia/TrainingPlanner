set( QML_DIALOGS
	"CalendarDialog.qml"
	"FirstTimeDialog.qml"
	"MainMenu.qml"
	"MuscularGroupPicker.qml"
	"NavBar.qml"
	"OnlineMessages.qml"
	"PasswordDialog.qml"
	"TimePicker.qml"
	"TimerDialog.qml"
	"TPFileDialog.qml"
	"TimePickerElements/ClockButton.qml"
)

qt_add_library(tp_dialogs_module STATIC)
qt_add_qml_module(tp_dialogs_module
	URI TpQml.Dialogs
	VERSION 1.0
	QML_FILES
		${QML_DIALOGS}
	RESOURCE_PREFIX
		"/"
)
