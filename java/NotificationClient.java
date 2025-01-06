// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

package org.vivenciasoftware.TrainingPlanner;

import org.qtproject.qt.android.QtNative;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.pm.PackageManager;
import android.content.Intent;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.graphics.BitmapFactory;
import android.app.NotificationChannel;
import android.util.Log;
import androidx.core.app.ShareCompat;

public class NotificationClient
{
	private static Context TPActivityContext;

	public static void setActivityContext(Context context)
	{
		TPActivityContext = context;
	}

    public static void notify(String title, String message, int action, int id)
    {
		try {
		    Bitmap icon = BitmapFactory.decodeResource(TPActivityContext.getResources(), R.drawable.icon);

		    NotificationManager m_notificationManager = (NotificationManager)
			    TPActivityContext.getSystemService(Context.NOTIFICATION_SERVICE);
		    NotificationChannel notificationChannel;
		    notificationChannel = new NotificationChannel("TP", "TrainingPlanner", NotificationManager.IMPORTANCE_DEFAULT);
		    m_notificationManager.createNotificationChannel(notificationChannel);

		    Intent launchIntent = TPActivityContext.getPackageManager().getLaunchIntentForPackage("org.vivenciasoftware.TrainingPlanner");
		    launchIntent.setAction(Intent.ACTION_MAIN);
		    launchIntent.addCategory(Intent.CATEGORY_LAUNCHER);
		    launchIntent.putExtra("TP_ACTION", action);
		    launchIntent.putExtra("TP_ID", id);
		    launchIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_SINGLE_TOP);
		    if (launchIntent.resolveActivity(TPActivityContext.getPackageManager()) != null) {
				PendingIntent notifyPendingIntent = PendingIntent.getActivity(TPActivityContext, id, launchIntent,
					PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE);

				Notification.Builder m_builder = new Notification.Builder(TPActivityContext, notificationChannel.getId());
				m_builder.setSmallIcon(R.drawable.icon)
				    .setLargeIcon(icon)
				    .setWhen(System.currentTimeMillis())
				    .setContentTitle(title)
				    .setContentText(message)
				    .setDefaults(Notification.DEFAULT_SOUND)
				    .setColor(Color.GREEN)
				    .setAutoCancel(true)
				    .setContentIntent(notifyPendingIntent);

				m_notificationManager.notify(id, m_builder.build());
		    }
		    else
				Log.i("************* NotificationClient ***************      ", "Could not resolve activity");
		} catch (Exception e) {
		    e.printStackTrace();
		}
    }

    public static void cancelNotify(int id) {
		try {
		    NotificationManager m_notificationManager = (NotificationManager)
			    TPActivityContext.getSystemService(Context.NOTIFICATION_SERVICE);
		    m_notificationManager.cancel(id);
		} catch (Exception e) {
		    e.printStackTrace();
		}
    }
}

