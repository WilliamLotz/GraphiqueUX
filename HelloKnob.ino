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

// --- MQTT / REAL DATA CONFIG ---
// #define ENABLE_REAL_DATA // <--- DECOMMENTER POUR ACTIVER LE WIFI/MQTT

#ifdef ENABLE_REAL_DATA
#include <WiFi.h>
#include <PubSubClient.h> // Installer la lib "PubSubClient"
#include <ArduinoJson.h>  // Installer la lib "ArduinoJson"

const char* ssid = "VOTRE_SSID";
const char* password = "VOTRE_MOT_DE_PASSE";
const char* mqtt_server = "192.168.1.XX"; // IP du Broker

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
    // Variation aléatoire de la puissance
    int delta = (rand() % 401) - 200; // -200 to 200
    int new_papp = (int)linky_data.papp + delta;
    
    // Bornes 0 - 9000 VA
    if (new_papp < 0) new_papp = 100;
    if (new_papp > 9000) new_papp = 9000;
    
    linky_data.papp = (uint16_t)new_papp;
    
    // Calcul IINST
    linky_data.iinst = linky_data.papp / 230;
    if(linky_data.iinst == 0 && linky_data.papp > 0) linky_data.iinst = 1;

    // Simulation tension 220-240V
    linky_data.voltage = 225 + (rand() % 18);
    
    // Incrémentation lente de l'index
    if ((rand() % 100) < 5) { // 5% de chance
        linky_data.index_base++;
    }
}

// Callbacks for Knob
static void _knob_left_cb(void *arg, void *data)
{
    // Important: LVGL input should ideally be handled in the main loop or thread safe
    // But for simplicity in this Arduino sketch, we just signal a flag or call directly if simple
    // Since we are running single threaded in loop(), we can use a flag.
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
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  // Convert payload to string
  char msg[length + 1];
  memcpy(msg, payload, length);
  msg[length] = '\0';
  // Serial.println(msg); // Debug

  // Parse JSON
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, msg);

  if (error) {
    Serial.print("JSON Failed: ");
    Serial.println(error.c_str());
    return;
  }

  // Update Global Data from JSON
  if (doc.containsKey("papp")) linky_data.papp = doc["papp"];
  if (doc.containsKey("voltage")) linky_data.voltage = doc["voltage"];
  if (doc.containsKey("iinst")) linky_data.iinst = doc["iinst"];
  
  // Index (Gère les différents cas)
  if (doc.containsKey("base")) linky_data.index_base = doc["base"];
  if (doc.containsKey("hp")) linky_data.index_hp = doc["hp"];
  if (doc.containsKey("hc")) linky_data.index_hc = doc["hc"];
  
  // Update UI immediately (Optional, or wait for next loop tick)
  // ui_linky_update(&linky_data);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("MQTT Connect...");
    String clientId = "KnobClient-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Connected");
      client.subscribe("home/linky/status"); // <-- TOPIC A ECOUTER
    } else {
      Serial.print("Fail rc=");
      Serial.print(client.state());
      Serial.println(" Retry 5s");
      delay(5000);
    }
  }
}
#endif


void setup() {
  Serial.begin(115200);
  delay(1000); 
  Serial.println("Starting LinkyMock...");
  Serial.println("!!! NOUVELLE VERSION TEST !!!");

  // 1. Init Drivers
  lcd_lvgl_Init(); // Safe (Dual Mode)
  
#if defined(CONFIG_IDF_TARGET_ESP32S3)
  Touch_Init();
  lcd_bl_pwm_bsp_init(BACKLIGHT_BRIGHTNESS);

  // 2. Init Knob
  knob_config_t cfg = {
    .gpio_encoder_a = EXAMPLE_ENCODER_ECA_PIN,
    .gpio_encoder_b = EXAMPLE_ENCODER_ECB_PIN,
  };
  s_knob = iot_knob_create(&cfg);
  iot_knob_register_cb(s_knob, KNOB_LEFT, _knob_left_cb, NULL);
  iot_knob_register_cb(s_knob, KNOB_RIGHT, _knob_right_cb, NULL);
#else
  Serial.println("Drivers Hardware (Touch/Knob/BL) desactives pour ESP32 Standard");
#endif
  
  // 3. Init UI
  ui_linky_init();
  
#ifdef ENABLE_REAL_DATA
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(mqtt_callback);
#endif
  
  Serial.println("Setup done.");
}

void loop() {
  static uint32_t last_update = 0;
  
  // LVGL Task
  lv_timer_handler();
  
  // Data Update
#ifdef ENABLE_REAL_DATA
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); // Handle MQTT
  
  // En mode réel, on update l'UI à chaque réception ou périodiquement ?
  // Mettons à jour l'UI périodiquement avec les dernières données reçues
  if (millis() - last_update > 500) { // 2Hz refresh UI
      ui_linky_update(&linky_data);
      last_update = millis();
  }
#else
  // Mock Data Update every 1s
  if (millis() - last_update > 1000) {
      update_mock_data();
      ui_linky_update(&linky_data);
      last_update = millis();
  }
#endif
  
  delay(5);
}
