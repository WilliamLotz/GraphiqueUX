#ifndef MOCK_DATA_H
#define MOCK_DATA_H

#include <stdint.h> // for standard types

typedef struct {
    uint16_t papp;          // Puissance apparente (VA)
    uint8_t iinst;          // Intensité instantanée (A)
    uint32_t index_base;    // Index Base (kWh)
    uint32_t index_hp;      // Index HP (kWh)
    uint32_t index_hc;      // Index HC (kWh)
    uint8_t isousc;         // Intensité souscrite (A)
    char option_tarif[10];  // "BASE" ou "HP.."
    char mot_etat[16];      // Message d'état (Encager to 16)
    uint8_t voltage;        // Tension (simulée)
} linky_data_t;

#endif
