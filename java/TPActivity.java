package org.vivenciasoftware.TrainingPlanner;

import org.vivenciasoftware.TrainingPlanner.QShareUtils;
import org.vivenciasoftware.TrainingPlanner.QSharePathResolver;
//import org.vivenciasoftware.TrainingPlanner.TPBroadcastReceiver;

import org.qtproject.qt.android.bindings.QtActivity;
import org.qtproject.qt.android.QtNative;

import android.os.*;
import android.content.*;
import android.app.*;
import android.net.Uri;
import android.util.Log;
import android.content.ContentResolver;
import android.webkit.MimeTypeMap;
import android.content.pm.PackageManager;
import android.content.pm.ApplicationInfo;
import android.content.pm.ResolveInfo;

import java.lang.String;
import java.io.File;

public class TPActivity extends QtActivity
{
	// native - must be implemented in Cpp via JNI
	// 'file' scheme or resolved from 'content' scheme:
	public static native void setFileUrlReceived(String url);
	// InputStream from 'content' scheme:
	public static native void setFileReceivedAndSaved(String url);
	//
	public static native void fireActivityResult(int requestCode, int resultCode);
	//
	public static native void notificationActionReceived(short action, short id);

	public static boolean isIntentPending;
	public static boolean isInitialized;
	public static String workingDirPath;

	public final static String TAG = "************** TPActivity ***************	  ";

	// Use a custom Chooser without providing own App as share target !
	// see QShareUtils.java createCustomChooserAndStartActivity()
	// Selecting your own App as target could cause AndroidOS to call
	// onCreate() instead of onNewIntent()
	// and then you are in trouble because we're using 'singleInstance' as LaunchMode
	// more details: my blog at Qt
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		Log.d(TAG, "onCreate TPActivity");
		workingDirPath = getFilesDir().getPath();

		/*
		Use, in the future, for system broadcasts, or some possible internal broadcast
		TPBroadcastReceiver broadCast = new TPBroadcastReceiver();
		IntentFilter intentFilter = new IntentFilter("org.vivenciasoftware.TrainingPlanner");
		registerReceiver(broadCast, intentFilter);
		*/

		// now we're checking if the App was started from another Android App via Intent
		Intent theIntent = getIntent();
		if (theIntent != null) {
			String theAction = theIntent.getAction();
			if (theAction != null) {
				Log.d("************** TPActivity ***************   onCreate ", theAction);
				// QML UI not ready yet, delay processIntent()
				isIntentPending = true;
			}
		}
	} // onCreate

	// WIP - trying to find a solution to survive a 2nd onCreate. ongoing discussion in QtMob (Slack)
	// from other Apps not respecting that you only have a singleInstance
	// there are problems per ex. sharing a file from Google Files App,
	// but working well using Xiaomi FileManager App
	@Override
	public void onDestroy() {
		Log.d(TAG, "onDestroy QShareActivity");
		// super.onDestroy();
		// System.exit() closes the App before doing onCreate() again
		// then the App was restarted, but looses context
		// This works for Samsung My Files
		// but Google Files doesn't call onDestroy()
		System.exit(0);
	}

// we start Activity with result code
// to test JNI with QAndroidActivityResultReceiver you must comment or rename
// this method here - otherwise you'll get wrong request or result codes
	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
	super.onActivityResult(requestCode, resultCode, data);
	// Check which request we're responding to
	Log.d("************** TPActivity ***************   onActivityResult", "requestCode: "+requestCode);
	if (resultCode == RESULT_OK) {
		Log.d("************** TPActivity ***************   onActivityResult - resultCode: ", "SUCCESS");
	} else {
	Log.d("************** TPActivity ***************   onActivityResult - resultCode: ", "CANCEL");
	}

	// Attention using FileDialog can trigger onActivityResult
	// with requestCode 1305
	// see https://code.qt.io/cgit/qt/qtbase.git/tree/src/plugins/platforms/android/qandroidplatformfiledialoghelper.cpp#n22
	if (requestCode == 1305) {
		Log.d("************** TPActivity ***************   onActivityResult - requestCode 1305 (Qt FileDialog): ", "IGNORE");
		return;
	}

	// hint: result comes back too fast for Action SEND
	// if you want to delete/move the File add a Timer w 500ms delay
	// see Example App main.qml - delayDeleteTimer
	// if you want to revoke permissions for older OS
	// it makes sense also do this after the delay
	fireActivityResult(requestCode, resultCode);
	}

	// if we are opened from other apps:
	@Override
	public void onNewIntent(Intent intent) {
		Log.d(TAG, "onNewIntent");
		super.onNewIntent(intent);
		setIntent(intent);
		//Intent will be processed, if all is initialized and Qt / QML can handle the event
		if (isInitialized)
			processIntent();
		else
			isIntentPending = true;
	} // onNewIntent

	public void checkPendingIntents() {
		isInitialized = true;
		if (isIntentPending) {
			isIntentPending = false;
			Log.d(TAG, "checkPendingIntents: true");
			processIntent();
		}
		else
			Log.d(TAG, "nothingPending");
	} // checkPendingIntents

	// process the Intent if Action is SEND or VIEW or MAIN
	private void processIntent() {
		Intent intent = getIntent();
		Log.d(TAG, "processIntent()");
		Log.d(TAG, intent.getAction());

		Uri intentUri;
		String intentScheme;
		String intentAction;

		if (intent.getAction().equals("android.intent.action.VIEW")) {
			intentAction = "VIEW";
			intentUri = intent.getData();
		}
		else if (intent.getAction().equals("android.intent.action.SEND")) {
			intentAction = "SEND";
			Bundle bundle = intent.getExtras();
			intentUri = (Uri)bundle.get(Intent.EXTRA_STREAM);
		}
		else if (intent.getAction().equals("android.intent.action.MAIN")) {
			Bundle extras = intent.getExtras();
			if(extras != null) {
				//com.sec.android.app.launcher
				/*for (String key : extras.keySet()) {
					Log.d(TAG, extras.get(key).toString());
				}*/
				short action = extras.getShort("TP_ACTION");
				short id = extras.getShort("TP_ID");
				notificationActionReceived(action, id);
			}
			return;
		}
		else {
			Log.d("************** TPActivity ***************   Intent unknown action:", intent.getAction());
			return;
		}

		Log.d("************** TPActivity ***************   action:", intentAction);
		if (intentUri == null) {
			Log.d("************** TPActivity ***************   Intent URI:", "is null");
			return;
		}

		Log.d("************** TPActivity ***************   Intent URI:", intentUri.toString());
		// content or file
		intentScheme = intentUri.getScheme();
		if (intentScheme == null) {
			Log.d("************** TPActivity ***************   Intent URI Scheme:", "is null");
			return;
		}
		else if (intentScheme.equals("file")) {
			// URI as encoded string
			Log.d("************** TPActivity ***************   Intent File URI: ", intentUri.toString());
			setFileUrlReceived(intentUri.toString());
			return;
		}
		else if (!intentScheme.equals("content")) {
			Log.d("************** TPActivity ***************   Intent URI unknown scheme: ", intentScheme);
			return;
		}

		/* ok - it's a content scheme URI. We will try to resolve the Path to a File URI.
			If this won't work or if the File cannot be opened, we'll try to copy the file
			into our App working dir via InputStream. Hopefully in most cases PathResolver will give a path
		*/

		ContentResolver cR = this.getContentResolver();
		MimeTypeMap mime = MimeTypeMap.getSingleton();
		String fileExtension = mime.getExtensionFromMimeType(cR.getType(intentUri));
		Log.d(TAG,"Intent extension: "+fileExtension);

		String mimeType = cR.getType(intentUri);
		Log.d(TAG," Intent MimeType: "+mimeType);

		String name = QShareUtils.getContentName(cR, intentUri);
		if(name != null)
			Log.d("************** TPActivity ***************   Intent Name:", name);
		else
			Log.d("************** TPActivity ***************   Intent Name:", "is NULL");

		//uri.getAuthority()

		/*String filePath = QSharePathResolver.getRealPathFromURI(this, intentUri);
		if(filePath == null)
			Log.d("************** TPActivity ***************   QSharePathResolver:", "filePath is NULL");
		else {
			Log.d("************** TPActivity ***************   QSharePathResolver:", filePath);
			setFileUrlReceived(filePath);
			return;
		}*/

		// trying the InputStream way:
		String filePath = QShareUtils.createFile(cR, intentUri, workingDirPath);
		if(filePath == null) {
			Log.d("************** TPActivity ***************   Intent FilePath:", "is NULL");
			return;
		}
		Log.d(TAG, "  calling setFileReceivedAndSaved(" + filePath + ")");
		setFileReceivedAndSaved(filePath);
	} // processIntent
}
