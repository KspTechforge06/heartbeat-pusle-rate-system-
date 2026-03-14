# 💓 ESP8266 Heart Rate Monitor

A real-time heart rate monitor built with **ESP8266**, **MAX30102** pulse oximeter sensor, and a **0.96" SSD1306 OLED display**. Displays live BPM with an animated ECG waveform and beat-flash heart icon.

---

## 📸 Preview

```
┌──────────────────────────────┐
│ ░░░░░░ PULSE MONITOR ░░░░░░░ │   ← Inverted header
│                              │
│ BPM              ♥           │   ← Heart flashes on beat
│      78                      │   ← Large BPM value
│                              │
│──────────────────────────────│
│ /\/\___/\__   SIG: ■■■■□    │   ← Live ECG + signal bar
│ NORMAL                       │   ← Status label
└──────────────────────────────┘
```

---

## 🛒 Hardware Required

| Component | Details |
|-----------|---------|
| ESP8266 | NodeMCU v3 or Wemos D1 Mini |
| MAX30102 | Heart rate & pulse oximeter sensor |
| OLED Display | 0.96" SSD1306 — 128×64 pixels, I2C |
| Jumper Wires | Male-to-female |
| Breadboard | Optional |

---

## 🔌 Wiring

Both OLED and MAX30102 use **I2C** — they share the same two data pins.

| ESP8266 Pin | OLED SSD1306 | MAX30102 |
|-------------|-------------|---------|
| 3.3V | VCC | VIN |
| GND | GND | GND |
| D1 (GPIO5) | SCK | SCL |
| D2 (GPIO4) | SDA | SDA |

> ⚠️ The MAX30102 INT pin is not used — leave it unconnected.

---

## 📦 Libraries

Install all from **Arduino IDE → Tools → Manage Libraries**:

| Library | Author |
|---------|--------|
| `Adafruit SSD1306` | Adafruit |
| `Adafruit GFX Library` | Adafruit |
| `SparkFun MAX3010x Pulse and Proximity Sensor Library` | SparkFun |

---

## 🚀 Getting Started

1. **Clone this repo**
   ```bash
   git clone https://github.com/KspTechforge06/heartbeat-pusle-rate-system-.git
   ```

2. **Open in Arduino IDE**
   ```
   File → Open → heart_rate_monitor.ino
   ```

3. **Select board**
   ```
   Tools → Board → NodeMCU 1.0 (ESP-12E Module)
   ```

4. **Select port**
   ```
   Tools → Port → (your COM port)
   ```

5. **Upload** and open Serial Monitor at `115200` baud

---

## 📟 How It Works

### Sensor
The **MAX30102** emits red and infrared light into your fingertip. Blood absorbs different amounts of light depending on the pulse cycle. The sensor reads the reflected IR value — this oscillates with every heartbeat.

### Beat Detection
The firmware uses the **SparkFun `heartRate.h` `checkForBeat()`** function which applies a derivative-based threshold algorithm on the raw IR waveform to detect each beat peak.

### BPM Calculation
```
BPM = 60 / (time between beats in seconds)
```
A rolling average of the last **4 readings** smooths out noise.

### Display
The OLED renders:
- **Large BPM value** — averaged over last 4 beats
- **Animated heart icon** — flashes solid white on every detected beat
- **Scrolling ECG waveform** — PQRST-style simulation triggered on each beat
- **Signal strength bar** — 5-segment bar mapped from raw IR value
- **Status label** — `NORMAL` (60–100), `LOW` (< 60), `HIGH` (> 100), `Measuring...`

---

## 🖐️ Usage Tips

- Place your **index fingertip** flat on the sensor
- Let it sit by **its own weight** — do NOT press hard
- Keep your **hand still** on a flat surface
- Wait **5–10 seconds** for the first stable reading
- Normal resting BPM: **60–100**

> ⚠️ Pressing too hard squeezes blood from the capillaries and kills the pulse signal — the most common cause of wrong readings!

---

## 📁 File Structure

```
esp8266-heart-rate-monitor/
│
├── heart_rate_monitor.ino   # Main firmware
├── README.md                # This file
└── assets/
    └── wiring_diagram.png   # Wiring reference (optional)
```

---

## 🐛 Troubleshooting

| Problem | Cause | Fix |
|---------|-------|-----|
| `MAX30102 Not Found` | Wiring issue | Check SDA/SCL pins and 3.3V power |
| BPM shows `--` | No finger or warming up | Place finger, wait 5–10 sec |
| BPM very low (5–10) | Finger pressed too hard | Lighten finger pressure |
| BPM jumping wildly | Finger moving | Rest hand on flat surface |
| OLED blank | Wrong I2C address | Try `0x3D` instead of `0x3C` |
| Stuck on "No Finger" | IR threshold issue | Check Serial Monitor IR values |

### Serial Monitor Debug

Open Serial Monitor at **115200 baud**. You should see:

```
IR=121408  RED=79892
IR=121396  RED=79863
```

| IR Value | Meaning |
|----------|---------|
| `< 1000` | Wiring issue |
| `1000 – 50000` | No finger detected |
| `50000 – 80000` | Weak contact |
| `> 80000` | Good signal ✅ |
| `> 100000` | Excellent signal ✅ |

---

## ⚙️ Configuration

You can tweak these defines at the top of the sketch:

```cpp
const byte RATE_SIZE = 4;   // Number of beats to average (increase for smoother readings)
```

And sensor LED brightness in `setup()`:
```cpp
particleSensor.setPulseAmplitudeRed(0x0A);  // Increase to 0x1F or 0x3F for weak signals
```

---

## 📋 Specifications

| Parameter | Value |
|-----------|-------|
| Microcontroller | ESP8266 (80 MHz) |
| Sensor | MAX30102 |
| Display | SSD1306 128×64 OLED |
| Communication | I2C (400 kHz) |
| BPM Range | 20 – 255 |
| Sample Average | 4 beats |
| Serial Baud Rate | 115200 |

---

## 📜 License

MIT License — free to use, modify and distribute.

---

## 🙌 Credits

- [SparkFun MAX3010x Library](https://github.com/sparkfun/SparkFun_MAX3010x_Sensor_Library)
- [Adafruit SSD1306 Library](https://github.com/adafruit/Adafruit_SSD1306)
- Built with ❤️ using ESP8266 + Arduino

---

> Made by **KSP** 
