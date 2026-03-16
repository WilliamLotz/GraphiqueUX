/*
 * SPDX-FileCopyrightText: 2016-2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 * 
 * 
 * Modified by planevina 2025-01-20
 */

#pragma once

#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef void (*rappel_encodeur_t)(void *, void *);
    typedef void *handle_encodeur_t;

    // Événements du bouton
    typedef enum
    {
        BOUTON_GAUCHE = 0,  // ÉVÉNEMENT : Rotation vers la gauche
        BOUTON_DROITE,     // ÉVÉNEMENT : Rotation vers la droite
        BOUTON_EVENEMENT_MAX, // ÉVÉNEMENT : Nombre d'événements
        BOUTON_AUCUN,      // ÉVÉNEMENT : Aucun événement
    } evenement_bouton_t;

    // Configuration du bouton
    typedef struct
    {
        uint8_t gpio_encodeur_a; // Broche de l'encodeur A
        uint8_t gpio_encodeur_b; // Broche de l'encodeur B
    } config_bouton_t;

    // Crée un encodeur rotatif selon la configuration donnée et retourne son handle.
    handle_encodeur_t creer_encodeur(const config_bouton_t *config);

    // Supprime un encodeur rotatif et libère ses ressources.
    esp_err_t supprimer_encodeur(handle_encodeur_t handle_encodeur);

    // Enregistre une fonction de rappel pour un événement spécifique de l'encodeur.
    esp_err_t enregistrer_rappel_encodeur(handle_encodeur_t handle_encodeur, evenement_bouton_t event, rappel_encodeur_t cb, void *usr_data);

    // Désenregistre la fonction de rappel d'un événement spécifique.
    esp_err_t desenregistrer_rappel_encodeur(handle_encodeur_t handle_encodeur, evenement_bouton_t event);

    // Récupère le dernier événement généré par l'encodeur.
    evenement_bouton_t obtenir_evenement_encodeur(handle_encodeur_t handle_encodeur);

    // Récupère la valeur actuelle du compteur de l'encodeur.
    int obtenir_valeur_compteur_encodeur(handle_encodeur_t handle_encodeur);

    // Réinitialise le compteur de l'encodeur à zéro.
    esp_err_t effacer_valeur_compteur_encodeur(handle_encodeur_t handle_encodeur);

    // Relance le timer de scrutation des encodeurs (nécessite iot_knob_create au préalable).
    esp_err_t reprendre_encodeur(void);

    // Arrête le timer de scrutation des encodeurs.
    esp_err_t arreter_encodeur(void);

    // Configure une broche GPIO en entrée avec résistance de tirage (pull-up) pour l'encodeur.
    esp_err_t init_gpio_encodeur(uint32_t gpio_num);

    // Réinitialise une broche GPIO utilisée par l'encodeur.
    esp_err_t deinit_gpio_encodeur(uint32_t gpio_num);

    // Lit et retourne l'état logique matériel d'une broche GPIO.
    uint8_t obtenir_niveau_gpio_encodeur(void *gpio_num);

#ifdef __cplusplus
}
#endif
