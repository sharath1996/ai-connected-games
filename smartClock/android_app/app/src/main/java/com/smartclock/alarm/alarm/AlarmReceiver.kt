package com.smartclock.alarm.alarm

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import com.smartclock.alarm.usb.UsbSerialManager
import kotlin.concurrent.thread

/**
 * Fires when AlarmManager delivers an alarm: sends the alarm's TRIGGER command
 * to the SmartClock over USB and posts a local notification as feedback/fallback.
 */
class AlarmReceiver : BroadcastReceiver() {

    override fun onReceive(context: Context, intent: Intent) {
        val alarmId = intent.getIntExtra(AlarmScheduler.EXTRA_ALARM_ID, -1)
        if (alarmId < 0) return

        val pendingResult = goAsync()
        thread {
            try {
                val store = AlarmStore(context)
                val alarm = store.get(alarmId)
                if (alarm != null && alarm.enabled) {
                    val ack = UsbSerialManager.get(context)
                        .sendTrigger(alarm.r, alarm.g, alarm.b, alarm.emoji, alarm.sound)
                    NotificationHelper.showAlarmFired(context, alarm, sentToClock = ack != null)

                    if (alarm.repeatDaily) {
                        AlarmScheduler.schedule(context, alarm) // re-arm for tomorrow
                    } else {
                        store.setEnabled(alarmId, false) // one-shot: mark done
                    }
                }
            } finally {
                pendingResult.finish()
            }
        }
    }
}
