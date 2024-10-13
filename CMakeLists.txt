cmake_minimum_required(VERSION 3.16)
project(trainingplanner LANGUAGES CXX)

set(CMAKE_CXX_FLAGS_DEBUG_INIT "-DDEBUG -g")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-DQT_NO_DEBUG -DNDEBUG -O3 -march=native -fomit-frame-pointer -funroll-loops -Ofast")
set(CMAKE_CXX_FLAGS_INIT "-DQT_USE_QSTRINGBUILDER -DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS -Werror -Wall -Wextra -pedantic finput-charset=UTF-8 -fexec-charset=UTF-8")

set(CMAKE_AUTOMOC ON)
set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_AUTOGEN_VERBOSE on)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Multimedia Qml Quick QuickControls2 Widgets Sql LinguistTools)
add_compile_definitions(REQUEST_PERMISSIONS_ON_ANDROID)
add_compile_options(-std=c++20 ${CMAKE_CXX_FLAGS_INIT})

set( RESOURCE_FILES app_resources.qrc )

set( CXX_SOURCES
	cxx/main.cpp
	cxx/tpglobals.h

	cxx/tpappcontrol.h cxx/tpappcontrol.cpp
	cxx/translationclass.cpp cxx/translationclass.h
	cxx/tputils.cpp cxx/tputils.h
	cxx/dbinterface.h cxx/dbinterface.cpp
	cxx/qmlitemmanager.h cxx/qmlitemmanager.cpp
	cxx/osinterface.h cxx/osinterface.cpp
	cxx/tptimer.h cxx/tptimer.cpp
	cxx/tpsettings.h cxx/tpsettings.cpp

	cxx/tpimageprovider.h cxx/tpimageprovider.cpp
	cxx/tpimage.h cxx/tpimage.cpp
	cxx/tpandroidnotification.h cxx/tpandroidnotification.cpp

	cxx/tpdatabasetable.h cxx/tpdatabasetable.cpp
	cxx/dbexercisestable.h cxx/dbexercisestable.cpp
	cxx/dbmesocylestable.h cxx/dbmesocylestable.cpp
	cxx/dbmesosplittable.h cxx/dbmesosplittable.cpp
	cxx/dbmesocalendartable.h cxx/dbmesocalendartable.cpp
	cxx/dbtrainingdaytable.h cxx/dbtrainingdaytable.cpp
	cxx/dbusertable.h cxx/dbusertable.cpp

	cxx/tplistmodel.h cxx/tplistmodel.cpp
	cxx/dbexercisesmodel.h cxx/dbexercisesmodel.cpp
	cxx/dbmesocyclesmodel.h cxx/dbmesocyclesmodel.cpp
	cxx/dbmesosplitmodel.h cxx/dbmesosplitmodel.cpp
	cxx/dbmesocalendarmodel.h cxx/dbmesocalendarmodel.cpp
	cxx/dbtrainingdaymodel.h cxx/dbtrainingdaymodel.cpp
	cxx/dbusermodel.h cxx/dbusermodel.cpp

	cxx/qmluserinterface.h cxx/qmluserinterface.cpp
	cxx/qmlexercisesdatabaseinterface.h cxx/qmlexercisesdatabaseinterface.cpp
	cxx/qmlmesointerface.h cxx/qmlmesointerface.cpp
	cxx/qmlmesocalendarinterface.h cxx/qmlmesocalendarinterface.cpp
	cxx/qmlmesosplitinterface.h cxx/qmlmesosplitinterface.cpp
	cxx/qmltdayinterface.h cxx/qmltdayinterface.cpp
	cxx/qmlsetsinterface.h cxx/qmlsetsinterface.cpp
	cxx/qmlexerciseinterface.h cxx/qmlexerciseinterface.cpp
	cxx/qmlexerciseentry.h cxx/qmlexerciseentry.cpp
	cxx/qmlsetentry.h cxx/qmlsetentry.cpp
)

set( JAVA_SOURCES
    java/TPActivity.java
    java/QShareUtils.java
    java/QSharePathResolver.java
    java/NotificationClient.java
    java/TPBroadcastReceiver.java
)

set( TRANSLATIONS_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/translations"
)
set( TS_FILES
    ${TRANSLATIONS_DIR}/tplanner.pt_BR.ts
    ${TRANSLATIONS_DIR}/tplanner.de_DE.ts
)
set( QM_FILES
    ${TRANSLATIONS_DIR}/tplanner.pt_BR.qm
    ${TRANSLATIONS_DIR}/tplanner.de_DE.qm
)

set( QML_SOURCES
    "qml/AppGlobals.qml"
    "qml/ImageViewer.qml"
    "qml/main.qml"
    "qml/VideoPlayer.qml"
    "qml/Dialogs/CalendarDialog.qml"
    "qml/Dialogs/DatePicker.qml"
    "qml/Dialogs/FirstTimeDialog.qml"
    "qml/Dialogs/ImportDialog.qml"
    "qml/Dialogs/MainMenu.qml"
    "qml/Dialogs/NavBar.qml"
    "qml/Dialogs/TimePicker.qml"
    "qml/Dialogs/TimerDialog.qml"
    "qml/ExercisesAndSets/ExerciseEntry.qml"
    "qml/ExercisesAndSets/ExerciseNameField.qml"
    "qml/ExercisesAndSets/ExercisesListView.qml"
    "qml/ExercisesAndSets/PageScrollButtons.qml"
    "qml/ExercisesAndSets/RepsAndWeightRow.qml"
    "qml/ExercisesAndSets/SetInputField.qml"
    "qml/ExercisesAndSets/SetNotesField.qml"
    "qml/ExercisesAndSets/SetTypeDrop.qml"
    "qml/ExercisesAndSets/SetTypeRegular.qml"
    "qml/ExercisesAndSets/SetTypeGiant.qml"
    "qml/ExercisesAndSets/SimpleExercisesListPanel.qml"
    "qml/Pages/ClientsOrCoachesPage.qml"
    "qml/Pages/ConfigurationPage.qml"
	"qml/Pages/ExercisesPage.qml"
    "qml/Pages/ExercisesPlanner.qml"
    "qml/Pages/HomePage.qml"
	"qml/Pages/MesoCalendar.qml"
    "qml/Pages/MesoCycle.qml"
    "qml/Pages/MesoSplitPlanner.qml"
    "qml/Pages/MesoSplitSetup.qml"
    "qml/Pages/SettingsPage.qml"
    "qml/Pages/TPPage.qml"
    "qml/Pages/TrainingDayInfo.qml"
    "qml/Pages/UserPage.qml"
    "qml/TPWidgets/TPBalloonTip.qml"
    "qml/TPWidgets/TPButton.qml"
    "qml/TPWidgets/TPButtonImage.qml"
    "qml/TPWidgets/TPFloatingButton.qml"
    "qml/TPWidgets/TPFloatingControl.qml"
    "qml/TPWidgets/TPCheckBox.qml"
    "qml/TPWidgets/TPColorRectangle.qml"
    "qml/TPWidgets/TPComboBox.qml"
    "qml/TPWidgets/TPComplexDialog.qml"
    "qml/TPWidgets/TPDigitalClock.qml"
    "qml/TPWidgets/TPFloatingMenuBar.qml"
    "qml/TPWidgets/TPGroupBox.qml"
    "qml/TPWidgets/TPImportDialog.qml"
    "qml/TPWidgets/TPLabel.qml"
    "qml/TPWidgets/TPMouseArea.qml"
    "qml/TPWidgets/TPPopup.qml"
    "qml/TPWidgets/TPRadioButton.qml"
    "qml/TPWidgets/TPSaveDialog.qml"
    "qml/TPWidgets/TPTextInput.qml"
    "qml/TPWidgets/ComplexDialogModules/TPAdjustMesoCalendarFrame.qml"
    "qml/TPWidgets/ComplexDialogModules/TPTDayIntentGroup.qml"
    "qml/TPWidgets/ComplexDialogModules/TPDialogWithMessageAndCheckBox.qml"
    "qml/User/AvatarsPopup.qml"
    "qml/User/UserCoach.qml"
    "qml/User/UserContact.qml"
    "qml/User/UserLanguage.qml"
    "qml/User/UserPersonalData.qml"
    "qml/User/UserProfile.qml"
    "qml/User/UserReady.qml"
    "qml/User/UserWelcome.qml"
)

qt_add_resources(${RESOURCE_FILES})
qt_add_executable(TrainingPlanner
	${CXX_SOURCES}
	${JAVA_SOURCES}
	${QML_SOURCES}
	${RESOURCE_FILES}
	${QM_FILES}
)

target_link_libraries(TrainingPlanner PUBLIC
	Qt::Core
	Qt::Gui
	Qt::Multimedia
	Qt::Widgets
	Qt::Sql
	Qt6::Qml
	Qt6::Quick
	Qt6::QuickControls2
	Qt6::CorePrivate
)

set(trainingplanner_resource_files
	"images/app_logo.png"
	"images/app_icon.png"
	"images/no_image.jpg"
	"app_resources.qrc"
	"qml/qmldir"
	${QML_SOURCES}
)

qt_add_resources(TrainingPlanner "TrainingPlanner"
	PREFIX
	    "/"
	FILES
	    ${trainingplanner_resource_files}
)

#qt_add_qml_module(trainingplanner
#    URI "tp_qml"
#    VERSION 1.0
#    QML_FILES
#	${QML_FILES}
#    SOURCES
#	${SOURCES}
#)

list(APPEND QML_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/cxx")
list(APPEND QML_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/qml")
set(QML_IMPORT_PATH
    ${QML_DIRS}
)

#qt_add_qml_module(TrainingPlanner
#    URI "Graph"
#    VERSION 1.0
#    QML_FILES
#	qml/GraphicsViewer.qml
#    SOURCES
#	cxx/graph.h cxx/graph.cpp
#    DEPENDENCIES QtQuick
#)

#target_sources(trainingplanner_other_files
#  PRIVATE
#    qml/qmldir
#)

install(TARGETS TrainingPlanner
	RUNTIME DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}"
	BUNDLE DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}"
	LIBRARY DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}"
)

set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")

set_property(TARGET TrainingPlanner APPEND PROPERTY
    QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android
)
