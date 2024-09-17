#include "osinterface.h"
#include "tpappcontrol.h"
#include "tputils.h"
#include "dbinterface.h"
#include "dbusermodel.h"

#ifdef Q_OS_ANDROID
#include "tpandroidnotification.h"

#include <QJniObject>
#include <QtGlobal>
#include <qnativeinterface.h>
	#if QT_VERSION == QT_VERSION_CHECK(6, 7, 2)
		#include <QtCore/6.7.2/QtCore/private/qandroidextras_p.h>
	#else
		#include <QtCore/6.6.3/QtCore/private/qandroidextras_p.h>
	#endif
#else
#include <QProcess>
extern "C"
{
	#include <unistd.h>
}
#endif

#include <QGuiApplication>
#include <QFileInfo>
#include <QStandardPaths>

OSInterface::OSInterface()
{
	connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(aboutToExit()));
	m_appDataFilesPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + u"/"_qs;
}

OSInterface::~OSInterface()
{
#ifdef Q_OS_ANDROID
	delete m_AndroidNotification;
#endif
}

void OSInterface::exitApp()
{
	qApp->exit(0);
	// When the main event loop is not running, the above function does nothing, so we must actually exit, then
	::exit(0);
}

void OSInterface::aboutToExit()
{
	appControl()->cleanUp();
}

#ifdef Q_OS_ANDROID
void OSInterface::checkPendingIntents() const
{
	QJniObject activity = QNativeInterface::QAndroidApplication::context();
	if(activity.isValid())
	{
		activity.callMethod<void>("checkPendingIntents","()V");
		return;
	}
	MSG_OUT("checkPendingIntents: Activity not valid")
}

/*
 * As default we're going the Java - way with one simple JNI call (recommended)
 * if altImpl is true we're going the pure JNI way
 * HINT: we don't use altImpl anymore
 *
 * If a requestId was set we want to get the Activity Result back (recommended)
 * We need the Request Id and Result Id to control our workflow
*/
bool OSInterface::sendFile(const QString& filePath, const QString& title, const QString& mimeType, const int& requestId) const
{
	QJniObject jsPath = QJniObject::fromString(filePath);
	QJniObject jsTitle = QJniObject::fromString(title);
	QJniObject jsMimeType = QJniObject::fromString(mimeType);
	jboolean ok = QJniObject::callStaticMethod<jboolean>("org/vivenciasoftware/TrainingPlanner/QShareUtils",
													"sendFile",
													"(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)Z",
													jsPath.object<jstring>(), jsTitle.object<jstring>(), jsMimeType.object<jstring>(), requestId);
	if(!ok)
	{
		MSG_OUT("Unable to resolve activity from Java")
		return false;
	}
	return true;
}

void OSInterface::androidOpenURL(const QString& address) const
{
	QString url;
	if (!address.startsWith(u"http"_qs))
		url = u"https://" + address;
	else
		url = address;

	QJniObject jsPath = QJniObject::fromString(url);
	jboolean ok = QJniObject::callStaticMethod<jboolean>("org/vivenciasoftware/TrainingPlanner/QShareUtils",
													"openURL",
													"(Ljava/lang/String;)Z",
													jsPath.object<jstring>());
	if(!ok)
		MSG_OUT("Unable to open the address: " << address)
}

bool OSInterface::androidSendMail(const QString& address, const QString& subject, const QString& attachment) const
{
	const QString attachment_file(attachment.isEmpty() ? QString() : u"file://" + attachment);
	QJniObject jsAddress = QJniObject::fromString(address);
	QJniObject jsSubject = QJniObject::fromString(subject);
	QJniObject jsAttach = QJniObject::fromString(attachment_file);
	jboolean ok = QJniObject::callStaticMethod<jboolean>("org/vivenciasoftware/TrainingPlanner/QShareUtils",
													"sendEmail",
													"(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z",
													jsAddress.object<jstring>(), jsSubject.object<jstring>(), jsAttach.object<jstring>());
	return ok;
}

bool OSInterface::viewFile(const QString& filePath, const QString& title) const
{
	QJniObject jsPath = QJniObject::fromString(filePath);
	QJniObject jsTitle = QJniObject::fromString(title);
	jboolean ok = QJniObject::callStaticMethod<jboolean>("org/vivenciasoftware/TrainingPlanner/QShareUtils",
													"viewFile",
													"(Ljava/lang/String;Ljava/lang/String;)Z",
													jsPath.object<jstring>(), jsTitle.object<jstring>());
	if(!ok)
	{
		MSG_OUT("Unable to resolve view activity from Java")
		return false;
	}
	return true;
}

void OSInterface::appStartUpNotifications()
{
	m_AndroidNotification = new TPAndroidNotification(this);
	if (appMesoModel()->count() > 0)
	{
		DBMesoCalendarTable* calTable(new DBMesoCalendarTable(m_DBFilePath));
		QStringList dayInfoList;
		calTable->dayInfo(QDate::currentDate(), dayInfoList);
		if (!dayInfoList.isEmpty())
		{
			if (dayInfoList.at(0).toUInt() == m_currentMesoManager->mesoId())
			{
				QString message;
				const QString splitLetter(dayInfoList.at(2));
				if (splitLetter != u"R"_qs) //day is training day
				{
					if (dayInfoList.at(3) == u"1"_qs) //day is completed
						message = tr("Your training routine seems to go well. Workout for the day is concluded");
					else
						message = tr("Today is training day. Start your workout number ") + dayInfoList.at(1) + tr(" division: ") + splitLetter;
				}
				else
					message = tr("Enjoy your day of rest from workouts!");
				m_AndroidNotification->sendNotification(u"Training Planner"_qs, message, WORKOUT_NOTIFICATION);
			}
		}
		delete calTable;
	}
}

void OSInterface::setFileUrlReceived(const QString &url) const
{
	QString androidUrl;
	if(url.isEmpty())
	{
		MSG_OUT("setFileUrlReceived: we got an empty URL");
		return;
	}
	else if(url.startsWith(u"file://"_qs))
	{
		androidUrl = url.right(url.length()-7);
		MSG_OUT("QFile needs this URL: " << androidUrl)
	}
	else
		androidUrl = url;

	// check if File exists
	QFileInfo fileInfo(androidUrl);
	if (fileInfo.exists())
		m_appDB->openRequestedFile(androidUrl);
	else
		MSG_OUT("setFileUrlReceived: FILE does NOT exist ")
}

void OSInterface::setFileReceivedAndSaved(const QString& url) const
{
	QString androidUrl;
	if(url.isEmpty())
	{
		MSG_OUT("setFileReceivedAndSaved: we got an empty URL");
		return;
	}
	else if(url.startsWith(u"file://"_qs))
	{
		androidUrl = url.right(url.length()-7);
		MSG_OUT("QFile needs this URL: " << androidUrl)
	}
	else
		androidUrl = url;

	// check if File exists
	QFileInfo fileInfo(androidUrl);
	if (fileInfo.exists())
		m_appDB->openRequestedFile(androidUrl);
	else
		MSG_OUT("setFileReceivedAndSaved: FILE does NOT exist ")
}

bool OSInterface::checkFileExists(const QString& url) const
{
	QString androidUrl;
	if(url.isEmpty())
	{
		MSG_OUT("checkFileExists: we got an empty URL");
		return false;
	}
	else if(url.startsWith(u"file://"_qs))
	{
		androidUrl = url.right(url.length()-7);
		MSG_OUT("QFile needs this URL: " << androidUrl)
	}
	else
		androidUrl = url;

	// check if File exists
	QFileInfo fileInfo(androidUrl);
	if (fileInfo.exists())
	{
		MSG_OUT("Yep: the File exists for Qt");
		return true;
	}
	else
	{
		MSG_OUT("Uuups: FILE does NOT exist ");
		return false;
	}
}

void OSInterface::onActivityResult(int requestCode, int resultCode)
{
	// we're getting RESULT_OK only if edit is done
	if (resultCode == -1)
		MSG_OUT("Send Activity Result OK")
	else if (resultCode == 0)
		MSG_OUT("Send Activity Result Canceled")
	else
		MSG_OUT("Send Activity wrong result code: " << resultCode << " from request: " << requestCode)
	emit activityFinishedResult(requestCode, resultCode);
}

void OSInterface::startNotificationAction(const QString& action)
{
	if (action.toUInt() == WORKOUT_NOTIFICATION)
		m_appDB->getTrainingDay(QDate::currentDate());
}

#ifdef __cplusplus
extern "C"
{
#endif

JNIEXPORT void JNICALL Java_org_vivenciasoftware_TrainingPlanner_TPActivity_setFileUrlReceived(
						JNIEnv *env, jobject obj, jstring url)
{
	const char *urlStr = env->GetStringUTFChars(url, NULL);
	Q_UNUSED (obj)
	handlerInstance()->setFileUrlReceived(urlStr);
	env->ReleaseStringUTFChars(url, urlStr);
	return;
}

JNIEXPORT void JNICALL Java_org_vivenciasoftware_TrainingPlanner_TPActivity_setFileReceivedAndSaved(
						JNIEnv *env, jobject obj, jstring url)
{
	const char *urlStr = env->GetStringUTFChars(url, NULL);
	Q_UNUSED (obj)
	handlerInstance()->setFileReceivedAndSaved(urlStr);
	env->ReleaseStringUTFChars(url, urlStr);
	return;
}

JNIEXPORT bool JNICALL Java_org_vivenciasoftware_TrainingPlanner_TPActivity_checkFileExists(
						JNIEnv *env, jobject obj, jstring url)
{
	const char *urlStr = env->GetStringUTFChars(url, NULL);
	Q_UNUSED (obj)
	bool exists = handlerInstance()->checkFileExists(urlStr);
	env->ReleaseStringUTFChars(url, urlStr);
	return exists;
}

JNIEXPORT void JNICALL Java_org_vivenciasoftware_TrainingPlanner_TPActivity_fireActivityResult(
						JNIEnv *env, jobject obj, jint requestCode, jint resultCode)
{
	Q_UNUSED (obj)
	Q_UNUSED (env)
	handlerInstance()->onActivityResult(requestCode, resultCode);
	return;
}

JNIEXPORT void JNICALL Java_org_vivenciasoftware_TrainingPlanner_TPActivity_notificationActionReceived(
						JNIEnv *env, jobject obj, jstring action)
{
	Q_UNUSED (obj)
	const char *actionStr = env->GetStringUTFChars(action, NULL);
	handlerInstance()->startNotificationAction(actionStr);
	env->ReleaseStringUTFChars(action, actionStr);
	return;
}

#ifdef __cplusplus
}
#endif

#else
void OSInterface::processArguments()
{
	const QStringList args(qApp->arguments());
	if (args.count() > 1)
	{
		QString filename;
		for (uint i(1); i < args.count(); ++i)
			filename += args.at(i) + ' ';
		filename.chop(1);
		const QFileInfo file(filename);
		if (file.isFile())
			openRequestedFile(filename);
	}
}

void OSInterface::restartApp()
{
	char* args[2] = { nullptr, nullptr };
	const QString argv0(qApp->arguments().at(0));
	args[0] = static_cast<char*>(::malloc(static_cast<size_t>(argv0.toLocal8Bit().size()) * sizeof(char)));
	::strncpy(args[0], argv0.toLocal8Bit().constData(), argv0.length());
	::execv(args[0], args);
	::free(args[0]);
	exitApp();
}

void OSInterface::openRequestedFile(const QString &filename)
{
	const QString nameOnly(filename.right(filename.length() - filename.lastIndexOf('/') - 1));
	QMetaObject::invokeMethod(appMainWindow(), "tryToOpenFile", Q_ARG(QString, filename), Q_ARG(QString, nameOnly));
}

bool OSInterface::exportToFile(const TPListModel* model, const QString& filename, const bool bShare, QFile* outFile) const
{
	QString fname(filename);
	if (filename.startsWith(u"file:"_qs))
		fname.remove(0, 7); //remove file://

	if (!outFile)
	{
		outFile = new QFile(m_appDataFilesPath + fname);
		outFile->deleteLater();
	}
	if (outFile->open(QIODeviceBase::ReadWrite|QIODeviceBase::Append|QIODeviceBase::Text))
	{
		model->exportToText(outFile);
		outFile->close();
		#ifdef Q_OS_ANDROID
		if (bShare)
			sendFile(exportFileName(), tr("Send file"), u"text/plain"_qs, 10);
		else
		#else
		if (!bShare)
		#endif

			QMetaObject::invokeMethod(appMainWindow(), "chooseFolderToSave", Q_ARG(QString, suggestedName));
		return true;
	}
	return false;
}

/*Return values
 *	 0: success
 *	-1: Failed to open file
 *	-2: File format was not recognized
 *	-3: Nothing was imported, either because file was missing info or error in formatting
 *	-4: File has been previously imported
 */
int OSInterface::importFromFile(QString filename, QFile* inFile)
{
	if (!inFile)
	{
		if (filename.startsWith(u"file:"_qs))
			filename.remove(0, 7); //remove file://
		inFile = new QFile(filename, this);
		inFile->deleteLater();
		if (!inFile->open(QIODeviceBase::ReadOnly|QIODeviceBase::Text))
			return -1;
	}

	TPListModel* model(nullptr);
	qint64 lineLength(0);
	char buf[128];
	QString inData;

	while ( (lineLength = inFile->readLine(buf, sizeof(buf))) != -1 )
	{
		if (lineLength > 2)
		{
			inData = buf;
			break;
		}
	}

	const bool bFancy(!inData.startsWith(u"##0x"_qs));
	int sep_idx(0);

	if (bFancy)
	{
		if (inData.indexOf(DBMesoSplitObjectName) != -1)
			model = new DBMesoSplitModel(this);
		else if (inData.indexOf(DBMesocyclesObjectName) != -1)
			model = new DBMesocyclesModel(this);
		//else if (inData.indexOf(DBTrainingDayObjectName) != -1)
			//model = new DBTrainingDayModel(this);
		else if (inData.indexOf(DBExercisesObjectName) != -1)
			model = new DBExercisesModel(this);
		else
			return -2;

		while ( (lineLength = inFile->readLine(buf, sizeof(buf))) != -1 )
		{
			if (lineLength > 2)
			{
				inData = buf;
				break;
			}
		}
	}
	else
	{
		inData = inData.left(2);
		inData.chop(1);
		switch (inData.toUInt())
		{
			case EXERCISES_TABLE_ID: model = new DBExercisesModel(this); break;
			case MESOCYCLES_TABLE_ID: model = new DBMesocyclesModel(this); break;
			case MESOSPLIT_TABLE_ID: model = new DBMesoSplitModel(this); break;
			//case TRAININGDAY_TABLE_ID: model = new DBTrainingDayModel(this); break;
			default:
				return -2;
		}
	}

	model->deleteLater();
	if (!model->importExtraInfo(inData))
		return -4;

	if (model->importFromText(inFile, inData))
	{
		if (!importFromModel(model))
			return -4;
	}

	if (!inFile->atEnd())
		return importFromFile(filename, inFile);
	else
		return 0;
}

bool OSInterface::importFromModel(TPListModel* model)
{
	mb_importMode = true;
	bool bOK(true);
	switch (model->tableID())
	{
		case EXERCISES_TABLE_ID:
			updateExercisesList(static_cast<DBExercisesModel*>(model));
		break;
		case MESOCYCLES_TABLE_ID:
			if (appMesoModel()->isDifferent(static_cast<DBMesocyclesModel*>(model)))
			{
				const uint meso_idx = createNewMesocycle(false);
				for (uint i(MESOCYCLES_COL_ID); i < MESOCYCLES_TOTAL_COLS; ++i)
					appMesoModel()->setFast(meso_idx, i, model->getFast(0, i));
				saveMesocycle(meso_idx);
				emit appMesoModel()->currentRowChanged(); //notify main.qml::btnWorkout to evaluate its enabled state
			}
			else
				bOK = false;
		break;
		case MESOSPLIT_TABLE_ID:
		{
			DBMesoSplitModel* splitModel = static_cast<DBMesoSplitModel*>(model);
			const uint meso_idx = splitModel->mesoIdx();
			QmlItemManager* itemMngr = m_itemManager.at(meso_idx);
			if (splitModel->completeSplit())
			{
				DBMesoSplitModel* mesoSplitModel(itemMngr->getSplitModel(splitModel->splitLetter().at(0)));
				if (mesoSplitModel->updateFromModel(splitModel))
				{
					saveMesoSplitComplete(mesoSplitModel);
					// I don't need to track when all the splits from the import file have been loaded. They will all have been loaded
					// by the time mb_splitsLoaded is ever checked upon
					mb_splitsLoaded = true;
				}
				else
					bOK = false;
			}
			else
			{
				for (uint i(0); i < SIMPLE_MESOSPLIT_TOTAL_COLS; ++i)
					appMesoModel()->mesoSplitModel()->setFast(meso_idx, i, splitModel->getFast(0, i));
				appMesoModel()->mesoSplitModel()->setFast(meso_idx, 1, appMesoModel()->getFast(meso_idx, MESOCYCLES_COL_ID));
				saveMesoSplit(meso_idx);
			}
		}
		break;
		case TRAININGDAY_TABLE_ID:
		{
			const QDate dayDate(model->getDate(0, 3));
			const uint meso_idx = static_cast<DBTrainingDayModel*>(model)->mesoIdx();
			DBTrainingDayModel* tDayModel(m_itemManager.at(meso_idx)->gettDayModel(dayDate));
			if (tDayModel->updateFromModel(model))
				getTrainingDay(meso_idx, dayDate);
			else
				bOK = false;
		}
		break;
	}
	mb_importMode = false;
	return bOK;
}

void OSInterface::saveFileDialogClosed(QString finalFileName, bool bResultOK)
{
	int resultCode(-12);
	if (finalFileName.startsWith(u"file:"_qs))
		finalFileName.remove(0, 7); //remove file://
	if (bResultOK)
	{
		bResultOK = QFile::copy(exportFileName(), finalFileName);
		resultCode = bResultOK ? 3 : -10;
	}
	QFile::remove(exportFileName());
	appMainWindow()->setProperty("importExportFilename", finalFileName);
	QMetaObject::invokeMethod(appMainWindow(), "displayResultMessage", Q_ARG(int, resultCode));
}

int OSInterface::parseFile(QString filename)
{
	if (filename.startsWith(u"file:"_qs))
		filename.remove(0, 7); //remove file://
	QFile* inFile(new QFile(filename, this));
	inFile->deleteLater();
	if (!inFile->open(QIODeviceBase::ReadOnly|QIODeviceBase::Text))
		return -1;

	qint64 lineLength(0);
	char buf[128];
	QString inData;
	QString tableMessage;
	bool createMessage[4] = { false };

	while ( (lineLength = inFile->readLine(buf, sizeof(buf))) != -1 )
	{
		if (lineLength > 10)
		{
			if (strstr(buf, "##") != NULL)
			{
				inData = buf;
				if (!inData.startsWith(u"##0x"_qs)) //Fancy
				{
					if (inData.indexOf(DBMesoSplitObjectName) != -1)
					{
						if (createMessage[1] && createMessage[0])
							continue;
						createMessage[0] = true;
					}
					else if (inData.indexOf(DBMesocyclesObjectName) != -1)
						createMessage[1] = true;
					else if (inData.indexOf(DBTrainingDayObjectName) != -1)
						createMessage[2] = true;
					else if (inData.indexOf(DBExercisesObjectName) != -1)
						createMessage[3] = true;
					else
						return -2;
				}
				else
				{
					inData = inData.left(2);
					inData.chop(1);
					switch (inData.toUInt())
					{
						case EXERCISES_TABLE_ID: createMessage[3] = true; break;
						case MESOCYCLES_TABLE_ID: createMessage[1] = true; break;
						case MESOSPLIT_TABLE_ID:
							if (createMessage[1] && createMessage[0])
								continue;
							createMessage[0] = true;
						break;
						case TRAININGDAY_TABLE_ID: createMessage[3] = true; break;
						default: return -2;
					}
				}
				if (createMessage[0])
				{
					if (tableMessage.isEmpty())
						tableMessage = tr("a new Training Split Exercise Plan");
					else
					{
						if (createMessage[1])
							tableMessage += tr("new Training Split Exercise Plans");
						else
							tableMessage = tr("new Training Split Exercise Plans");
					}
				}
				else if (createMessage[1])
					tableMessage = tr("an entire Mesocycle Plan, including ");
				else if (createMessage[2])
					tableMessage = tr("One Training Day");
				else if (createMessage[3])
				{
					if (!createMessage[1])
						tableMessage = tr("An updated exercises database list");
					else
						tableMessage.append(tr("and an updated exercises database list"));
				}
			}
		}
	}

	if (createMessage[0] || createMessage[1] || createMessage[2] || createMessage[3])
	{
		if (!createMessage[1] && appMesoModel()->count() == 0)
			return -5;
		const QString message(tr("This will import data to create: %1"));
		QMetaObject::invokeMethod(appMainWindow(), "confirmImport", Q_ARG(QString, message.arg(tableMessage)));
		return 1;
	}
	else
		return -3;
}

void OSInterface::exportMeso(const uint meso_idx, const bool bShare, const bool bCoachInfo)
{
	if (!mb_splitsLoaded)
		loadCompleteMesoSplits(false);
	const QString suggestedName(appMesoModel()->getFast(meso_idx, MESOCYCLES_COL_NAME) + tr(" - TP Complete Meso.txt"));
	setExportFileName(suggestedName);
	QFile* outFile(nullptr);
	appMesoModel()->setExportRow(meso_idx);

	if (bCoachInfo)
	{
		appUserModel()->setExportRow(appUserModel()->getRowByCoachName(appMesoModel()->getFast(meso_idx, MESOCYCLES_COL_COACH)));
		exportToFile(appUserModel(), exportFileName(), outFile);
	}

	if (exportToFile(appMesoModel(), exportFileName(), outFile))
	{
		appMesoModel()->mesoSplitModel()->setExportRow(meso_idx);
		exportToFile(appMesoModel()->mesoSplitModel(), QString(), outFile);
		exportMesoSplit(itemManager(meso_idx), u"X"_qs, bShare, outFile);

		#ifdef Q_OS_ANDROID
		if (bShare)
			sendFile(exportFileName(), tr("Send file"), u"text/plain"_qs, 10);
		else
		#else
		if (!bShare)
		#endif
			QMetaObject::invokeMethod(appMainWindow(), "chooseFolderToSave", Q_ARG(QString, suggestedName));
	}
	else
	{
		QFile::remove(exportFileName());
		appMainWindow()->setProperty("importExportFilename", exportFileName());
		QMetaObject::invokeMethod(appMainWindow(), "displayResultMessage", Q_ARG(int, -10));
	}
}

void OSInterface::openURL(const QString& address) const
{
	#ifdef Q_OS_ANDROID
	androidOpenURL(address);
	#else
	auto* __restrict proc(new QProcess());
	proc->startDetached(u"xdg-open"_qs, QStringList() << address);
	delete proc;
	#endif
}

void OSInterface::startChatApp(const QString& phone, const QString& appname) const
{
	if (phone.length() < 17)
		return;
	QString phoneNumbers;
	QString::const_iterator itr(phone.constBegin());
	const QString::const_iterator itr_end(phone.constEnd());
	do {
		if ((*itr).isDigit())
			phoneNumbers += *itr;
	} while (++itr != itr_end);

	QString address;
	if (appname.contains(u"Whats"_qs))
		address = u"https://wa.me/"_qs + phoneNumbers;
	else
		address = u"https://t.me/+"_qs + phoneNumbers;

	openURL(address);
}

void OSInterface::sendMail(const QString& address, const QString& subject, const QString& attachment_file) const
{
	#ifdef Q_OS_ANDROID
	if (!androidSendMail(address, subject, attachment_file))
	{
		if (appUserModel()->email(0).contains(u"gmail.com"_qs))
		{
			const QString gmailURL(QStringLiteral("https://mail.google.com/mail/u/%1/?view=cm&to=%2&su=%3").arg(appUserModel()->email(0), address, subject));
			openURL(gmailURL);
		}
	}
	#else
	const QStringList args (QStringList() <<
		u"--utf8"_qs << u"--subject"_qs << QChar('\'') + subject + QChar('\'') << u"--attach"_qs << attachment_file <<
			QChar('\'') + address + QChar('\''));
	auto* __restrict proc(new QProcess ());
	proc->start(u"xdg-email"_qs, args);
	connect(proc, &QProcess::finished, this, [&,proc,address,subject] (int exitCode, QProcess::ExitStatus)
	{
		if (exitCode != 0)
		{
			if (appUserModel()->email(0).contains(u"gmail.com"_qs))
			{
				const QString gmailURL(QStringLiteral("https://mail.google.com/mail/u/%1/?view=cm&to=%2&su=%3").arg(appUserModel()->email(0), address, subject));
				openURL(gmailURL);
			}
		}
		proc->deleteLater();
	});
	#endif
}

void OSInterface::viewExternalFile(const QString& filename) const
{
	if (!appUtils()->canReadFile(appUtils()->getCorrectPath(filename)))
		return;
	#ifdef Q_OS_ANDROID
	const QString localFile(mAppDataFilesPath + u"tempfile"_qs + filename.right(4));
	static_cast<void>(QFile::remove(localFile));
	if (QFile::copy(filename, localFile))
		viewFile(localFile, tr("View file with..."));
	else
		qDebug() << "coud not copy:  " << filename << "    to   " << localFile;
	#else
	openURL(filename);
	#endif
}
#endif //Q_OS_ANDROID
