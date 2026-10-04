package com.smartclock.alarm.alarm

import android.app.AlarmManager
import android.app.PendingIntent
import android.content.Context
import android.content.Intent

/**
 * Schedules/cancels exact alarms via AlarmManager.
 * One-shot alarms fire once; daily alarms are rescheduled by AlarmReceiver after each fire.
 */
object AlarmScheduler {

    const val EXTRA_ALARM_ID = "alarm_id"

    fun schedule(context: Context, alarm: Alarm) {
        if (!alarm.enabled) {
            cancel(context, alarm.id)
            return
        }
        val alarmManager = context.getSystemService(Context.ALARM_SERVICE) as AlarmManager
        val triggerAt = alarm.nextTriggerMillis()
        val pendingIntent = pendingIntent(context, alarm.id)
        try {
            alarmManager.setExactAndAllowWhileIdle(AlarmManager.RTC_WAKEUP, triggerAt, pendingIntent)
        } catch (e: SecurityException) {
            // Exact-alarm permission not granted (e.g. SCHEDULE_EXACT_ALARM denied):
            // fall back to an inexact alarm so the alert still fires.
            alarmManager.setAndAllowWhileIdle(AlarmManager.RTC_WAKEUP, triggerAt, pendingIntent)
        }
    }

    fun cancel(context: Context, alarmId: Int) {
        val alarmManager = context.getSystemService(Context.ALARM_SERVICE) as AlarmManager
        alarmManager.cancel(pendingIntent(context, alarmId))
    }

    fun rescheduleAll(context: Context) {
        for (alarm in AlarmStore(context).loadAll()) {
            schedule(context, alarm)
        }
    }

    private fun pendingIntent(context: Context, alarmId: Int): PendingIntent =
        PendingIntent.getBroadcast(
            context,
            alarmId,
            Intent(context, AlarmReceiver::class.java).putExtra(EXTRA_ALARM_ID, alarmId),
            PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE
        )
}
