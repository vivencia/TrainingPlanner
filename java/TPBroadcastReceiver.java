package org.vivenciasoftware.TrainingPlanner;

import org.qtproject.qt.android.QtNative;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Intent;
import android.content.Context;
import android.util.Log;
import android.content.BroadcastReceiver;
import android.os.Bundle;
import androidx.core.app.ShareCompat;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.content.pm.ActivityInfo;
import android.content.ComponentName;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

public class TPBroadcastReceiver extends BroadcastReceiver
{
    public static native void notificationActionReceived(String action);

    @Override
    public void onReceive(Context context, Intent intent) {
	//Only one intent action is listened(view TPActivity.java) so far. No need to check
	//if (intent.getAction().equals("org.vivenciasoftware.TrainingPlanner.NOTIFICATION_ACTION"))
	Bundle extras = intent.getExtras();
	if(extras != null) {
	    String action = extras.getString("TP_ACTION");
	    Log.i("TPBroadcastReceiver action requested: ", action);

	Intent intent2 = new Intent();
	intent2.setPackage("org.vivenciasoftware.TrainingPlanner");
//Intent intent2 = ShareCompat.IntentBuilder.from(QtNative.activity()).getIntent();
    PackageManager pm = QtNative.activity().getPackageManager();
    List<ResolveInfo> resolveInfos = pm.queryIntentActivities(intent2, 0);
    Collections.sort(resolveInfos, new ResolveInfo.DisplayNameComparator(pm));

    if(resolveInfos.size() > 0) {
	ResolveInfo launchable = resolveInfos.get(0);
	ActivityInfo activity = launchable.activityInfo;
	ComponentName name=new ComponentName(activity.applicationInfo.packageName,
		activity.name);
	//Intent i=new Intent(Intent.ACTION_MAIN);
	intent2.setAction(Intent.ACTION_MAIN);
	intent2.addCategory(Intent.CATEGORY_LAUNCHER);
	intent2.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_SINGLE_TOP);
	intent2.setComponent(name);

	QtNative.activity().startActivity(intent2);
    }

/*	    Intent intent2 = ShareCompat.IntentBuilder.from(QtNative.activity()).getIntent();
	    intent2.setAction(Intent.ACTION_MAIN);
	    intent2.addCategory(Intent.CATEGORY_LAUNCHER);
	    intent2.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_SINGLE_TOP);
	    QtNative.activity().startActivity(intent2);*/
	    notificationActionReceived(action);
	}
    }
}
