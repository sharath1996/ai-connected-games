# AiluClock — Android App

An Android alarm app that fires the SmartClock firmware's `TRIGGER` command over a
USB OTG cable at the scheduled time: it sets the LED array color, shows a symbol on
the OLED, and optionally sounds the buzzer. See
[../docs/serial_commands.md](../docs/serial_commands.md) for the firmware-side protocol.

## Features

- One-time alarms (specific date + time) or daily repeating alarms
- Per-alarm trigger settings: LED color (R/G/B sliders), OLED symbol, buzzer on/off
- **Color Profiles**: save any editor configuration as a named profile ("Save as
  profile" in the alarm editor). Profiles appear as chips on the main screen —
  **tap to apply** instantly (sends TRIGGER right away), **long-press to delete**.
  This acts as app-side persistence: the clock itself is stateless, but your saved
  looks are one tap away.
- Exact alarms via `AlarmManager` (fires even in doze mode); alarms are re-armed after reboot
- USB status card with **Connect** and **Test** (sends `TRIGGER 255 0 0 ♥ 1`) buttons
- If the clock isn't connected when an alarm fires, a high-priority notification on the
  phone acts as the fallback alert

## Requirements

- **Android Studio** (any recent version, e.g. Ladybug or newer) — provides the Gradle
  toolchain and Android SDK this project builds with
- Android phone with **USB OTG** support (most modern phones) running **Android 8.0+**
- A **USB OTG adapter/cable** (USB-C or micro-USB depending on the phone)
- The SmartClock (ESP32-C3 Super Mini) running the firmware from this repo

## Build & install

1. Install Android Studio from <https://developer.android.com/studio> (default install
   includes the Android SDK and an emulator you won't need).
2. Open **this folder** (`android_app/`) in Android Studio.
3. Let Gradle sync (it downloads AGP, Kotlin, Compose, and `usb-serial-for-android`
   from JitPack on first sync — needs internet).
4. Connect your phone, enable **USB debugging** (Developer options), then press **Run**.
   Or use **Build > Build APK(s)** and sideload the APK.

There is no checked-in Gradle wrapper — Android Studio supplies its own Gradle on sync.

## Release build (for sharing with others)

Debug APKs are signed with your machine's debug key — different PCs produce
incompatible signatures, so recipients must uninstall first. A release APK signed
with one stable key avoids that; updates install cleanly on top.

1. **Create the keystore (once).** In Android Studio:
   **Build → Generate Signed App Bundle / APK → APK → Create new…**
   - Key store path: `android_app/ailuclock.keystore`
   - Alias: `ailuclock`, validity 25+ years
   - **Back up the keystore file and passwords** — losing them means you can never
     update installed apps again.
2. **Fill in `android_app/keystore.properties`** (already git-ignored) with the
   keystore password, alias, and key password.
3. **Build the signed APK**, either:
   - Android Studio: **Build → Generate Signed App Bundle / APK → APK**, choose the
     keystore, pick **release** → output in `app/release/`, or
   - Command line (from `android_app/`): `gradlew :app:assembleRelease` →
     `app/build/outputs/apk/release/app-release.apk`
4. Share `app-release.apk` (network share, local HTTP server, internal chat —
   recipients just tap to install and allow "unknown apps" for their browser).

## First-run setup

1. Plug the SmartClock into the phone via the OTG cable. The app should auto-launch
   (the manifest registers for the ESP32-C3's USB VID 0x303A); grant the USB permission
   dialog and optionally tick "use by default".
2. Otherwise open the app and tap **Connect**, then grant the permission.
3. Tap **Test** — the clock should turn red, show ♥, and beep, and the app shows the
   firmware's `ACK TRIGGER` reply.
4. Tap **+** to create an alarm: pick time, date (or daily repeat), LED color, OLED
   symbol, and buzzer on/off.
5. When prompted, allow **notifications** so the fallback alert can make sound.

## How it works

- `UsbSerialManager` opens the ESP32-C3's native USB CDC port at **115200 8N1** using
  the `usb-serial-for-android` library and writes one line: `TRIGGER R G B Emoji Sound\n`
- `AlarmScheduler` arms a `setExactAndAllowWhileIdle` alarm per entry; `AlarmReceiver`
  sends the command on a background thread, then re-arms daily alarms or disables
  one-shot alarms. `BootReceiver` re-arms everything after a reboot.
- Alarms persist as JSON in SharedPreferences (`AlarmStore`).

## Notes & limitations

- **OLED symbols**: the firmware font (`u8g2_font_unifont_t_symbols`) only contains
  simple symbols — ♥ ★ ☀ ☁ ☂ ♪ ⚡ ✓ ✗ are offered in the picker. Custom text is allowed
  but glyphs missing from the font render blank.
- **USB permission** persists while the device stays attached. If you unplug/replug the
  clock or reboot the phone, open the app once (or replug with auto-launch) to re-grant;
  otherwise the next alarm falls back to a phone notification.
- The phone powers the ESP32-C3 over OTG — fine for a few LEDs, but keep the phone
  charged for overnight alarms.
- `USE_EXACT_ALARM` is declared (this is an alarm app), so exact scheduling works
  without a settings prompt on Android 12+.
- Emoji are sent as UTF-8 bytes; the firmware parses the first token after B, so use a
  single symbol (no spaces).
