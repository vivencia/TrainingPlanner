#include "thread_manager.h"
#include "dbexerciseslistmodel.h"

#include "dbusermodel.h"
#include "osinterface.h"
#include "qmlitemmanager.h"
#include "tpsettings.h"
#include "tputils.h"
#include "translationclass.h"
#include "thread_manager.h"
#include "online_services/tponlineservices.h"
#include "tpkeychain/tpkeychain.h"

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QSurfaceFormat>

#ifdef Q_OS_ANDROID
#include <QtCore/private/qandroidextras_p.h>
#endif

/*#include <librats.h>

void testLibRats()
{
	librats::RatsClient client(8081);

	// Set up message handlers using the modern API
	client.on("chat", [](const std::string& peer_id, const nlohmann::json& data) {
		std::cout << "[CHAT] " << peer_id << ": " << data["message"].get<std::string>() << std::endl;
	});

	client.on("user_join", [](const std::string& peer_id, const nlohmann::json& data) {
		std::cout << "[JOIN] " << data["username"].get<std::string>() << " joined" << std::endl;
	});

	// Connection callback
	client.set_connection_callback([&](socket_t socket, const std::string& peer_id) {
		std::cout << "✅ Peer connected: " << peer_id << std::endl;

		// Send welcome message
		nlohmann::json welcome;
		welcome["username"] = "User_" + client.get_our_peer_id().substr(0, 8);
		client.send("user_join", welcome);
	});

	client.start();

	// Send a chat message
	nlohmann::json chat_msg;
	chat_msg["message"] = "Hello, P2P chat!";
	chat_msg["timestamp"] = std::time(nullptr);
	client.send("chat", chat_msg);
}
*/

int main(int argc, char *argv[])
{
#ifdef Q_OS_ANDROID
	if (argc > 1 && strcmp(argv[1], "-service") == 0)
	{
		qInfo() << "Service starting with from the same .so file";
		QAndroidService app(argc, argv);
		return app.exec();
	}
#endif
	QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
	// Set the default surface format before creating the application
	QSurfaceFormat format;
	format.setVersion(3, 1); // Request OpenGL ES 3.1
	format.setProfile(QSurfaceFormat::CoreProfile); // Use Core Profile
	QSurfaceFormat::setDefaultFormat(format);
	QApplication app{argc, argv};

	app.setOrganizationName("Vivencia Software"_L1);
	app.setOrganizationDomain("org.vivenciasoftware"_L1);
	app.setApplicationName("TrainingPlanner"_L1);

	TPUtils tpUtils{};
	ThreadManager threadManager{};
	TPSettings tpSettings{};
	TPKeyChain tpKeyChain{};
	TranslationClass appTranslations{};
	TPOnlineServices appTOS{};
	OSInterface osInterface{};
	DBUserModel userModel{};
	DBExercisesListModel exercisesModel{};
	QQmlApplicationEngine qmlEngine;
	QmlItemManager rootQmlManager{&qmlEngine};

	//testLibRats();
	return app.exec();
}
