package com.smartclock.alarm.ui

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.DatePicker
import androidx.compose.material3.DatePickerDialog
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Switch
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.material3.TimePicker
import androidx.compose.material3.rememberDatePickerState
import androidx.compose.material3.rememberTimePickerState
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableFloatStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import com.smartclock.alarm.alarm.Alarm
import java.text.SimpleDateFormat
import java.util.Calendar
import java.util.Date
import java.util.Locale
import java.util.TimeZone

// Symbol choices come from CommonControls.kt (EMOJI_CHOICES, same package).

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun AlarmEditorDialog(
    initial: Alarm?,
    onDismiss: () -> Unit,
    onSave: (Alarm) -> Unit,
    onSaveProfile: ((name: String, r: Int, g: Int, b: Int, emoji: String, sound: Boolean) -> Unit)? = null
) {
    val now = remember { Calendar.getInstance() }
    var label by remember { mutableStateOf(initial?.label ?: "") }
    var hour by remember { mutableStateOf(initial?.hour ?: (now.get(Calendar.HOUR_OF_DAY) + 1) % 24) }
    var minute by remember { mutableStateOf(initial?.minute ?: 0) }
    var repeatDaily by remember { mutableStateOf(initial?.repeatDaily ?: false) }
    var dateMillis by remember { mutableStateOf(initial?.dateMillis ?: localMidnightToday()) }
    var r by remember { mutableFloatStateOf((initial?.r ?: 255).toFloat()) }
    var g by remember { mutableFloatStateOf((initial?.g ?: 0).toFloat()) }
    var b by remember { mutableFloatStateOf((initial?.b ?: 0).toFloat()) }
    var emoji by remember { mutableStateOf(initial?.emoji ?: "♥") }
    var sound by remember { mutableStateOf(initial?.sound ?: true) }
    var showTimePicker by remember { mutableStateOf(false) }
    var showDatePicker by remember { mutableStateOf(false) }

    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text(if (initial == null) "New alarm" else "Edit alarm") },
        text = {
            Column(
                Modifier.verticalScroll(rememberScrollState()),
                verticalArrangement = Arrangement.spacedBy(10.dp)
            ) {
                OutlinedTextField(
                    value = label,
                    onValueChange = { label = it },
                    label = { Text("Label (optional)") },
                    singleLine = true,
                    modifier = Modifier.fillMaxWidth()
                )

                Row(verticalAlignment = Alignment.CenterVertically) {
                    Text("Time", Modifier.weight(1f))
                    TextButton(onClick = { showTimePicker = true }) {
                        Text(
                            "%02d:%02d".format(hour, minute),
                            style = MaterialTheme.typography.titleLarge
                        )
                    }
                }

                Row(verticalAlignment = Alignment.CenterVertically) {
                    Text("Repeat daily", Modifier.weight(1f))
                    Switch(checked = repeatDaily, onCheckedChange = { repeatDaily = it })
                }
                if (!repeatDaily) {
                    Row(verticalAlignment = Alignment.CenterVertically) {
                        Text("Date", Modifier.weight(1f))
                        TextButton(onClick = { showDatePicker = true }) {
                            Text(
                                SimpleDateFormat("EEE, MMM d", Locale.getDefault())
                                    .format(Date(dateMillis))
                            )
                        }
                    }
                }

                HorizontalDivider()

                ColorPickerSection(r, g, b, { r = it }, { g = it }, { b = it })

                EmojiPickerSection(emoji) { emoji = it }
                OutlinedTextField(
                    value = emoji,
                    onValueChange = { emoji = it },
                    label = { Text("Custom symbol (must exist in the clock font)") },
                    singleLine = true,
                    modifier = Modifier.fillMaxWidth()
                )

                Row(verticalAlignment = Alignment.CenterVertically) {
                    Text("Sound buzzer", Modifier.weight(1f))
                    Switch(checked = sound, onCheckedChange = { sound = it })
                }
            }
        },
        confirmButton = {
            TextButton(onClick = {
                val base = initial ?: Alarm(id = 0, hour = hour, minute = minute)
                onSave(
                    base.copy(
                        label = label.trim(),
                        hour = hour,
                        minute = minute,
                        repeatDaily = repeatDaily,
                        dateMillis = if (repeatDaily) null else dateMillis,
                        r = r.toInt(),
                        g = g.toInt(),
                        b = b.toInt(),
                        emoji = emoji.trim().ifEmpty { "♥" },
                        sound = sound,
                        enabled = true
                    )
                )
            }) { Text("Save") }
        },
        dismissButton = {
            Row {
                if (onSaveProfile != null) {
                    TextButton(onClick = {
                        val name = label.trim().ifEmpty { "Profile" }
                        onSaveProfile.invoke(
                            name, r.toInt(), g.toInt(), b.toInt(),
                            emoji.trim().ifEmpty { "♥" }, sound
                        )
                        onDismiss()
                    }) { Text("Save as profile") }
                }
                TextButton(onClick = onDismiss) { Text("Cancel") }
            }
        }
    )

    if (showTimePicker) {
        val timeState = rememberTimePickerState(hour, minute, is24Hour = true)
        AlertDialog(
            onDismissRequest = { showTimePicker = false },
            confirmButton = {
                TextButton(onClick = {
                    hour = timeState.hour
                    minute = timeState.minute
                    showTimePicker = false
                }) { Text("OK") }
            },
            dismissButton = {
                TextButton(onClick = { showTimePicker = false }) { Text("Cancel") }
            },
            text = { TimePicker(timeState) }
        )
    }

    if (showDatePicker) {
        // DatePicker works in UTC-midnight millis; convert to local midnight on save.
        val dateState = rememberDatePickerState(initialSelectedDateMillis = localMidnightToUtc(dateMillis))
        DatePickerDialog(
            onDismissRequest = { showDatePicker = false },
            confirmButton = {
                TextButton(onClick = {
                    dateState.selectedDateMillis?.let { dateMillis = utcToLocalMidnight(it) }
                    showDatePicker = false
                }) { Text("OK") }
            },
            dismissButton = {
                TextButton(onClick = { showDatePicker = false }) { Text("Cancel") }
            }
        ) { DatePicker(dateState) }
    }
}

private fun localMidnightToday(): Long {
    val cal = Calendar.getInstance()
    cal.set(Calendar.HOUR_OF_DAY, 0)
    cal.set(Calendar.MINUTE, 0)
    cal.set(Calendar.SECOND, 0)
    cal.set(Calendar.MILLISECOND, 0)
    return cal.timeInMillis
}

private fun localMidnightToUtc(localMillis: Long): Long {
    val local = Calendar.getInstance().apply { timeInMillis = localMillis }
    val utc = Calendar.getInstance(TimeZone.getTimeZone("UTC"))
    utc.clear()
    utc.set(local.get(Calendar.YEAR), local.get(Calendar.MONTH), local.get(Calendar.DAY_OF_MONTH))
    return utc.timeInMillis
}

private fun utcToLocalMidnight(utcMillis: Long): Long {
    val utc = Calendar.getInstance(TimeZone.getTimeZone("UTC")).apply { timeInMillis = utcMillis }
    val local = Calendar.getInstance()
    local.clear()
    local.set(utc.get(Calendar.YEAR), utc.get(Calendar.MONTH), utc.get(Calendar.DAY_OF_MONTH))
    return local.timeInMillis
}
