set( QML_PAGES
	"ClientsPage.qml"
	"CoachesPage.qml"
	"ExercisesListPage.qml"
	"ExercisesPlanner.qml"
	"HomePage.qml"
	"HomePageElements/MesosList.qml"
	"MesoCalendarPage.qml"
	"MesoCalendarElements/CalendarEntry.qml"
	"MesocyclePage.qml"
	"MesocyclePageElements/MesoSplitSetup.qml"
	"SettingsPage.qml"
	"StatisticsPage.qml"
	"TPPage.qml"
	"UserPage.qml"
	"WeatherPage.qml"
	"WeatherPageElements/NextDaysForecast.qml"
	"WeatherPageElements/TodayForecast.qml"
	"WorkoutElements/WorkoutOptionsDialog.qml"
	"WorkoutPage.qml"
)

qt_add_library(tp_pages_module STATIC)
qt_add_qml_module(tp_pages_module
	URI TpQml.Pages
	VERSION 1.0
	QML_FILES
		${QML_PAGES}
	RESOURCE_PREFIX
		"/"
)
