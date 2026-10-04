package com.smartclock.alarm.alarm

import android.content.Context
import org.json.JSONArray
import org.json.JSONObject

/** Persists Color Profiles as a JSON array in SharedPreferences. */
class ProfileStore(context: Context) {

    private val prefs =
        context.applicationContext.getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)

    fun loadAll(): List<Profile> {
        val json = prefs.getString(KEY_PROFILES, "[]") ?: "[]"
        val array = JSONArray(json)
        val profiles = mutableListOf<Profile>()
        for (i in 0 until array.length()) {
            val o = array.getJSONObject(i)
            profiles.add(
                Profile(
                    id = o.getInt("id"),
                    name = o.optString("name", "Profile"),
                    r = o.optInt("r", 255),
                    g = o.optInt("g", 0),
                    b = o.optInt("b", 0),
                    emoji = o.optString("emoji", "♥"),
                    sound = o.optBoolean("sound", false)
                )
            )
        }
        return profiles
    }

    fun saveAll(profiles: List<Profile>) {
        val array = JSONArray()
        for (p in profiles) {
            array.put(
                JSONObject().apply {
                    put("id", p.id)
                    put("name", p.name)
                    put("r", p.r)
                    put("g", p.g)
                    put("b", p.b)
                    put("emoji", p.emoji)
                    put("sound", p.sound)
                }
            )
        }
        prefs.edit().putString(KEY_PROFILES, array.toString()).apply()
    }

    fun nextId(): Int {
        val id = prefs.getInt(KEY_NEXT_ID, 1)
        prefs.edit().putInt(KEY_NEXT_ID, id + 1).apply()
        return id
    }

    fun add(profile: Profile) {
        saveAll(loadAll() + profile)
    }

    fun delete(id: Int) {
        saveAll(loadAll().filter { it.id != id })
    }

    companion object {
        private const val PREFS_NAME = "smartclock_profiles"
        private const val KEY_PROFILES = "profiles"
        private const val KEY_NEXT_ID = "next_id"
    }
}
