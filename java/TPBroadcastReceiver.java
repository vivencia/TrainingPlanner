package org.vivenciasoftware.TrainingPlanner;

import android.content.Intent;
import android.content.Context;
import android.util.Log;
import android.content.BroadcastReceiver;
import org.qtproject.qt.android.bindings.QtService;
//import android.os.Bundle;

public class TPBroadcastReceiver extends BroadcastReceiver
{
	@Override
	public void onReceive(Context context, Intent intent) {
		Class<?> c = Class.forName("org.vivenciasoftware.TrainingPlanner.TPService");
		Intent startServiceIntent = new Intent(context, c);
		context.startService(startServiceIntent);
    }

    /*@Override
	public void onReceive(Context context, Intent intent) {
		//Only one intent action is listened(view TPActivity.java) so far. No need to check
		//if (intent.getAction().equals("org.vivenciasoftware.TrainingPlanner.NOTIFICATION_ACTION"))
		Bundle extras = intent.getExtras();
		if(extras != null) {
			String action = extras.getString("TP_ACTION");
			Log.i("TPBroadcastReceiver action requested: ", action);
			//notificationActionReceived(action);
		}
	}*/
}
