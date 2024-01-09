package org.qtproject.example.androidnotifier;

import android.app.Notification;
import android.app.NotificationManager;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.graphics.BitmapFactory;
import android.app.NotificationChannel;

public class NotificationClient
{
    public static void notify(Context context, String message) {
	try {
	    NotificationManager m_notificationManager = (NotificationManager)
		    context.getSystemService(Context.NOTIFICATION_SERVICE);

	    Notification.Builder m_builder;
	    if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
		int importance = NotificationManager.IMPORTANCE_DEFAULT;
		NotificationChannel notificationChannel;
		notificationChannel = new NotificationChannel("Qt", "Qt Notifier", importance);
		m_notificationManager.createNotificationChannel(notificationChannel);
		m_builder = new Notification.Builder(context, notificationChannel.getId());
	    } else {
		m_builder = new Notification.Builder(context);
	    }

	    m_builder.setContentTitle("A message from Qt!")
		    .setContentText(message)
		    .setDefaults(Notification.DEFAULT_SOUND)
		    .setColor(Color.GREEN)
		    .setAutoCancel(true);

	    m_notificationManager.notify(0, m_builder.build());
	} catch (Exception e) {
	    e.printStackTrace();
	}
    }
}


/*import android.content.Context;
import android.content.Intent;
import android.util.Log;
import org.qtproject.qt.android.bindings.QtService;
import java.util.Timer;
import java.util.TimerTask;

public class QtAndroidService extends QtService
{
    private static final String TAG = "QtAndroidService";
    private Timer m_timer;
    private TimerTask m_task;

    @Override
    public void onCreate() {
	super.onCreate();
	Log.i(TAG, "Creating Service");
	Timer m_timer = new Timer(true);
    }

    @Override
    public void onDestroy() {
	super.onDestroy();
	Log.i(TAG, "Destroying Service");
	m_timer.cancel();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId, int totalSecs) {
	int ret = super.onStartCommand(intent, flags, startId);

	m_task = new TimerTask();
	m_totalTime = totalSecs;
	m_timer.scheduleAtFixedRate(m_task, 0, totalSecs * 1000);

	return ret;
    }
}

public class MyTimerTask extends TimerTask {

    @Override
    public void run()
    {
        System.out.println("Timer task started at:"+new Date());
        completeTask();
        System.out.println("Timer task finished at:"+new Date());
    }

    public static void TimeTicked() {
      if  (!messagesList.isEmpty()) {
	return messagesList.remove(0);
      } else {
	return "";
      }
    }
}
*/
