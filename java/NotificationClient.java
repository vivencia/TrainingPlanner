// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

package org.vivenciasoftware.TrainingPlanner;

import org.qtproject.qt.android.QtNative;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
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
    public static void notify(String title, String message, String action, int id) {
	try {
	    final Context context = QtNative.activity();
	    Bitmap icon = BitmapFactory.decodeResource(context.getResources(), R.drawable.icon);

	    NotificationManager m_notificationManager = (NotificationManager)
		    context.getSystemService(Context.NOTIFICATION_SERVICE);
	    NotificationChannel notificationChannel;
	    notificationChannel = new NotificationChannel("TP", "TrainingPlanner", NotificationManager.IMPORTANCE_DEFAULT);
	    m_notificationManager.createNotificationChannel(notificationChannel);

	    Intent notifyIntent = new Intent("org.vivenciasoftware.TrainingPlanner.NOTIFICATION_ACTION");
	    // Set the Activity to start in a new, empty task.
	    notifyIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
	    notifyIntent.putExtra("TP_ACTION", action);
	    // Create the PendingIntent.
	    PendingIntent notifyPendingIntent = PendingIntent.getBroadcast(context, id, notifyIntent,
		PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE);

	    Notification.Builder m_builder = new Notification.Builder(context, notificationChannel.getId());
	    m_builder.setSmallIcon(R.drawable.icon)
		    .setLargeIcon(icon)
		    .setContentTitle(title)
		    .setContentText(message)
		    .setDefaults(Notification.DEFAULT_SOUND)
		    .setColor(Color.GREEN)
		    .setAutoCancel(true)
		    .setContentIntent(notifyPendingIntent);

	    m_notificationManager.notify(id, m_builder.build());
	} catch (Exception e) {
	    e.printStackTrace();
	}
    }

    public static void cancelNotify(int id) {
	try {
	    final Context context = QtNative.activity();
	    NotificationManager m_notificationManager = (NotificationManager)
		    context.getSystemService(Context.NOTIFICATION_SERVICE);
	    m_notificationManager.cancel(id);
	} catch (Exception e) {
	    e.printStackTrace();
	}
    }
}

