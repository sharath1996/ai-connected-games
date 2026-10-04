package com.smartclock.alarm.alarm

import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.PendingIntent
import android.content.Context
import android.content.Intent
import androidx.core.app.NotificationCompat
import com.smartclock.alarm.MainActivity
import com.smartclock.alarm.R

object NotificationHelper {

    private const val CHANNEL_ID = "alarms"

    private fun ensureChannel(context: Context) {
        val manager = context.getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
        if (manager.getNotificationChannel(CHANNEL_ID) == null) {
            val channel = NotificationChannel(
                CHANNEL_ID,
                "Alarms",
                NotificationManager.IMPORTANCE_HIGH
            ).apply {
                description = "SmartClock alarm alerts"
                enableVibration(true)
            }
            manager.createNotificationChannel(channel)
        }
    }

    /**
     * Posted when an alarm fires. If the clock wasn't reachable over USB, the
     * notification (with sound/vibration) is the fallback alert on the phone.
     */
    fun showAlarmFired(context: Context, alarm: Alarm, sentToClock: Boolean) {
        ensureChannel(context)
        val manager = context.getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager

        val title = alarm.label.ifBlank { "SmartClock Alarm" }
        val text = if (sentToClock) {
            "Trigger sent to SmartClock ${alarm.emoji}"
        } else {
            "SmartClock not connected — plug in the OTG cable and open the app once"
        }

        val contentIntent = PendingIntent.getActivity(
            context,
            0,
            Intent(context, MainActivity::class.java),
            PendingIntent.FLAG_IMMUTABLE
        )

        val notification = NotificationCompat.Builder(context, CHANNEL_ID)
            .setSmallIcon(R.drawable.ic_notification)
            .setContentTitle(title)
            .setContentText(text)
            .setContentIntent(contentIntent)
            .setAutoCancel(true)
            .setPriority(NotificationCompat.PRIORITY_HIGH)
            .setCategory(NotificationCompat.CATEGORY_ALARM)
            .build()

        manager.notify(alarm.id, notification)
    }
}
