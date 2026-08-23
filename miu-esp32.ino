#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <Wire.h>
#include <ESP32Servo.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#define BUZZER_PIN 16
#include "face-bitmaps.h"
#include "movement-sequences.h"
#include "captive-portal.h"

// --- Access Point Configuration ---
// Bağlanamazsa Miu kendi ağını oluşturur
#define AP_SSID  "miu-controller"
#define AP_PASS  "YOUR_PASSWORD"

// --- Station Mode Configuration ---
// Miu önce bu ağa bağlanmayı dener; başarısız olursa AP oluşturur
#define NETWORK_SSID "YOUR_WIFI_SSIF"
#define NETWORK_PASS "YOUR_PASSWORD"
#define ENABLE_NETWORK_MODE true

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_I2C_ADDR 0x3C

// I2C Pins for Distro Board V2 / V3
//#define I2C_SDA 8
//#define I2C_SCL 9

// I2C Pins for Distro Board V1
//#define I2C_SDA 21
//#define I2C_SCL 22

// I2C Pins for S2 Mini Board
#define I2C_SDA 33
#define I2C_SCL 35


// DNS Server for Captive Portal
DNSServer dnsServer;
const byte DNS_PORT = 53;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
WebServer server(80);

// Global state for animations
String currentCommand = "";
String currentFaceName = "default";
const unsigned char* const* currentFaceFrames = nullptr;
uint8_t currentFaceFrameCount = 0;
uint8_t currentFaceFrameIndex = 0;
unsigned long lastFaceFrameMs = 0;
int faceFps = 8;
FaceAnimMode currentFaceMode = FACE_ANIM_LOOP;
int8_t faceFrameDirection = 1;
bool faceAnimFinished = false;
int currentFaceFps = 0;
bool idleActive = false;
bool idleBlinkActive = false;
unsigned long nextIdleBlinkMs = 0;
uint8_t idleBlinkRepeatsLeft = 0;

// WiFi Info Scrolling
unsigned long lastInputTime = 0;
bool firstInputReceived = false;
bool showingWifiInfo = false;
int wifiScrollPos = 0;
unsigned long lastWifiScrollMs = 0;
String wifiInfoText = "";

// Network Mode
bool networkConnected = false;
IPAddress networkIP;
String deviceHostname = "miu-robot";

// Servo Pins for Distro Board
// ======================================================================
// Pin numbers are coorisponding to the ESP32 GPIO pins and may differ based on which board you use.
// If you are using a different board, please adjust the servoPins array accordingly.
// ======================================================================
Servo servos[8];
// Miu Distro Board V3 Pinout [NEW]
//const int servoPins[8] = {4, 5, 6, 7, 10, 11, 12, 13};

// Miu Distro Board V2 Pinout (Legacy)
//const int servoPins[8] = {4, 5, 6, 7, 15, 16, 17, 18};

// Miu Distro Board V1 Pinout (Legacy)
//const int servoPins[8] = {15, 2, 23, 19, 4, 16, 17, 18};

// Lolin S2 Mini Pinout
const int servoPins[8] = {1, 2, 4, 6, 8, 10, 13, 14};

// Subtrim values for each servo (offset in degrees)
int8_t servoSubtrim[8] = {0, 0, 0, 0, 0, 0, 0, 0};

static const char* const autoPoses[] = {"wave", "dance", "point", "bow", "cute", "freaky", "worm", "shake", "shrug", "dead", "crab", "knead"};

// ======================================================================
// --- TOUCH SENSOR (Head Pet Sensor) ---
// GPIO 12: Boş ve ESP32-S2 touch-capable pin (servo pinleri disinda)
// ESP32-S2'de touchRead() kapasitif deger döndürür:
//   - Dokunulmadığında: yüksek değer (örn. 40000-80000)
//   - Dokunulduğunda:   düşük değer (örn. 5000-15000)
// ======================================================================
// --- TOUCH SENSOR (TTP223 Digital Touch Sensor) ---
// ======================================================================
#define TOUCH_PIN 12

bool touchActive = false;
bool touchWasActive = false;
unsigned long lastTouchPollMs = 0;
unsigned long touchCooldownMs = 0;  // Touch emote tekrar tetiklenmesini önlemek için
const unsigned long TOUCH_COOLDOWN = 4000;  // 4 saniye cooldown

// ======================================================================
// --- BUZZER (Passive) ---
// GPIO 16: Pasif buzzer (PWM suruculu)
// BAGLANTI: GPIO 16 --[100 ohm]--> Buzzer+ --> Buzzer- --> GND
// ======================================================================
// GPIO 16: Pasif buzzer
// BAGLANTI: GPIO 16 --[100 ohm]--> Buzzer+ --> Buzzer- --> GND
// ======================================================================
// ESP32 Core 3.x native tone() kullanilacak

static const int melodyStartupFreqs[] = {523, 659, 784, 1047, 784, 1047};
static const int melodyStartupDurs[]  = {80,  80,  80,  200,  60,  300 };
const int melodyStartupLen = 6;

static const int melodyLoveFreqs[] = {1047, 988, 1047, 1175, 988, 784, 880};
static const int melodyLoveDurs[]  = {100,  80,  100,  150,  100, 150, 250};
const int melodyLoveLen = 7;

// ======================================================================
// --- JUKEBOX MELODILERI ---
// ======================================================================
static const int melodyMarioFreqs[]  = {659,659,0,659,0,523,659,0,784,0,392,0,523,0,392,0,330,0,440,494,466,440,0,392,659,784,880,698,784,0,659,523,554,494};
static const int melodyMarioDurs[]   = {150,150,150,150,150,150,150,300,150,300,150,300,150,300,150,300,150,300,150,150,150,150,300,100,100,100,150,150,150,300,150,150,150,150};
const int melodyMarioLen = 34;

static const int melodyNokiaFreqs[]  = {659,587,370,415,554,494,294,330,523,466,280,349,392,392,392};
static const int melodyNokiaDurs[]   = {125,125,250,250,125,125,500,125,125,250,250,125,125,250,500};
const int melodyNokiaLen = 15;

static const int melodyBirthdayFreqs[] = {264,264,297,264,352,330,264,264,297,264,396,352,264,264,528,440,352,330,297};
static const int melodyBirthdayDurs[]  = {150,150,300,300,300,600,150,150,300,300,300,600,150,150,300,300,300,300,600};
const int melodyBirthdayLen = 19;

static const int melodyStarWarsFreqs[] = {440,440,440,349,523,440,349,523,440,659,659,659,698,523,415,349,523,440};
static const int melodyStarWarsDurs[]  = {500,500,500,350,150,500,350,150,1000,500,500,500,350,150,500,350,150,1000};
const int melodyStarWarsLen = 18;

static const int melodyTetrisFreqs[] = {659,494,523,587,523,494,440,440,523,659,587,523,494,523,587,659,523,440,440};
static const int melodyTetrisDurs[]  = {150,75,75,150,75,75,150,150,150,150,75,75,150,75,75,150,150,150,300};
const int melodyTetrisLen = 19;

static const int melodySurpriseFreqs[] = {523,659,784,1047,1319,1047,784,659,523};
static const int melodySurpriseDurs[]  = {80, 80, 80, 100, 300, 100, 80, 80, 400};
const int melodySurpriseLen = 9;

const int* buzzerFreqPtr = nullptr;
const int* buzzerDurPtr  = nullptr;
int        buzzerLen     = 0;
int        buzzerIdx     = 0;
unsigned long buzzerNextMs = 0;

// ======================================================================
// --- BATTERY ADC ---
// GPIO 3: ADC pin (ESP32-S2)
// DONANIM: 100kohm (bat+ -> GPIO3) + 100kohm (GPIO3 -> GND) voltaj bolucusu
// LiPo 3.7V: 4.2V = %100 | 3.7V ≈ %50 | 3.2V = %0
// ======================================================================
#define BATTERY_PIN 3
int batteryPercent = -1;
unsigned long lastBatteryMs = 0;
const unsigned long BATTERY_INTERVAL = 30000;

// ======================================================================
// --- MOOD SYSTEM ---
// Miu'nun ruh hali: etkilesim + rastgelelik
// ======================================================================
enum MoodState : uint8_t {
  MOOD_HAPPY   = 0,
  MOOD_NEUTRAL = 1,
  MOOD_SAD     = 2,
  MOOD_EXCITED = 3,
  MOOD_CURIOUS = 4,
  MOOD_SLEEPY  = 5,
  MOOD_ANGRY   = 6,  // Gidiklama kizginligi icin
  MOOD_COLD    = 7   // Usume modu
};

MoodState currentMood    = MOOD_NEUTRAL;
String    moodIdleFace   = "idle";
unsigned long nextMoodChangeMs = 0;

static const char* const moodFaceNames[] = {"happy", "idle", "sad", "excited", "thinking", "sleepy", "angry", "sad"};

// --- NEW CUTE FEATURES STATE ---
bool hiccupsActive = false;
unsigned long nextHiccupCheckMs = 0;
unsigned long nextHiccupEventMs = 0;
uint8_t hiccupCureCount = 0;

unsigned long nextSleepTwitchMs = 0;
unsigned long nextShiverMs = 0;
bool shiverState = false;
unsigned long touchStartMs = 0;
unsigned long lastHiccupTapMs = 0;

// NEW CUTE FEATURES STATE
unsigned long nextDreamMs = 0;
unsigned long nextBellyRumbleMs = 0;
unsigned long lastTickleTapMs = 0;
uint8_t tickleTapCount = 0;
bool isCatResting = false;
// -------------------------------

// Kontrol modu
bool joystickEnabled = true;

// Otonom Mod (Pet Mode)
bool autonomousMode = false;
unsigned long nextAutoActionMs = 0;

// ======================================================================
// --- SPEECH BUBBLE & OLED LOCK ---
// ======================================================================
unsigned long speechEndTime = 0;
bool speechActive = false;
String speechMessage = "";
unsigned long oledLockTime = 0;

// ======================================================================
// --- CUSTOM MELODY BUFFER ---
// ======================================================================
#define MAX_CUSTOM_NOTES 32
int customFreqs[MAX_CUSTOM_NOTES];
int customDurs[MAX_CUSTOM_NOTES];
int customLen = 0;

// ======================================================================
// --- PET INTERACTIONS (Feed, Tickle, Bleeps) ---
// ======================================================================
unsigned long lastFedMs = 0;
volatile int tickleCount = 0;   // volatile: HTTP handler'dan degistiriliyor
unsigned long lastTickleMs = 0;
unsigned long nextIdleBleepMs = 0;
int hungerPercent = 100;
unsigned long lastHungerUpdateMs = 0;

// ======================================================================
// --- LOVE METER ---
// ======================================================================
int lovePercent = 50;
unsigned long lastLoveUpdateMs = 0;
bool lovePeakTriggered = false;  // %100 dogum sonu melodisi icin

// ======================================================================
// --- DAILY LOOT BOX ---
// ======================================================================
unsigned long lastLootMs = 0;
const unsigned long LOOT_COOLDOWN = 3600000UL; // 1 saat cooldown

// ======================================================================
// --- CUSTOM PIXEL ART FACE ---
// ======================================================================
uint8_t customFaceData[1024] = {0};
const unsigned char* const customFaceFrames[] = { customFaceData };

// Animation constants
int frameDelay = 100;
int walkCycles = 10;
int motorCurrentDelay = 20; // ms delay between motor movements to prevent over-current

struct FaceEntry {
  const char* name;
  const unsigned char* const* frames;
  uint8_t maxFrames;
};

static const uint8_t MAX_FACE_FRAMES = 6;

#define MAKE_FACE_FRAMES(name) \
  const unsigned char* const face_##name##_frames[] = { \
    epd_bitmap_##name, epd_bitmap_##name##_1, epd_bitmap_##name##_2, \
    epd_bitmap_##name##_3, epd_bitmap_##name##_4, epd_bitmap_##name##_5 \
  };

#define X(name) MAKE_FACE_FRAMES(name)
FACE_LIST
#undef X
#undef MAKE_FACE_FRAMES

const FaceEntry faceEntries[] = {
#define X(name) { #name, face_##name##_frames, MAX_FACE_FRAMES },
  FACE_LIST
#undef X
  { "default", face_default_frames, MAX_FACE_FRAMES }
};

struct FaceFpsEntry {
  const char* name;
  uint8_t fps;
};

const FaceFpsEntry faceFpsEntries[] = {
  { "walk", 1 },
  { "rest", 1 },
  { "swim", 1 },
  { "dance", 1 },
  { "wave", 1 },
  { "point", 5 },
  { "stand", 1 },
  { "cute", 1 },
  { "pushup", 1 },
  { "freaky", 1 },
  { "bow", 1 },
  { "worm", 1 },
  { "shake", 1 },
  { "shrug", 1 },
  { "dead", 2 },
  { "crab", 1 },
  { "idle", 1 },
  { "idle_blink", 7 },
  { "default", 1 },
  // Conversational faces (manually controlled by Python - no auto-animation)
  { "happy", 1 },
  { "talk_happy", 1 },
  { "sad", 1 },
  { "talk_sad", 1 },
  { "angry", 1 },
  { "talk_angry", 1 },
  { "surprised", 1 },
  { "talk_surprised", 1 },
  { "sleepy", 1 },
  { "talk_sleepy", 1 },
  { "love", 1 },
  { "talk_love", 1 },
  { "excited", 1 },
  { "talk_excited", 1 },
  { "confused", 1 },
  { "talk_confused", 1 },
  { "thinking", 1 },
  { "talk_thinking", 1 },
};


// Prototypes
void setServoAngle(uint8_t channel, int angle);
void updateFaceBitmap(const unsigned char* bitmap);
void setFace(const String& faceName);
void setFaceMode(FaceAnimMode mode);
void setFaceWithMode(const String& faceName, FaceAnimMode mode);
void updateAnimatedFace();
void delayWithFace(unsigned long ms);
void enterIdle();
void exitIdle();
void updateIdleBlink();
int getFaceFpsForName(const String& faceName);
bool pressingCheck(String cmd, int ms);
void handleGetSettings();
void handleSetSettings();
void handleGetStatus();
void handleApiCommand();
void handleGetTouchStatus();
void handleSetFace();
void handleGetBattery();
void handleSpeech();
void handleCustomMelody();
void updateWifiInfoScroll();
void recordInput();
void updateTouchSensor();
void playMelody(const int* freqs, const int* durs, int len);
void updateBuzzer();
void updateBattery();
void updateMood();
void checkAutoRest();
void showIPOnOLED();
void setAvatarPose(int pitch, int roll);

void handleRoot() {
  server.send(200, "text/html", index_html);
}

void handleCommandWeb() {
  // We send 200 OK immediately so the web browser doesn't hang waiting for animation to finish
  if (server.hasArg("pose")) {
    currentCommand = server.arg("pose");
    recordInput();
    exitIdle();
    server.send(200, "text/plain", "OK"); 
  } 
  else if (server.hasArg("go")) {
    currentCommand = server.arg("go");
    recordInput();
    exitIdle();
    server.send(200, "text/plain", "OK");
  } 
  else if (server.hasArg("stop")) {
    currentCommand = "";
    recordInput();
    server.send(200, "text/plain", "OK");
  }
  else if (server.hasArg("avatarPitch") && server.hasArg("avatarRoll")) {
    currentCommand = "avatar";
    int pitch = server.arg("avatarPitch").toInt();
    int roll = server.arg("avatarRoll").toInt();
    setAvatarPose(pitch, roll);
    recordInput();
    exitIdle();
    server.send(200, "text/plain", "OK");
  }
  else if (server.hasArg("motor") && server.hasArg("value")) {
    int motorNum = server.arg("motor").toInt();
    int servoIdx = servoNameToIndex(server.arg("motor"));
    int angle = server.arg("value").toInt();
    if (motorNum >= 1 && motorNum <= 8 && angle >= 0 && angle <= 180) {
      setServoAngle(motorNum - 1, angle); // Convert 1-based to 0-based index
      recordInput();
      server.send(200, "text/plain", "OK");
    } else if (server.arg("motor") == "0" && angle >= 0 && angle <= 180) {
      setServoAngle(0, angle);
      recordInput();
      server.send(200, "text/plain", "OK");
    } else if (servoIdx != -1 && angle >= 0 && angle <= 180) {
      setServoAngle(servoIdx, angle);
      recordInput();
      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "Invalid motor or angle");
    }
  }
  else {
    server.send(400, "text/plain", "Bad Args");
  }
}

void setAvatarPose(int pitch, int roll) {
  // Map angles to servo offset (-45 to 45 deg maps to -30 to 30 servo deg)
  int p_offset = map(constrain(pitch, -45, 45), -45, 45, -30, 30);
  int r_offset = map(constrain(roll, -45, 45), -45, 45, -30, 30);
  
  // Inverse Kinematics Logic across 4 shoulder servos:
  // Pitch > 0 (lean forward), Roll > 0 (lean right)
  int r1 = constrain(135 + p_offset - r_offset, 0, 180);
  int l1 = constrain(45 + p_offset + r_offset, 0, 180);
  int r2 = constrain(45 + p_offset - r_offset, 0, 180);
  int l2 = constrain(135 + p_offset + r_offset, 0, 180);

  // Apply angles immediately
  setServoAngle(0, r1);  // R1
  setServoAngle(1, r2);  // R2
  setServoAngle(2, l1);  // L1
  setServoAngle(3, l2);  // L2
  setServoAngle(4, 0);   // R4
  setServoAngle(5, 180); // R3
  setServoAngle(6, 0);   // L3
  setServoAngle(7, 180); // L4
}

void handleGetSettings() {
  String json;
  json.reserve(256);
  json = "{";
  json += "\"frameDelay\":" + String(frameDelay) + ",";
  json += "\"walkCycles\":" + String(walkCycles) + ",";
  json += "\"motorCurrentDelay\":" + String(motorCurrentDelay) + ",";
  json += "\"faceFps\":" + String(faceFps) + ",";
  json += "\"joystickEnabled\":" + String(joystickEnabled ? "true" : "false") + ",";
  json += "\"autonomousMode\":" + String(autonomousMode ? "true" : "false") + ",";
  json += "\"batteryPercent\":" + String(batteryPercent);
  json += "}";
  server.send(200, "application/json", json);
}

void handleSetSettings() {
  if (server.hasArg("frameDelay")) frameDelay = server.arg("frameDelay").toInt();
  if (server.hasArg("walkCycles")) walkCycles = server.arg("walkCycles").toInt();
  if (server.hasArg("motorCurrentDelay")) motorCurrentDelay = server.arg("motorCurrentDelay").toInt();
  if (server.hasArg("faceFps")) faceFps = (int)max(1L, server.arg("faceFps").toInt());
  if (server.hasArg("joystickEnabled")) joystickEnabled = (server.arg("joystickEnabled") == "true");
  if (server.hasArg("autonomousMode")) autonomousMode = (server.arg("autonomousMode") == "true");
  server.send(200, "text/plain", "OK");
}

// Touch sensor status endpoint
void handleGetTouchStatus() {
  String json = "{";
  json += "\"touched\":" + String(touchActive ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}

// Yuz degistirme endpoint'i
void handleSetFace() {
  if (!server.hasArg("face")) { server.send(400, "text/plain", "Missing face"); return; }
  String face = server.arg("face");
  setFace(face);
  if (idleActive) moodIdleFace = face;
  recordInput();
  server.send(200, "text/plain", "OK");
}

// Batarya seviyesi endpoint'i
void handleGetBattery() {
  String json = "{\"percent\":" + String(batteryPercent) + "}";
  server.send(200, "application/json", json);
}

// Konusma balonu endpoint'i
void showSpeech(String msg);
void handleSpeech() {
  if (!server.hasArg("text")) { server.send(400, "text/plain", "Missing text"); return; }
  String text = server.arg("text");
  showSpeech(text);
  recordInput();
  server.send(200, "text/plain", "OK");
}

// Ozel melodi endpoint'i
void handleCustomMelody() {
  if (!server.hasArg("freqs") || !server.hasArg("durs")) {
    server.send(400, "text/plain", "Missing freqs/durs"); return;
  }
  
  // Eger o an calan sarki buysa (customFreqs), bozmamak icin once durdur
  if (buzzerFreqPtr == customFreqs) {
    buzzerFreqPtr = nullptr;
    noTone(BUZZER_PIN);
  }
  
  String fStr = server.arg("freqs");
  String dStr = server.arg("durs");
  
  customLen = 0;
  int startF = 0, startD = 0;
  
  while (startF < fStr.length() && customLen < MAX_CUSTOM_NOTES) {
    int pos = fStr.indexOf(',', startF);
    int val = 0;
    if (pos == -1) { val = fStr.substring(startF).toInt(); startF = fStr.length(); }
    else { val = fStr.substring(startF, pos).toInt(); startF = pos + 1; }
    if (val > 0) { customFreqs[customLen] = val; customLen++; }
  }
  
  int len2 = 0;
  while (startD < dStr.length() && len2 < MAX_CUSTOM_NOTES) {
    int pos = dStr.indexOf(',', startD);
    int val = 0;
    if (pos == -1) { val = dStr.substring(startD).toInt(); startD = dStr.length(); }
    else { val = dStr.substring(startD, pos).toInt(); startD = pos + 1; }
    if (val > 0) { customDurs[len2] = val; len2++; }
  }
  
  customLen = min(customLen, len2);
  
  if (customLen > 0) {
    playMelody(customFreqs, customDurs, customLen);
    setFace("excited");
    moodIdleFace = "excited";
    currentCommand = "dance";
    exitIdle();
  }
  
  recordInput();
  server.send(200, "text/plain", "OK");
}

// Ozel yuz (Pixel Art) endpoint'i
void handleCustomFace() {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
    return;
  }
  String body = server.arg("plain");
  if (body.length() == 2048) { // 1024 bytes * 2 hex chars
    for (int i = 0; i < 1024; i++) {
      char hex[3] = { body.charAt(i*2), body.charAt(i*2+1), '\0' };
      customFaceData[i] = (uint8_t)strtol(hex, NULL, 16);
    }
    
    // Set face to custom
    currentFaceName = "custom";
    currentFaceFrames = customFaceFrames;
    currentFaceFrameCount = 1;
    currentFaceFrameIndex = 0;
    currentFaceMode = FACE_ANIM_ONCE;
    
    // Update immediately
    oledLockTime = millis() + 15000;
    updateFaceBitmap(customFaceData);
    recordInput();
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Bad length");
  }
}

// ======================================================================
// --- PET INTERACTION ENDPOINTS ---
// ======================================================================
void handleGetHunger() {
  String json = "{\"hunger\":" + String(hungerPercent) + "}";
  server.send(200, "application/json", json);
}

void handleFeed() {
  lastFedMs = millis();
  hungerPercent += 30;
  if (hungerPercent > 100) hungerPercent = 100;
  lovePercent += 15;
  if (lovePercent > 100) lovePercent = 100;

  // Sürpriz Kutusu: %20 sans, 1 saat cooldown
  unsigned long now = millis();
  bool lootFired = false;
  if (now - lastLootMs > LOOT_COOLDOWN && random(0, 100) < 20) {
    lastLootMs = now;
    lootFired = true;
    showSpeech("SURPRIZ! Hediye kutusu!");
    setFace("excited");
    moodIdleFace = "excited";
    currentCommand = autoPoses[random(4, 11)]; // cute/freaky/worm/shake/shrug/dead/crab
    playMelody(melodySurpriseFreqs, melodySurpriseDurs, melodySurpriseLen);
  }
  if (!lootFired) {
    setFace("love");
    moodIdleFace = "love";
    currentCommand = "dance";
    playMelody(melodyLoveFreqs, melodyLoveDurs, melodyLoveLen);
  }
  exitIdle();
  recordInput();
  server.send(200, "text/plain", "Yummy!");
}

void handleTickle() {
  unsigned long now = millis();
  if (now - lastTickleMs > 2000) {
    tickleCount = 0;
  }
  lastTickleMs = now;
  tickleCount = tickleCount + 1;
  
  if (tickleCount >= 5) {
    currentMood = MOOD_ANGRY;
    moodIdleFace = "angry";
    setFace("angry");
    currentCommand = "left"; 
    nextMoodChangeMs = millis() + 30000;
    tickleCount = 0;
    lovePercent -= 10;  // Cok gidiklaninca sevilmiyor!
    if (lovePercent < 0) lovePercent = 0;
  } else {
    setFace("surprised");
    currentCommand = "shake";
    lovePercent += 10;
    if (lovePercent > 100) lovePercent = 100;
    if (buzzerFreqPtr == nullptr) {
      tone(BUZZER_PIN, random(1000, 2500), 50);
    }
  }
  exitIdle();
  recordInput();
  server.send(200, "text/plain", "Tickle!");
}

void handleWater() {
  if (hiccupsActive) {
    hiccupsActive = false;
    setFace("happy");
    currentCommand = "dance";
    playMelody(melodyLoveFreqs, melodyLoveDurs, melodyLoveLen);
    exitIdle();
    recordInput();
    server.send(200, "text/plain", "Hiccups cured!");
  } else {
    server.send(200, "text/plain", "Not thirsty!");
  }
}

void handleFindMiu() {
  setFace("surprised");
  currentCommand = "wave"; 
  lovePercent += 5;
  if (lovePercent > 100) lovePercent = 100;
  
  static const int findFreqs[] = {880, 1047, 1319, 1047, 1319};
  static const int findDurs[]  = {100, 100,  150,  100,  250};
  playMelody(findFreqs, findDurs, 5);
  
  exitIdle();
  recordInput();
  server.send(200, "text/plain", "I'm here!");
}

// ======================================================================
// --- LOVE METER ENDPOINT ---
// ======================================================================
void handleGetLove() {
  String json = "{\"love\":" + String(lovePercent) + "}";
  server.send(200, "application/json", json);
}

// ======================================================================
// --- JUKEBOX ENDPOINT ---
// ======================================================================
void handleJukebox() {
  if (!server.hasArg("song")) { server.send(400, "text/plain", "Missing song"); return; }
  String song = server.arg("song");
  
  setFace("excited");
  moodIdleFace = "excited";
  currentCommand = "dance";
  exitIdle();
  recordInput();

  if      (song == "mario")    playMelody(melodyMarioFreqs,    melodyMarioDurs,    melodyMarioLen);
  else if (song == "nokia")    playMelody(melodyNokiaFreqs,    melodyNokiaDurs,    melodyNokiaLen);
  else if (song == "birthday") { playMelody(melodyBirthdayFreqs, melodyBirthdayDurs, melodyBirthdayLen); setFace("love"); currentCommand = "wave"; }
  else if (song == "starwars") playMelody(melodyStarWarsFreqs, melodyStarWarsDurs, melodyStarWarsLen);
  else if (song == "tetris")   playMelody(melodyTetrisFreqs,   melodyTetrisDurs,   melodyTetrisLen);
  else { server.send(400, "text/plain", "Unknown song"); return; }
  
  server.send(200, "text/plain", "Playing!");
}

// ======================================================================
// --- EMOJI MIRROR ENDPOINT ---
// ======================================================================
void handleEmoji() {
  if (!server.hasArg("mood")) { server.send(400, "text/plain", "Missing mood"); return; }
  String mood = server.arg("mood");
  
  if (mood == "happy") {
    setFace("happy"); moodIdleFace = "happy";
    currentCommand = "wave";
    playMelody(melodyStartupFreqs, melodyStartupDurs, melodyStartupLen);
    lovePercent += 10; if (lovePercent > 100) lovePercent = 100;
  } else if (mood == "sad") {
    setFace("sad"); moodIdleFace = "sad";
    currentCommand = "shrug";
    if (buzzerFreqPtr == nullptr) tone(BUZZER_PIN, 220, 400);
    lovePercent -= 5; if (lovePercent < 0) lovePercent = 0;
  } else if (mood == "angry") {
    setFace("angry"); moodIdleFace = "angry";
    currentCommand = "shake";
    if (buzzerFreqPtr == nullptr) { tone(BUZZER_PIN, 150, 200); }
    lovePercent -= 15; if (lovePercent < 0) lovePercent = 0;
  } else if (mood == "excited") {
    setFace("excited"); moodIdleFace = "excited";
    currentCommand = "dance";
    playMelody(melodyNokiaFreqs, melodyNokiaDurs, melodyNokiaLen);
    lovePercent += 20; if (lovePercent > 100) lovePercent = 100;
  } else if (mood == "sleepy") {
    setFace("sleepy"); moodIdleFace = "sleepy";
    currentCommand = "rest";
    if (buzzerFreqPtr == nullptr) tone(BUZZER_PIN, 180, 600);
  } else if (mood == "love") {
    setFace("love"); moodIdleFace = "love";
    currentCommand = "cute";
    playMelody(melodyLoveFreqs, melodyLoveDurs, melodyLoveLen);
    lovePercent += 25; if (lovePercent > 100) lovePercent = 100;
  } else {
    server.send(400, "text/plain", "Unknown mood"); return;
  }
  
  exitIdle();
  recordInput();
  server.send(200, "text/plain", "Mirroring!");
}

// API endpoint for network clients to get robot status
void handleGetStatus() {
  String json;
  json.reserve(256);
  json = "{";
  json += "\"currentCommand\":\"" + currentCommand + "\",";
  json += "\"currentFace\":\"" + currentFaceName + "\",";
  json += "\"hungerPercent\":" + String(hungerPercent) + ",";
  json += "\"batteryPercent\":" + String(batteryPercent) + ",";
  json += "\"lovePercent\":" + String(lovePercent) + ",";
  json += "\"currentMood\":\"" + moodIdleFace + "\",";
  json += "\"networkConnected\":" + String(networkConnected ? "true" : "false") + ",";
  json += "\"apIP\":\"" + WiFi.softAPIP().toString() + "\"";
  if (networkConnected) {
    json += ",\"networkIP\":\"" + networkIP.toString() + "\"";
  }
  json += "}";
  server.send(200, "application/json", json);
}

// API endpoint for network clients to send commands (JSON-based)
void handleApiCommand() {
  if (server.method() != HTTP_POST) {
    server.send(405, "application/json", "{\"error\":\"Method not allowed\"}");
    return;
  }
  
  String body = server.arg("plain");
  
  Serial.println("API Command received:");
  Serial.println(body);
  
#if ARDUINOJSON_VERSION_MAJOR >= 7
  JsonDocument doc;
#else
  DynamicJsonDocument doc(512);
#endif

  DeserializationError error = deserializeJson(doc, body);
  if (error) {
    Serial.println("Error: invalid JSON");
    server.send(400, "application/json", "{\"error\":\"Invalid JSON format\"}");
    return;
  }
  
  String command = doc["command"] | "";
  String face = doc["face"] | "";
  
  if (face.length() > 0) {
    setFace(face);
  }
  
  bool faceOnly = (face.length() > 0 && command.length() == 0);
  
  if (faceOnly) {
    recordInput();
    server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Face updated\"}");
    return;
  }
  
  if (command.length() == 0) {
    Serial.println("Error: command field not found");
    server.send(400, "application/json", "{\"error\":\"Missing command field\"}");
    return;
  }
  
  if (command == "stop") {
    currentCommand = "";
    recordInput();
    server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Command stopped\"}");
  } else {
    currentCommand = command;
    recordInput();
    exitIdle();
    server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Command executed\"}");
  }
}

void setup() {
  Serial.begin(115200);
  randomSeed(micros());
  
  // I2C Init for ESP32
  Wire.begin(I2C_SDA, I2C_SCL);

  // OLED Init
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
    Serial.println(F("SSD1306 allocation failed."));
    while (1);
  }
  
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println(F("Setting up WiFi..."));
  display.display();

  // --- WIFI CONFIGURATION ---
  // Try to connect to network first if configured
  if (ENABLE_NETWORK_MODE && String(NETWORK_SSID).length() > 0) {
    Serial.println("Attempting to connect to network: " + String(NETWORK_SSID));
    WiFi.mode(WIFI_AP_STA); // Enable both AP and Station modes
    WiFi.setHostname(deviceHostname.c_str());
    WiFi.begin(NETWORK_SSID, NETWORK_PASS);
    
    // Wait up to 10 seconds for connection
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      networkConnected = true;
      networkIP = WiFi.localIP();
      Serial.println();
      Serial.print("Connected to network! IP: ");
      Serial.println(networkIP);
    } else {
      Serial.println();
      Serial.println("Failed to connect to network. Running in AP-only mode.");
      WiFi.mode(WIFI_AP); // Fall back to AP-only
    }
  } else {
    WiFi.mode(WIFI_AP);
    Serial.println("Network mode disabled. Running in AP-only mode.");
  }
  
  // --- ACCESS POINT CONFIGURATION ---
  WiFi.softAP(AP_SSID, AP_PASS);
  IPAddress myIP = WiFi.softAPIP();
  
  Serial.print("AP Created. IP: ");
  Serial.println(myIP);

  // Build WiFi info text for scrolling
  if (networkConnected) {
    wifiInfoText = "AP: " + String(AP_SSID) + " (" + myIP.toString() + ")  |  Network: " + String(NETWORK_SSID) + " (" + networkIP.toString() + ") or " + deviceHostname + ".local  |  ";
  } else {
    wifiInfoText = "Connect to WiFi: " + String(AP_SSID) + "  |  Pass: " + String(AP_PASS) + "  |  IP: " + myIP.toString() + "  |  Captive Portal will auto-open!  |  ";
  }
  
  // Initialize input tracking
  lastInputTime = millis();
  firstInputReceived = false;
  showingWifiInfo = false;

  // Start mDNS responder for local network discovery
  if (MDNS.begin(deviceHostname.c_str())) {
    Serial.println("mDNS responder started");
    Serial.print("Access controller at: http://");
    Serial.print(deviceHostname);
    Serial.println(".local");
    MDNS.addService("http", "tcp", 80);
  } else {
    Serial.println("Error setting up mDNS responder!");
  }

  // Start DNS Server for Captive Portal
  // This redirects ALL domain requests to the ESP32's IP
  dnsServer.start(DNS_PORT, "*", myIP);

  // Web Server Routes
  server.on("/", handleRoot);
  server.on("/cmd", handleCommandWeb);
  server.on("/getSettings", handleGetSettings);
  server.on("/setSettings", handleSetSettings);
  server.on("/getTouchStatus", handleGetTouchStatus);
  server.on("/setFace",        handleSetFace);
  server.on("/getBattery",     handleGetBattery);
  server.on("/speech",         handleSpeech);
  server.on("/customMelody",   handleCustomMelody);
  
  // API endpoints for network communication
  server.on("/api/status",   handleGetStatus);
  server.on("/api/command",  handleApiCommand);
  server.on("/api/customFace", handleCustomFace);
  
  server.on("/api/getHunger", handleGetHunger);
  server.on("/api/feed",   handleFeed);
  server.on("/api/tickle", handleTickle);
  server.on("/api/water",  handleWater);
  server.on("/api/find",   handleFindMiu);
  server.on("/api/getLove",  handleGetLove);
  server.on("/api/jukebox",  handleJukebox);
  server.on("/api/emoji",    handleEmoji);
  
  // Catch-all route for captive portal
  server.onNotFound(handleRoot);
  
  server.begin();

  // Touch sensor init (TTP223)
  pinMode(TOUCH_PIN, INPUT);
  touchCooldownMs = 0;

  // Buzzer init (ESP32 Core 3.x native tone)
  pinMode(BUZZER_PIN, OUTPUT);
  noTone(BUZZER_PIN);

  // Battery ADC: 0-3.3V araligini kullan
  analogSetPinAttenuation(BATTERY_PIN, ADC_11db);
  lastBatteryMs = 0;  // Ilk dongude hemen oku

  // PWM Init
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  
  for (int i = 0; i < 8; i++) {
    servos[i].setPeriodHertz(50);
    // Map 0-180 to approx 732-2929us
    servos[i].attach(servoPins[i], 732, 2929);
  }
  delay(10);
  
  // OLED'de baslangiç yuzunu goster
  setFace("rest");

  // OLED'de IP adresini 5 saniye goster
  showIPOnOLED();

  // Baslangic melodisi cal
  playMelody(melodyStartupFreqs, melodyStartupDurs, melodyStartupLen);

  // Mood sistemi: ilk degisim 2-5 dk sonra
  nextMoodChangeMs = millis() + (unsigned long)random(120000, 300000);

  // BUGFIX: lastHungerUpdateMs = 0 olarak baslayinca setup() biter bitmez
  // showIPOnOLED() delay(5000)'i gectigindan hungerPercent aninda duser.
  // millis() ile baslat.
  lastHungerUpdateMs = millis();
  lastLoveUpdateMs   = millis();

  Serial.println(F("HTTP server & Captive Portal started."));
}

void loop() {
  // Process DNS requests for captive portal
  dnsServer.processNextRequest();
  
  server.handleClient();
  updateAnimatedFace();
  updateIdleBlink();
  updateWifiInfoScroll();
  updateTouchSensor();
  updateBuzzer();
  updateBattery();
  checkAutoRest();
  updateMood();
  updateIdleBleeps();
  updateHunger();
  updateLove();

  // New Cute Features
  updateSleepTwitch();
  updateHiccups();
  updateShivering();
  updateSneeze();
  updateDreaming();
  updateBellyRumbles();
  updateCatPurr();
  if (currentCommand != "") {
    if (currentCommand != "cat rest") isCatResting = false;
    String cmd = currentCommand;
    if (cmd == "forward") runWalkPose();
    else if (cmd == "backward") runWalkBackward();
    else if (cmd == "left") runTurnLeft();
    else if (cmd == "right") runTurnRight();
    else if (cmd == "rest") { runRestPose(); if (currentCommand == "rest") currentCommand = ""; }
    else if (cmd == "cat rest") { isCatResting = true; runCatRestPose(); if (currentCommand == "cat rest") currentCommand = ""; }
    else if (cmd == "stand") { runStandPose(1); if (currentCommand == "stand") currentCommand = ""; }
    else if (cmd == "yawn") runYawnPose();
    else if (cmd == "sneeze") runSneezePose();
    else if (cmd == "stargaze") runStargazePose();
    else if (cmd == "wave") runWavePose();
    else if (cmd == "dance") runDancePose();
    else if (cmd == "swim") runSwimPose();
    else if (cmd == "point") runPointPose();
    else if (cmd == "pushup") runPushupPose();
    else if (cmd == "bow") runBowPose();
    else if (cmd == "cute") runCutePose();
    else if (cmd == "freaky") runFreakyPose();
    else if (cmd == "worm") runWormPose();
    else if (cmd == "shake") runShakePose();
    else if (cmd == "shrug") runShrugPose();
    else if (cmd == "dead") runDeadPose();
    else if (cmd == "crab") runCrabPose();
    else if (cmd == "knead") runKneadPose();
    else if (cmd == "idle") { enterIdle(); if (currentCommand == "idle") currentCommand = ""; }
    else if (cmd == "stop") { currentCommand = ""; }
  }
  
  // Serial CLI for debugging (can be used to diagnose servo position issues and wiring)
  if (Serial.available()) {
    static char command_buffer[32];
    static byte buffer_pos = 0;
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (buffer_pos > 0) {
        command_buffer[buffer_pos] = '\0';
        int motorNum, angle;
        recordInput();
        if(strcmp(command_buffer, "run walk") == 0 || strcmp(command_buffer, "rn wf") == 0) { currentCommand = "forward"; runWalkPose(); currentCommand = ""; }
        else if(strcmp(command_buffer, "rn wb") == 0) { currentCommand = "backward"; runWalkBackward(); currentCommand = ""; }
        else if(strcmp(command_buffer, "rn tl") == 0) { currentCommand = "left"; runTurnLeft(); currentCommand = ""; }
        else if(strcmp(command_buffer, "rn tr") == 0) { currentCommand = "right"; runTurnRight(); currentCommand = ""; }
        else if(strcmp(command_buffer, "run rest") == 0 || strcmp(command_buffer, "rn rs") == 0) runRestPose();
        else if(strcmp(command_buffer, "run cat rest") == 0 || strcmp(command_buffer, "rn cr") == 0) { isCatResting = true; runCatRestPose(); }
        else if(strcmp(command_buffer, "run stand") == 0 || strcmp(command_buffer, "rn st") == 0) runStandPose(1);
        else if(strcmp(command_buffer, "rn wv") == 0) { currentCommand = "wave"; runWavePose(); }
        else if(strcmp(command_buffer, "rn dn") == 0) { currentCommand = "dance"; runDancePose(); }
        else if(strcmp(command_buffer, "rn sw") == 0) { currentCommand = "swim"; runSwimPose(); }
        else if(strcmp(command_buffer, "rn pt") == 0) { currentCommand = "point"; runPointPose(); }
        else if(strcmp(command_buffer, "rn pu") == 0) { currentCommand = "pushup"; runPushupPose(); }
        else if(strcmp(command_buffer, "rn bw") == 0) { currentCommand = "bow"; runBowPose(); }
        else if(strcmp(command_buffer, "rn ct") == 0) { currentCommand = "cute"; runCutePose(); }
        else if(strcmp(command_buffer, "rn fk") == 0) { currentCommand = "freaky"; runFreakyPose(); }
        else if(strcmp(command_buffer, "rn wm") == 0) { currentCommand = "worm"; runWormPose(); }
        else if(strcmp(command_buffer, "rn sk") == 0) { currentCommand = "shake"; runShakePose(); }
        else if(strcmp(command_buffer, "rn sg") == 0) { currentCommand = "shrug"; runShrugPose(); }
        else if(strcmp(command_buffer, "rn dd") == 0) { currentCommand = "dead"; runDeadPose(); }
        else if(strcmp(command_buffer, "rn cb") == 0) { currentCommand = "crab"; runCrabPose(); }
        else if (strcmp(command_buffer, "subtrim") == 0 || strcmp(command_buffer, "st") == 0) {
          Serial.println("Subtrim values:");
          for (int i = 0; i < 8; i++) {
            Serial.print("Motor "); Serial.print(i); Serial.print(": ");
            if (servoSubtrim[i] >= 0) Serial.print("+");
            Serial.println(servoSubtrim[i]);
          }
        }
        else if (strcmp(command_buffer, "subtrim save") == 0 || strcmp(command_buffer, "st save") == 0) {
          Serial.println("Copy and paste this into your code:");
          Serial.print("int8_t servoSubtrim[8] = {");
          for (int i = 0; i < 8; i++) {
            Serial.print(servoSubtrim[i]);
            if (i < 7) Serial.print(", ");
          }
          Serial.println("};");
        }
        else if (strncmp(command_buffer, "subtrim reset", 13) == 0 || strncmp(command_buffer, "st reset", 8) == 0) {
          for (int i = 0; i < 8; i++) servoSubtrim[i] = 0;
          Serial.println("All subtrim values reset to 0");
        }
        else if (strncmp(command_buffer, "subtrim ", 8) == 0 || strncmp(command_buffer, "st ", 3) == 0) {
          const char* params = (command_buffer[1] == 't') ? command_buffer + 3 : command_buffer + 8;
          int trimMotor, trimValue;
          if (sscanf(params, "%d %d", &trimMotor, &trimValue) == 2) {
            if (trimMotor >= 0 && trimMotor < 8) {
              if (trimValue >= -90 && trimValue <= 90) {
                servoSubtrim[trimMotor] = trimValue;
                Serial.print("Motor "); Serial.print(trimMotor); Serial.print(" subtrim set to ");
                if (trimValue >= 0) Serial.print("+");
                Serial.println(trimValue);
              } else {
                Serial.println("Subtrim value must be between -90 and +90");
              }
            } else {
              Serial.println("Invalid motor number (0-7)");
            }
          }
        }
        else if (strncmp(command_buffer, "all ", 4) == 0) {
             if (sscanf(command_buffer + 4, "%d", &angle) == 1) {
                 for (int i = 0; i < 8; i++) setServoAngle(i, angle);
                 Serial.print("All servos set to "); Serial.println(angle);
             }
        }
        else if (sscanf(command_buffer, "%d %d", &motorNum, &angle) == 2) {
             if (motorNum >= 0 && motorNum < 8) {
                 setServoAngle(motorNum, angle);
                 Serial.print("Servo "); Serial.print(motorNum); Serial.print(" set to "); Serial.println(angle);
             } else {
                 Serial.println("Invalid motor number (0-7)");
             }
        }
        buffer_pos = 0;
      }
    } else if (buffer_pos < sizeof(command_buffer) - 1) {
      command_buffer[buffer_pos++] = c;
    }
  }
}

// Function to update the robot's face
void updateFaceBitmap(const unsigned char* bitmap) {
  if (speechActive) return; // Eger konusma balonu aktifse yuzu cizme
  display.clearDisplay();
  display.drawBitmap(0, 0, bitmap, 128, 64, SSD1306_WHITE);
  display.display();
}

uint8_t countFrames(const unsigned char* const* frames, uint8_t maxFrames) {
  if (frames == nullptr || frames[0] == nullptr) return 0;
  uint8_t count = 0;
  for (uint8_t i = 0; i < maxFrames; i++) {
    if (frames[i] == nullptr) break;
    count++;
  }
  return count;
}

void setFace(const String& faceName) {
  if (millis() < oledLockTime && faceName != "custom") return;
  if (faceName == currentFaceName && currentFaceFrames != nullptr) return;

  currentFaceName = faceName;
  currentFaceFrameIndex = 0;
  lastFaceFrameMs = 0;
  faceFrameDirection = 1;
  faceAnimFinished = false;
  currentFaceFps = getFaceFpsForName(faceName);

  currentFaceFrames = face_default_frames;
  currentFaceFrameCount = countFrames(face_default_frames, MAX_FACE_FRAMES);

  for (size_t i = 0; i < (sizeof(faceEntries) / sizeof(faceEntries[0])); i++) {
    if (faceName.equalsIgnoreCase(faceEntries[i].name)) {
      currentFaceFrames = faceEntries[i].frames;
      currentFaceFrameCount = countFrames(faceEntries[i].frames, faceEntries[i].maxFrames);
      break;
    }
  }

  if (currentFaceFrameCount == 0) {
    currentFaceFrames = face_default_frames;
    currentFaceFrameCount = countFrames(face_default_frames, MAX_FACE_FRAMES);
    currentFaceName = "default";
    currentFaceFps = getFaceFpsForName(currentFaceName);
  }

  if (currentFaceFrameCount > 0 && currentFaceFrames[0] != nullptr) {
    updateFaceBitmap(currentFaceFrames[0]);
  }
}

void setFaceMode(FaceAnimMode mode) {
  currentFaceMode = mode;
  faceFrameDirection = 1;
  faceAnimFinished = false;
}

void setFaceWithMode(const String& faceName, FaceAnimMode mode) {
  setFaceMode(mode);
  if (faceName.equalsIgnoreCase(currentFaceName) && currentFaceFrames != nullptr) {
    currentFaceFrameIndex = 0;
    lastFaceFrameMs = 0;
    faceFrameDirection = 1;
    faceAnimFinished = false;
    if (currentFaceFrameCount > 0 && currentFaceFrames[0] != nullptr) {
      updateFaceBitmap(currentFaceFrames[0]);
    }
  } else {
    setFace(faceName);
  }
}

int getFaceFpsForName(const String& faceName) {
  for (size_t i = 0; i < (sizeof(faceFpsEntries) / sizeof(faceFpsEntries[0])); i++) {
    if (faceName.equalsIgnoreCase(faceFpsEntries[i].name)) {
      return faceFpsEntries[i].fps;
    }
  }
  return faceFps;
}

void updateAnimatedFace() {
  if (speechActive) {
    if (millis() > speechEndTime) {
      speechActive = false;
      // Konusma bitti, aninda eski yuzu ciz
      if (currentFaceFrames != nullptr && currentFaceFrameCount > 0) {
        display.clearDisplay();
        display.drawBitmap(0, 0, currentFaceFrames[currentFaceFrameIndex], 128, 64, SSD1306_WHITE);
        display.display();
      }
    } else {
      return; // Konusma suresince animasyon yapma
    }
  }

  if (millis() < oledLockTime) return; // Pixel art vb. icin genel kilit

  if (currentFaceFrames == nullptr || currentFaceFrameCount <= 1) return;
  if (currentFaceMode == FACE_ANIM_ONCE && faceAnimFinished) return;

  unsigned long now = millis();
  int fps = max(1, (currentFaceFps > 0 ? currentFaceFps : faceFps));
  unsigned long interval = 1000UL / fps;
  if (now - lastFaceFrameMs >= interval) {
    lastFaceFrameMs = now;
    if (currentFaceMode == FACE_ANIM_LOOP) {
      currentFaceFrameIndex = (currentFaceFrameIndex + 1) % currentFaceFrameCount;
    } else if (currentFaceMode == FACE_ANIM_ONCE) {
      if (currentFaceFrameIndex + 1 >= currentFaceFrameCount) {
        currentFaceFrameIndex = currentFaceFrameCount - 1;
        faceAnimFinished = true;
      } else {
        currentFaceFrameIndex++;
      }
    } else {
      if (faceFrameDirection > 0) {
        if (currentFaceFrameIndex + 1 >= currentFaceFrameCount) {
          faceFrameDirection = -1;
          if (currentFaceFrameIndex > 0) currentFaceFrameIndex--;
        } else {
          currentFaceFrameIndex++;
        }
      } else {
        if (currentFaceFrameIndex == 0) {
          faceFrameDirection = 1;
          if (currentFaceFrameCount > 1) currentFaceFrameIndex++;
        } else {
          currentFaceFrameIndex--;
        }
      }
    }
    updateFaceBitmap(currentFaceFrames[currentFaceFrameIndex]);
  }
}

void delayWithFace(unsigned long ms) {
  unsigned long start = millis();
  String initialCommand = currentCommand;
  while (millis() - start < ms) {
    if (currentCommand != initialCommand) break;
    updateAnimatedFace();
    server.handleClient();
    dnsServer.processNextRequest();
    updateTouchSensor();
    updateBattery();
    updateMood();
    updateIdleBleeps();
    updateHunger();
    updateLove();
    updateSleepTwitch();
    updateHiccups();
    updateShivering();
    updateDreaming();
    updateBellyRumbles();
    updateCatPurr();
    delay(5);
  }
}

void scheduleNextIdleBlink(unsigned long minMs, unsigned long maxMs) {
  unsigned long now = millis();
  unsigned long interval = (unsigned long)random(minMs, maxMs);
  nextIdleBlinkMs = now + interval;
}

void enterIdle() {
  idleActive = true;
  idleBlinkActive = false;
  idleBlinkRepeatsLeft = 0;
  // Mood yuzunu kullan; "idle" icin boomerang, digerleri icin loop
  FaceAnimMode m = (moodIdleFace == "idle") ? FACE_ANIM_BOOMERANG : FACE_ANIM_LOOP;
  setFaceWithMode(moodIdleFace, m);
  scheduleNextIdleBlink(3000, 7000);
}

void exitIdle() {
  idleActive = false;
  idleBlinkActive = false;
}

void updateIdleBlink() {
  if (!idleActive) return;

  if (!idleBlinkActive) {
    if (millis() >= nextIdleBlinkMs) {
      idleBlinkActive = true;
      if (idleBlinkRepeatsLeft == 0 && random(0, 100) < 30) {
        idleBlinkRepeatsLeft = 1; // double blink
      }
      setFaceWithMode("idle_blink", FACE_ANIM_ONCE);
    }
    return;
  }

  if (currentFaceMode == FACE_ANIM_ONCE && faceAnimFinished) {
    idleBlinkActive = false;
    FaceAnimMode m = (moodIdleFace == "idle") ? FACE_ANIM_BOOMERANG : FACE_ANIM_LOOP;
    setFaceWithMode(moodIdleFace, m);  // Mood yuzune don
    if (idleBlinkRepeatsLeft > 0) {
      idleBlinkRepeatsLeft--;
      scheduleNextIdleBlink(120, 220);
    } else {
      scheduleNextIdleBlink(3000, 7000);
    }
  }
}

// ====== HELPERS ======
void setServoAngle(uint8_t channel, int angle) { 
  if (channel < 8) {
    int adjustedAngle = constrain(angle + servoSubtrim[channel], 0, 180);
    servos[channel].write(adjustedAngle);
    delayWithFace(motorCurrentDelay);
  }
}

bool pressingCheck(String cmd, int ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    server.handleClient();
    dnsServer.processNextRequest();
    updateAnimatedFace();
    updateTouchSensor();
    updateBattery();
    updateMood();
    updateIdleBleeps();
    updateHunger();
    updateLove();
    updateSleepTwitch();
    updateHiccups();
    updateShivering();
    updateDreaming();
    updateBellyRumbles();
    updateCatPurr();
    if (currentCommand != cmd) {
      return false;
    }
    yield();
  }
  return true;
}

void recordInput() {
  lastInputTime = millis();
  firstInputReceived = true;
  nextAutoActionMs = millis() + 30000;
  if (idleActive) {
    exitIdle();
  }
  showingWifiInfo = false;
}

void updateWifiInfoScroll() {
  // Don't show WiFi info if first input has been received
  if (firstInputReceived) {
    if (showingWifiInfo) {
      showingWifiInfo = false;
      // Restore the current face
      if (currentFaceFrames != nullptr && currentFaceFrameCount > 0) {
        updateFaceBitmap(currentFaceFrames[currentFaceFrameIndex]);
      }
    }
    return;
  }
  
  unsigned long now = millis();
  
  // Check if 30 seconds have passed without input
  if (!showingWifiInfo && (now - lastInputTime >= 30000)) {
    showingWifiInfo = true;
    wifiScrollPos = 0;
    lastWifiScrollMs = now;
  }
  
  if (!showingWifiInfo) return;
  if (speechActive || millis() < oledLockTime) return; // Speech veya Pixel art uzerine cizmesin
  
  // Update scroll every 150ms
  if (now - lastWifiScrollMs >= 150) {
    lastWifiScrollMs = now;
    
    // Clear and redraw with current face in background
    display.clearDisplay();
    
    // Draw the face bitmap in the background
    if (currentFaceFrames != nullptr && currentFaceFrameCount > 0) {
      display.drawBitmap(0, 0, currentFaceFrames[currentFaceFrameIndex], 128, 64, SSD1306_WHITE);
    }
    
    // Draw black bar for text background on top row
    display.fillRect(0, 0, 128, 10, SSD1306_BLACK);
    
    // Draw scrolling text
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setTextWrap(false);
    display.setCursor(-wifiScrollPos, 1);
    display.print(wifiInfoText);
    display.setTextWrap(true);
    
    display.display();
    
    // Advance scroll position
    wifiScrollPos += 2;
    if (wifiScrollPos >= (int)(wifiInfoText.length() * 6)) {
      wifiScrollPos = 0;
    }
  }
}

// ======================================================================
// --- TOUCH SENSOR UPDATE ---
// Her 100ms'de bir GPIO 12'yi okur.
// Değer threshold altına düşerse robot seviniyor: "love" yüzü + "wave" hareketi
// ======================================================================
void updateTouchSensor() {
  unsigned long now = millis();
  if (now - lastTouchPollMs < 100) return;  // 100ms poll aralığı
  lastTouchPollMs = now;

  // TTP223 digital touch module reads HIGH when touched
  touchActive = (digitalRead(TOUCH_PIN) == HIGH);

  if (touchActive && !touchWasActive) {
    touchStartMs = now;
    
    // Hiccup kürü icin hizli dokunuslari say (2 saniye icinde 3 dokunus)
    if (hiccupsActive) {
      if (now - lastHiccupTapMs > 2000) hiccupCureCount = 0;
      lastHiccupTapMs = now;
      hiccupCureCount++;
      if (hiccupCureCount >= 3) {
        hiccupsActive = false;
        setFace("happy");
        currentCommand = "dance";
        playMelody(melodyLoveFreqs, melodyLoveDurs, melodyLoveLen);
        exitIdle();
        recordInput();
        Serial.println(F("[Hiccups] Korkutularak hicrik gecirildi!"));
      }
    }
    else {
      // Gidiklama ve Hapsirma mantigi
      if (now - lastTickleTapMs > 800) tickleTapCount = 0;
      lastTickleTapMs = now;
      tickleTapCount++;

      if (tickleTapCount >= 4) {
        tickleTapCount = 0;
        currentCommand = "sneeze";
        exitIdle();
        recordInput();
        touchCooldownMs = now + TOUCH_COOLDOWN;
        Serial.println(F("[Touch] Cok gidiklandi, hapsiriyor!"));
      }
    }
  }

  // Single tap timer: dokunma bittikten 400ms sonra hala baska dokunus gelmediyse tek dokunusu islet
  if (!touchActive && tickleTapCount > 0 && tickleTapCount < 4 && (now - lastTickleTapMs >= 400)) {
    if (now >= touchCooldownMs) {
      Serial.println(F("[Touch] Kafa dokunusu algilandi! Sevinme animasyonu baslatiliyor..."));
      setFace("love");
      moodIdleFace = "love";
      playMelody(melodyLoveFreqs, melodyLoveDurs, melodyLoveLen);
      currentCommand = "wave";
      exitIdle();
      recordInput();
      touchCooldownMs = now + TOUCH_COOLDOWN;
    }
    tickleTapCount = 0;
  }
  
  // Usume Kürü (Long Press)
  if (touchActive && currentMood == MOOD_COLD) {
    if (now - touchStartMs > 4000) {
       currentMood = MOOD_HAPPY;
       moodIdleFace = "happy";
       setFace("happy");
       playMelody(melodyLoveFreqs, melodyLoveDurs, melodyLoveLen);
       currentCommand = "dance";
       exitIdle();
       recordInput();
       Serial.println(F("[Cold] Miu ellerinizle isitildi!"));
       touchStartMs = now + 10000; // Ayni anda tekrar tetiklenmemesi icin
    }
  }

  touchWasActive = touchActive;
}

// ======================================================================
// --- BUZZER (Non-blocking melody player) ---
// ======================================================================
void playMelody(const int* freqs, const int* durs, int len) {
  if (!freqs || len <= 0) return;
  buzzerFreqPtr = freqs;
  buzzerDurPtr  = durs;
  buzzerLen     = len;
  buzzerIdx     = 0;
  if (freqs[0] > 0) tone(BUZZER_PIN, freqs[0]);
  else noTone(BUZZER_PIN);
  buzzerNextMs  = millis() + durs[0];
}

void updateBuzzer() {
  if (!buzzerFreqPtr) return;
  if (millis() < buzzerNextMs) return;
  buzzerIdx++;
  if (buzzerIdx < buzzerLen) {
    if (buzzerFreqPtr[buzzerIdx] > 0) tone(BUZZER_PIN, buzzerFreqPtr[buzzerIdx]);
    else noTone(BUZZER_PIN);
    buzzerNextMs = millis() + buzzerDurPtr[buzzerIdx];
  } else {
    noTone(BUZZER_PIN);
    buzzerFreqPtr = nullptr;
  }
}

// ======================================================================
// --- BATTERY ADC ---
// Voltaj bolucusu: 100kohm + 100kohm -> Vbat = Vadc * 2
// LiPo: 4200mV = %100, 3200mV = %0
// ======================================================================
void updateBattery() {
  if (lastBatteryMs != 0 && millis() - lastBatteryMs < BATTERY_INTERVAL) return;
  lastBatteryMs = millis();
  uint32_t rawMv  = analogReadMilliVolts(BATTERY_PIN);
  
  static float filteredMv = -1;
  if (filteredMv < 0) {
    filteredMv = rawMv; // Ilk okuma
  } else {
    filteredMv = (filteredMv * 0.8) + (rawMv * 0.2); // Low pass filter (Averaging)
  }
  
  uint32_t battMv = (uint32_t)filteredMv * 2;
  if      (battMv >= 4200) batteryPercent = 100;
  else if (battMv <= 3200) batteryPercent = 0;
  else                     batteryPercent = (int)((battMv - 3200) * 100 / 1000);
  Serial.print(F("[Battery] %"));
  Serial.println(batteryPercent);
}

// ======================================================================
// --- AUTO REST & PET MODE ---
// ======================================================================

void checkAutoRest() {
  if (!autonomousMode) return;
  unsigned long now = millis();
  unsigned long idleTime = now - lastInputTime;

  // 1) 5 dakika hareketsizlik varsa esneyip rest'e geç ve uykuda kal
  if (idleTime >= 300000) {
    if (currentFaceName != "rest" && currentCommand != "rest" && currentCommand != "yawn" && !isCatResting) {
      Serial.println(F("[AutoRest] 5 dakika doldu, once esneme sonra rest"));
      currentCommand = "yawn";
      currentMood = MOOD_SLEEPY;
      moodIdleFace = "rest";
      exitIdle();
    }
    return; // Uykudayken kendi kendine uyanıp dans etme
  }
  
  static bool didStargaze = false;
  if (idleTime < 200000) didStargaze = false;
  if (idleTime >= 240000 && idleTime < 300000 && !didStargaze) {
    if (currentCommand == "" && currentFaceName != "rest" && !isCatResting) {
      didStargaze = true;
      currentCommand = "stargaze";
      exitIdle();
    }
  }

  // 2) Tam otonom (Pet) modu: Mood ve sevgiye gore huzurlu, dogal kedi davranislari
  if (autonomousMode && idleActive && !hiccupsActive && currentMood != MOOD_COLD && currentMood != MOOD_SLEEPY && currentFaceName != "rest" && !isCatResting) {
    if (millis() >= nextAutoActionMs && idleTime >= 45000) {
      String pose = "cute";
      if (currentMood == MOOD_HAPPY || lovePercent > 60) {
        const char* happyPoses[] = {"knead", "cute", "wave", "dance"};
        pose = happyPoses[random(0, 4)];
      } else if (currentMood == MOOD_EXCITED) {
        const char* excitedPoses[] = {"dance", "shake", "wave"};
        pose = excitedPoses[random(0, 3)];
      } else if (currentMood == MOOD_CURIOUS) {
        const char* curiousPoses[] = {"point", "stargaze", "shrug"};
        pose = curiousPoses[random(0, 3)];
      } else {
        // Notr modda daha az hareket, daha cok hamur yogurma/cute durus
        const char* calmPoses[] = {"knead", "cute", "bow"};
        pose = calmPoses[random(0, 3)];
      }
      
      Serial.print(F("[PetMode] Akilli otonom hareket: "));
      Serial.println(pose);
      currentCommand = pose;
      exitIdle();
      // 50-80 saniye arasi sakin bekleme araligi (robot surekli cirpinmasin)
      nextAutoActionMs = millis() + random(50000, 80000);
    }
  }
}

// ======================================================================
// --- MOOD SYSTEM ---
// Etkilesim suresine ve rastgeleliğe gore Miu'nun ruh halini guncelle
// ======================================================================
void updateMood() {
  if (!autonomousMode) return;
  if (!idleActive) return;            // Sadece idle modda calis
  if (millis() < nextMoodChangeMs) return;

  unsigned long timeSince = millis() - lastInputTime;
  int r = (int)random(0, 100);
  MoodState newMood;

  if (hungerPercent < 20) { // Cok actiysa
    newMood = (r < 50) ? MOOD_SAD : MOOD_SLEEPY;
  } else if (timeSince < 120000) {           // Son 2 dakikada oynandi
    if      (r < 45) newMood = MOOD_HAPPY;
    else if (r < 72) newMood = MOOD_EXCITED;
    else if (r < 92) newMood = MOOD_CURIOUS;
    else             newMood = MOOD_NEUTRAL;
  } else if (timeSince < 300000) {    // 2-5 dk
    if      (r < 25) newMood = MOOD_HAPPY;
    else if (r < 55) newMood = MOOD_NEUTRAL;
    else if (r < 73) newMood = MOOD_CURIOUS;
    else if (r < 90) newMood = MOOD_SAD;
    else             newMood = MOOD_SLEEPY;
  } else {                            // 5+ dk
    if      (r < 12) newMood = MOOD_NEUTRAL;
    else if (r < 30) newMood = MOOD_SAD;
    else if (r < 60) newMood = MOOD_COLD;   // %30 ihtimalle usume modu!
    else if (r < 75) newMood = MOOD_SLEEPY;
    else if (r < 88) newMood = MOOD_CURIOUS;
    else             newMood = MOOD_HAPPY;  // Aniden mutlu :)
  }

  currentMood  = newMood;
  moodIdleFace = String(moodFaceNames[currentMood]);

  FaceAnimMode m = (moodIdleFace == "idle") ? FACE_ANIM_BOOMERANG : FACE_ANIM_LOOP;
  setFaceWithMode(moodIdleFace, m);

  // Sonraki degisim: 1.5-4 dk arasi rastgele
  nextMoodChangeMs = millis() + (unsigned long)random(90000, 240000);

  Serial.print(F("[Mood] Miu'nun yeni mood: "));
  Serial.println(moodIdleFace);
}

// ======================================================================
// --- CUTE FEATURES (Hiccups, Shivering, Sleep Twitch) ---
// ======================================================================

void updateSleepTwitch() {
  if (!autonomousMode) return;
  if (!idleActive) return;
  if (currentCommand != "") return; // Hali hazirda bir animasyon donuyorsa ezme
  if (moodIdleFace != "sleepy" && moodIdleFace != "rest" && currentFaceName != "rest" && !isCatResting) return;
  
  unsigned long now = millis();
  if (nextSleepTwitchMs == 0) {
    nextSleepTwitchMs = now + random(10000, 30000);
  }
  
  if (now > nextSleepTwitchMs) {
    int leg = random(0, 4);
    int ch = (leg == 0) ? 6 : (leg == 1) ? 7 : (leg == 2) ? 4 : 5; // L3, L4, R4, R3 (Dizler)
    
    int baseAngle;
    int targetAngle;
    if (isCatResting) {
      baseAngle = (ch == 6 || ch == 4) ? 180 : 0; // L3=180, R4=180, R3=0, L4=0
      targetAngle = baseAngle + ((baseAngle == 0) ? 20 : -20);
    } else if (currentFaceName == "rest") {
      baseAngle = 90;
      targetAngle = 90 + ((ch == 6 || ch == 7) ? 20 : -20);
    } else {
      // Stand pose: R3=180, L4=180, L3=0, R4=0
      baseAngle = (ch == 5 || ch == 7) ? 180 : 0;
      targetAngle = baseAngle + ((baseAngle == 0) ? 20 : -20);
    }
    
    setServoAngle(ch, targetAngle);
    delayWithFace(120);
    setServoAngle(ch, baseAngle);
    
    nextSleepTwitchMs = now + random(15000, 35000);
  }
}

void updateHiccups() {
  if (!autonomousMode) return;
  unsigned long now = millis();
  
  // Sans tabanli hiccup baslatma (her 1 dakikada %5)
  if (nextHiccupCheckMs == 0) nextHiccupCheckMs = now + 60000;
  if (!hiccupsActive && now > nextHiccupCheckMs) {
    if (random(0, 100) < 5) {
      hiccupsActive = true;
      hiccupCureCount = 0;
      nextHiccupEventMs = now + random(2000, 5000);
      Serial.println(F("[Hiccups] Miu hicrik krizine girdi!"));
    }
    nextHiccupCheckMs = now + 60000;
  }
  
  // Hiccup olayi
  if (hiccupsActive && now > nextHiccupEventMs) {
    if (currentCommand != "") return; // Baska bir emote calisiyorsa bekle
    
    int l1_base = 45, r1_base = 135;
    if (isCatResting) { l1_base = 0; r1_base = 180; }
    else if (currentFaceName == "rest") { l1_base = 90; r1_base = 90; }

    // Omuzlari hizlica sars
    setServoAngle(2, constrain(l1_base + 15, 0, 180)); // L1
    setServoAngle(0, constrain(r1_base - 15, 0, 180)); // R1
    if (buzzerFreqPtr == nullptr) tone(BUZZER_PIN, 2000, 40);
    
    // 500ms surprised yuzu
    if (currentFaceFrames != nullptr) {
      setFace("surprised");
    }
    
    delayWithFace(150);
    
    // Reset shoulders to original pose
    setServoAngle(2, l1_base); 
    setServoAngle(0, r1_base);
    
    if (idleActive) {
       FaceAnimMode m = (moodIdleFace == "idle") ? FACE_ANIM_BOOMERANG : FACE_ANIM_LOOP;
       setFaceWithMode(moodIdleFace, m);
    }
    
    nextHiccupEventMs = now + random(3000, 8000);
  }
}

void updateShivering() {
  if (!autonomousMode) return;
  if (currentMood != MOOD_COLD) return;
  if (currentCommand != "") return; // Baska emote calisiyorsa titreme
  
  unsigned long now = millis();
  if (now > nextShiverMs) {
    shiverState = !shiverState;
    int l1_base = 45, r1_base = 135;
    if (isCatResting) { l1_base = 0; r1_base = 180; }
    else if (currentFaceName == "rest") { l1_base = 90; r1_base = 90; }

    // Titreme efekti: omuz motorlarina cok ufak (2-3 derece) oynamalar verilir
    int offset = shiverState ? 3 : -3;
    setServoAngle(2, constrain(l1_base + offset, 0, 180));
    setServoAngle(0, constrain(r1_base - offset, 0, 180));
    
    nextShiverMs = now + 80; // Cok hizli titreme
  }
}

unsigned long nextAutoSneezeMs = 0;
void updateSneeze() {
  if (!autonomousMode) return;
  if (!idleActive) return;
  if (currentMood == MOOD_SLEEPY || moodIdleFace == "rest") return;
  if (currentCommand != "") return;
  
  unsigned long now = millis();
  if (nextAutoSneezeMs == 0) nextAutoSneezeMs = now + random(120000, 240000);
  
  if (now > nextAutoSneezeMs) {
    if (random(0, 100) < 30) {
      currentCommand = "sneeze";
      exitIdle();
    }
    nextAutoSneezeMs = now + random(60000, 240000);
  }
}

// ======================================================================
// --- IDLE BLEEPS ---
// ======================================================================
void updateIdleBleeps() {
  if (!autonomousMode) return;
  if (!idleActive || isCatResting || currentFaceName == "rest" || currentCommand != "") return;
  // Sadece neseli veya merakli mood'da iken yumusak minik bir cik cik sesi
  if (currentMood != MOOD_HAPPY && currentMood != MOOD_CURIOUS && currentMood != MOOD_EXCITED) return;
  if (buzzerFreqPtr != nullptr) return;
  
  unsigned long now = millis();
  if (nextIdleBleepMs == 0) {
    nextIdleBleepMs = now + random(45000, 90000);
  }
  
  if (now > nextIdleBleepMs) {
    tone(BUZZER_PIN, random(800, 1400), random(25, 45)); // Cok kisa ve tatli
    nextIdleBleepMs = now + random(45000, 90000);
  }
}

// ======================================================================
// --- HUNGER SYSTEM ---
// ======================================================================
void updateHunger() {
  unsigned long now = millis();
  // 10 saniyede bir yuzde 1 acikir
  if (now - lastHungerUpdateMs > 10000) {
    if (hungerPercent > 0) hungerPercent--;
    lastHungerUpdateMs = now;
  }
}

// ======================================================================
// --- LOVE SYSTEM ---
// ======================================================================
void updateLove() {
  unsigned long now = millis();
  // 30 saniyede bir 1 azalir (acliktan daha yavash)
  if (now - lastLoveUpdateMs > 30000) {
    if (lovePercent > 0) lovePercent--;
    lastLoveUpdateMs = now;
  }
  // %100 dolunca ozel melodi + kalp yuzü (bir kez)
  if (lovePercent >= 100 && !lovePeakTriggered) {
    lovePeakTriggered = true;
    setFace("love");
    moodIdleFace = "love";
    if (buzzerFreqPtr == nullptr) {
      playMelody(melodyLoveFreqs, melodyLoveDurs, melodyLoveLen);
    }
    Serial.println(F("[Love] Miu cok mutlu! Love %100!"));
  }
  if (lovePercent < 100) lovePeakTriggered = false; // Tekrar tetiklenebilmesi icin sifirla
}

// ======================================================================
// --- IP OLED DISPLAY ---
// Baslangicta 5 saniye IP adresini goster
// ======================================================================
void showIPOnOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Baslik
  display.setCursor(20, 0);
  display.println(F("~ Miu Online! ~"));
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

  IPAddress apIP = WiFi.softAPIP();

  if (networkConnected) {
    display.setCursor(0, 14);
    display.println(F("WiFi baglandi:"));
    display.println(networkIP.toString());
    display.println();
    display.print(F("AP: "));
    display.println(apIP.toString());
  } else {
    display.setCursor(0, 14);
    display.println(F("AP Modu:"));
    display.println(F("miu-controller"));
    display.println(apIP.toString());
    display.println();
    display.println(F("Pass: hsyn1234"));
  }

  display.display();
  delay(5000);  // 5 saniye goster (setup() icinde, blocking OK)
}

// ======================================================================
// --- SPEECH BUBBLE DISPLAY ---
// ======================================================================
void showSpeech(String msg) {
  speechMessage = msg;
  speechEndTime = millis() + 5000;
  speechActive = true;
  oledLockTime = millis() + 5000;
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  // Balon cizimi
  display.drawRoundRect(0, 0, 128, 64, 8, SSD1306_WHITE);
  display.setCursor(6, 6);
  
  // Basit kelime kaydirma
  int len = msg.length();
  int maxCharsPerLine = 19; 
  for(int i=0; i<len; i+=maxCharsPerLine) {
    display.println(msg.substring(i, i+maxCharsPerLine));
    display.setCursor(6, display.getCursorY());
  }
  
  display.display();
}

// ======================================================================
// --- NEW CUTE INNOVATIONS ---
// ======================================================================
void updateDreaming() {
  if (!autonomousMode) return;
  if (currentCommand != "rest") return;
  if (buzzerFreqPtr != nullptr) return;
  unsigned long now = millis();
  if (nextDreamMs == 0) nextDreamMs = now + random(60000, 120000);
  
  if (now > nextDreamMs) {
    Serial.println(F("[Dreaming] Miu ruya goruyor..."));
    setFaceWithMode("love", FACE_ANIM_ONCE);
    for(int i=0; i<3; i++) {
      if (buzzerFreqPtr == nullptr) tone(BUZZER_PIN, random(200, 400), 100);
      delayWithFace(200);
    }
    setFaceWithMode("rest", FACE_ANIM_BOOMERANG);
    nextDreamMs = now + random(60000, 120000);
  }
}

void updateBellyRumbles() {
  if (!autonomousMode) return;
  if (!idleActive || currentCommand != "" || hungerPercent >= 20) return;
  if (buzzerFreqPtr != nullptr) return;
  unsigned long now = millis();
  if (nextBellyRumbleMs == 0) nextBellyRumbleMs = now + random(20000, 40000);
  
  if (now > nextBellyRumbleMs) {
    Serial.println(F("[Hunger] Miu'nun karni gurulduyor..."));
    setFace("sad");
    setServoAngle(2, 0); // L1 karni tut
    setServoAngle(0, 180); // R1 karni tut
    for(int i=0; i<4; i++) {
      if (buzzerFreqPtr == nullptr) tone(BUZZER_PIN, 100 + i*20, 50);
      delayWithFace(100);
    }
    setServoAngle(2, 45); // L1 stand'e don
    setServoAngle(0, 135); // R1 stand'e don
    FaceAnimMode m = (moodIdleFace == "idle") ? FACE_ANIM_BOOMERANG : FACE_ANIM_LOOP;
    setFaceWithMode(moodIdleFace, m);
    nextBellyRumbleMs = now + random(30000, 60000);
  }
}

void updateCatPurr() {
  if (!autonomousMode) return;
  if (!isCatResting) return;
  unsigned long now = millis();
  static unsigned long nextPurrToggle = 0;
  static bool purrState = false;
  
  if (now > nextPurrToggle) {
    purrState = !purrState;
    if (purrState) {
      if (buzzerFreqPtr == nullptr) tone(BUZZER_PIN, random(35, 45)); // Low rumble purr
      nextPurrToggle = now + random(100, 250);
    } else {
      if (buzzerFreqPtr == nullptr) noTone(BUZZER_PIN);
      nextPurrToggle = now + random(30, 80);
    }
  }
}
