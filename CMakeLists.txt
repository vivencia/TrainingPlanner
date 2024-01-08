cmake_minimum_required(VERSION 3.16)
project(trainingplanner LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

#if (${ANDROID_ABI})
#    find_package(Qt6 REQUIRED COMPONENTS Core Gui Multimedia Qml Quick QuickControls2 Widgets Sql AndroidExtras)
#else ()
    find_package(Qt6 REQUIRED COMPONENTS Core Gui Multimedia Qml Quick QuickControls2 Widgets Sql LinguistTools)

find_package(Qt6 REQUIRED COMPONENTS )
add_compile_definitions(REQUEST_PERMISSIONS_ON_ANDROID QT_USE_QSTRINGBUILDER QT_USE_FAST_CONCATENATION QT_USE_FAST_OPERATOR_PLUS)

set( CMAKE_VERBOSE_MAKEFILE on )

set(CMAKE_AUTORCC ON)
set( RESOURCE_FILES app_resources.qrc )

set( SOURCES
	cxx/main.cpp
	cxx/backupclass.cpp cxx/backupclass.h
	cxx/translationclass.cpp cxx/translationclass.h
	cxx/runcommands.cpp cxx/runcommands.h
	cxx/dbmanager.h cxx/dbmanager.cpp
)

qt_add_resources(${RESOURCE_FILES})
qt_add_executable(trainingplanner
	${SOURCES}
	${RESOURCE_FILES}
)

set( TS_FILES
    translations/tplanner.pt_BR.ts
    translations/tplanner.de_DE.ts
)
set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION ${PROJECT_BINARY_DIR}/translations)

qt_add_lupdate(trainingplanner TS_FILES ${TS_FILES})
qt_add_lrelease(trainingplanner TS_FILES ${TS_FILES})

#set_target_properties(trainingplanner PROPERTIES
#	WIN32_EXECUTABLE TRUE
#	MACOSX_BUNDLE TRUE
#	MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
#)

#target_compile_definitions(trainingplanner PUBLIC
#	FREQUENCYMONITOR_SUPPORT
#	PERFORMANCEMONITOR_SUPPORT
#)

#if (${ANDROID_ABI})
#    target_link_libraries(trainingplanner PUBLIC
#	Qt::Core
#	Qt::Gui
#	Qt::Multimedia
#	Qt::Widgets
#	Qt6::Qml
#	Qt6::Quick
#	Qt6::QuickControls2
#	Qt6::AndroidExtras)
#else()
    target_link_libraries(trainingplanner PUBLIC
	Qt::Core
	Qt::Gui
	Qt::Multimedia
	Qt::Widgets
	Qt::Sql
	Qt6::Qml
	Qt6::Quick
	Qt6::QuickControls2)
#endif()

set(trainingplanner_resource_files
	"images/app_logo.png"
	"images/app_icon.png"
	"images/no_image.jpg"
	"app_resources.qrc"
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

set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")

set_property(TARGET trainingplanner APPEND PROPERTY
    QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android
)
