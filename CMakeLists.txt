cmake_minimum_required(VERSION 4.1.2)
project(TrainingPlanner LANGUAGES CXX)

set(CMAKE_CXX_FLAGS_DEBUG_INIT "-DDEBUG -g")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-DQT_NO_DEBUG -DNDEBUG -O3 -march=native -fomit-frame-pointer -funroll-loops -Ofast")
set(CMAKE_CXX_FLAGS_INIT "-DQT_USE_QSTRINGBUILDER -DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS -Werror -Wall -Wextra -pedantic finput-charset=UTF-8 -fexec-charset=UTF-8")

set(CMAKE_AUTOMOC ON)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_AUTOGEN_VERBOSE ON)
set(CMAKE_AUTORCC ON)
set(QT_CREATOR_SKIP_MAINTENANCE_TOOL_PROVIDER ON)
set(QT_FORCE_CMP0156_TO_VALUE NEW)
set(QT_QML_GENERATE_QMLLS_INI ON)

# GLOBAL TOGGLE: Disable QML-to-C++ compilation in Debug mode
# This prevents 'qmlcachegen' from creating heavy C++ files that trigger
# full recompiles every time a .qml file changes.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Fast QML iteration enabled (No AOT Compilation)")
    set(QT_QML_NO_CACHEGEN TRUE) # Affects all following qt_add_qml_module calls (Qt 6.8+)
endif()

# Ensure the output directory structure matches the QML module's target path to enable correct functionality for Qt.
# For example, the 'tp_exercises_module' will be placed in ${CMAKE_BINARY_DIR}/qml/TpQml/Exercises
set(QT_QML_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}" CACHE PATH "Install path for QML" FORCE)
set(QML_IMPORT_PATH "${CMAKE_BINARY_DIR}/qml" CACHE STRING "Import paths for Qt Creator" FORCE)

if (ANDROID)

	find_package(
		Qt6 REQUIRED COMPONENTS
		Charts
		Core
		CorePrivate
		Gui
		LinguistTools
		Multimedia
		Network
		Pdf
		Positioning
		Qml
		Quick
		QuickControls2
		Sql
		WebSockets
		Widgets
	)

	add_compile_definitions(
		REQUEST_PERMISSIONS_ON_ANDROID
	)

else()

	find_package(
		Qt6 REQUIRED COMPONENTS
		Charts
		Core
		Gui
		LinguistTools
		Multimedia
		Network
		Pdf
		Positioning
		Qml
		Quick
		QuickControls2
		Sql
		WebSockets
		Widgets
	)

endif(ANDROID)

find_package(
	PkgConfig REQUIRED
)

add_compile_options(
	-std=c++20
	${CMAKE_CXX_FLAGS_INIT}
)

add_definitions(
	-DLOCAL_TPSERVER=1
)

qt_standard_project_setup(
	REQUIRES 6.9
	I18N_SOURCE_LANGUAGE en_EN
	I18N_TRANSLATED_LANGUAGES pt_BR de_DE
)
message(STATUS "QT_I18N_SOURCE_LANGUAGE: ${QT_I18N_SOURCE_LANGUAGE}")
message(STATUS "QT_I18N_TRANSLATED_LANGUAGES: ${QT_I18N_TRANSLATED_LANGUAGES}")

set( RESOURCE_FILES
		app_resources.qrc
)

set( CXX_SOURCES_QML_INTEGRATION
		cxx/qmlitemmanager.h cxx/qmlitemmanager.cpp
		cxx/osinterface.h cxx/osinterface.cpp
		cxx/tpfileops.h cxx/tpfileops.cpp
		cxx/tpimage.h cxx/tpimage.cpp
		cxx/tpmediacontrols.h cxx/tpmediacontrols.cpp
		cxx/tpsettings.h cxx/tpsettings.cpp
		cxx/tptimer.h cxx/tptimer.cpp
		cxx/tputils.cpp cxx/tputils.h
		cxx/translationclass.cpp cxx/translationclass.h

		cxx/online_services/tpmessagesmanager.h cxx/online_services/tpmessagesmanager.cpp
		cxx/online_services/tpchat.h cxx/online_services/tpchat.cpp
		cxx/weather/weatherinfo.h cxx/weather/weatherinfo.cpp

		cxx/dbexerciseslistmodel.h cxx/dbexerciseslistmodel.cpp
		cxx/dbmesocyclesmodel.h cxx/dbmesocyclesmodel.cpp
		cxx/homepagemesomodel.h cxx/homepagemesomodel.cpp
		cxx/dbexercisesmodel.h cxx/dbexercisesmodel.cpp
		cxx/dbcalendarmodel.h cxx/dbcalendarmodel.cpp
		cxx/dbusermodel.h cxx/dbusermodel.cpp
		cxx/pageslistmodel.h cxx/pageslistmodel.cpp

		cxx/qmluserinterface.h cxx/qmluserinterface.cpp
		cxx/qmlexercisesdatabaseinterface.h cxx/qmlexercisesdatabaseinterface.cpp
		cxx/qmlmesointerface.h cxx/qmlmesointerface.cpp
		cxx/qmlmesocalendarinterface.h cxx/qmlmesocalendarinterface.cpp
		cxx/qmlmesosplitinterface.h cxx/qmlmesosplitinterface.cpp
		cxx/qmlworkoutinterface.h cxx/qmlworkoutinterface.cpp
)

set( CXX_SOURCES
		cxx/qml_singleton.h
		cxx/return_codes.h
		cxx/main.cpp

		cxx/thread_manager.h cxx/thread_manager.cpp
		cxx/tpimageprovider.h cxx/tpimageprovider.cpp
		cxx/tpandroidnotification.h cxx/tpandroidnotification.cpp

		cxx/tpdatabasetable.h cxx/tpdatabasetable.cpp
		cxx/dbexerciseslisttable.h cxx/dbexerciseslisttable.cpp
		cxx/dbmesocyclestable.h cxx/dbmesocyclestable.cpp
		cxx/dbmesocalendartable.h cxx/dbmesocalendartable.cpp
		cxx/dbworkoutsorsplitstable.h cxx/dbworkoutsorsplitstable.cpp
		cxx/dbusertable.h cxx/dbusertable.cpp
		cxx/dbmodelinterface.h cxx/dbmodelinterface.cpp

		cxx/weather/openweathermapbackend.h cxx/weather/openweathermapbackend.cpp
		cxx/statistics/tpstatistics.h cxx/statistics/tpstatistics.cpp

		cxx/online_services/onlineuserinfo.h cxx/online_services/onlineuserinfo.cpp
		cxx/online_services/scan_network.h
		cxx/online_services/tpchatdb.h cxx/online_services/tpchatdb.cpp
		cxx/online_services/tpmessage.h cxx/online_services/tpmessage.cpp
		cxx/online_services/tponlineservices.h cxx/online_services/tponlineservices.cpp
		cxx/online_services/websocketserver.h cxx/online_services/websocketserver.cpp

		cxx/tpkeychain/tpkeychain.h cxx/tpkeychain/tpkeychain.cpp
		cxx/tpkeychain/keychain.h cxx/tpkeychain/keychain_p.h cxx/tpkeychain/keychain.cpp
		cxx/tpkeychain/androidkeystore_p.h cxx/tpkeychain/androidkeystore.cpp
		cxx/tpkeychain/keychain_android.cpp
		cxx/tpkeychain/keychain_unix.cpp
		cxx/tpkeychain/libsecret_p.h cxx/tpkeychain/libsecret.cpp
		cxx/tpkeychain/plaintextstore_p.h cxx/tpkeychain/plaintextstore.cpp
)

set( JAVA_SOURCES
		java/TPService.java
		java/TPActivity.java
		java/QShareUtils.java
		java/QSharePathResolver.java
		java/NotificationClient.java
		java/TPBroadcastReceiver.java
)

set ( ALL_MY_LANGUAGES
	    ${QT_I18N_SOURCE_LANGUAGE}
		${QT_I18N_TRANSLATED_LANGUAGES}
)

list(REMOVE_DUPLICATES ALL_MY_LANGUAGES)
set(MY_LANGUAGE_TS_FILES "")
foreach(lang ${ALL_MY_LANGUAGES})
  list(APPEND MY_LANGUAGE_TS_FILES "translations/${MY_APP}_${lang}.ts")
endforeach()
message(STATUS "MY_LANGUAGE_TS_FILES: ${MY_LANGUAGE_TS_FILES}")

#No .qm files needed from Qt 6.7 and up. CMake generates/updates them automatically(they are placed in the build directory)
qt_add_translations(TrainingPlanner
    TS_FILE_DIR "translations"
    RESOURCE_PREFIX "/"
    NO_GENERATE_PLURALS_TS_FILE
)

set( SERVER_FILES
		web-server/function_library.sh
		web-server/init_script.sh
		web-server/nginx.conf
		web-server/php.ini
		web-server/runcmds.sh
		web-server/tpserver.conf
		web-server/url_parser.php
		web-server/user.fields
		web-server/usersdb.sh
		web-server/www.conf
)

set ( ANDROID_FILES
		android/AndroidManifest.xml
		#android/build.gradle
		#android/res/values/libs.xml
		android/res/xml/qtprovider_paths.xml
		#android/gradle/wrapper/gradle-wrapper.jar
		android/gradle/wrapper/gradle-wrapper.properties
		android/gradle.properties
		android/local.properties
		#android/gradlew
		#android/gradlew.bat
)

set( QML_SOURCES
		"qml/AppGlobals.qml"
		"qml/Main.qml"
		"qml/Tests.qml"
)

qt_policy(SET QTP0004 NEW)
qt_policy(SET QTP0001 NEW)

set_source_files_properties("qml/AppGlobals.qml" PROPERTIES
	QT_QML_SINGLETON_TYPE TRUE
)

add_subdirectory(qml/Widgets/)
add_subdirectory(qml/Pages/)
add_subdirectory(qml/User/)
add_subdirectory(qml/Exercises/)
add_subdirectory(qml/Dialogs/)

qt_add_executable(TrainingPlanner
	${CXX_SOURCES}
	${JAVA_SOURCES}
	${SERVER_FILES}
	${ANDROID_FIES}
)

qt_add_qml_module(TrainingPlanner
	URI TpQml
	VERSION 1.0
	QML_FILES
		${QML_SOURCES}
	SOURCES
		${CXX_SOURCES_QML_INTEGRATION}
	RESOURCES
		${RESOURCE_FILES}
	RESOURCE_PREFIX
		"/"
)

target_link_libraries(TrainingPlanner PRIVATE
	tp_widgets_module
	tp_pages_module
	tp_user_module
	tp_exercises_module
	tp_dialogs_module
)

target_include_directories(TrainingPlanner PUBLIC cxx cxx/online_services cxx/tpkeychain cxx/weather)

if(NOT ANDROID)
	pkg_check_modules(LIBSECRET REQUIRED libsecret-1)
	add_definitions(-DHAVE_LIBSECRET=1)
	INCLUDE_DIRECTORIES(${LIBSECRET_INCLUDE_DIRS})
	LINK_DIRECTORIES(${LIBSECRET_LIBRARY_DIRS})
	list(APPEND keychain_LIBRARIES_PRIVATE ${LIBSECRET_LIBRARIES})

	#add_library(LIBRATS STATIC IMPORTED)
	#set_target_properties(LIBRATS PROPERTIES
	#	IMPORTED_LOCATION "/DATA/librats/build/lib/librats.a"
	#	INTERFACE_INCLUDE_DIRECTORIES "/DATA/librats/src"
	#)

	target_link_libraries(TrainingPlanner PUBLIC
		Qt6::Core
		Qt6::Gui
		Qt6::Multimedia
		Qt6::Qml
		Qt6::Quick
		Qt6::QuickControls2
		Qt6::Sql
		Qt6::Widgets
		Qt6::Network
		Qt6::Positioning
		Qt6::Charts
		Qt6::WebSockets
		Qt6::Pdf
		${keychain_LIBRARIES_PRIVATE}
		#LIBRATS
	)
else()
	set (keychain_LIBRARIES_PRIVATE "")

	target_link_libraries(TrainingPlanner PUBLIC
		Qt6::Core
		Qt6::CorePrivate
		Qt6::Gui
		Qt6::Multimedia
		Qt6::Qml
		Qt6::Quick
		Qt6::QuickControls2
		Qt6::Sql
		Qt6::Widgets
		Qt6::Network
		Qt6::Positioning
		Qt6::Charts
		Qt6::WebSockets
		Qt6::Pdf
	)
target_link_libraries(TrainingPlanner PRIVATE Qt6::Core)
target_link_libraries(TrainingPlanner PRIVATE Qt6::Core)
target_link_libraries(TrainingPlanner PRIVATE Qt6::Core)
endif(NOT ANDROID)

install(TARGETS TrainingPlanner
	RUNTIME DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}"
	BUNDLE DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}"
	LIBRARY DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}"
)

if (ANDROID)
	set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")

	set_property(TARGET TrainingPlanner APPEND PROPERTY
		QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android
	)

	include(FetchContent)
	FetchContent_Declare(
		android_openssl
		DOWNLOAD_EXTRACT_TIMESTAMP true
		URL https://github.com/KDAB/android_openssl/archive/refs/heads/master.zip
		#URL_HASH MD5=c97d6ad774fab16be63b0ab40f78d945 #optional
	)
	FetchContent_MakeAvailable(android_openssl)
	include(${android_openssl_SOURCE_DIR}/android_openssl.cmake)

	add_android_openssl_libraries(TrainingPlanner)
	# Android for TLS backend
	#set_target_properties(TrainingPlanner PROPERTIES
	#	QT_ANDROID_EXTRA_LIBS
	#	"${ANDROID_SDK_ROOT}/android_openssl/ssl_3/${CMAKE_ANDROID_ARCH_ABI}/libcrypto_3.so;${ANDROID_SDK_ROOT}/android_openssl/ssl_3/${CMAKE_ANDROID_ARCH_ABI}/libssl_3.so"
	#)

	#set_target_properties(TrainingPlanner PROPERTIES
	#	QT_ANDROID_TARGET_SDK_VERSION 32
	#	QT_ANDROID_COMPILE_SDK_VERSION 32
	#)
endif(ANDROID)
