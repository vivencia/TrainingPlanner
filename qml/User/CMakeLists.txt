set( QML_USER
	"AllUsers.qml"
	"AvatarsPopup.qml"
	"ChatWindow.qml"
	"UserChangePassword.qml"
	"TPCoachesAndClientsList.qml"
	"UserCoachRequest.qml"
	"UserCoach.qml"
	"UserContact.qml"
	"UserExistingFromNet.qml"
	"UserLanguage.qml"
	"UserPersonalData.qml"
	"UserProfile.qml"
	"UserReady.qml"
	"UserWelcome.qml"
	"AllUsersElements/TableDelegate.qml"
)

qt_add_library(tp_user_module STATIC)
qt_add_qml_module(tp_user_module
	URI TpQml.User
	VERSION 1.0
	QML_FILES
		${QML_USER}
	RESOURCE_PREFIX
		"/"
)
