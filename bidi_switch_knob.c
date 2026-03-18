/*
 * SPDX-FileCopyrightText: 2016-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 *
 *
 * Modified by planevina 2025-01-20
 */

#include <stdio.h>
#include "driver/gpio.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "bidi_switch_knob.h"

static const char *TAG = "Knob";

#define TICKS_INTERVAL 3
#define DEBOUNCE_TICKS 2

#define KNOB_CHECK(a, str, ret_val)                               \
    if (!(a))                                                     \
    {                                                             \
        ESP_LOGE(TAG, "%s(%d): %s", __FUNCTION__, __LINE__, str); \
        return (ret_val);                                         \
    }

#define KNOB_CHECK_GOTO(a, str, label)                                         \
    if (!(a))                                                                  \
    {                                                                          \
        ESP_LOGE(TAG, "%s:%d (%s):%s", __FILE__, __LINE__, __FUNCTION__, str); \
        goto label;                                                            \
    }

#define CALL_EVENT_CB(ev) \
    if (knob->cb[ev])     \
    knob->cb[ev](knob, knob->usr_data[ev])

typedef struct Knob
{
    bool encodeur_a_change;                          // true signifie que la phase de l'encodeur A est inversée
    bool encodeur_b_change;                          // true signifie que la phase de l'encodeur B est inversée
    uint8_t anti_rebond_a_cpt;                         // Compteur d'anti-rebond de la phase de l'encodeur A
    uint8_t anti_rebond_b_cpt;                         // Compteur d'anti-rebond de la phase de l'encodeur B
    uint8_t niveau_encodeur_a;                        // Niveau actuel de la phase de l'encodeur A
    uint8_t niveau_encodeur_b;                        // Niveau actuel de la phase de l'encodeur B
    evenement_bouton_t evenement;                             // Événement actuel
    int valeur_compteur;                                // Valeur du compteur du bouton
    uint8_t (*hal_niveau_bouton)(void *hardware_data); // Obtenir le niveau actuel
    void *encodeur_a;                                // Numéro de GPIO de la phase de l'encodeur A
    void *encodeur_b;                                // Numéro de GPIO de la phase de l'encodeur B
    void *donnees_usr[BOUTON_EVENEMENT_MAX];                 // Données utilisateur pour l'événement
    rappel_encodeur_t rappel[BOUTON_EVENEMENT_MAX];                   // Fonction de rappel d'événement
    struct Knob *suivant;                              // Pointeur suivant
} dev_bouton_t;

static dev_bouton_t *s_tete_handle = NULL;
static esp_timer_handle_t s_knob_timer_handle;
static bool s_timer_en_cours = false;

// Traite l'état logique d'un canal, gère l'anti-rebond, et met à jour le compteur/événements.
static void process_knob_channel(uint8_t current_level, uint8_t *prev_level,
                                 uint8_t *debounce_cnt, int *count_value,
                                 evenement_bouton_t event, bool is_increment, dev_bouton_t *knob)
{
    if (current_level == 0)
    {
        if (current_level != *prev_level)
            *debounce_cnt = 0;
        else
            (*debounce_cnt)++;
    }
    else
    {
        if (current_level != *prev_level && ++(*debounce_cnt) >= DEBOUNCE_TICKS)
        {
            *debounce_cnt = 0;
            *count_value += is_increment ? 1 : -1;
            knob->evenement = event;
            if (knob->rappel[event])
                knob->rappel[event](knob, knob->donnees_usr[event]);
        }
        else
            *debounce_cnt = 0;
    }
    *prev_level = current_level;
}

// Lit l'état matériel des canaux A/B et met à jour le compteur (gauche/droite) via process_knob_channel.
static void knob_handler(dev_bouton_t *knob)
{
    uint8_t pha_value = knob->hal_niveau_bouton(knob->encodeur_a);
    uint8_t phb_value = knob->hal_niveau_bouton(knob->encodeur_b);

    process_knob_channel(pha_value, &knob->niveau_encodeur_a,
                         &knob->anti_rebond_a_cpt, &knob->valeur_compteur,
                         BOUTON_DROITE, true, knob);

    process_knob_channel(phb_value, &knob->niveau_encodeur_b,
                         &knob->anti_rebond_b_cpt, &knob->valeur_compteur,
                         BOUTON_GAUCHE, false, knob);
}

// Callback périodique du timer matériel scrutant l'état de tous les encodeurs enregistrés.
static void rappel_bouton_cb(void *args)
{
    dev_bouton_t *cible;
    for (cible = s_tete_handle; cible; cible = cible->suivant)
    {
        knob_handler(cible);
    }
}

// Alloue et configure un encodeur rotatif (GPIOs), l'ajoute à la liste et démarre le timer si besoin.
handle_encodeur_t creer_encodeur(const config_bouton_t *config)
{
    KNOB_CHECK(NULL != config, "config pointer can't be NULL!", NULL)
    KNOB_CHECK(config->gpio_encodeur_a != config->gpio_encodeur_b, "encoder A can't be the same as encoder B", NULL);

    dev_bouton_t *knob = (dev_bouton_t *)calloc(1, sizeof(dev_bouton_t));
    KNOB_CHECK(NULL != knob, "alloc knob failed", NULL);

    esp_err_t ret = ESP_OK;
    ret = init_gpio_encodeur(config->gpio_encodeur_a);
    KNOB_CHECK(ESP_OK == ret, "encoder A gpio init failed", NULL);
    ret = init_gpio_encodeur(config->gpio_encodeur_b);
    KNOB_CHECK_GOTO(ESP_OK == ret, "encoder B gpio init failed", _encoder_deinit);

    knob->hal_niveau_bouton = obtenir_niveau_gpio_encodeur;
    knob->encodeur_a = (void *)(long)config->gpio_encodeur_a;
    knob->encodeur_b = (void *)(long)config->gpio_encodeur_b;

    knob->niveau_encodeur_a = knob->hal_niveau_bouton(knob->encodeur_a);
    knob->niveau_encodeur_b = knob->hal_niveau_bouton(knob->encodeur_b);

    knob->evenement = BOUTON_AUCUN;

    knob->suivant = s_tete_handle;
    s_tete_handle = knob;

    if (!s_knob_timer_handle)
    {
        esp_timer_create_args_t knob_timer = {0};
        knob_timer.arg = NULL;
        knob_timer.callback = rappel_bouton_cb;
        knob_timer.dispatch_method = ESP_TIMER_TASK;
        knob_timer.name = "knob_timer";
        esp_timer_create(&knob_timer, &s_knob_timer_handle);
    }

    if (!s_timer_en_cours)
    {
        esp_timer_start_periodic(s_knob_timer_handle, TICKS_INTERVAL * 1000U);
        s_timer_en_cours = true;
    }

    ESP_LOGI(TAG, "Iot Knob Config Succeed, encoder A:%d, encoder B:%d", config->gpio_encodeur_a, config->gpio_encodeur_b);
    return (handle_encodeur_t)knob;

_encoder_deinit:
    deinit_gpio_encodeur(config->gpio_encodeur_b);
    deinit_gpio_encodeur(config->gpio_encodeur_a);
    return NULL;
}

// Libère les ressources (GPIOs), retire l'encodeur de la liste et arrête le timer si c'est le dernier.
esp_err_t supprimer_encodeur(handle_encodeur_t handle_encodeur)
{
    esp_err_t ret = ESP_OK;
    KNOB_CHECK(NULL != handle_encodeur, "Pointer of handle is invalid", ESP_ERR_INVALID_ARG);
    dev_bouton_t *knob = (dev_bouton_t *)handle_encodeur;
    ret = deinit_gpio_encodeur((int)(knob->donnees_usr));
    KNOB_CHECK(ESP_OK == ret, "knob deinit failed", ESP_FAIL);
    dev_bouton_t **courant;
    for (courant = &s_tete_handle; *courant;)
    {
        dev_bouton_t *entree = *courant;
        if (entree == knob)
        {
            *courant = entree->suivant;
            free(entree);
        }
        else
        {
            courant = &entree->suivant;
        }
    }

    uint16_t nombre = 0;
    dev_bouton_t *cible = s_tete_handle;
    while (cible)
    {
        cible = cible->suivant;
        nombre++;
    }
    ESP_LOGD(TAG, "remain knob number=%d", nombre);

    if (0 == nombre && s_timer_en_cours)
    {
        esp_timer_stop(s_knob_timer_handle);
        esp_timer_delete(s_knob_timer_handle);
        s_timer_en_cours = false;
    }

    return ESP_OK;
}

// Associe une fonction de rappel (callback) utilisateur à un événement (ex: BOUTON_GAUCHE, BOUTON_DROITE).
esp_err_t enregistrer_rappel_encodeur(handle_encodeur_t handle_encodeur, evenement_bouton_t evenement, rappel_encodeur_t rappel, void *donnees_usr)
{
    KNOB_CHECK(NULL != handle_encodeur, "Pointer of handle is invalid", ESP_ERR_INVALID_ARG);
    KNOB_CHECK(evenement < BOUTON_EVENEMENT_MAX, "event is invalid", ESP_ERR_INVALID_ARG);
    dev_bouton_t *knob = (dev_bouton_t *)handle_encodeur;
    knob->rappel[evenement] = rappel;
    knob->donnees_usr[evenement] = donnees_usr;
    return ESP_OK;
}

// Désenregistre une fonction de rappel pour un événement spécifique.
esp_err_t desenregistrer_rappel_encodeur(handle_encodeur_t handle_encodeur, evenement_bouton_t evenement)
{
    KNOB_CHECK(NULL != handle_encodeur, "Pointer of handle is invalid", ESP_ERR_INVALID_ARG);
    KNOB_CHECK(evenement < BOUTON_EVENEMENT_MAX, "event is invalid", ESP_ERR_INVALID_ARG);
    dev_bouton_t *knob = (dev_bouton_t *)handle_encodeur;
    knob->rappel[evenement] = NULL;
    knob->donnees_usr[evenement] = NULL;
    return ESP_OK;
}

// Récupère le dernier événement généré par l'encodeur rotatif.
evenement_bouton_t obtenir_evenement_encodeur(handle_encodeur_t handle_encodeur)
{
    KNOB_CHECK(NULL != handle_encodeur, "Pointer of handle is invalid", ESP_ERR_INVALID_ARG);
    dev_bouton_t *knob = (dev_bouton_t *)handle_encodeur;
    return knob->evenement;
}

// Récupère la valeur actuelle du compteur de l'encodeur rotatif.
int obtenir_valeur_compteur_encodeur(handle_encodeur_t handle_encodeur)
{
    KNOB_CHECK(NULL != handle_encodeur, "Pointer of handle is invalid", ESP_ERR_INVALID_ARG);
    dev_bouton_t *knob = (dev_bouton_t *)handle_encodeur;
    return knob->valeur_compteur;
}

// Réinitialise la valeur du compteur de l'encodeur rotatif à zéro.
esp_err_t effacer_valeur_compteur_encodeur(handle_encodeur_t handle_encodeur)
{
    KNOB_CHECK(NULL != handle_encodeur, "Pointer of handle is invalid", ESP_ERR_INVALID_ARG);
    dev_bouton_t *knob = (dev_bouton_t *)handle_encodeur;
    knob->valeur_compteur = 0;
    return ESP_OK;
}

// Relance manuellement le timer de scrutation des encodeurs.
esp_err_t reprendre_encodeur(void)
{
    KNOB_CHECK(s_knob_timer_handle, "knob timer handle is invalid", ESP_ERR_INVALID_STATE);
    KNOB_CHECK(!s_timer_en_cours, "knob timer is already running", ESP_ERR_INVALID_STATE);

    esp_err_t err = esp_timer_start_periodic(s_knob_timer_handle, TICKS_INTERVAL * 1000U);
    KNOB_CHECK(ESP_OK == err, "knob timer start failed", ESP_FAIL);
    s_timer_en_cours = true;
    return ESP_OK;
}

// Arrête manuellement le timer de scrutation des encodeurs.
esp_err_t arreter_encodeur(void)
{
    KNOB_CHECK(s_knob_timer_handle, "knob timer handle is invalid", ESP_ERR_INVALID_STATE);
    KNOB_CHECK(s_timer_en_cours, "knob timer is not running", ESP_ERR_INVALID_STATE);

    esp_err_t err = esp_timer_stop(s_knob_timer_handle);
    KNOB_CHECK(ESP_OK == err, "knob timer stop failed", ESP_FAIL);
    s_timer_en_cours = false;
    return ESP_OK;
}

// Initialise une broche GPIO en entrée avec résistance de tirage (pull-up).
esp_err_t init_gpio_encodeur(uint32_t gpio_num)
{
    gpio_config_t gpio_cfg = {
        .pin_bit_mask = (1ULL << gpio_num),
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_up_en = 1,
    };
    esp_err_t ret = gpio_config(&gpio_cfg);

    return ret;
}

// Désinitialise une broche GPIO (réinitialise à l'état par défaut).
esp_err_t deinit_gpio_encodeur(uint32_t gpio_num)
{
    return gpio_reset_pin(gpio_num);
}

// Lit l'état logique matériel d'une broche GPIO.
uint8_t obtenir_niveau_gpio_encodeur(void *gpio_num)
{
    return (uint8_t)gpio_get_level((uint32_t)gpio_num);
}
