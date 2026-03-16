#ifndef LCD_BSP_H
#define LCD_BSP_H
#include "Arduino.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_commands.h"
#include "lvgl.h"
// #include "demos/lv_demos.h"
#include "esp_check.h"
#include "driver/gpio.h"
// #include "ui.h"
#ifdef __cplusplus
extern "C" {
#endif 

// Callback interne notifiant LVGL qu'un transfert d'image (flush) est prêt.
static bool notifier_lvgl_flush_pret(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx);

// Callback LVGL appelé pour envoyer les pixels dans la zone désignée de l'écran.
static void rappel_flush_lvgl(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map);

// Callback LVGL appelé pour ajuster/arrondir les coordonnées selon l'exigence matérielle.
void rappel_arrondi_lvgl(struct _lv_disp_drv_t *disp_drv, lv_area_t *area);

// Incrémente le compteur de temps interne de LVGL (tick).
static void incrementer_tick_lvgl(void *arg);

// Tâche système (non utilisée globalement ici) dédiée au portabilité de LVGL.
static void tache_port_lvgl(void *arg);

// Libère le mutex LVGL après avoir manipulé des widgets (FreeRTOS).
static void deverrouiller_lvgl(void);

// Bloque et acquiert le mutex LVGL avec un timeout pour modifier l'interface en sécurité.
static bool verrouiller_lvgl(int timeout_ms);

// Initialise le bus d'affichage, LVGL et le gestionnaire des entrées tactiles.
void lcd_lvgl_Init(void);

// Callback LVGL pour lire la position du doigt sur l'écran tactile.
static void rappel_tactile_lvgl(lv_indev_drv_t *drv, lv_indev_data_t *data);
#ifdef __cplusplus
}
#endif

#endif