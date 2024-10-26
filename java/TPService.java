package org.vivenciasoftware.TrainingPlanner;

import android.content.Context;
import android.content.Intent;
import android.util.Log;
import org.qtproject.qt.android.bindings.QtService;

public class TPService extends QtService
{
	private static final String TAG = "TPService";

	@Override
	public void onCreate() {
		super.onCreate();
		Log.i(TAG, "Creating Service");
	}

	@Override
	public void onDestroy() {
		super.onDestroy();
		Log.i(TAG, "Destroying Service");
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		int ret = super.onStartCommand(intent, flags, startId);
		// Do some work
		return ret;
	}

	public static void startTPService(Context context) {
		context.startService(new Intent(context, TPService.class));
	}
}
