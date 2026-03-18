/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
*/
#include <stdlib.h>
#include <sys/cdefs.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_commands.h"
#include "esp_log.h"

#include "esp_lcd_sh8601.h"

#define LCD_OPCODE_WRITE_CMD        (0x02ULL)
#define LCD_OPCODE_READ_CMD         (0x03ULL)
#define LCD_OPCODE_WRITE_COLOR      (0x32ULL)

static const char *TAG = "sh8601";

static esp_err_t panel_sh8601_del(esp_lcd_panel_t *panel);
static esp_err_t panel_sh8601_reset(esp_lcd_panel_t *panel);
static esp_err_t panel_sh8601_init(esp_lcd_panel_t *panel);
static esp_err_t panel_sh8601_draw_bitmap(esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end, const void *color_data);
static esp_err_t panel_sh8601_invert_color(esp_lcd_panel_t *panel, bool invert_color_data);
static esp_err_t panel_sh8601_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y);
static esp_err_t panel_sh8601_swap_xy(esp_lcd_panel_t *panel, bool swap_axes);
static esp_err_t panel_sh8601_set_gap(esp_lcd_panel_t *panel, int x_gap, int y_gap);
static esp_err_t panel_sh8601_disp_on_off(esp_lcd_panel_t *panel, bool off);

typedef struct {
    esp_lcd_panel_t base;
    esp_lcd_panel_io_handle_t io;
    int num_gpio_reinit;
    int ecart_x;
    int ecart_y;
    uint8_t fb_bits_per_pixel;
    uint8_t madctl_val; // enregistre la valeur actuelle du registre LCD_CMD_MADCTL
    uint8_t colmod_val; // enregistre la valeur actuelle du registre LCD_CMD_COLMOD
    const commande_init_lcd_sh8601_t *commandes_init;
    uint16_t taille_commandes_init;
    struct {
        unsigned int utiliser_interface_qspi: 1;
        unsigned int niveau_reinit: 1;
    } drapeaux;
} panneau_sh8601_t;

// Initialise un nouveau panneau SH8601 et configure l'ordre des couleurs, l'interface SPI/QSPI et les callbacks.
esp_err_t nouveau_panneau_sh8601_esp_lcd(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *panel_dev_config, esp_lcd_panel_handle_t *ret_panel)
{
    ESP_RETURN_ON_FALSE(io && panel_dev_config && ret_panel, ESP_ERR_INVALID_ARG, TAG, "invalid argument");

    esp_err_t ret = ESP_OK;
    panneau_sh8601_t *sh8601 = NULL;
    sh8601 = calloc(1, sizeof(panneau_sh8601_t));
    ESP_GOTO_ON_FALSE(sh8601, ESP_ERR_NO_MEM, err, TAG, "no mem for sh8601 panel");

    if (panel_dev_config->reset_gpio_num >= 0) {
        gpio_config_t io_conf = {
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = 1ULL << panel_dev_config->reset_gpio_num,
        };
        ESP_GOTO_ON_ERROR(gpio_config(&io_conf), err, TAG, "configure GPIO for RST line failed");
    }

    switch (panel_dev_config->rgb_ele_order) {
    case LCD_RGB_ELEMENT_ORDER_RGB:
        sh8601->madctl_val = 0;
        break;
    case LCD_RGB_ELEMENT_ORDER_BGR:
        sh8601->madctl_val |= LCD_CMD_BGR_BIT;
        break;
    default:
        ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, TAG, "unsupported color element order");
        break;
    }

    uint8_t fb_bits_per_pixel = 0;
    switch (panel_dev_config->bits_per_pixel) {
    case 16: // RGB565
        sh8601->colmod_val = 0x55;
        fb_bits_per_pixel = 16;
        break;
    case 18: // RGB666
        sh8601->colmod_val = 0x66;
        // Chaque composante de couleur (R/V/B) doit occuper les 6 bits de poids fort d'un octet, nécessitant 3 octets complets pour un pixel
        fb_bits_per_pixel = 18;
        break;
    case 24: // RGB888
        sh8601->colmod_val = 0x77;
        fb_bits_per_pixel = 24;
        break;
    default:
        ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, TAG, "unsupported pixel width");
        break;
    }

    sh8601->io = io;
    sh8601->num_gpio_reinit = panel_dev_config->reset_gpio_num;
    sh8601->fb_bits_per_pixel = fb_bits_per_pixel;
    config_fournisseur_sh8601_t *vendor_config = (config_fournisseur_sh8601_t *)panel_dev_config->vendor_config;
    if (vendor_config) {
        sh8601->commandes_init = vendor_config->commandes_init;
        sh8601->taille_commandes_init = vendor_config->taille_commandes_init;
        sh8601->drapeaux.utiliser_interface_qspi = vendor_config->drapeaux.utiliser_interface_qspi;
    }
    sh8601->drapeaux.niveau_reinit = panel_dev_config->flags.reset_active_high;
    sh8601->base.del = panel_sh8601_del;
    sh8601->base.reset = panel_sh8601_reset;
    sh8601->base.init = panel_sh8601_init;
    sh8601->base.draw_bitmap = panel_sh8601_draw_bitmap;
    sh8601->base.invert_color = panel_sh8601_invert_color;
    sh8601->base.set_gap = panel_sh8601_set_gap;
    sh8601->base.mirror = panel_sh8601_mirror;
    sh8601->base.swap_xy = panel_sh8601_swap_xy;
    sh8601->base.disp_on_off = panel_sh8601_disp_on_off;
    *ret_panel = &(sh8601->base);
    ESP_LOGD(TAG, "new sh8601 panel @%p", sh8601);

    //ESP_LOGI(TAG, "LCD panel create success, version: %d.%d.%d", ESP_LCD_SH8601_VER_MAJOR, ESP_LCD_SH8601_VER_MINOR,
             //ESP_LCD_SH8601_VER_PATCH);

    return ESP_OK;

err:
    if (sh8601) {
        if (panel_dev_config->reset_gpio_num >= 0) {
            gpio_reset_pin(panel_dev_config->reset_gpio_num);
        }
        free(sh8601);
    }
    return ret;
}

// Transmet une commande et ses paramètres vers le panneau LCD via l'interface d'E/S.
static esp_err_t tx_param(panneau_sh8601_t *sh8601, esp_lcd_panel_io_handle_t io, int lcd_cmd, const void *param, size_t param_size)
{
    if (sh8601->drapeaux.utiliser_interface_qspi) {
        lcd_cmd &= 0xff;
        lcd_cmd <<= 8;
        lcd_cmd |= LCD_OPCODE_WRITE_CMD << 24;
    }
    return esp_lcd_panel_io_tx_param(io, lcd_cmd, param, param_size);
}

// Transmet les données de couleur (pixels) vers le panneau LCD via l'interface d'E/S.
static esp_err_t tx_color(panneau_sh8601_t *sh8601, esp_lcd_panel_io_handle_t io, int lcd_cmd, const void *param, size_t param_size)
{
    if (sh8601->drapeaux.utiliser_interface_qspi) {
        lcd_cmd &= 0xff;
        lcd_cmd <<= 8;
        lcd_cmd |= LCD_OPCODE_WRITE_COLOR << 24;
    }
    return esp_lcd_panel_io_tx_color(io, lcd_cmd, param, param_size);
}

// Supprime le panneau SH8601 instancié et libère la mémoire allouée.
static esp_err_t panel_sh8601_del(esp_lcd_panel_t *panel)
{
    panneau_sh8601_t *sh8601 = __containerof(panel, panneau_sh8601_t, base);

    if (sh8601->num_gpio_reinit >= 0) {
        gpio_reset_pin(sh8601->num_gpio_reinit);
    }
    ESP_LOGD(TAG, "del sh8601 panel @%p", sh8601);
    free(sh8601);
    return ESP_OK;
}

// Effectue une réinitialisation matérielle (RST PIN) ou logicielle (commande SWRESET) du panneau LCD.
static esp_err_t panel_sh8601_reset(esp_lcd_panel_t *panel)
{
    panneau_sh8601_t *sh8601 = __containerof(panel, panneau_sh8601_t, base);
    esp_lcd_panel_io_handle_t io = sh8601->io;

    // Effectue une réinitialisation matérielle
    if (sh8601->num_gpio_reinit >= 0) {
        gpio_set_level(sh8601->num_gpio_reinit, sh8601->drapeaux.niveau_reinit);
        vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level(sh8601->num_gpio_reinit, !sh8601->drapeaux.niveau_reinit);
        vTaskDelay(pdMS_TO_TICKS(150));
    } else { // Effectue une réinitialisation logicielle
        ESP_RETURN_ON_ERROR(tx_param(sh8601, io, LCD_CMD_SWRESET, NULL, 0), TAG, "send command failed");
        vTaskDelay(pdMS_TO_TICKS(80));
    }

    return ESP_OK;
}

static const commande_init_lcd_sh8601_t vendor_specific_init_default[] = {
//  {commande, { donnees }, octets_donnees, delay_ms}
    {0x44, (uint8_t []){0x01, 0xD1}, 2, 0},
    {0x35, (uint8_t []){0x00}, 0, 0},
    {0x53, (uint8_t []){0x20}, 1, 25},
};

// Envoie la séquence d'initialisation spécifique au panneau SH8601.
static esp_err_t panel_sh8601_init(esp_lcd_panel_t *panel)
{
    panneau_sh8601_t *sh8601 = __containerof(panel, panneau_sh8601_t, base);
    esp_lcd_panel_io_handle_t io = sh8601->io;
    const commande_init_lcd_sh8601_t *init_cmds = NULL;
    uint16_t init_cmds_size = 0;
    bool is_cmd_overwritten = false;

    ESP_RETURN_ON_ERROR(tx_param(sh8601, io, LCD_CMD_MADCTL, (uint8_t[]) {
        sh8601->madctl_val,
    }, 1), TAG, "send command failed");
    ESP_RETURN_ON_ERROR(tx_param(sh8601, io, LCD_CMD_COLMOD, (uint8_t[]) {
        sh8601->colmod_val,
    }, 1), TAG, "send command failed");

    // Initialisation spécifique au fournisseur, peut différer selon les fabricants
    // Consulter le fournisseur de l'écran LCD pour le code de séquence d'initialisation
    if (sh8601->commandes_init) {
        init_cmds = sh8601->commandes_init;
        init_cmds_size = sh8601->taille_commandes_init;
    } else {
        init_cmds = vendor_specific_init_default;
        init_cmds_size = sizeof(vendor_specific_init_default) / sizeof(commande_init_lcd_sh8601_t);
    }

    for (int i = 0; i < init_cmds_size; i++) {
        // Vérifie si la commande a été utilisée ou est en conflit avec les internes
        switch (init_cmds[i].commande) {
        case LCD_CMD_MADCTL:
            is_cmd_overwritten = true;
            sh8601->madctl_val = ((uint8_t *)init_cmds[i].donnees)[0];
            break;
        case LCD_CMD_COLMOD:
            is_cmd_overwritten = true;
            sh8601->colmod_val = ((uint8_t *)init_cmds[i].donnees)[0];
            break;
        default:
            is_cmd_overwritten = false;
            break;
        }

        if (is_cmd_overwritten) {
            ESP_LOGW(TAG, "The %02Xh command has been used and will be overwritten by external initialization sequence", init_cmds[i].commande);
        }

        ESP_RETURN_ON_ERROR(tx_param(sh8601, io, init_cmds[i].commande, init_cmds[i].donnees, init_cmds[i].octets_donnees), TAG,
                            "send command failed");
        vTaskDelay(pdMS_TO_TICKS(init_cmds[i].delai_ms));
    }
    ESP_LOGD(TAG, "send init commands success");

    return ESP_OK;
}

// Transfère un bloc de pixels (bitmap) dans la fenêtre de mémoire d'affichage CASET/RASET du panneau.
static esp_err_t panel_sh8601_draw_bitmap(esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end, const void *color_data)
{
    panneau_sh8601_t *sh8601 = __containerof(panel, panneau_sh8601_t, base);
    assert((x_start < x_end) && (y_start < y_end) && "start position must be smaller than end position");
    esp_lcd_panel_io_handle_t io = sh8601->io;

    x_start += sh8601->ecart_x ;
    x_end += sh8601->ecart_x ;
    y_start += sh8601->ecart_y;
    y_end += sh8601->ecart_y;

    // Définit une zone de la mémoire de trame à laquelle le MCU peut accéder
    ESP_RETURN_ON_ERROR(tx_param(sh8601, io, LCD_CMD_CASET, (uint8_t[]) {
        (x_start >> 8) & 0xFF,
        x_start & 0xFF,
        ((x_end - 1) >> 8) & 0xFF,
        (x_end - 1) & 0xFF,
    }, 4), TAG, "send command failed");
    ESP_RETURN_ON_ERROR(tx_param(sh8601, io, LCD_CMD_RASET, (uint8_t[]) {
        (y_start >> 8) & 0xFF,
        y_start & 0xFF,
        ((y_end - 1) >> 8) & 0xFF,
        (y_end - 1) & 0xFF,
    }, 4), TAG, "send command failed");
    // Transfère le tampon de trame
    size_t len = (x_end - x_start) * (y_end - y_start) * sh8601->fb_bits_per_pixel / 8;
    tx_color(sh8601, io, LCD_CMD_RAMWR, color_data, len);

    return ESP_OK;
}

// Active ou désactive l'inversion des couleurs sur le panneau LCD.
static esp_err_t panel_sh8601_invert_color(esp_lcd_panel_t *panel, bool invert_color_data)
{
    panneau_sh8601_t *sh8601 = __containerof(panel, panneau_sh8601_t, base);
    esp_lcd_panel_io_handle_t io = sh8601->io;
    int command = 0;
    if (invert_color_data) {
        command = LCD_CMD_INVON;
    } else {
        command = LCD_CMD_INVOFF;
    }
    ESP_RETURN_ON_ERROR(tx_param(sh8601, io, command, NULL, 0), TAG, "send command failed");
    return ESP_OK;
}

// Configure le miroir horizontal (ou vertical si supporté) de l'affichage via le registre MADCTL.
static esp_err_t panel_sh8601_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y)
{
    panneau_sh8601_t *sh8601 = __containerof(panel, panneau_sh8601_t, base);
    esp_lcd_panel_io_handle_t io = sh8601->io;
    esp_err_t ret = ESP_OK;

    if (mirror_x) {
        sh8601->madctl_val |= BIT(6);
    } else {
        sh8601->madctl_val &= ~BIT(6);
    }
    if (mirror_y) {
        ESP_LOGE(TAG, "mirror_y is not supported by this panel");
        ret = ESP_ERR_NOT_SUPPORTED;
    }
    ESP_RETURN_ON_ERROR(tx_param(sh8601, io, LCD_CMD_MADCTL, (uint8_t[]) {
        sh8601->madctl_val
    }, 1), TAG, "send command failed");
    return ret;
}

// Échange les axes x et y (non supporté sur ce panneau).
static esp_err_t panel_sh8601_swap_xy(esp_lcd_panel_t *panel, bool swap_axes)
{
    ESP_LOGE(TAG, "swap_xy is not supported by this panel");
    return ESP_ERR_NOT_SUPPORTED;
}

// Définit un décalage (gap) pour les coordonnées x et y de la zone d'affichage.
static esp_err_t panel_sh8601_set_gap(esp_lcd_panel_t *panel, int ecart_x, int ecart_y)
{
    panneau_sh8601_t *sh8601 = __containerof(panel, panneau_sh8601_t, base);
    sh8601->ecart_x = ecart_x;
    sh8601->ecart_y = ecart_y;
    return ESP_OK;
}

// Active ou désactive l'affichage du panneau LCD.
static esp_err_t panel_sh8601_disp_on_off(esp_lcd_panel_t *panel, bool on_off)
{
    panneau_sh8601_t *sh8601 = __containerof(panel, panneau_sh8601_t, base);
    esp_lcd_panel_io_handle_t io = sh8601->io;
    int command = 0;

    if (on_off) {
        command = LCD_CMD_DISPON;
    } else {
        command = LCD_CMD_DISPOFF;
    }
    ESP_RETURN_ON_ERROR(tx_param(sh8601, io, command, NULL, 0), TAG, "send command failed");
    return ESP_OK;
}
