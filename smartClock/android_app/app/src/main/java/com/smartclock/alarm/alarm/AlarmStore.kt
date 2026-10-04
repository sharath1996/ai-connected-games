package com.smartclock.alarm.alarm

import android.content.Context
import org.json.JSONArray
import org.json.JSONObject

/** Persists alarms as a JSON array in SharedPreferences (no DB needed at this scale). */
class AlarmStore(context: Context) {

    private val prefs =
        context.applicationContext.getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)

    fun loadAll(): MutableList<Alarm> {
        val json = prefs.getString(KEY_ALARMS, "[]") ?: "[]"
        val array = JSONArray(json)
        val alarms = mutableListOf<Alarm>()
        for (i in 0 until array.length()) {
            val o = array.getJSONObject(i)
            alarms.add(
                Alarm(
                    id = o.getInt("id"),
                    label = o.optString("label", ""),
                    hour = o.getInt("hour"),
                    minute = o.getInt("minute"),
                    dateMillis = if (o.isNull("dateMillis")) null else o.getLong("dateMillis"),
                    repeatDaily = o.optBoolean("repeatDaily", false),
                    r = o.optInt("r", 255),
                    g = o.optInt("g", 0),
                    b = o.optInt("b", 0),
                    emoji = o.optString("emoji", "♥"),
                    sound = o.optBoolean("sound", true),
                    enabled = o.optBoolean("enabled", true)
                )
            )
        }
        return alarms.sortedBy { it.nextTriggerMillis() }.toMutableList()
    }

    fun saveAll(alarms: List<Alarm>) {
        val array = JSONArray()
        for (alarm in alarms) {
            array.put(
                JSONObject().apply {
                    put("id", alarm.id)
                    put("label", alarm.label)
                    put("hour", alarm.hour)
                    put("minute", alarm.minute)
                    if (alarm.dateMillis == null) {
                        put("dateMillis", JSONObject.NULL)
                    } else {
                        put("dateMillis", alarm.dateMillis)
                    }
                    put("repeatDaily", alarm.repeatDaily)
                    put("r", alarm.r)
                    put("g", alarm.g)
                    put("b", alarm.b)
                    put("emoji", alarm.emoji)
                    put("sound", alarm.sound)
                    put("enabled", alarm.enabled)
                }
            )
        }
        prefs.edit().putString(KEY_ALARMS, array.toString()).apply()
    }

    fun nextId(): Int {
        val id = prefs.getInt(KEY_NEXT_ID, 1)
        prefs.edit().putInt(KEY_NEXT_ID, id + 1).apply()
        return id
    }

    fun get(id: Int): Alarm? = loadAll().firstOrNull { it.id == id }

    fun upsert(alarm: Alarm) {
        val all = loadAll()
        val index = all.indexOfFirst { it.id == alarm.id }
        if (index >= 0) all[index] = alarm else all.add(alarm)
        saveAll(all)
    }

    fun delete(id: Int) {
        saveAll(loadAll().filter { it.id != id })
    }

    fun setEnabled(id: Int, enabled: Boolean) {
        get(id)?.let { upsert(it.copy(enabled = enabled)) }
    }

    companion object {
        private const val PREFS_NAME = "smartclock_alarms"
        private const val KEY_ALARMS = "alarms"
        private const val KEY_NEXT_ID = "next_id"
    }
}
