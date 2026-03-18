#ifndef MOCK_DATA_H
#define MOCK_DATA_H

#include <stdint.h> // pour les types standard

// Type de structure contenant l'état, les index et l'historique de consommation du compteur Linky.
typedef struct {
    uint16_t papp;          // Puissance apparente (VA)
    uint8_t iinst;          // Intensité instantanée (A)
    uint32_t index_base;    // Index Base (kWh)
    uint32_t index_hp;      // Index HP (kWh)
    uint32_t index_hc;      // Index HC (kWh)
    uint8_t isousc;         // Intensité souscrite (A)
    char option_tarif[10];  // Option tarifaire ("BASE" ou "HP..")
    char mot_etat[16];      // Message d'état
    uint8_t tension;        // Tension (simulée ou réelle)
    uint16_t historique_semaine[7];  // Conso des 7 derniers jours (Wh)
    uint32_t historique_annee[12]; // Conso des 12 derniers mois (Wh)
} donnees_linky_t;

#endif
