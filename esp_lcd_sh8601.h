/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
  */

#pragma once

#include <stdint.h>

#include "esp_lcd_panel_vendor.h"

#ifdef __cplusplus
extern "C" {
#endif

// Commande d'initialisation du panneau LCD.
typedef struct {
    int commande;                // Commande LCD spécifique
    const void *donnees;       // Tampon de données spécifique à la commande
    size_t octets_donnees;      // Taille des données en octets
    unsigned int delai_ms;  // Délai en millisecondes après la commande
} commande_init_lcd_sh8601_t;

// Configuration spécifique (interface et commandes d'initialisation) passée à `vendor_config`.
typedef struct {
    const commande_init_lcd_sh8601_t *commandes_init;    // Pointeur vers le tableau de commandes d'initialisation
    uint16_t taille_commandes_init;                   // Nombre de commandes
    struct {
        unsigned int utiliser_interface_qspi: 1;    // 1 pour QSPI, 0 pour SPI
    } drapeaux;
} config_fournisseur_sh8601_t;

// Crée et initialise un nouveau panneau LCD pour le modèle SH8601.
esp_err_t nouveau_panneau_sh8601_esp_lcd(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *panel_dev_config, esp_lcd_panel_handle_t *ret_panel);

// Macros de configuration du bus SPI et QSPI pour le panneau LCD.
#define SH8601_PANEL_BUS_SPI_CONFIG(sclk, mosi, max_trans_sz)   \
    {                                                           \
        .sclk_io_num = sclk,                                    \
        .mosi_io_num = mosi,                                    \
        .miso_io_num = -1,                                      \
        .quadhd_io_num = -1,                                    \
        .quadwp_io_num = -1,                                    \
        .max_transfer_sz = max_trans_sz,                        \
    }
#define SH8601_PANEL_BUS_QSPI_CONFIG(sclk, d0, d1, d2, d3, max_trans_sz) \
    {                                                           \
        .data0_io_num = d0,                                     \
        .data1_io_num = d1,                                     \
        .sclk_io_num = sclk,                                    \
        .data2_io_num = d2,                                     \
        .data3_io_num = d3,                                     \
        .max_transfer_sz = max_trans_sz,                        \
    }

// Macros de configuration IO SPI et IO QSPI pour le panneau LCD.
#define SH8601_PANEL_IO_SPI_CONFIG(cs, dc, cb, cb_ctx)          \
    {                                                           \
        .cs_gpio_num = cs,                                      \
        .dc_gpio_num = dc,                                      \
        .spi_mode = 0,                                          \
        .pclk_hz = 40 * 1000 * 1000,                            \
        .trans_queue_depth = 10,                                \
        .on_color_trans_done = cb,                              \
        .user_ctx = cb_ctx,                                     \
        .lcd_cmd_bits = 8,                                      \
        .lcd_param_bits = 8,                                    \
    }
#define SH8601_PANEL_IO_QSPI_CONFIG(cs, cb, cb_ctx)             \
    {                                                           \
        .cs_gpio_num = cs,                                      \
        .dc_gpio_num = -1,                                      \
        .spi_mode = 0,                                          \
        .pclk_hz = 40 * 1000 * 1000,                            \
        .trans_queue_depth = 10,                                \
        .on_color_trans_done = cb,                              \
        .user_ctx = cb_ctx,                                     \
        .lcd_cmd_bits = 32,                                     \
        .lcd_param_bits = 8,                                    \
        .flags = {                                              \
            .quad_mode = true,                                  \
        },                                                      \
    }

#ifdef __cplusplus
}
#endif
