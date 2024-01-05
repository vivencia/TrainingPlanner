TEMPLATE = app
TARGET = trainingplanner

QT += core gui quick quickcontrols2 multimedia
android: qtHaveModule(androidextras) {
	QT += androidextras
	DEFINES += REQUEST_PERMISSIONS_ON_ANDROID
}

DEFINES += \
	QT_USE_QSTRINGBUILDER \
	QT_USE_FAST_CONCATENATION \
	QT_USE_FAST_OPERATOR_PLUS

SOURCES += \
	cxx/main.cpp \
	cxx/backupclass.cpp \
	cxx/translationclass.cpp

HEADERS += \
	cxx/backupclass.h \
	cxx/translationbackupclass.h

resources.files = \
	images/app_logo.png \
	images/app_icon.png \
	image/no_image.jpg \
	resources/images.qrc \
	resources/sounds.qrc \
	resources/translations.qrc \
	qml/main.qml \
	qml/AppSettings.qml \
	qml/Database.qml \
	qml/FontSizePage.qml \
	qml/HomePage.qml \
	qml/NavBar.qml \
	qml/DatePicker.qml \
	qml/TimePicker.qml \
	qml/CalendarDialog.qml \
	qml/MesoCycle.qml \
	qml/MesoContent.qml \
	qml/jsfunctions.js \
	qml/ButtonFlat.qml \
	qml/ButtonIconActive.qml \
	qml/IconActive.qml \
	qml/ExerciseEntry.qml \
	qml/ExercisesDatabase.qml \
	qml/TimerDialog.qml \
	qml/SetTypeRegular.qml \
	qml/SetTypeCluster.qml \
	qml/SetTypeDrop.qml \
	qml/SetTypeGiant.qml \
	qml/SetTypeMyoReps.qml \
	qml/SetTypePyramid.qml \
	qml/DevSettingsPage.qml \
	qml/ThemeSettingsPage.qml \
	qml/RepsAndWeightRow.qml \
	qml/FloatingButton.qml \
	qml/MainMenu.qml \
	qml/MealsDayInfo.qml \
	qml/TrainingDayInfo.qml \
	qml/TrainingDayProgressDialog.qml \
	qml/SetInputField.qml \
	qml/LanguageSettingsPage.qml \
	qml/PageScrollButtons.qml \
	qml/ImageViewer.qml \
	qml/VideoPlayer.qml \
	qml/qmldir

resources.prefix = /

RESOURCES += resources
TRANSLATIONS += \
	translations/tplanner.pt_BR.qm \
	translations/tplanner.de_DE.qm

target.path = $$PWD
INSTALLS += target
