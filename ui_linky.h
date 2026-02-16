#ifndef UI_LINKY_H
#define UI_LINKY_H

#include "lvgl.h"
#include "mock_data.h"

#ifdef __cplusplus
extern "C" {
#endif

// Initialisation de l'UI
void ui_linky_init();

// Appelé quand le knob tourne (+1 ou -1)
void ui_linky_change_page(int direction);

// Appelé périodiquement pour mettre à jour les valeurs
void ui_linky_update(linky_data_t *data);

// Variables partagées pour le WiFi
extern char wifi_ssid[32];
extern char wifi_pwd[32];
extern volatile bool wifi_connect_requested;

// Fonction pour mettre à jour le statut WiFi depuis le main
void ui_linky_set_wifi_status(const char* msg, bool success);

#ifdef __cplusplus
}
#endif

#endif
