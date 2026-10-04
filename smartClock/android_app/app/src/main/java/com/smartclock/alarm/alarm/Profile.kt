package com.smartclock.alarm.alarm

/** A saved TRIGGER preset the user can apply instantly ("Color Profile"). */
data class Profile(
    val id: Int,
    val name: String,
    val r: Int,
    val g: Int,
    val b: Int,
    val emoji: String,
    val sound: Boolean
) {
    fun command(): String = "TRIGGER $r $g $b $emoji ${if (sound) 1 else 0}"
}
