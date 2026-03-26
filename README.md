# KnobTouch — Afficheur Linky Connecté

> **Projet Intégré — L3 NEC**
> Module d'interface graphique pour la visualisation en temps réel de la consommation électrique issue d'un compteur Linky, sur un écran rond tactile piloté par un ESP32-S3.

---

## Table des matières

1. [Introduction](#introduction)
2. [Compilation et déploiement](#compilation-et-déploiement)
3. [Configuration du mode de fonctionnement](#configuration-du-mode-de-fonctionnement)
4. [Configuration MQTT](#configuration-mqtt)
5. [Configuration WiFi](#configuration-wifi)
6. [Configuration matérielle](#configuration-matérielle)
7. [Configuration de l'affichage](#configuration-de-laffichage)
8. [Persistance des données](#persistance-des-données)
9. [Navigation dans l'interface](#navigation-dans-linterface)

---

## Introduction

### Contexte

En France, le déploiement des compteurs communicants **Linky** par Enedis a équipé plus de 35 millions de foyers. Ces compteurs disposent d'une sortie **Télé-Information Client (TIC)** qui transmet en temps réel les données de consommation électrique (puissance, index, intensité, tension, option tarifaire, etc.).

Cependant, ces données ne sont pas directement exploitables par l'utilisateur : elles circulent sous forme de trames série sur un fil dédié, dans un format technique peu lisible. Il est donc nécessaire de concevoir un **dispositif intermédiaire** capable de :

1. **Collecter** les trames TIC du compteur via un module téléinfo
2. **Transmettre** ces données sur un réseau (WiFi / MQTT)
3. **Afficher** les informations de consommation de manière claire et intuitive

C'est dans ce cadre que s'inscrit le projet **KnobTouch** : un afficheur connecté autonome, basé sur un microcontrôleur **ESP32-S3**, qui reçoit les données Linky via le protocole **MQTT** et les présente sur un **écran rond tactile** au travers d'une interface graphique riche et interactive.

Ce projet a été réalisé dans le cadre du **Projet Intégré de L3 NEC**, avec pour objectif de couvrir l'ensemble de la chaîne — de la réception des données réseau jusqu'à leur rendu graphique embarqué — en mobilisant des compétences en programmation embarquée, communication réseau (WiFi/MQTT), conception d'interfaces graphiques (LVGL), et gestion de périphériques matériels (écran QSPI, tactile I2C, encodeur rotatif, carte SD).

### Technologies utilisées

- **ESP32-S3** — Microcontrôleur principal du projet (dual-core Xtensa LX7, WiFi/BLE intégré, 512 Ko SRAM)
- **Arduino Framework** (≥ 2.0) — Framework de développement simplifiant la programmation de l'ESP32 (structure setup/loop, Serial, GPIO...)
- **ESP-IDF** (intégrée via Arduino-ESP32 ≥ 3.x) — SDK bas-niveau d'Espressif, utilisé pour les drivers matériels (SPI, I2C, LEDC, SD_MMC)
- **LVGL** (v8.3.11) — Bibliothèque d'interface graphique embarquée : widgets, animations, gestes tactiles, clavier virtuel
- **PubSubClient** (≥ 2.8) — Client MQTT pour Arduino, gère la connexion au broker et la réception des messages
- **ArduinoJson** (v6.x / v7.x, compatible) — Désérialisation des payloads JSON reçus via MQTT
- **WiFi** (intégrée ESP32) — Connexion au réseau WiFi local pour la communication MQTT
- **SD_MMC** (intégrée ESP32) — Accès à la carte micro-SD via le bus MMC pour la journalisation CSV de l'historique
- **EEPROM** (intégrée ESP32) — Stockage non-volatile des identifiants WiFi et de l'état du compteur (256 octets)
- **FreeRTOS** (intégrée ESP-IDF) — Système d'exploitation temps réel, utilisé pour les mutex LVGL et la gestion des tâches
- **MQTT** (protocole v3.1.1) — Protocole de messagerie léger pour recevoir les données Linky en temps réel
- **NTP** (pool.ntp.org) — Synchronisation de l'heure via Internet (fuseau CET/CEST)
- **SH8601** (driver custom Espressif) — Driver bas-niveau pour l'écran rond AMOLED, support SPI et QSPI
- **CST816** (driver custom) — Driver I2C pour le contrôleur tactile capacitif de l'écran
- **C / C++** (C11 / C++11) — Langages de programmation utilisés pour l'ensemble du firmware
- **Python** (3.x) — Scripts utilitaires de génération d'images (`gen_bg.py`, `gen_boot.py`)

---

## Compilation et déploiement

### Prérequis

| Outil | Version |
|---|---|
| Arduino IDE | ≥ 2.0 |
| Board Manager | ESP32 by Espressif (≥ 3.x) |
| Carte sélectionnée | ESP32S3 Dev Module |

### Bibliothèques à installer

| Bibliothèque | Usage |
|---|---|
| **LVGL** | Framework d'interface graphique |
| **PubSubClient** | Client MQTT |
| **ArduinoJson** | Parsing du payload MQTT |

Les bibliothèques **WiFi**, **SD_MMC** et **EEPROM** sont intégrées au framework ESP32.

### Configuration LVGL

Le fichier `lv_conf.h` doit être configuré avec :
- `LV_COLOR_DEPTH 16`
- `LV_FONT_MONTSERRAT_16 1`
- `LV_FONT_MONTSERRAT_20 1`
- `LV_FONT_MONTSERRAT_30 1`
- `LV_FONT_MONTSERRAT_40 1`

Un fichier de référence `COPIE lv.conf` est fourni dans le projet.

### Étapes de compilation

1. Ouvrir `HelloKnob.ino` dans l'Arduino IDE
2. Sélectionner la carte **ESP32S3 Dev Module**
3. Vérifier que toutes les bibliothèques sont installées
4. Compiler et téléverser

---

## Configuration du mode de fonctionnement

Dans `HelloKnob.ino` (ligne 16) :

```c
#define ENABLE_REAL_DATA   // Activer le WiFi + MQTT (données réelles)
```

| Mode | Comment l'activer | Comportement |
|------|------------------|-------------|
| **Données réelles** | Laisser `#define ENABLE_REAL_DATA` | Se connecte au WiFi et reçoit les données via MQTT |
| **Données simulées (Mock)** | Commenter la ligne (`//#define ENABLE_REAL_DATA`) | Génère des données aléatoires, pas besoin de réseau |
| **Fallback** | Automatique | Si le WiFi est connecté mais qu'aucune donnée MQTT n'arrive, des données Mock sont affichées en attendant |

### Fuseau horaire

```c
configTzTime("CET-1CEST,M3.5.0/2,M10.5.0/3", "pool.ntp.org");
```

Par défaut configuré pour la **France (CET/CEST)** avec changement d'heure automatique. Modifier cette ligne pour un autre fuseau.

---

## Configuration MQTT

### Paramètres de connexion

Dans `HelloKnob.ino` :

```c
const char* serveur_mqtt = "test.mosquitto.org";   // Ligne 37 : adresse du broker
```

```c
client_mqtt.subscribe("lotz/home/linky/status");   // Ligne 168 : topic à écouter
```

| Paramètre | Valeur par défaut | Comment modifier |
|---|---|---|
| Broker | `test.mosquitto.org` | Modifier `serveur_mqtt` (ligne 37) |
| Port | 1883 | Modifier dans `client_mqtt.setServer(...)` (ligne 305) |
| Topic | `lotz/home/linky/status` | Modifier dans `client_mqtt.subscribe(...)` (ligne 168) |

### Format du payload JSON attendu

```json
{
  "papp": 450,        // Puissance Apparente (VA) — OBLIGATOIRE
  "base": 12345678,   // Index Base (Wh) — obligatoire (ou hp+hc)
  "hp": 9876543,      // Index Heures Pleines (Wh) — optionnel
  "hc": 1234567,      // Index Heures Creuses (Wh) — optionnel
  "iinst": 2,         // Intensité Instantanée (A)
  "tension": 232      // Tension (V)
}
```

**Message minimal :**
```json
{ "papp": 1200, "base": 50000 }
```

**Champs obligatoires :**
- `papp` — alimente la jauge principale
- Au moins un index : `base` OU `hp` + `hc`

---

## Configuration WiFi

### Via l'écran tactile (recommandé)

1. Glisser jusqu'à la **page WiFi** (page 5)
2. Toucher le champ **SSID** → le clavier AZERTY apparaît
3. Saisir le nom du réseau
4. Toucher le champ **Mot de passe**
5. Saisir le mot de passe
6. Appuyer sur **Connexion**
7. Le statut passe en orange → puis vert (succès) ou rouge (échec)
8. Les identifiants sont **automatiquement sauvegardés en EEPROM** pour les prochains démarrages

### Via l'EEPROM (automatique)

Au démarrage, si un WiFi a déjà été configuré via l'écran, l'ESP32 se reconnecte automatiquement sans intervention.

---

## Configuration matérielle

### Broches (`lcd_config.h`)

```c
// Écran LCD (QSPI)
#define EXAMPLE_PIN_NUM_LCD_CS      14
#define EXAMPLE_PIN_NUM_LCD_PCLK    13
#define EXAMPLE_PIN_NUM_LCD_DATA0   15
#define EXAMPLE_PIN_NUM_LCD_DATA1   16
#define EXAMPLE_PIN_NUM_LCD_DATA2   17
#define EXAMPLE_PIN_NUM_LCD_DATA3   18
#define EXAMPLE_PIN_NUM_LCD_RST     21

// Tactile (I2C)
#define EXAMPLE_TOUCH_ADDR          0x15
#define EXAMPLE_PIN_NUM_TOUCH_SCL   12
#define EXAMPLE_PIN_NUM_TOUCH_SDA   11

// Rétroéclairage
#define BROCHE_RETROECLAIRAGE       47
```

### Encodeur rotatif (`HelloKnob.ino`)

```c
#define EXAMPLE_ENCODER_ECA_PIN  8
#define EXAMPLE_ENCODER_ECB_PIN  7
```

### Carte SD (MMC)

Broches configurées dans `HelloKnob.ino` :
```c
SD_MMC.setPins(4, 3, 5, 6, 42, 2);
```

---

## Configuration de l'affichage

### Luminosité (`HelloKnob.ino`)

```c
#define BACKLIGHT_BRIGHTNESS 150   // Valeur de 0 (éteint) à 255 (max)
```

### Résolution (`lcd_config.h`)

```c
#define ECRAN_RES_LARG  360   // Largeur (px)
#define ECRAN_RES_HAUT  360   // Hauteur (px)
#define LCD_BIT_PER_PIXEL 16  // RGB565
```

---

## Persistance des données

### EEPROM

Sauvegarde automatiquement :
- **Identifiants WiFi** (SSID + mot de passe) → restaurés au démarrage
- **Dernières données Linky** → toutes les 60 secondes si changement

### Carte SD

Fichier : `/historique_linky.csv`

```
Date,Base(Wh),HP(Wh),HC(Wh),Papp(VA),Iinst(A),Isousc(A),Voltage(V)
2026-03-26,50000,9876,1234,450,2,30,232
```

- **Écriture** : une ligne par jour à minuit
- **Lecture** : au démarrage pour reconstruire les graphiques semaine/année
- **Effacement** : via la page **Reset SD** (page 6) → redémarrage automatique après suppression

---

## Navigation dans l'interface

L'interface comprend **7 pages** accessibles par **glissement tactile** (swipe gauche/droite) :

| Page | Nom | Contenu |
|------|-----|---------|
| 0 | **Jauge** | Puissance instantanée (kW) + 6 barres d'historique + heure/date |
| 1 | **Index** | Compteurs Base, Heures Pleines, Heures Creuses (kWh) |
| 2 | **Semaine** | Graphique radial : consommation sur 7 jours |
| 3 | **Historique** | Graphique radial : consommation sur 12 mois |
| 4 | **Infos** | Tension (V) et intensité (A) avec min/avg/max |
| 5 | **WiFi** | Saisie SSID/mot de passe + bouton connexion |
| 6 | **Reset SD** | Suppression de l'historique carte SD |

**Navigation circulaire** : après la page 6, on revient à la page 0.

**Écran d'accueil** : au démarrage, « Bienvenue sur le KnobTouch ! » s'affiche 5 secondes avec barre de progression, puis fondu vers la jauge.

---

## Auteurs

Projet réalisé dans le cadre du **Projet Intégré L3 NEC**.

---
