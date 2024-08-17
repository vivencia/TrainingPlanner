// (c) 2017 Ekkehard Gentz (ekke)
// this project is based on ideas from
// http://blog.lasconic.com/share-on-ios-and-android-using-qml/
// see github project https://github.com/lasconic/ShareUtils-QML
// also inspired by:
// https://www.androidcode.ninja/android-share-intent-example/
// https://www.calligra.org/blogs/sharing-with-qt-on-android/
// https://stackoverflow.com/questions/7156932/open-file-in-another-app
// http://www.qtcentre.org/threads/58668-How-to-use-QAndroidJniObject-for-intent-setData
// https://stackoverflow.com/questions/5734678/custom-filtering-of-intent-chooser-based-on-installed-android-package-name
// see also /COPYRIGHT and /LICENSE
// (c) 2023 Ekkehard Gentz (ekke)
// switched from android.support.v4.content.FileProvider to androidx.core.content.FileProvider library.
// changes in build.gradle and Android Manifest
// also added to gradle.properties:
//     android.useAndroidX=true
//     android.enableJetifier=true

package org.vivenciasoftware.TrainingPlanner;

import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.database.Cursor;
import android.net.Uri;
import android.os.Build;
import android.os.Parcelable;
import android.provider.MediaStore;
import androidx.core.app.ShareCompat;

import androidx.core.content.FileProvider;
import android.util.Log;

import org.qtproject.qt.android.QtNative;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.lang.String;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

public class QShareUtils {
    // reference Authority as defined in AndroidManifest.xml
    private static String AUTHORITY = "org.vivenciasoftware.TrainingPlanner.fileprovider";
    private static String TAG = "QShareUtils";

    protected QShareUtils() {
	// Log.d(TAG, "QShareUtils()");
    }

    public static boolean checkMimeTypeView(String mimeType) {
	if (QtNative.activity() == null) {
	    return false;
	}
	Intent myIntent = new Intent();
	myIntent.setAction(Intent.ACTION_VIEW);
	// without an URI resolve always fails
	// an empty URI allows to resolve the Activity
	File fileToShare = new File("");
	Uri uri = Uri.fromFile(fileToShare);
	myIntent.setDataAndType(uri, mimeType);

	// Verify that the intent will resolve to an activity
	if (myIntent.resolveActivity(QtNative.activity().getPackageManager()) != null) {
	    // Log.d(TAG, "checkMime YEP - we can go on and View");
	    return true;
	} else {
	    // Log.d(TAG, "checkMime sorry - no App available to View");
	}
	return false;
    }

    public static boolean share(String text, String url) {
	if (QtNative.activity() == null) {
	    return false;
	}
	Intent sendIntent = new Intent();
	sendIntent.setAction(Intent.ACTION_SEND);
	sendIntent.putExtra(Intent.EXTRA_TEXT, text + " " + url);
	sendIntent.setType("text/plain");

	// Verify that the intent will resolve to an activity
	if (sendIntent.resolveActivity(QtNative.activity().getPackageManager()) != null) {
	    QtNative.activity().startActivity(sendIntent);
	    return true;
	} else {
	    Log.d(TAG, "share Intent not resolved");
	}
	return false;
    }

    public static boolean openURL(String url) {
	if (QtNative.activity() == null) {
	    return false;
	}

	Intent intent = new Intent();
	intent.setAction(Intent.ACTION_VIEW);
	intent.setData(Uri.parse(url));
	if (intent.resolveActivity(QtNative.activity().getPackageManager()) != null) {
	    QtNative.activity().startActivity(intent);
	    return true;
	}
	return false;
    }

    public static boolean sendEmail(String address, String subject, String attachment) {
	if (QtNative.activity() == null) {
	    return false;
	}

	Intent intent = new Intent();
	intent.setAction(Intent.ACTION_SENDTO);
	intent.setData(Uri.parse("mailto:")); // only email apps should handle this
	String[] addresses = new String[] { address };
	intent.putExtra(Intent.EXTRA_EMAIL, addresses);
	intent.putExtra(Intent.EXTRA_SUBJECT, subject);

	if (!attachment.isEmpty()) {
	    final Context context = QtNative.activity();
	    Uri uri = Uri.parse(attachment);
	    context.grantUriPermission("android", uri, Intent.FLAG_GRANT_READ_URI_PERMISSION);
	    intent.putExtra(Intent.EXTRA_STREAM, uri);
	}

	if (intent.resolveActivity(QtNative.activity().getPackageManager()) != null) {
	    //QtNative.activity().startActivity(intent);
	    QtNative.activity().startActivity(Intent.createChooser(intent, "Send mail..."));
	    return true;
	}
	return false;
    }

    // thx @oxied and @pooks for the idea:
    // https://stackoverflow.com/a/18835895/135559 theIntent is already configured
    // with all needed properties and flags so we only have to add the packageName
    // of targeted app
    public static boolean createCustomChooserAndStartActivity(Intent theIntent,
							      String title,
							      int requestId,
							      Uri uri) {
	final Context context = QtNative.activity();
	final PackageManager packageManager = context.getPackageManager();

	// MATCH_DEFAULT_ONLY: Resolution and querying flag. if set, only filters
	// that support the CATEGORY_DEFAULT will be considered for matching. Check
	// if there is a default app for this type of content.
	/*ResolveInfo defaultAppInfo =
		packageManager.resolveActivity(theIntent, PackageManager.MATCH_DEFAULT_ONLY);
	if (defaultAppInfo == null) {
	    Log.d(TAG, title + " PackageManager cannot resolve Activity");
	    return false;
	}*/

	// Retrieve all apps for our intent. Check if there are any apps returned
	List<ResolveInfo> appInfoList = packageManager.queryIntentActivities(theIntent, PackageManager.MATCH_ALL); // 0 or PackageManager.MATCH_DEFAULT_ONLY
	if (appInfoList.isEmpty()) {
	    Log.d(TAG, title + " appInfoList.isEmpty");
	    return false;
	}
	// Log.d(TAG, title + " appInfoList: " + appInfoList.size());

	// Sort in alphabetical order
	/*Collections.sort(appInfoList, new Comparator<ResolveInfo>() {
	    @Override
	    public int compare(ResolveInfo first, ResolveInfo second) {
		String firstName = first.loadLabel(packageManager).toString();
		String secondName = second.loadLabel(packageManager).toString();
		return firstName.compareToIgnoreCase(secondName);
	    }
	});*/

	List<Intent> targetedIntents = new ArrayList<Intent>();
	// Filter itself and create intent with the rest of the apps.
	for (ResolveInfo appInfo : appInfoList) {
	    // get the target PackageName
	    String targetPackageName = appInfo.activityInfo.packageName;
	    // we don't want to share with our own app
	    // in fact sharing with own app with resultCode will crash because doesn't
	    // work well with launch mode 'singleInstance'
	    if (targetPackageName.equals(context.getPackageName())) {
		continue;
	    }
	    // if you have a blacklist of apps please exclude them here

	    // we create the targeted Intent based on our already configured Intent
	    Intent targetedIntent = new Intent(theIntent);
	    // now add the target packageName so this Intent will only find the one
	    // specific App
	    targetedIntent.setPackage(targetPackageName);
	    //Log.d(TAG, "################# Adding app: " + targetPackageName);
	    // collect all these targetedIntents
	    targetedIntents.add(targetedIntent);
	    context.grantUriPermission("android", uri, Intent.FLAG_GRANT_READ_URI_PERMISSION);
	}

	/*boolean isAppInstalled;
	String appName;

	appName = "org.telegram.messenger.web";
	try
	{
	    packageManager.getPackageInfo(appName, PackageManager.GET_ACTIVITIES);
	    isAppInstalled =  true;
	}
	catch (PackageManager.NameNotFoundException e)
	{
	    isAppInstalled = false;
	}

	if (isAppInstalled) {
	    Intent telegramIntent = new Intent(theIntent);
	    telegramIntent.setPackage(appName);
	    telegramIntent.setType("text/plain");
	    telegramIntent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
	    telegramIntent.putExtra(Intent.EXTRA_STREAM, uri);
	    targetedIntents.add(telegramIntent);
	}
	else {
	    Log.d(TAG, title + " Telegram not installed");
	}

	appName = "com.whatsapp";
	try
	    {
		packageManager.getPackageInfo(appName, PackageManager.GET_ACTIVITIES);
		isAppInstalled =  true;
	    }
	    catch (PackageManager.NameNotFoundException e)
	    {
		isAppInstalled = false;
	    }

	if (isAppInstalled) {
	    Intent whatsappIntent = new Intent(theIntent);
	    whatsappIntent.setPackage(appName);
	    whatsappIntent.setType("text/plain");
	    whatsappIntent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
	    whatsappIntent.putExtra(Intent.EXTRA_STREAM, uri);
	    targetedIntents.add(whatsappIntent);
	}
	else {
	    Log.d(TAG, title + " WhatsApp not installed");
	}

	Intent bluetoothIntent = new Intent(theIntent);
	bluetoothIntent.setPackage("com.android.bluetooth");
	bluetoothIntent.setType("text/plain");
	bluetoothIntent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
	bluetoothIntent.putExtra(Intent.EXTRA_STREAM, uri);
	targetedIntents.add(bluetoothIntent);
	*/

	// check if there are apps found for our Intent to avoid that there was only
	// our own removed app before
	if (targetedIntents.isEmpty()) {
	    Log.d(TAG, title + " targetedIntents.isEmpty");
	    return false;
	}

	// now we can create our Intent with custom Chooser
	// we need all collected targetedIntents as EXTRA_INITIAL_INTENTS
	// we're using the last targetedIntent as initializing Intent, because
	// chooser adds its initializing intent to the end of EXTRA_INITIAL_INTENTS
	// :)
	Intent chooserIntent =
		Intent.createChooser(theIntent, title);

	    chooserIntent.putExtra(Intent.EXTRA_INITIAL_INTENTS, targetedIntents.toArray(new Parcelable[] {}));

	// Verify that the intent will resolve to an activity
	if (chooserIntent.resolveActivity(QtNative.activity().getPackageManager()) != null) {
	    if (requestId > 0) {
		QtNative.activity().startActivityForResult(chooserIntent, requestId);
	    } else {
		QtNative.activity().startActivity(chooserIntent);
	    }
	    return true;
	}
	return false;
    }

    // I am deleting the files from shared folder when Activity was done or
    // canceled so probably I don't have to revike FilePermissions for older OS if
    // you don't delete or move the file: here's what you must done to revoke the
    // access
    public static void revokeFilePermissions(String filePath) {
	final Context context = QtNative.activity();
	if (Build.VERSION.SDK_INT <= Build.VERSION_CODES.KITKAT) {
	    File file = new File(filePath);
	    Uri uri = FileProvider.getUriForFile(context, AUTHORITY, file);
	    context.revokeUriPermission(
		    uri,
		    Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
	}
    }

    public static boolean sendFile(String filePath, String title, String mimeType, int requestId) {
	if (QtNative.activity() == null) {
	    return false;
	}

	// using v4 support library create the Intent from ShareCompat
	// Intent sendIntent = new Intent();
	Intent sendIntent = ShareCompat.IntentBuilder.from(QtNative.activity()).getIntent();
	sendIntent.setAction(Intent.ACTION_SEND);

	File fileToShare = new File(filePath);

	// Using FileProvider you must get the URI from FileProvider using your
	// AUTHORITY Uri uri = Uri.fromFile(imageFileToShare);
	Uri uri;
	try {
	    uri = FileProvider.getUriForFile(QtNative.activity(), AUTHORITY, fileToShare);
	} catch (IllegalArgumentException e) {
	    Log.d("error", e.getMessage());
	    Log.d(TAG, "sendFile - cannot be shared: " + filePath);
	    return false;
	}

	sendIntent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
	sendIntent.putExtra(Intent.EXTRA_STREAM, uri);

	if (mimeType == null || mimeType.isEmpty()) {
	    // fallback if mimeType not set
	    mimeType = QtNative.activity().getContentResolver().getType(uri);
	    // Log.d(TAG, "sendFile guessed mimeType:" + mimeType);
	} else {
	    // Log.d(TAG, "sendFile w mimeType:" + mimeType);
	}

	sendIntent.setType(mimeType);

	sendIntent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
	sendIntent.addFlags(Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
	return createCustomChooserAndStartActivity(sendIntent, title, requestId, uri);
    }

    public static boolean viewFile(String filePath, String title, String mimeType, int requestId) {
	if (QtNative.activity() == null) {
	    return false;
	}

	// using v4 support library create the Intent from ShareCompat
//         Intent viewIntent = new Intent();
	Intent viewIntent = ShareCompat.IntentBuilder.from(QtNative.activity()).getIntent();
	viewIntent.setAction(Intent.ACTION_VIEW);

	File fileToShare = new File(filePath);

	// Using FileProvider you must get the URI from FileProvider using your
	// AUTHORITY
	Uri uri;
	try {
	    uri = FileProvider.getUriForFile(QtNative.activity(), AUTHORITY, fileToShare);
	} catch (IllegalArgumentException e) {
	     Log.d(TAG, "viewFile - cannot be viewed: " + filePath, e);
	    return false;
	}
	// now we got a content URI per ex
	// content://org.ekkescorner.examples.sharex.fileprovider/my_shared_files/qt-logo.png
	// from a fileUrl:
	// /data/user/0/org.ekkescorner.examples.sharex/files/share_example_x_files/qt-logo.png
	// Log.d(TAG, "viewFile from file path: " + filePath);
	// Log.d(TAG, "viewFile to content URI: " + uri.toString());

	if (mimeType == null || mimeType.isEmpty()) {
	    // fallback if mimeType not set
	    mimeType = QtNative.activity().getContentResolver().getType(uri);
	    // Log.d(TAG, "viewFile guessed mimeType:" + mimeType);
	} else {
	    // Log.d(TAG, "viewFile w mimeType:" + mimeType);
	}

	viewIntent.setDataAndType(uri, mimeType);

	viewIntent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
	viewIntent.addFlags(Intent.FLAG_GRANT_WRITE_URI_PERMISSION);

	return createCustomChooserAndStartActivity(viewIntent, title, requestId, uri);
    }

    public static boolean editFile(String filePath, String title, String mimeType, int requestId) {
	if (QtNative.activity() == null)
	    return false;

	// using v4 support library create the Intent from ShareCompat
	// Intent editIntent = new Intent();
	Intent editIntent = ShareCompat.IntentBuilder.from(QtNative.activity()).getIntent();
	editIntent.setAction(Intent.ACTION_EDIT);

	File imageFileToShare = new File(filePath);

	// Using FileProvider you must get the URI from FileProvider using your
	// AUTHORITY Uri uri = Uri.fromFile(imageFileToShare);
	Uri uri;
	try {
	    uri = FileProvider.getUriForFile(QtNative.activity(), AUTHORITY, imageFileToShare);
	} catch (IllegalArgumentException e) {
	    // Log.d(TAG, "editFile - cannot be shared: " + filePath);
	    return false;
	}
	// Log.d(TAG, "editFile" + uri.toString());

	if (mimeType == null || mimeType.isEmpty()) {
	    // fallback if mimeType not set
	    mimeType = QtNative.activity().getContentResolver().getType(uri);
	    // Log.d(TAG, "editFile guessed mimeType:" + mimeType);
	} else {
	    // Log.d(TAG, "editFile w mimeType:" + mimeType);
	}

	editIntent.setDataAndType(uri, mimeType);

	editIntent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
	editIntent.addFlags(Intent.FLAG_GRANT_WRITE_URI_PERMISSION);

	return createCustomChooserAndStartActivity(editIntent, title, requestId, uri);
    }

    public static String getContentName(ContentResolver cR, Uri uri) {
	Cursor cursor = cR.query(uri, null, null, null, null);
	cursor.moveToFirst();
	int nameIndex = cursor.getColumnIndex(MediaStore.MediaColumns.DISPLAY_NAME);
	if (nameIndex >= 0) {
	    return cursor.getString(nameIndex);
	} else {
	    return null;
	}
    }

    public static String createFile(ContentResolver cR, Uri uri, String fileLocation) {
	String filePath = null;
	try {
	    InputStream iStream = cR.openInputStream(uri);
	    if (iStream != null) {
		String name = getContentName(cR, uri);
		if (name != null) {
		    filePath = fileLocation + "/" + name;
		    // Log.d(TAG, "- create File" + filePath);
		    File f = new File(filePath);
		    FileOutputStream tmp = new FileOutputStream(f);
		    // Log.d(TAG, "- create File new FileOutputStream");

		    int bufferSize = 1024;
		    byte[] buffer = new byte[bufferSize];
		    ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
		    while (iStream.available() > 0) {
			int n = iStream.read(buffer);
			// write to output stream
			outputStream.write(buffer, 0, n);
		    }
		    // write output stream to file in one chunk
		    tmp.write(outputStream.toByteArray());
		    outputStream.close();
		    tmp.close();
		    iStream.close();
		    return filePath;
		} // name
	    }     // iStream
	} catch (FileNotFoundException e) {
	    e.printStackTrace();
	    return filePath;
	} catch (IOException e) {
	    e.printStackTrace();
	    return filePath;
	} catch (Exception e) {
	    e.printStackTrace();
	    return filePath;
	}
	return filePath;
    }
}
