package com.smartclock.alarm

import android.Manifest
import android.content.Intent
import android.content.pm.PackageManager
import android.hardware.usb.UsbDevice
import android.hardware.usb.UsbManager
import android.os.Build
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.compose.setContent
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Add
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material.icons.filled.Notifications
import androidx.compose.material3.Card
import androidx.compose.material3.CenterAlignedTopAppBar
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.FloatingActionButton
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Scaffold
import androidx.compose.material3.SnackbarHost
import androidx.compose.material3.SnackbarHostState
import androidx.compose.material3.Switch
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.material3.darkColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateListOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import com.smartclock.alarm.alarm.Alarm
import com.smartclock.alarm.alarm.AlarmScheduler
import com.smartclock.alarm.alarm.AlarmStore
import com.smartclock.alarm.ui.AlarmEditorDialog
import com.smartclock.alarm.usb.UsbSerialManager
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale
import kotlin.concurrent.thread

class MainActivity : ComponentActivity() {

    private lateinit var store: AlarmStore
    private lateinit var usb: UsbSerialManager
    private val alarms = mutableStateListOf<Alarm>()
    private val usbStatus = mutableStateOf("No device")
    private val snackMsg = mutableStateOf<String?>(null)

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        store = AlarmStore(this)
        usb = UsbSerialManager.get(this)
        reloadAlarms()
        handleAttachIntent(intent)
        setContent { AppContent() }
    }

    override fun onNewIntent(intent: Intent) {
        super.onNewIntent(intent)
        handleAttachIntent(intent)
    }

    override fun onResume() {
        super.onResume()
        usbStatus.value = usb.status
    }

    private fun handleAttachIntent(intent: Intent?) {
        if (intent?.action != UsbManager.ACTION_USB_DEVICE_ATTACHED) return
        val device: UsbDevice? = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            intent.getParcelableExtra(UsbManager.EXTRA_DEVICE, UsbDevice::class.java)
        } else {
            @Suppress("DEPRECATION")
            intent.getParcelableExtra(UsbManager.EXTRA_DEVICE)
        }
        (device ?: usb.findDevice())?.let { requestPermissionAndConnect(it) }
    }

    private fun requestPermissionAndConnect(device: UsbDevice) {
        usb.requestPermission(device) { granted ->
            if (!granted) {
                showSnack("USB permission denied")
                return@requestPermission
            }
            thread {
                val connected = usb.connect()
                runOnUiThread {
                    usbStatus.value = usb.status
                    showSnack(if (connected) "SmartClock connected" else "Connect failed: ${usb.status}")
                }
            }
        }
    }

    private fun reloadAlarms() {
        alarms.clear()
        alarms.addAll(store.loadAll())
    }

    private fun saveAlarm(alarm: Alarm) {
        store.upsert(alarm)
        AlarmScheduler.schedule(this, alarm)
        reloadAlarms()
        showSnack("Alarm set for ${formatDateTime(alarm.nextTriggerMillis())}")
    }

    private fun deleteAlarm(alarm: Alarm) {
        store.delete(alarm.id)
        AlarmScheduler.cancel(this, alarm.id)
        reloadAlarms()
    }

    private fun toggleAlarm(alarm: Alarm, enabled: Boolean) {
        val updated = alarm.copy(enabled = enabled)
        store.upsert(updated)
        if (enabled) {
            AlarmScheduler.schedule(this, updated)
        } else {
            AlarmScheduler.cancel(this, updated.id)
        }
        reloadAlarms()
    }

    private fun showSnack(message: String) {
        snackMsg.value = message
    }

    // ---------------- UI ----------------

    @Composable
    private fun AppContent() {
        var showEditor by remember { mutableStateOf(false) }
        var editing by remember { mutableStateOf<Alarm?>(null) }
        val snackbarHostState = remember { SnackbarHostState() }

        val notificationPermission = rememberLauncherForActivityResult(
            ActivityResultContracts.RequestPermission()
        ) { /* granted or not, alarms still work silently */ }

        LaunchedEffect(Unit) {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU &&
                checkSelfPermission(Manifest.permission.POST_NOTIFICATIONS) != PackageManager.PERMISSION_GRANTED
            ) {
                notificationPermission.launch(Manifest.permission.POST_NOTIFICATIONS)
            }
        }

        LaunchedEffect(snackMsg.value) {
            snackMsg.value?.let {
                snackbarHostState.showSnackbar(it)
                snackMsg.value = null
            }
        }

        MaterialTheme(colorScheme = darkColorScheme()) {
            Scaffold(
                topBar = { SmartClockTopBar() },
                snackbarHost = { SnackbarHost(snackbarHostState) },
                floatingActionButton = {
                    FloatingActionButton(onClick = { editing = null; showEditor = true }) {
                        Icon(Icons.Default.Add, contentDescription = "Add alarm")
                    }
                }
            ) { padding ->
                Column(
                    Modifier
                        .padding(padding)
                        .fillMaxSize()
                ) {
                    UsbCard()
                    if (alarms.isEmpty()) {
                        Box(Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
                            Text("No alarms yet — tap + to add one", color = Color.Gray)
                        }
                    } else {
                        LazyColumn(Modifier.fillMaxSize()) {
                            items(alarms, key = { it.id }) { alarm ->
                                AlarmRow(
                                    alarm = alarm,
                                    onToggle = { enabled -> toggleAlarm(alarm, enabled) },
                                    onEdit = { editing = alarm; showEditor = true },
                                    onDelete = { deleteAlarm(alarm) }
                                )
                                HorizontalDivider(thickness = 0.5.dp)
                            }
                        }
                    }
                }

                if (showEditor) {
                    AlarmEditorDialog(
                        initial = editing,
                        onDismiss = { showEditor = false },
                        onSave = { alarm ->
                            val withId = if (alarm.id == 0) alarm.copy(id = store.nextId()) else alarm
                            saveAlarm(withId)
                            showEditor = false
                        },
                        onSaveBootDefaults = { r, g, b, brightness, emoji ->
                            sendBootDefaults(r, g, b, brightness, emoji)
                        }
                    )
                }
            }
        }
    }

    @OptIn(ExperimentalMaterial3Api::class)
    @Composable
    private fun SmartClockTopBar() {
        CenterAlignedTopAppBar(title = { Text("AiluClock") })
    }

    /** Sends PERSIST_LED + PERSIST_EMOJI so the clock boots into these defaults. */
    private fun sendBootDefaults(r: Int, g: Int, b: Int, brightness: Int?, emoji: String) {
        showSnack("Saving boot defaults…")
        thread {
            val ackLed = usb.sendPersistLed(r, g, b, brightness)
            val ackEmoji = usb.sendPersistEmoji(emoji)
            runOnUiThread {
                usbStatus.value = usb.status
                val ok = ackLed != null && ackEmoji != null
                showSnack(if (ok) "Boot defaults saved on the clock" else "Save failed — connect the clock first")
            }
        }
    }

    @Composable
    private fun UsbCard() {
        Card(
            Modifier
                .fillMaxWidth()
                .padding(16.dp)
        ) {
            Row(
                Modifier
                    .padding(horizontal = 16.dp, vertical = 8.dp)
                    .fillMaxWidth(),
                verticalAlignment = Alignment.CenterVertically
            ) {
                Column(Modifier.weight(1f)) {
                    Text("AiluClock (USB OTG)", style = MaterialTheme.typography.titleMedium)
                    Text(
                        usbStatus.value,
                        style = MaterialTheme.typography.bodySmall,
                        color = if (usbStatus.value == "Connected") Color(0xFF66BB6A) else Color.Gray
                    )
                }
                TextButton(onClick = {
                    val device = usb.findDevice()
                    if (device == null) {
                        showSnack("No USB device — plug in the SmartClock via OTG")
                    } else {
                        requestPermissionAndConnect(device)
                    }
                }) { Text("Connect") }
                TextButton(onClick = {
                    showSnack("Sending test trigger…")
                    thread {
                        val ack = usb.sendTrigger(255, 0, 0, "♥", true)
                        runOnUiThread {
                            usbStatus.value = usb.status
                            showSnack(ack?.let { "Clock replied: $it" } ?: "Send failed — not connected?")
                        }
                    }
                }) { Text("Test") }
            }
            Row(
                Modifier
                    .padding(start = 16.dp, end = 16.dp, bottom = 8.dp)
                    .fillMaxWidth(),
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    "Boot defaults: color + symbol shown at power-on",
                    style = MaterialTheme.typography.bodySmall,
                    color = Color.Gray,
                    modifier = Modifier.weight(1f)
                )
                TextButton(onClick = {
                    // Persist green ♥ as a safe factory-style default.
                    sendBootDefaults(0, 128, 0, null, "♥")
                }) { Text("Set as boot default") }
            }
        }
    }

    @Composable
    private fun AlarmRow(
        alarm: Alarm,
        onToggle: (Boolean) -> Unit,
        onEdit: () -> Unit,
        onDelete: () -> Unit
    ) {
        Row(
            Modifier
                .fillMaxWidth()
                .clickable(onClick = onEdit)
                .padding(horizontal = 16.dp, vertical = 8.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Box(
                Modifier
                    .size(14.dp)
                    .clip(CircleShape)
                    .background(Color(alarm.r, alarm.g, alarm.b))
            )
            Spacer(Modifier.width(12.dp))
            Column(Modifier.weight(1f)) {
                Text(
                    "%02d:%02d".format(alarm.hour, alarm.minute),
                    style = MaterialTheme.typography.headlineSmall
                )
                val subtitle = buildString {
                    append(if (alarm.repeatDaily) "Daily" else formatDate(alarm.dateMillis))
                    if (alarm.label.isNotBlank()) append("  •  ${alarm.label}")
                }
                Text(subtitle, style = MaterialTheme.typography.bodySmall, color = Color.Gray)
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Text(alarm.emoji, style = MaterialTheme.typography.bodyMedium)
                    if (alarm.sound) {
                        Spacer(Modifier.width(6.dp))
                        Icon(
                            Icons.Default.Notifications,
                            contentDescription = "Sound on",
                            modifier = Modifier.size(14.dp),
                            tint = Color.Gray
                        )
                    }
                }
            }
            Switch(checked = alarm.enabled, onCheckedChange = onToggle)
            IconButton(onClick = onDelete) {
                Icon(Icons.Default.Delete, contentDescription = "Delete", tint = Color.Gray)
            }
        }
    }

    private fun formatDate(dateMillis: Long?): String =
        if (dateMillis == null) "One-time"
        else SimpleDateFormat("EEE, MMM d", Locale.getDefault()).format(Date(dateMillis))

    private fun formatDateTime(millis: Long): String =
        SimpleDateFormat("EEE, MMM d, HH:mm", Locale.getDefault()).format(Date(millis))
}
