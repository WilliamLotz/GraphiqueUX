#include "lcd_bsp.h"
#include "cst816.h"
#include "lcd_bl_pwm_bsp.h"
#include "lvgl.h"
#include "bidi_switch_knob.h" 

#include "mock_data.h"
#include "ui_linky.h"

// Configuration
#define BACKLIGHT_BRIGHTNESS 150 
#define EXAMPLE_ENCODER_ECA_PIN    8
#define EXAMPLE_ENCODER_ECB_PIN    7

// --- REAL DATA CONFIG (WIFI ON) ---
#define ENABLE_REAL_DATA // <--- ACTIVÉ

#ifdef ENABLE_REAL_DATA
#include <WiFi.h>
#include <PubSubClient.h> 
#include <ArduinoJson.h>  
#include <time.h>

const char* ssid = "Livebox-6FF0";      // <--- METTRE VOTRE SSID
const char* password = "sdP9kTkycpa2vTCuVi";  // <--- METTRE VOTRE PASSWORD
const char* mqtt_server = "192.168.1.100";

WiFiClient espClient;
PubSubClient client(espClient);
#endif

// Knob Global Variables (Software Mode)
volatile int lastEncoded = 0;
volatile long encoderValue = 0;
volatile int knob_event_request = 0; // 0=None, 1=Right, -1=Left

// ISR (Interruption) Simplifiée (Fiable)
void IRAM_ATTR updateEncoder() {
  int a = digitalRead(EXAMPLE_ENCODER_ECA_PIN);
  int b = digitalRead(EXAMPLE_ENCODER_ECB_PIN);

  // Logique Simple Quadrature
  if (a == b) encoderValue++;
  else encoderValue--;
  
  static long last_val = 0;
  // Seuil de 2 pour filtrer un peu
  if (abs(encoderValue - last_val) >= 2) { 
      if (encoderValue > last_val) knob_event_request = 1;
      else knob_event_request = -1;
      last_val = encoderValue;
  }
}

// Linky Data Global Instance
linky_data_t linky_data = {
    .papp = 0, .iinst = 0, .index_base = 12345, .index_hp = 5600, .index_hc = 4200,
    .isousc = 45, .option_tarif = "BASE", .mot_etat = "PAS D'ETAT", .voltage = 230
};

// Mock Update Function (Fallback)
void update_mock_data() {
    int delta = (rand() % 401) - 200; 
    int new_papp = (int)linky_data.papp + delta;
    if (new_papp < 0) new_papp = 100;
    if (new_papp > 9000) new_papp = 9000;
    linky_data.papp = (uint16_t)new_papp;
    linky_data.iinst = linky_data.papp / 230;
    if(linky_data.iinst == 0 && linky_data.papp > 0) linky_data.iinst = 1;
    linky_data.voltage = 225 + (rand() % 18);
    if ((rand() % 100) < 5) linky_data.index_base++;
}

#ifdef ENABLE_REAL_DATA
void setup_wifi() {
  WiFi.begin(ssid, password);
  
  // Timeout 5s (Non-blocking boot)
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 10) {
    delay(500); Serial.print("."); tries++;
  }
  Serial.println("");
  
  if (WiFi.status() == WL_CONNECTED) {
      Serial.println("WiFi Connected!");
      Serial.println(WiFi.localIP());
  } else {
      Serial.println("WiFi Timeout -> Continuing Offline");
  }
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  // Parsing MQTT...
}
void reconnect() {
   // Reconnect Loop... (A completer si besoin)
}
#endif

void setup() {
  Serial.begin(115200);
  delay(1000); 
  Serial.println("Starting LinkyMock (WiFi + SoftKnob)...");

#ifdef ENABLE_REAL_DATA
    Serial.println("MODE WIFI ACTIF");
    setup_wifi();
    // client.setServer...
    configTime(3600, 3600, "pool.ntp.org");
#else
    Serial.println("MODE MOCK ACTIF");
    // Mock Time 2026/01/26 22:22
    struct tm tm;
    tm.tm_year = 2026 - 1900; tm.tm_mon = 0; tm.tm_mday = 26;
    tm.tm_hour = 22; tm.tm_min = 22; tm.tm_sec = 0;
    time_t t = mktime(&tm);
    struct timeval tv = { .tv_sec = t };
    settimeofday(&tv, NULL);
#endif

  // 1. Init Drivers LCD
  lcd_lvgl_Init(); 
  
  // 2. Hardware Init Check
#if defined(CONFIG_IDF_TARGET_ESP32S3)
  Serial.println("S3 Init Hardware...");
  Touch_Init();
  lcd_bl_pwm_bsp_init(BACKLIGHT_BRIGHTNESS);

  // 3. KNOB SOFTWARE INIT (WiFi COMPATIBLE)
  Serial.println("KNOB SETUP (SOFTWARE MODE)...");
  pinMode(EXAMPLE_ENCODER_ECA_PIN, INPUT_PULLUP);
  pinMode(EXAMPLE_ENCODER_ECB_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(EXAMPLE_ENCODER_ECA_PIN), updateEncoder, CHANGE);
  // attachInterrupt(digitalPinToInterrupt(EXAMPLE_ENCODER_ECB_PIN), updateEncoder, CHANGE); // Only A needed (Step 2334 config)
  Serial.println("KNOB INIT DONE");
#endif

  // 4. Init UI
  ui_linky_init();
  Serial.println("Setup done.");
}

void loop() {
  // 1. Check Knob Flag (Thread Safe UI Update)
  if (knob_event_request != 0) {
      Serial.print("Knob Event: "); Serial.println(knob_event_request);
      ui_linky_change_page(knob_event_request);
      knob_event_request = 0;
  }
  
  // Debug Raw Value
  static long last_debug_enc = 0;
  if(encoderValue != last_debug_enc) {
      Serial.print("Raw Enc: "); Serial.println(encoderValue);
      last_debug_enc = encoderValue;
  }

  static uint32_t last_update = 0;
  
  // 2. LVGL Engine
  lv_timer_handler();
  
  // 3. Data Flow
#ifdef ENABLE_REAL_DATA
  if (millis() - last_update > 500) { 
      // HYBRID DEMO: Mock Data even with WiFi enabled
      update_mock_data();
      ui_linky_update(&linky_data);
      last_update = millis();
  }
#else
  // Mock Flow
  if (millis() - last_update > 1000) {
      update_mock_data();
      ui_linky_update(&linky_data);
      last_update = millis();
  }
#endif
  
  delay(5);
}
