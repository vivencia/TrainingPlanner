cmake_minimum_required(VERSION 3.16)
project(trainingplanner LANGUAGES CXX)

set(CMAKE_CXX_FLAGS_DEBUG_INIT "-DDEBUG -g")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "QT_NO_DEBUG -O3 -march=native -fomit-frame-pointer -funroll-loops -Ofast")
set(CMAKE_CXX_FLAGS_INIT "-DQT_USE_QSTRINGBUILDER -DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS -Werror -Wall -Wextra -pedantic -std=c++14 -finput-charset=UTF-8 -fexec-charset=UTF-8")

set(CMAKE_AUTOMOC ON)
set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_AUTOGEN_VERBOSE on)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Multimedia Qml Quick QuickControls2 Widgets Sql LinguistTools)
add_compile_definitions(REQUEST_PERMISSIONS_ON_ANDROID QT_USE_QSTRINGBUILDER QT_USE_FAST_CONCATENATION QT_USE_FAST_OPERATOR_PLUS DEBUG)
#add_compile_options(-std=c++14 -Werror -Wall -Wextra -pedantic)

set( RESOURCE_FILES app_resources.qrc )

set( SOURCES
	cxx/main.cpp

	cxx/translationclass.cpp cxx/translationclass.h
	cxx/runcommands.cpp cxx/runcommands.h
	cxx/dbmanager.h cxx/dbmanager.cpp
	cxx/tpmesocycleclass.h cxx/tpmesocycleclass.cpp
	cxx/tptimer.h cxx/tptimer.cpp
	cxx/urihandler.h cxx/urihandler.cpp

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
)

set( JAVASOURCES
    java/TPActivity.java
    java/QShareUtils.java
    java/QSharePathResolver.java
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

set( QML_FILES
    "qml/AppSettings.qml"
    "qml/ImageViewer.qml"
    "qml/inexportMethods.js"
    "qml/jsfunctions.js"
    "qml/main.qml"
    "qml/VideoPlayer.qml"
    "qml/Dialogs/CalendarDialog.qml"
    "qml/Dialogs/DatePicker.qml"
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
    "qml/Pages/BackupPage.qml"
    "qml/Pages/ExercisesDatabase.qml"
    "qml/Pages/ExercisesPlanner.qml"
    "qml/Pages/HomePage.qml"
    "qml/Pages/MesoContent.qml"
    "qml/Pages/MesoCycle.qml"
    "qml/Pages/MesoSplitPlanner.qml"
    "qml/Pages/OpenEndedPlan.qml"
    "qml/Pages/SettingsPage.qml"
    "qml/Pages/TPPage.qml"
    "qml/Pages/TrainingDayInfo.qml"
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
    "qml/TPWidgets/TPImportMessageBox.qml"
    "qml/TPWidgets/TPPopup.qml"
    "qml/TPWidgets/TPRadioButton.qml"
    "qml/TPWidgets/TPSaveDialog.qml"
    "qml/TPWidgets/TPRoundButton.qml"
    "qml/TPWidgets/TPTextInput.qml"
    "qml/TPWidgets/ComplexDialogModules/TPAdjustMesoCalendarFrame.qml"
    "qml/TPWidgets/ComplexDialogModules/TPTDayIntentGroup.qml"
    "qml/TPWidgets/ComplexDialogModules/TPDialogWithMessageAndCheckBox.qml"
)

qt_add_resources(${RESOURCE_FILES})
qt_add_executable(trainingplanner
	${SOURCES}
	${JAVASOURCES}
	${QML_FILES}
	${RESOURCE_FILES}
	${QM_FILES}
)

target_link_libraries(trainingplanner PUBLIC
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
	${QML_FILES}
)

qt_add_resources(trainingplanner "trainingplanner" images.qrc sounds.qrc
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

#qt_add_qml_module(trainingplanner
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

install(TARGETS trainingplanner
	RUNTIME DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}"
	BUNDLE DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}"
	LIBRARY DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}"
)

set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")

set_property(TARGET trainingplanner APPEND PROPERTY
    QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android
)
