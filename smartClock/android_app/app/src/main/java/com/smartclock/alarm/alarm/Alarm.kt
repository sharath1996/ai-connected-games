package com.smartclock.alarm.alarm

import java.util.Calendar

/**
 * One scheduled trigger of the SmartClock.
 *
 * [dateMillis] is local midnight of the chosen day; null + [repeatDaily] means
 * "every day at hour:minute". A non-null [dateMillis] with repeatDaily=false is
 * a one-time alarm on that date.
 */
data class Alarm(
    val id: Int,
    val label: String = "",
    val hour: Int,
    val minute: Int,
    val dateMillis: Long? = null,
    val repeatDaily: Boolean = false,
    val r: Int = 255,
    val g: Int = 0,
    val b: Int = 0,
    val emoji: String = "♥",
    val sound: Boolean = true,
    val enabled: Boolean = true
) {
    /** Next epoch-millis at which this alarm should fire. */
    fun nextTriggerMillis(now: Long = System.currentTimeMillis()): Long {
        val cal = Calendar.getInstance()
        if (dateMillis != null && !repeatDaily) {
            cal.timeInMillis = dateMillis
        } else {
            cal.timeInMillis = now
        }
        cal.set(Calendar.HOUR_OF_DAY, hour)
        cal.set(Calendar.MINUTE, minute)
        cal.set(Calendar.SECOND, 0)
        cal.set(Calendar.MILLISECOND, 0)
        if (dateMillis == null || repeatDaily) {
            // Daily-style alarm: roll to tomorrow if today's time has passed.
            if (cal.timeInMillis <= now) cal.add(Calendar.DAY_OF_YEAR, 1)
        }
        return cal.timeInMillis
    }

    /** The exact serial command this alarm sends (docs/serial_commands.md). */
    fun command(): String =
        "TRIGGER $r $g $b $emoji ${if (sound) 1 else 0}"
}
