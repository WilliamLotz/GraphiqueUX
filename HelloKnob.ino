#include "lcd_bsp.h"
#include "cst816.h"
#include "lcd_bl_pwm_bsp.h"
#include "lvgl.h"
#include "bidi_switch_knob.h" // Knob Driver

#include "mock_data.h"
#include "ui_linky.h"

// Configuration
#define BACKLIGHT_BRIGHTNESS 150 
#define EXAMPLE_ENCODER_ECA_PIN    8
#define EXAMPLE_ENCODER_ECB_PIN    7

// --- REAL DATA CONFIG (DISABLED) ---
// #define ENABLE_REAL_DATA // <--- DECOMMENTER POUR WIFI

#ifdef ENABLE_REAL_DATA
#include <WiFi.h>
#include <PubSubClient.h> 
#include <ArduinoJson.h>  
#include <time.h>

const char* ssid = "VOTRE_SSID";
const char* password = "VOTRE_PASS";
const char* mqtt_server = "192.168.1.100";

WiFiClient espClient;
PubSubClient client(espClient);
#endif

// Knob Handle
static knob_handle_t s_knob = NULL;

// Linky Data Global Instance
linky_data_t linky_data = {
    .papp = 0,
    .iinst = 0,
    .index_base = 12345,
    .index_hp = 5600,
    .index_hc = 4200,
    .isousc = 45,
    .option_tarif = "BASE",
    .mot_etat = "PAS D'ETAT",
    .voltage = 230
};

// Mock Update Function
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

// Callbacks for Knob (Direct Call - Hardware Mode)
static void _knob_left_cb(void *arg, void *data)
{
    ui_linky_change_page(-1);
    Serial.println("Knob Left");
}

static void _knob_right_cb(void *arg, void *data)
{
    ui_linky_change_page(1);
    Serial.println("Knob Right");
}

#ifdef ENABLE_REAL_DATA
void setup_wifi() {
  WiFi.begin(ssid, password);
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 10) {
    delay(500); Serial.print("."); tries++;
  }
  Serial.println("");
  if (WiFi.status() == WL_CONNECTED) {
      Serial.println("WiFi connected");
      Serial.println(WiFi.localIP());
  } else {
      Serial.println("WiFi Timeout");
  }
}
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  // MQTT Logic... (Simplifié pour restore)
}
void reconnect() {
  // Reconnect Logic...
}
#endif

void setup() {
  Serial.begin(115200);
  delay(1000); 
  Serial.println("Starting LinkyMock RESTORED...");

#ifdef ENABLE_REAL_DATA
    Serial.println("MODE WIFI ACTIF");
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(mqtt_callback); 
    configTime(3600, 3600, "pool.ntp.org");
#else
    Serial.println("MODE MOCK ACTIF");
    struct tm tm;
    tm.tm_year = 2026 - 1900; tm.tm_mon = 0; tm.tm_mday = 26;
    tm.tm_hour = 22; tm.tm_min = 22; tm.tm_sec = 0;
    time_t t = mktime(&tm);
    struct timeval tv = { .tv_sec = t };
    settimeofday(&tv, NULL);
#endif

  // 1. Init Drivers LCD
  lcd_lvgl_Init(); 
  
  // 2. Hardware Init
#if defined(CONFIG_IDF_TARGET_ESP32S3)
  Serial.println("S3 Init Hardware...");
  Touch_Init();
  lcd_bl_pwm_bsp_init(BACKLIGHT_BRIGHTNESS);

  knob_config_t cfg = {
    .gpio_encoder_a = EXAMPLE_ENCODER_ECA_PIN,
    .gpio_encoder_b = EXAMPLE_ENCODER_ECB_PIN,
  };
  s_knob = iot_knob_create(&cfg);
  iot_knob_register_cb(s_knob, KNOB_LEFT, _knob_left_cb, NULL);
  iot_knob_register_cb(s_knob, KNOB_RIGHT, _knob_right_cb, NULL);
#endif

  // 3. Init UI
  ui_linky_init();
  Serial.println("Setup done.");
}

void loop() {
  static uint32_t last_update = 0;
  
  lv_timer_handler();
  
#ifdef ENABLE_REAL_DATA
  if (!client.connected()) reconnect();
  client.loop(); 
  if (millis() - last_update > 500) { 
      ui_linky_update(&linky_data);
      last_update = millis();
  }
#else
  if (millis() - last_update > 1000) {
      update_mock_data();
      ui_linky_update(&linky_data);
      last_update = millis();
  }
#endif
  delay(5);
}
