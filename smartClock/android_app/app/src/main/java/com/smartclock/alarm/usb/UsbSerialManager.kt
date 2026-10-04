package com.smartclock.alarm.usb

import android.app.PendingIntent
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.hardware.usb.UsbDevice
import android.hardware.usb.UsbManager
import androidx.core.content.ContextCompat
import com.hoho.android.usbserial.driver.UsbSerialPort
import com.hoho.android.usbserial.driver.UsbSerialProber

/**
 * Owns the USB-serial link to the SmartClock (ESP32-C3 native USB CDC).
 * Serial settings: 115200 8N1, matching `Serial.begin(115200)` in the firmware.
 *
 * All public methods are safe to call from any thread; write operations are
 * blocking with timeouts, so call them off the main thread.
 */
class UsbSerialManager private constructor(context: Context) {

    private val appContext = context.applicationContext
    private val usbManager = appContext.getSystemService(Context.USB_SERVICE) as UsbManager

    @Volatile
    private var port: UsbSerialPort? = null

    @Volatile
    var status: String = "No device"
        private set

    private var permissionCallback: ((Boolean) -> Unit)? = null

    private val permissionReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
            if (intent.action != ACTION_USB_PERMISSION) return
            val granted = intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)
            try {
                appContext.unregisterReceiver(this)
            } catch (_: Exception) {
                // already unregistered
            }
            val callback = permissionCallback
            permissionCallback = null
            callback?.invoke(granted)
        }
    }

    /** First attached device the prober recognizes, or null. */
    fun findDevice(): UsbDevice? =
        UsbSerialProber.getDefaultProber().findAllDrivers(usbManager).firstOrNull()?.device

    /** Shows the system USB permission dialog if needed; [onResult] fires with the outcome. */
    fun requestPermission(device: UsbDevice, onResult: (Boolean) -> Unit) {
        if (usbManager.hasPermission(device)) {
            onResult(true)
            return
        }
        permissionCallback = onResult
        try {
            ContextCompat.registerReceiver(
                appContext,
                permissionReceiver,
                IntentFilter(ACTION_USB_PERMISSION),
                ContextCompat.RECEIVER_NOT_EXPORTED
            )
            val pendingIntent = PendingIntent.getBroadcast(
                appContext,
                0,
                Intent(ACTION_USB_PERMISSION).setPackage(appContext.packageName),
                PendingIntent.FLAG_IMMUTABLE
            )
            usbManager.requestPermission(device, pendingIntent)
        } catch (e: Exception) {
            permissionCallback = null
            status = "Error: ${e.message}"
            onResult(false)
        }
    }

    /** Opens the first available serial port. Requires permission to be granted already. */
    @Synchronized
    fun connect(): Boolean {
        disconnect()
        val driver = UsbSerialProber.getDefaultProber().findAllDrivers(usbManager).firstOrNull()
        if (driver == null) {
            status = "No device"
            return false
        }
        if (!usbManager.hasPermission(driver.device)) {
            status = "No permission"
            return false
        }
        val connection = usbManager.openDevice(driver.device)
        if (connection == null) {
            status = "Open failed"
            return false
        }
        val newPort = driver.ports.firstOrNull()
        if (newPort == null) {
            status = "No port"
            return false
        }
        return try {
            newPort.open(connection)
            newPort.setParameters(
                115200,
                8,
                UsbSerialPort.STOPBITS_1,
                UsbSerialPort.PARITY_NONE
            )
            newPort.dtr = true
            newPort.rts = true
            port = newPort
            status = "Connected"
            true
        } catch (e: Exception) {
            status = "Error: ${e.message}"
            try {
                newPort.close()
            } catch (_: Exception) {
            }
            false
        }
    }

    /**
     * Sends one newline-terminated command and waits briefly for the firmware's
     * ACK/ERR reply. Returns the reply line, or null on failure/timeout.
     * Reconnects lazily if the port is not open.
     */
    @Synchronized
    fun sendCommand(command: String): String? {
        var activePort = port
        if (activePort == null) {
            if (!connect()) return null
            activePort = port
        }
        activePort ?: return null
        return try {
            activePort.write((command.trim() + "\n").toByteArray(Charsets.UTF_8), WRITE_TIMEOUT_MS)
            readReply(activePort)
        } catch (e: Exception) {
            status = "Error: ${e.message}"
            disconnect()
            null
        }
    }

    /** Builds and sends the firmware's TRIGGER command (see docs/serial_commands.md). */
    fun sendTrigger(r: Int, g: Int, b: Int, emoji: String, sound: Boolean): String? =
        sendCommand("TRIGGER $r $g $b $emoji ${if (sound) 1 else 0}")

    /** Saves default LED color/brightness in the clock's flash (PERSIST_LED). */
    fun sendPersistLed(r: Int, g: Int, b: Int, brightness: Int? = null): String? =
        if (brightness == null) sendCommand("PERSIST_LED $r $g $b")
        else sendCommand("PERSIST_LED $r $g $b $brightness")

    /** Saves the default OLED symbol in the clock's flash (PERSIST_EMOJI). */
    fun sendPersistEmoji(emoji: String): String? =
        sendCommand("PERSIST_EMOJI $emoji")

    @Synchronized
    fun disconnect() {
        try {
            port?.close()
        } catch (_: Exception) {
        }
        port = null
    }

    private fun readReply(activePort: UsbSerialPort): String? {
        val buffer = ByteArray(256)
        val reply = StringBuilder()
        val deadline = System.currentTimeMillis() + REPLY_TIMEOUT_MS
        while (System.currentTimeMillis() < deadline) {
            val count = try {
                activePort.read(buffer, READ_SLICE_TIMEOUT_MS)
            } catch (e: Exception) {
                break
            }
            if (count > 0) {
                reply.append(String(buffer, 0, count, Charsets.UTF_8))
                if (reply.contains('\n')) break
            } else if (reply.isNotEmpty()) {
                break
            }
        }
        return reply.toString().trim().ifEmpty { null }
    }

    companion object {
        private const val ACTION_USB_PERMISSION = "com.smartclock.alarm.USB_PERMISSION"
        private const val WRITE_TIMEOUT_MS = 2000
        private const val READ_SLICE_TIMEOUT_MS = 300
        private const val REPLY_TIMEOUT_MS = 1200

        @Volatile
        private var instance: UsbSerialManager? = null

        fun get(context: Context): UsbSerialManager =
            instance ?: synchronized(this) {
                instance ?: UsbSerialManager(context.applicationContext).also { instance = it }
            }
    }
}
