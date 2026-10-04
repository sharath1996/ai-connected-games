package com.smartclock.alarm.ui

import androidx.compose.foundation.background
import androidx.compose.foundation.horizontalScroll
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.material3.FilterChip
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Slider
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp

// Symbols known to exist in the firmware's OLED font (u8g2_font_unifont_t_symbols).
val EMOJI_CHOICES = listOf("♥", "★", "☀", "☁", "☂", "♪", "⚡", "✓", "✗")

@Composable
fun ColorSlider(label: String, value: Float, onChange: (Float) -> Unit) {
    Row(verticalAlignment = Alignment.CenterVertically) {
        Text(label, Modifier.width(12.dp))
        Slider(
            value = value,
            onValueChange = onChange,
            valueRange = 0f..255f,
            modifier = Modifier
                .weight(1f)
                .height(24.dp)
        )
        Text(
            "%3d".format(value.toInt()),
            Modifier.width(32.dp),
            style = MaterialTheme.typography.labelSmall
        )
    }
}

/** R/G/B sliders with a live color preview dot. */
@Composable
fun ColorPickerSection(r: Float, g: Float, b: Float, onR: (Float) -> Unit, onG: (Float) -> Unit, onB: (Float) -> Unit) {
    Text("LED color", style = MaterialTheme.typography.labelLarge)
    Row(verticalAlignment = Alignment.CenterVertically) {
        Box(
            Modifier
                .size(36.dp)
                .clip(CircleShape)
                .background(Color(r.toInt(), g.toInt(), b.toInt()))
        )
        Spacer(Modifier.width(12.dp))
        Column(Modifier.weight(1f)) {
            ColorSlider("R", r, onR)
            ColorSlider("G", g, onG)
            ColorSlider("B", b, onB)
        }
    }
}

/** Horizontal row of symbol chips. */
@Composable
fun EmojiPickerSection(emoji: String, onEmoji: (String) -> Unit) {
    Text("OLED symbol", style = MaterialTheme.typography.labelLarge)
    Row(Modifier.horizontalScroll(rememberScrollState())) {
        for (choice in EMOJI_CHOICES) {
            FilterChip(
                selected = emoji == choice,
                onClick = { onEmoji(choice) },
                label = { Text(choice) },
                modifier = Modifier.padding(end = 6.dp)
            )
        }
    }
}
