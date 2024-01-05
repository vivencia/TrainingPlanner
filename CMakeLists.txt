cmake_minimum_required(VERSION 3.16)
project(trainingplanner LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Multimedia Qml Quick QuickControls2)

add_compile_definitions(REQUEST_PERMISSIONS_ON_ANDROID QT_USE_QSTRINGBUILDER QT_USE_FAST_CONCATENATION QT_USE_FAST_OPERATOR_PLUS)

set( CMAKE_VERBOSE_MAKEFILE on )

set(RESOURCE_FILES images.qrc sounds.qrc)
QT6_ADD_RESOURCES(PROJECT_RESOURCES ${RESOURCE_FILES})

qt_add_executable(trainingplanner
	cxx/main.cpp
	cxx/backupclass.cpp cxx/backupclass.h
	cxx/translationclass.cpp cxx/translationclass.h
	${PROJECT_RESOURCES}
)

#set_target_properties(trainingplanner PROPERTIES
#	WIN32_EXECUTABLE TRUE
#	MACOSX_BUNDLE TRUE
#	MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
#)

#target_compile_definitions(trainingplanner PUBLIC
#	FREQUENCYMONITOR_SUPPORT
#	PERFORMANCEMONITOR_SUPPORT
#)

target_link_libraries(trainingplanner PUBLIC
	Qt::Core
	Qt::Gui
	Qt::Multimedia
	Qt::Qml
	Qt::Quick
	Qt6::QuickControls2
)

set(trainingplanner_resource_files
	"images/app_logo.png"
	"images/app_icon.png"
	"images/no_image.jpg"
	"images.qrc"
	"sounds.qrc"
	"translations.qrc"
	"qml/main.qml"
	"qml/AppSettings.qml"
	"qml/Database.qml"
	"qml/FontSizePage.qml"
	"qml/HomePage.qml"
	"qml/NavBar.qml"
	"qml/DatePicker.qml"
	"qml/TimePicker.qml"
	"qml/CalendarDialog.qml"
	"qml/MesoCycle.qml"
	"qml/MesoContent.qml"
	"qml/jsfunctions.js"
	"qml/ButtonFlat.qml"
	"qml/ButtonIconActive.qml"
	"qml/IconActive.qml"
	"qml/ExerciseEntry.qml"
	"qml/ExercisesDatabase.qml"
	"qml/TimerDialog.qml"
	"qml/SetTypeRegular.qml"
	"qml/SetTypeCluster.qml"
	"qml/SetTypeDrop.qml"
	"qml/SetTypeGiant.qml"
	"qml/SetTypeMyoReps.qml"
	"qml/SetTypePyramid.qml"
	"qml/DevSettingsPage.qml"
	"qml/ThemeSettingsPage.qml"
	"qml/RepsAndWeightRow.qml"
	"qml/FloatingButton.qml"
	"qml/MainMenu.qml"
	"qml/MealsDayInfo.qml"
	"qml/TrainingDayInfo.qml"
	"qml/TrainingDayProgressDialog.qml"
	"qml/SetInputField.qml"
	"qml/LanguageSettingsPage.qml"
	"qml/PageScrollButtons.qml"
	"qml/ImageViewer.qml"
	"qml/VideoPlayer.qml"
	"qml/qmldir"
)

qt_add_resources(trainingplanner "trainingplanner" images.qrc sounds.qrc
	PREFIX
	    "/"
	FILES
	    ${trainingplanner_resource_files}
)

install(TARGETS trainingplanner
	RUNTIME DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}"
	BUNDLE DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}"
	LIBRARY DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}"
)

set_property(TARGET trainingplanner APPEND PROPERTY
    QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android
)
