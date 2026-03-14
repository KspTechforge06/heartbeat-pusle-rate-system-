#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MAX30105.h"
#include "heartRate.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

MAX30105 particleSensor;

const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;
float beatsPerMinute;
int beatAvg;

// ── Beat flash animation ──
bool heartBeat = false;
unsigned long beatFlashTime = 0;

// ── Fake ECG waveform buffer ──
#define ECG_WIDTH 64
int ecgBuffer[ECG_WIDTH];
int ecgIndex = 0;
bool beatTrigger = false;
int ecgPhase = 0;

// ── ECG waveform generator ──
// Simulates a simple PQRST-like shape on beat
int getECGSample() {
  if (beatTrigger) {
    ecgPhase = 1;
    beatTrigger = false;
  }
  int val = 0;
  if (ecgPhase > 0) {
    if      (ecgPhase <= 2)  val = 3;          // P wave
    else if (ecgPhase <= 4)  val = 0;
    else if (ecgPhase == 5)  val = -4;         // Q dip
    else if (ecgPhase == 6)  val = 20;         // R spike (tall)
    else if (ecgPhase == 7)  val = -6;         // S dip
    else if (ecgPhase <= 10) val = 5;          // T wave
    else if (ecgPhase <= 13) val = 2;
    else                     val = 0;
    ecgPhase++;
    if (ecgPhase > 20) ecgPhase = 0;
  }
  return val;
}

void setup() {
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextColor(WHITE);

  showSplash();

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 was not found.");
    while (1);
  }
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A);

  // init ECG buffer flat
  for (int i = 0; i < ECG_WIDTH; i++) ecgBuffer[i] = 0;
}

void loop() {
  long irValue = particleSensor.getIR();
  bool fingerOn = irValue >= 50000;

  if (fingerOn && checkForBeat(irValue)) {
    long delta = millis() - lastBeat;
    lastBeat = millis();
    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20) {
      rates[rateSpot++] = (byte)beatsPerMinute;
      rateSpot %= RATE_SIZE;
      beatAvg = 0;
      for (byte x = 0; x < RATE_SIZE; x++) beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }

    // trigger animations
    heartBeat = true;
    beatFlashTime = millis();
    beatTrigger = true;
  }

  // turn off heart flash after 180ms
  if (millis() - beatFlashTime > 180) heartBeat = false;

  // push new ECG sample
  ecgBuffer[ecgIndex % ECG_WIDTH] = getECGSample();
  ecgIndex++;

  // draw display
  drawDisplay(irValue, fingerOn);
}

// ─── Main Display ──────────────────────────────────────────

void drawDisplay(long irValue, bool fingerOn) {
  display.clearDisplay();

  if (!fingerOn) {
    drawNoFinger();
  } else {
    drawHeartRateScreen(irValue);
  }

  display.display();
}

// ─── No Finger Screen ─────────────────────────────────────

void drawNoFinger() {
  // Outer border
  display.drawRoundRect(0, 0, 128, 64, 3, WHITE);

  // Title
  display.setTextSize(1);
  display.setCursor(28, 4);
  display.println("PULSE MONITOR");

  // Divider
  display.drawLine(1, 14, 126, 14, WHITE);

  // Pulsing finger icon (3 concentric arcs)
  int cx = 64, cy = 38;
  display.drawCircle(cx, cy, 6,  WHITE);
  display.drawCircle(cx, cy, 11, WHITE);
  display.drawCircle(cx, cy, 16, WHITE);
  display.fillCircle(cx, cy, 4, WHITE);

  // Message
  display.setTextSize(1);
  display.setCursor(20, 55);
  display.println("Place Finger On Sensor");
}

// ─── Heart Rate Screen ────────────────────────────────────

void drawHeartRateScreen(long irValue) {

  // ── Top bar ──
  display.fillRect(0, 0, 128, 12, WHITE);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.setCursor(30, 2);
  display.print("PULSE MONITOR");
  display.setTextColor(WHITE);

  // ── BPM label ──
  display.setTextSize(1);
  display.setCursor(2, 16);
  display.print("BPM");

  // ── BPM value (large) ──
  display.setTextSize(3);
  if (beatAvg > 0) {
    int xPos = (beatAvg >= 100) ? 26 : 34;
    display.setCursor(xPos, 13);
    display.print(beatAvg);
  } else {
    display.setCursor(26, 13);
    display.print("--");
  }

  // ── Heart icon (flashes on beat) ──
  if (heartBeat) {
    drawHeartFilled(100, 14);
  } else {
    drawHeartOutline(100, 14);
  }

  // ── Divider ──
  display.drawLine(0, 38, 127, 38, WHITE);

  // ── ECG waveform (bottom strip) ──
  int baseline = 52;
  int startX   = 0;
  for (int i = 0; i < ECG_WIDTH - 1; i++) {
    int idx1 = (ecgIndex + i)     % ECG_WIDTH;
    int idx2 = (ecgIndex + i + 1) % ECG_WIDTH;
    int x1 = startX + i * 2;
    int x2 = startX + (i + 1) * 2;
    int y1 = baseline - ecgBuffer[idx1];
    int y2 = baseline - ecgBuffer[idx2];
    // clamp to display
    y1 = constrain(y1, 39, 63);
    y2 = constrain(y2, 39, 63);
    display.drawLine(x1, y1, x2, y2, WHITE);
  }

  // ── Signal strength dots ──
  int bars = map(constrain(irValue, 50000, 130000), 50000, 130000, 0, 5);
  display.setTextSize(1);
  display.setCursor(88, 42);
  display.print("SIG:");
  for (int i = 0; i < 5; i++) {
    if (i < bars) display.fillRect(88 + i * 7, 52, 5, 5, WHITE);
    else          display.drawRect(88 + i * 7, 52, 5, 5, WHITE);
  }

  // ── Status text ──
  display.setTextSize(1);
  display.setCursor(2, 56);
  if (beatAvg == 0) {
    display.print("Measuring...");
  } else if (beatAvg < 60) {
    display.print("LOW");
  } else if (beatAvg <= 100) {
    display.print("NORMAL");
  } else {
    display.print("HIGH");
  }
}

// ── Heart icons ──────────────────────────────────────────

void drawHeartFilled(int x, int y) {
  display.fillCircle(x + 3, y + 3, 3, WHITE);
  display.fillCircle(x + 9, y + 3, 3, WHITE);
  display.fillTriangle(x, y + 4, x + 12, y + 4, x + 6, y + 12, WHITE);
}

void drawHeartOutline(int x, int y) {
  display.drawCircle(x + 3, y + 3, 3, WHITE);
  display.drawCircle(x + 9, y + 3, 3, WHITE);
  display.drawTriangle(x, y + 4, x + 12, y + 4, x + 6, y + 12, WHITE);
  display.drawLine(x,      y + 4, x + 3, y + 2, WHITE);
  display.drawLine(x + 12, y + 4, x + 9, y + 2, WHITE);
}

void showSplash() {
  display.clearDisplay();
  display.drawRoundRect(0, 0, 128, 64, 3, WHITE);
  display.setTextSize(1);
  display.setCursor(28, 8);
  display.println("PULSE MONITOR");
  display.drawLine(1, 18, 126, 18, WHITE);
  drawHeartFilled(56, 22);
  display.setCursor(20, 42);
  display.println("Initializing...");
  display.display();
  delay(2000);
}
