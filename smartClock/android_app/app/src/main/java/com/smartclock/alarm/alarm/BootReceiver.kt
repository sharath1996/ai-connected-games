package com.smartclock.alarm.alarm

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent

/** Re-arms all enabled alarms after a device reboot (AlarmManager state is wiped on boot). */
class BootReceiver : BroadcastReceiver() {
    override fun onReceive(context: Context, intent: Intent) {
        if (intent.action == Intent.ACTION_BOOT_COMPLETED) {
            AlarmScheduler.rescheduleAll(context)
        }
    }
}
