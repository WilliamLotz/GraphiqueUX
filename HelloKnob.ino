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

// --- CONFIGURATION DONNÉES RÉELLES (WIFI ACTIVÉ) ---
#define ENABLE_REAL_DATA // <--- ACTIVÉ

#ifdef ENABLE_REAL_DATA
#include <WiFi.h>
#include <PubSubClient.h> 
#include <ArduinoJson.h>  
#include <time.h>
#include <EEPROM.h> 
#include "FS.h"
#include "SD_MMC.h"

// Configuration EEPROM
#define EEPROM_SIZE 256
struct AppConfig {
    char nom_reseau[32];
    char mot_de_passe[32];
    uint8_t initialise; // 0xA5 si valide
    donnees_linky_t donnees_sauvegardees;
};
AppConfig conf_app;

const char* serveur_mqtt = "test.mosquitto.org";

WiFiClient clientEsp;
PubSubClient client_mqtt(clientEsp);
#endif

// Variables Globales du Bouton (Mode Logiciel)
volatile int dernierEncodeur = 0;
volatile long valeurEncodeur = 0;
volatile int requete_evenement_bouton = 0; // 0=Aucun, 1=Droite, -1=Gauche

// Routine d'interruption (ISR) pour détecter la rotation de l'encodeur par lecture en quadrature.
void IRAM_ATTR majEncodeur() {
  int a = digitalRead(EXAMPLE_ENCODER_ECA_PIN);
  int b = digitalRead(EXAMPLE_ENCODER_ECB_PIN);

  if (a == b) valeurEncodeur++;
  else valeurEncodeur--;
  
  static long derniere_val = 0;
  if (abs(valeurEncodeur - derniere_val) >= 2) { 
      if (valeurEncodeur > derniere_val) requete_evenement_bouton = 1;
      else requete_evenement_bouton = -1;
      derniere_val = valeurEncodeur;
  }
}

// Instance globale stockant l'état simulé ou réel du compteur Linky.
donnees_linky_t donnees_linky = {
    .papp = 0, .iinst = 0, .index_base = 0, .index_hp = 0, .index_hc = 0,
    .isousc = 0, .option_tarif = "BASE", .mot_etat = "ATTENTE", .tension = 0
};

// Met à jour les valeurs de consommation du compteur avec des données semi-aléatoires (mode Mock).
void maj_donnees_fictives() {
    int variation = (rand() % 401) - 200; 
    int nouvelle_papp = (int)donnees_linky.papp + variation;
    if (nouvelle_papp < 0) nouvelle_papp = 100;
    if (nouvelle_papp > 9000) nouvelle_papp = 9000;
    donnees_linky.papp = (uint16_t)nouvelle_papp;
    donnees_linky.iinst = donnees_linky.papp / 230;
    if(donnees_linky.iinst == 0 && donnees_linky.papp > 0) donnees_linky.iinst = 1;
    donnees_linky.tension = 225 + (rand() % 18);
    if ((rand() % 100) < 5) donnees_linky.index_base++;
}

// Remplit les historiques avec des données aléatoires s'ils sont vides
void generer_historique_aleatoire() {
    bool est_vide = true;
    for (int i = 0; i < 12; i++) {
        if (donnees_linky.historique_annee[i] != 0) {
            est_vide = false;
            break;
        }
    }
    
    if (est_vide) {
        Serial.println("Génération d'un historique aléatoire (Semaine & Année)...");
        for (int i = 0; i < 7; i++) {
            donnees_linky.historique_semaine[i] = 15 + (rand() % 40); // 15 à 55 kWh
        }
        for (int i = 0; i < 12; i++) {
            donnees_linky.historique_annee[i] = (120 + (rand() % 160)) * 1000UL; // 120 à 280 kWh (en Wh)
        }
    }
}

#ifdef ENABLE_REAL_DATA


bool mqtt_a_donnees = false;

// Callback appelé lors de la réception d'un message MQTT pour la mise à jour des paramètres Linky.
void rappel_mqtt(char* topic, byte* payload, unsigned int length) {
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

  donnees_linky_t anciennes_donnees = donnees_linky;

  if (doc.containsKey("papp")) donnees_linky.papp = doc["papp"];
  if (doc.containsKey("base")) donnees_linky.index_base = doc["base"];
  if (doc.containsKey("hp")) donnees_linky.index_hp = doc["hp"];
  if (doc.containsKey("hc")) donnees_linky.index_hc = doc["hc"];
  if (doc.containsKey("iinst")) donnees_linky.iinst = doc["iinst"];
  if (doc.containsKey("tension")) donnees_linky.tension = doc["tension"];

  mqtt_a_donnees = true;

  static uint32_t derniere_sauvegarde_eeprom = 0;
  if (memcmp(&anciennes_donnees, &donnees_linky, sizeof(donnees_linky_t)) != 0) {
      if (millis() - derniere_sauvegarde_eeprom > 60000) {
          conf_app.donnees_sauvegardees = donnees_linky;
          EEPROM.put(0, conf_app);
          EEPROM.commit();
          derniere_sauvegarde_eeprom = millis();
          Serial.println("Donnees Linky sauvegardees en EEPROM");
      }
  }
}

// Tente de reconnecter le client_mqtt MQTT au broker avec un délai anti-spam de 5 secondes.
void reconnexion_mqtt() {
  static uint32_t derniere_tentative_reconnexion = 0;
  if (!client_mqtt.connected() && (millis() - derniere_tentative_reconnexion > 5000)) {
    Serial.print("Connexion au broker MQTT...");
    String identifiantClient = "ESP32-Linky-";
    identifiantClient += String(random(0xffff), HEX);
    
    if (client_mqtt.connect(identifiantClient.c_str())) {
      Serial.println("Connecté!");
      client_mqtt.subscribe("lotz/home/linky/status");
    } else {
      Serial.print("Echec, rc=");
      Serial.print(client_mqtt.state());
      Serial.println(" nouvel essai dans 5s");
    }
    derniere_tentative_reconnexion = millis();
  }
}
#endif


// --- CARTE SD (Historique) ---
bool carte_sd_ok = false;

// Lit le fichier CSV d'historique sur la SD et reconstruit les différents tableaux de statistiques.
void lire_historique_depuis_sd() {
    if (!SD_MMC.exists("/historique_linky.csv")) return;
    
    File f = SD_MMC.open("/historique_linky.csv", FILE_READ);
    if (!f) return;

    for (int i=0; i<7; i++) donnees_linky.historique_semaine[i] = 0;
    for (int i=0; i<12; i++) donnees_linky.historique_annee[i] = 0;

    String entete = f.readStringUntil('\n');
    uint32_t dernier_index = 0;
    
    while(f.available()) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) continue;
        
        int premiere_virgule = line.indexOf(',');
        if (premiere_virgule == -1) continue;
        String chaine_date = line.substring(0, premiere_virgule);
        
        int deuxieme_virgule = line.indexOf(',', premiere_virgule+1);
        if (deuxieme_virgule == -1) continue;
        String chaine_base = line.substring(premiere_virgule+1, deuxieme_virgule);
        uint32_t index_actuel = chaine_base.toInt();
        
        if (dernier_index > 0 && index_actuel >= dernier_index) {
            uint32_t conso_jour_wh = index_actuel - dernier_index;
            
            int tiret1 = chaine_date.indexOf('-');
            int tiret2 = chaine_date.lastIndexOf('-');
            if (tiret1 != -1 && tiret2 != -1) {
                int annee = chaine_date.substring(0, tiret1).toInt();
                int mois = chaine_date.substring(tiret1+1, tiret2).toInt();
                int jour = chaine_date.substring(tiret2+1).toInt();
                
                if (mois >= 1 && mois <= 12) {
                    donnees_linky.historique_annee[mois - 1] += conso_jour_wh;
                }
                
                if (mois < 3) { mois += 12; annee -= 1; }
                int K = annee % 100; int J = annee / 100;
                int h = (jour + 13*(mois+1)/5 + K + K/4 + J/4 + 5*J) % 7;
                int jour_semaine = ((h + 5) % 7); 
                
                donnees_linky.historique_semaine[jour_semaine] = (uint16_t)(conso_jour_wh / 1000); 
            }
        }
        dernier_index = index_actuel;
    }
    f.close();
    Serial.println("Historique CSV lu depuis la carte SD.");
}

// Initialise la communication SD, monte le disque virtuel et crée/lit le fichier d'historique au besoin.
void initialisation_carte_sd() {
    SD_MMC.setPins(4, 3, 5, 6, 42, 2); 
    
    if(!SD_MMC.begin("/sdcard", false, true)) {
        Serial.println("Erreur Montage Carte SD");
        carte_sd_ok = false;
        return;
    }
    uint8_t typeCarte = SD_MMC.cardType();
    if(typeCarte == CARD_NONE){
        Serial.println("Aucune Carte SD n'est inseree");
        carte_sd_ok = false;
        return;
    }
    Serial.printf("Carte SD OK ! Taille: %lluMB\n", SD_MMC.cardSize() / (1024 * 1024));
    carte_sd_ok = true;
    
    if (!SD_MMC.exists("/historique_linky.csv")) {
        File f = SD_MMC.open("/historique_linky.csv", FILE_WRITE);
        if (f) {
            f.println("Date,Base(Wh),HP(Wh),HC(Wh),Papp(VA),Iinst(A),Isousc(A),Voltage(V)");
            f.close();
            Serial.println("Fichier historique CSV initialise.");
        }
    } else {
        lire_historique_depuis_sd();
    }
}

// Enregistre les statistiques de la journée dans un fichier CSV sur la carte SD à minuit pile.
void journaliser_jour_sur_sd() {
    if (!carte_sd_ok || !mqtt_a_donnees) return;
    
    static int dernier_jour_journalise = -1;
    time_t maintenant;
    time(&maintenant);
    struct tm *temps_actuel = localtime(&maintenant);
    
    if (temps_actuel->tm_year < 120) return; 
    
    if (dernier_jour_journalise != -1 && temps_actuel->tm_mday != dernier_jour_journalise) {
        File f = SD_MMC.open("/historique_linky.csv", FILE_APPEND);
        if (f) {
            char ligne_journal[128];
            snprintf(ligne_journal, sizeof(ligne_journal), "%04d-%02d-%02d,%lu,%lu,%lu,%u,%u,%u,%u", 
                     temps_actuel->tm_year+1900, temps_actuel->tm_mon+1, temps_actuel->tm_mday, 
                     donnees_linky.index_base, donnees_linky.index_hp, donnees_linky.index_hc,
                     donnees_linky.papp, donnees_linky.iinst, donnees_linky.isousc, donnees_linky.tension);
            f.println(ligne_journal);
            f.close();
            Serial.print("Historique sauvegarde sur SD : ");
            Serial.println(ligne_journal);
        }
    }
    dernier_jour_journalise = temps_actuel->tm_mday;
}

// Initialisation globale du système (LCD, I2C, WiFi, MQTT, LVGL, SD, EEPROM).
void setup() {
  Serial.begin(115200);
  delay(1000); 
  Serial.println("Starting LinkyMock (WiFi + SoftKnob)...");

#ifdef ENABLE_REAL_DATA
    Serial.println("MODE WIFI ACTIF");

    client_mqtt.setServer(serveur_mqtt, 1883);
    client_mqtt.setCallback(rappel_mqtt);
    configTzTime("CET-1CEST,M3.5.0/2,M10.5.0/3", "pool.ntp.org");
#else
    Serial.println("MODE MOCK ACTIF");
    struct tm temps_m;
    temps_m.tm_year = 2026 - 1900; temps_m.tm_mon = 0; temps_m.tm_mday = 26;
    temps_m.tm_hour = 22; temps_m.tm_min = 22; temps_m.tm_sec = 0;
    time_t temps = mktime(&temps_m);
    struct timeval temps_val = { .tv_sec = temps };
    settimeofday(&temps_val, NULL);
#endif

  lcd_lvgl_Init(); 
  
#if defined(CONFIG_IDF_TARGET_ESP32S3)
  Serial.println("S3 Init Hardware...");
  initialisation_tactile();
  initialisation_pwm_lcd_bsp(BACKLIGHT_BRIGHTNESS);

  Serial.println("KNOB SETUP (SOFTWARE MODE)...");
  pinMode(EXAMPLE_ENCODER_ECA_PIN, INPUT_PULLUP);
  pinMode(EXAMPLE_ENCODER_ECB_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(EXAMPLE_ENCODER_ECA_PIN), majEncodeur, CHANGE);
  Serial.println("KNOB INIT DONE");
#endif

  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(0, conf_app);
  if (conf_app.initialise == 0xA5) {
      Serial.println("EEPROM WiFi Found:");
      Serial.println(conf_app.nom_reseau);
      strcpy(wifi_reseau, conf_app.nom_reseau);
      strcpy(wifi_mdp, conf_app.mot_de_passe);
      connexion_wifi_demandee = true;
      
      donnees_linky = conf_app.donnees_sauvegardees;
      mqtt_a_donnees = true;
  } else {
      Serial.println("No WiFi saved in EEPROM."); // Aucun WiFi sauvegardé dans l'EEPROM.
  }
  
  initialisation_carte_sd();

  // generer_historique_aleatoire(); // Désactivé pour afficher le réel état vide après suppression SD

  interface_linky_initialisation();

  Serial.println("Setup done."); // Fin de l'initialisation.
}

// Boucle principale qui maintient la connexion WiFi/MQTT, met à jour l'UID LVGL et synchronise les données affichées.
void loop() {
  if (effacement_sd_demande) {
      effacement_sd_demande = false;
      Serial.println("Demande d'effacement SD recue");
      if (carte_sd_ok) {
          SD_MMC.remove("/historique_linky.csv");
          // Forçage de l'effacement par l'écrasement immédiat avec un fichier vide
          File f = SD_MMC.open("/historique_linky.csv", FILE_WRITE);
          if (f) {
              f.println("Date,Base(Wh),HP(Wh),HC(Wh),Papp(VA),Iinst(A),Isousc(A),Voltage(V)");
              f.close();
              Serial.println("Fichier ecrase avec succes.");
          }
      }
      
      // Vider totalement l'état en mémoire
      donnees_linky = {0};
      strcpy(donnees_linky.option_tarif, "BASE");
      strcpy(donnees_linky.mot_etat, "EFFACE");
      
      if (conf_app.initialise == 0xA5) {
          conf_app.donnees_sauvegardees = donnees_linky;
          EEPROM.put(0, conf_app);
          EEPROM.commit();
          Serial.println("EEPROM mise a zero pour l'historique.");
      }

      Serial.println("Redemarrage de l'ESP en cours...");
      Serial.flush();
      delay(1000);
      ESP.restart();
  }

  if (requete_evenement_bouton != 0) {
      Serial.print("Knob Event: "); Serial.println(requete_evenement_bouton);
      // interface_linky_changer_page(requete_evenement_bouton); // Désactivé: Remplacé par les gestes tactiles
      requete_evenement_bouton = 0;
  }
  
  static long dernier_enc_debug = 0;
  if(valeurEncodeur != dernier_enc_debug) {
      Serial.print("Raw Enc: "); Serial.println(valeurEncodeur);
      dernier_enc_debug = valeurEncodeur;
  }

  static uint32_t derniere_maj = 0;
  
  lv_timer_handler();
  
#ifdef ENABLE_REAL_DATA
  if (WiFi.status() == WL_CONNECTED) {
      if (!client_mqtt.connected()) {
          reconnexion_mqtt();
      }
      client_mqtt.loop();
  }

  if (millis() - derniere_maj > 500) { 
      if (!mqtt_a_donnees) {
          maj_donnees_fictives();
      }
      interface_linky_actualiser(&donnees_linky);
      derniere_maj = millis();
      journaliser_jour_sur_sd();
  }
#else
  if (millis() - derniere_maj > 1000) {
      maj_donnees_fictives();
      interface_linky_actualiser(&donnees_linky);
      derniere_maj = millis();
  }
#endif
  
  if (connexion_wifi_demandee) {

      connexion_wifi_demandee = false;
      
      String chaine_ssid = String(wifi_reseau); chaine_ssid.trim();
      String chaine_mdp = String(wifi_mdp); chaine_mdp.trim();
      
      Serial.print("Connecting to: ["); Serial.print(chaine_ssid); Serial.println("]");
      
      WiFi.mode(WIFI_STA);
      WiFi.disconnect(true);
      delay(100);
      WiFi.begin(chaine_ssid.c_str(), chaine_mdp.c_str());
      
      uint32_t debut_wifi = millis();
      bool connecte = false;
      while(millis() - debut_wifi < 20000) {
          if(WiFi.status() == WL_CONNECTED) {
              connecte = true;
              break;
          }
          lv_timer_handler();
          delay(100);
      }
      
      if(connecte) {
          Serial.println("WiFi Connected!");
          Serial.println(WiFi.localIP());
          
          strcpy(conf_app.nom_reseau, chaine_ssid.c_str());
          strcpy(conf_app.mot_de_passe, chaine_mdp.c_str());
          conf_app.initialise = 0xA5;
          EEPROM.put(0, conf_app);
          EEPROM.commit();
          Serial.println("WiFi Saved to EEPROM!");

          String ip = WiFi.localIP().toString();
          String msg = "Connecte " + ip;
          interface_linky_statut_wifi(msg.c_str(), true);
      } else {
          Serial.print("WiFi Failed, Status: "); Serial.println(WiFi.status());
          interface_linky_statut_wifi("Echec Connexion", false);
      }
  }

  delay(5);
}
