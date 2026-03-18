#ifndef UI_LINKY_H
#define UI_LINKY_H

#include "lvgl.h"
#include "mock_data.h"

#ifdef __cplusplus
extern "C" {
#endif

// Initialise toutes les pages de l'interface graphique Linky et charge l'écran principal.
void interface_linky_initialisation();

// Change la page affichée à l'écran (direction = +1 ou -1) selon la navigation de l'encodeur.
void interface_linky_changer_page(int direction);

// Met à jour périodiquement les éléments graphiques avec les dernières données du compteur Linky.
void interface_linky_actualiser(donnees_linky_t *data);

extern char wifi_reseau[32];
extern char wifi_mdp[32];
extern volatile bool connexion_wifi_demandee;
extern volatile bool effacement_sd_demande;

// Modifie le texte et la couleur du label de statut WiFi sur la page de configuration.
void interface_linky_statut_wifi(const char* msg, bool success);

#ifdef __cplusplus
}
#endif

#endif
