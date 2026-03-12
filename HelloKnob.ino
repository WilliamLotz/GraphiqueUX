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
#include <EEPROM.h> 
#include "FS.h"
#include "SD_MMC.h"

// EEPROM Config
#define EEPROM_SIZE 256
struct AppConfig {
    char ssid[32];
    char pwd[32];
    uint8_t initialized; // 0xA5 if valid
    linky_data_t saved_data;
};
AppConfig app_conf;

const char* mqtt_server = "test.mosquitto.org";

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


bool mqtt_has_data = false;

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message MQTT recu [");
  Serial.print(topic);
  Serial.print("] ");
  
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

#if ARDUINOJSON_VERSION_MAJOR >= 7
  JsonDocument doc;
#else
  StaticJsonDocument<512> doc;
#endif

  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.print("Erreur JSON: ");
    Serial.println(error.c_str());
    return;
  }

  linky_data_t old_data = linky_data;

  if (doc.containsKey("papp")) linky_data.papp = doc["papp"];
  if (doc.containsKey("base")) linky_data.index_base = doc["base"];
  if (doc.containsKey("hp")) linky_data.index_hp = doc["hp"];
  if (doc.containsKey("hc")) linky_data.index_hc = doc["hc"];
  if (doc.containsKey("iinst")) linky_data.iinst = doc["iinst"];
  if (doc.containsKey("voltage")) linky_data.voltage = doc["voltage"];

 // "C:\Program Files\mosquitto\mosquitto_pub.exe" -h test.mosquitto.org -t "lotz/home/linky/status" -m "{\"papp\": 3000, \"voltage\": 236, \"iinst\": 5}"
  
  mqtt_has_data = true;

  // Sauvegarder dans la mémoire mais on se limite à une fois toutes les 60 secondes pour ne pas griller la mémoire Flash
  static uint32_t last_eeprom_save = 0;
  if (memcmp(&old_data, &linky_data, sizeof(linky_data_t)) != 0) {
      if (millis() - last_eeprom_save > 60000) {
          app_conf.saved_data = linky_data;
          EEPROM.put(0, app_conf);
          EEPROM.commit();
          last_eeprom_save = millis();
          Serial.println("Donnees Linky sauvegardees en EEPROM");
      }
  }
}

void reconnect() {
  static uint32_t last_reconnect_attempt = 0;
  if (!client.connected() && (millis() - last_reconnect_attempt > 5000)) {
    Serial.print("Connexion au broker MQTT...");
    String clientId = "ESP32-Linky-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("Connecté!");
      client.subscribe("lotz/home/linky/status");
    } else {
      Serial.print("Echec, rc=");
      Serial.print(client.state());
      Serial.println(" nouvel essai dans 5s");
    }
    last_reconnect_attempt = millis();
  }
}
#endif


// --- CARTE SD (Historique) ---
bool sd_card_ok = false;

// Lit le CSV et reconstruit les tableaux history_week et history_year
void read_history_from_sd() {
    if (!SD_MMC.exists("/historique_linky.csv")) return;
    
    File f = SD_MMC.open("/historique_linky.csv", FILE_READ);
    if (!f) return;

    // Reset des tableaux
    for (int i=0; i<7; i++) linky_data.history_week[i] = 0;
    for (int i=0; i<12; i++) linky_data.history_year[i] = 0;

    String header = f.readStringUntil('\n'); // Passer l'entête
    
    uint32_t last_index = 0;
    
    while(f.available()) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) continue;
        
        // Parsing basique: 2026-02-28,12500000,0,0,3000,5,45,236
        int first_comma = line.indexOf(',');
        if (first_comma == -1) continue;
        String date_str = line.substring(0, first_comma);
        
        int second_comma = line.indexOf(',', first_comma+1);
        if (second_comma == -1) continue;
        String base_str = line.substring(first_comma+1, second_comma);
        uint32_t current_index = base_str.toInt();
        
        // Si on a un index precedent, la conso du jour est la difference
        if (last_index > 0 && current_index >= last_index) {
            uint32_t conso_jour_wh = current_index - last_index;
            
            // Extraire Mois (01-12)
            int dash1 = date_str.indexOf('-');
            int dash2 = date_str.lastIndexOf('-');
            if (dash1 != -1 && dash2 != -1) {
                int year = date_str.substring(0, dash1).toInt();
                int month = date_str.substring(dash1+1, dash2).toInt();
                int day = date_str.substring(dash2+1).toInt();
                
                // Ajouter à l'année
                if (month >= 1 && month <= 12) {
                    linky_data.history_year[month - 1] += conso_jour_wh;
                }
                
                // Calcul empirique du jour de la semaine pour Zeller ou juste mettre dans week
                // Pour simplifier et ne pas implémenter Zeller en entier ici, 
                // on va remplir le dernier mois dans l'année. 
                // Dans la réalité, the week history is complex to map strictly to "last 7 days" without full UNIX timestamps.
                // On met une implémentation approchée (jour % 7 pour l'instant) ou juste laisser LVGL gerer une pseudo-data si trop vieux
                
                // Calcul Jour de la semaine (0=Dimanche, 1=Lundi...) // Algo de Zeller
                if (month < 3) { month += 12; year -= 1; }
                int K = year % 100; int J = year / 100;
                int h = (day + 13*(month+1)/5 + K + K/4 + J/4 + 5*J) % 7;
                // convert to 0=Lundi, 6=Dimanche
                int wday = ((h + 5) % 7); 
                
                // On ecrase le jour de la semaine (ca gardera les 7 derniers jours par rotation)
                linky_data.history_week[wday] = (uint16_t)(conso_jour_wh / 1000); // En kWh pour l'UI
                
                // Note : Pour l'année on garde en Wh, on divisera dans l'UI
            }
        }
        last_index = current_index;
    }
    f.close();
    Serial.println("Historique CSV lu depuis la carte SD.");
}

void init_sd_card() {
    // Broches pour la carte SD du module Waveshare (D3=2, CMD=3, CLK=4, D0=5, D1=6, D2=42)
    SD_MMC.setPins(4, 3, 5, 6, 42, 2); 
    
    if(!SD_MMC.begin("/sdcard", false, true)) { // format_if_empty = true
        Serial.println("Erreur Montage Carte SD");
        sd_card_ok = false;
        return;
    }
    uint8_t cardType = SD_MMC.cardType();
    if(cardType == CARD_NONE){
        Serial.println("Aucune Carte SD n'est inseree");
        sd_card_ok = false;
        return;
    }
    Serial.printf("Carte SD OK ! Taille: %lluMB\n", SD_MMC.cardSize() / (1024 * 1024));
    sd_card_ok = true;
    
    // Creer fichier historique avec entetes si absent
    if (!SD_MMC.exists("/historique_linky.csv")) {
        File f = SD_MMC.open("/historique_linky.csv", FILE_WRITE);
        if (f) {
            f.println("Date,Base(Wh),HP(Wh),HC(Wh),Papp(VA),Iinst(A),Isousc(A),Voltage(V)");
            f.close();
            Serial.println("Fichier historique CSV initialise.");
        }
    } else {
        read_history_from_sd(); // Charger l'historique en memoire
    }
}

void log_daily_to_sd() {
    if (!sd_card_ok || !mqtt_has_data) return;
    
    static int last_logged_day = -1;
    time_t now;
    time(&now);
    struct tm *t = localtime(&now);
    
    // Ne pas logger si l'heure n'est pas encore synchro
    if (t->tm_year < 120) return; 
    
    // On log la conso à chaque changement de jour (à minuit pile) ou au premier demarrage si le jour a changé
    if (last_logged_day != -1 && t->tm_mday != last_logged_day) {
        File f = SD_MMC.open("/historique_linky.csv", FILE_APPEND);
        if (f) {
            char log_line[128];
            snprintf(log_line, sizeof(log_line), "%04d-%02d-%02d,%lu,%lu,%lu,%u,%u,%u,%u", 
                     t->tm_year+1900, t->tm_mon+1, t->tm_mday, 
                     linky_data.index_base, linky_data.index_hp, linky_data.index_hc,
                     linky_data.papp, linky_data.iinst, linky_data.isousc, linky_data.voltage);
            f.println(log_line);
            f.close();
            Serial.print("Historique sauvegarde sur SD : ");
            Serial.println(log_line);
        }
    }
    last_logged_day = t->tm_mday;
}

void setup() {
  Serial.begin(115200);
  delay(1000); 
  Serial.println("Starting LinkyMock (WiFi + SoftKnob)...");

#ifdef ENABLE_REAL_DATA
    Serial.println("MODE WIFI ACTIF");

    client.setServer(mqtt_server, 1883);
    client.setCallback(mqtt_callback);
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

  // Load Flash memory from EEPROM
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(0, app_conf);
  if (app_conf.initialized == 0xA5) {
      Serial.println("EEPROM WiFi Found:");
      Serial.println(app_conf.ssid);
      // Auto-connect on boot
      strcpy(wifi_ssid, app_conf.ssid);
      strcpy(wifi_pwd, app_conf.pwd);
      wifi_connect_requested = true; // Trigger connection
      
      // Restaurer les dernières données Linky connues
      linky_data = app_conf.saved_data;
      mqtt_has_data = true; // Bloquer le mock, utiliser les vraies données fixes en attendant le vrai message
  } else {
      Serial.println("No WiFi saved in EEPROM.");
  }
  
  // 5. Init SD Card
  init_sd_card();

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
  if (WiFi.status() == WL_CONNECTED) {
      if (!client.connected()) {
          reconnect();
      }
      client.loop(); // Traite les messages MQTT entrants
  }

  if (millis() - last_update > 500) { 
      if (!mqtt_has_data) {
          update_mock_data(); // Valeurs fictives tant qu'on n'a pas recu de vrai message MQTT
      }
      ui_linky_update(&linky_data);
      last_update = millis();
      log_daily_to_sd(); // Verifie s'il faut logger (changement de date)
  }
#else
  // Mock Flow
  if (millis() - last_update > 1000) {
      update_mock_data();
      ui_linky_update(&linky_data);
      last_update = millis();
  }
#endif
  
  // 4. WiFi Handler (Dynamic)
  if (wifi_connect_requested) {

      wifi_connect_requested = false;
      
      // Trim potential newlines from UI input
      String ssid_str = String(wifi_ssid); ssid_str.trim();
      String pwd_str = String(wifi_pwd); pwd_str.trim();
      
      Serial.print("Connecting to: ["); Serial.print(ssid_str); Serial.println("]");
      
      WiFi.mode(WIFI_STA); // Force Station mode
      WiFi.disconnect(true); // Full disconnect/cleanup
      delay(100);
      WiFi.begin(ssid_str.c_str(), pwd_str.c_str());
      
      uint32_t start_wifi = millis();
      bool connected = false;
      while(millis() - start_wifi < 20000) { // 20s Timeout (More time)
          if(WiFi.status() == WL_CONNECTED) {
              connected = true;
              break;
          }
          lv_timer_handler(); // Keep UI alive
          delay(100);
      }
      
      if(connected) {
          Serial.println("WiFi Connected!");
          Serial.println(WiFi.localIP());
          
          // Save to EEPROM
          strcpy(app_conf.ssid, ssid_str.c_str());
          strcpy(app_conf.pwd, pwd_str.c_str());
          app_conf.initialized = 0xA5;
          EEPROM.put(0, app_conf);
          EEPROM.commit();
          Serial.println("WiFi Saved to EEPROM!");

          String ip = WiFi.localIP().toString();
          String msg = "Connecte " + ip;
          ui_linky_set_wifi_status(msg.c_str(), true);
      } else {
          Serial.print("WiFi Failed, Status: "); Serial.println(WiFi.status());
          ui_linky_set_wifi_status("Echec Connexion", false);
      }
  }

  delay(5);
}
