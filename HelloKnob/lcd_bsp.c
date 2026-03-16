#include "lcd_bsp.h"
#include "lcd_config.h"

// Vérifie si la compilation est pour S3
#if defined(CONFIG_IDF_TARGET_ESP32S3)

#include "esp_lcd_sh8601.h"
#include "cst816.h"
static SemaphoreHandle_t lvgl_mux = NULL; //sémaphores mutex
#define LCD_HOST    SPI2_HOST

#define SH8601_ID 0x86
#define CO5300_ID 0xff


static esp_lcd_panel_io_handle_t amoled_panel_io_handle = NULL; 

static const sh8601_lcd_init_cmd_t lcd_init_cmds[] = 
{
  {0xF0, (uint8_t[]){0x28}, 1, 0},
  {0xF2, (uint8_t[]){0x28}, 1, 0},
  {0x73, (uint8_t[]){0xF0}, 1, 0},
  {0x7C, (uint8_t[]){0xD1}, 1, 0},
  {0x83, (uint8_t[]){0xE0}, 1, 0},
  {0x84, (uint8_t[]){0x61}, 1, 0},
  {0xF2, (uint8_t[]){0x82}, 1, 0},
  {0xF0, (uint8_t[]){0x00}, 1, 0},
  {0xF0, (uint8_t[]){0x01}, 1, 0},
  {0xF1, (uint8_t[]){0x01}, 1, 0},
  {0xB0, (uint8_t[]){0x56}, 1, 0},
  {0xB1, (uint8_t[]){0x4D}, 1, 0},
  {0xB2, (uint8_t[]){0x24}, 1, 0},
  {0xB4, (uint8_t[]){0x87}, 1, 0},
  {0xB5, (uint8_t[]){0x44}, 1, 0},
  {0xB6, (uint8_t[]){0x8B}, 1, 0},
  {0xB7, (uint8_t[]){0x40}, 1, 0},
  {0xB8, (uint8_t[]){0x86}, 1, 0},
  {0xBA, (uint8_t[]){0x00}, 1, 0},
  {0xBB, (uint8_t[]){0x08}, 1, 0},
  {0xBC, (uint8_t[]){0x08}, 1, 0},
  {0xBD, (uint8_t[]){0x00}, 1, 0},
  {0xC0, (uint8_t[]){0x80}, 1, 0},
  {0xC1, (uint8_t[]){0x10}, 1, 0},
  {0xC2, (uint8_t[]){0x37}, 1, 0},
  {0xC3, (uint8_t[]){0x80}, 1, 0},
  {0xC4, (uint8_t[]){0x10}, 1, 0},
  {0xC5, (uint8_t[]){0x37}, 1, 0},
  {0xC6, (uint8_t[]){0xA9}, 1, 0},
  {0xC7, (uint8_t[]){0x41}, 1, 0},
  {0xC8, (uint8_t[]){0x01}, 1, 0},
  {0xC9, (uint8_t[]){0xA9}, 1, 0},
  {0xCA, (uint8_t[]){0x41}, 1, 0},
  {0xCB, (uint8_t[]){0x01}, 1, 0},
  {0xD0, (uint8_t[]){0x91}, 1, 0},
  {0xD1, (uint8_t[]){0x68}, 1, 0},
  {0xD2, (uint8_t[]){0x68}, 1, 0},
  {0xF5, (uint8_t[]){0x00, 0xA5}, 2, 0},
  {0xDD, (uint8_t[]){0x4F}, 1, 0},
  {0xDE, (uint8_t[]){0x4F}, 1, 0},
  {0xF1, (uint8_t[]){0x10}, 1, 0},
  {0xF0, (uint8_t[]){0x00}, 1, 0},
  {0xF0, (uint8_t[]){0x02}, 1, 0},
  {0xE0, (uint8_t[]){0xF0, 0x0A, 0x10, 0x09, 0x09, 0x36, 0x35, 0x33, 0x4A, 0x29, 0x15, 0x15, 0x2E, 0x34}, 14, 0},
  {0xE1, (uint8_t[]){0xF0, 0x0A, 0x0F, 0x08, 0x08, 0x05, 0x34, 0x33, 0x4A, 0x39, 0x15, 0x15, 0x2D, 0x33}, 14, 0},
  {0xF0, (uint8_t[]){0x10}, 1, 0},
  {0xF3, (uint8_t[]){0x10}, 1, 0},
  {0xE0, (uint8_t[]){0x07}, 1, 0},
  {0xE1, (uint8_t[]){0x00}, 1, 0},
  {0xE2, (uint8_t[]){0x00}, 1, 0},
  {0xE3, (uint8_t[]){0x00}, 1, 0},
  {0xE4, (uint8_t[]){0xE0}, 1, 0},
  {0xE5, (uint8_t[]){0x06}, 1, 0},
  {0xE6, (uint8_t[]){0x21}, 1, 0},
  {0xE7, (uint8_t[]){0x01}, 1, 0},
  {0xE8, (uint8_t[]){0x05}, 1, 0},
  {0xE9, (uint8_t[]){0x02}, 1, 0},
  {0xEA, (uint8_t[]){0xDA}, 1, 0},
  {0xEB, (uint8_t[]){0x00}, 1, 0},
  {0xEC, (uint8_t[]){0x00}, 1, 0},
  {0xED, (uint8_t[]){0x0F}, 1, 0},
  {0xEE, (uint8_t[]){0x00}, 1, 0},
  {0xEF, (uint8_t[]){0x00}, 1, 0},
  {0xF8, (uint8_t[]){0x00}, 1, 0},
  {0xF9, (uint8_t[]){0x00}, 1, 0},
  {0xFA, (uint8_t[]){0x00}, 1, 0},
  {0xFB, (uint8_t[]){0x00}, 1, 0},
  {0xFC, (uint8_t[]){0x00}, 1, 0},
  {0xFD, (uint8_t[]){0x00}, 1, 0},
  {0xFE, (uint8_t[]){0x00}, 1, 0},
  {0xFF, (uint8_t[]){0x00}, 1, 0},
  {0x60, (uint8_t[]){0x40}, 1, 0},
  {0x61, (uint8_t[]){0x04}, 1, 0},
  {0x62, (uint8_t[]){0x00}, 1, 0},
  {0x63, (uint8_t[]){0x42}, 1, 0},
  {0x64, (uint8_t[]){0xD9}, 1, 0},
  {0x65, (uint8_t[]){0x00}, 1, 0},
  {0x66, (uint8_t[]){0x00}, 1, 0},
  {0x67, (uint8_t[]){0x00}, 1, 0},
  {0x68, (uint8_t[]){0x00}, 1, 0},
  {0x69, (uint8_t[]){0x00}, 1, 0},
  {0x6A, (uint8_t[]){0x00}, 1, 0},
  {0x6B, (uint8_t[]){0x00}, 1, 0},
  {0x70, (uint8_t[]){0x40}, 1, 0},
  {0x71, (uint8_t[]){0x03}, 1, 0},
  {0x72, (uint8_t[]){0x00}, 1, 0},
  {0x73, (uint8_t[]){0x42}, 1, 0},
  {0x74, (uint8_t[]){0xD8}, 1, 0},
  {0x75, (uint8_t[]){0x00}, 1, 0},
  {0x76, (uint8_t[]){0x00}, 1, 0},
  {0x77, (uint8_t[]){0x00}, 1, 0},
  {0x78, (uint8_t[]){0x00}, 1, 0},
  {0x79, (uint8_t[]){0x00}, 1, 0},
  {0x7A, (uint8_t[]){0x00}, 1, 0},
  {0x7B, (uint8_t[]){0x00}, 1, 0},
  {0x80, (uint8_t[]){0x48}, 1, 0},
  {0x81, (uint8_t[]){0x00}, 1, 0},
  {0x82, (uint8_t[]){0x06}, 1, 0},
  {0x83, (uint8_t[]){0x02}, 1, 0},
  {0x84, (uint8_t[]){0xD6}, 1, 0},
  {0x85, (uint8_t[]){0x04}, 1, 0},
  {0x86, (uint8_t[]){0x00}, 1, 0},
  {0x87, (uint8_t[]){0x00}, 1, 0},
  {0x88, (uint8_t[]){0x48}, 1, 0},
  {0x89, (uint8_t[]){0x00}, 1, 0},
  {0x8A, (uint8_t[]){0x08}, 1, 0},
  {0x8B, (uint8_t[]){0x02}, 1, 0},
  {0x8C, (uint8_t[]){0xD8}, 1, 0},
  {0x8D, (uint8_t[]){0x04}, 1, 0},
  {0x8E, (uint8_t[]){0x00}, 1, 0},
  {0x8F, (uint8_t[]){0x00}, 1, 0},
  {0x90, (uint8_t[]){0x48}, 1, 0},
  {0x91, (uint8_t[]){0x00}, 1, 0},
  {0x92, (uint8_t[]){0x0A}, 1, 0},
  {0x93, (uint8_t[]){0x02}, 1, 0},
  {0x94, (uint8_t[]){0xDA}, 1, 0},
  {0x95, (uint8_t[]){0x04}, 1, 0},
  {0x96, (uint8_t[]){0x00}, 1, 0},
  {0x97, (uint8_t[]){0x00}, 1, 0},
  {0x98, (uint8_t[]){0x48}, 1, 0},
  {0x99, (uint8_t[]){0x00}, 1, 0},
  {0x9A, (uint8_t[]){0x0C}, 1, 0},
  {0x9B, (uint8_t[]){0x02}, 1, 0},
  {0x9C, (uint8_t[]){0xDC}, 1, 0},
  {0x9D, (uint8_t[]){0x04}, 1, 0},
  {0x9E, (uint8_t[]){0x00}, 1, 0},
  {0x9F, (uint8_t[]){0x00}, 1, 0},
  {0xA0, (uint8_t[]){0x48}, 1, 0},
  {0xA1, (uint8_t[]){0x00}, 1, 0},
  {0xA2, (uint8_t[]){0x05}, 1, 0},
  {0xA3, (uint8_t[]){0x02}, 1, 0},
  {0xA4, (uint8_t[]){0xD5}, 1, 0},
  {0xA5, (uint8_t[]){0x04}, 1, 0},
  {0xA6, (uint8_t[]){0x00}, 1, 0},
  {0xA7, (uint8_t[]){0x00}, 1, 0},
  {0xA8, (uint8_t[]){0x48}, 1, 0},
  {0xA9, (uint8_t[]){0x00}, 1, 0},
  {0xAA, (uint8_t[]){0x07}, 1, 0},
  {0xAB, (uint8_t[]){0x02}, 1, 0},
  {0xAC, (uint8_t[]){0xD7}, 1, 0},
  {0xAD, (uint8_t[]){0x04}, 1, 0},
  {0xAE, (uint8_t[]){0x00}, 1, 0},
  {0xAF, (uint8_t[]){0x00}, 1, 0},
  {0xB0, (uint8_t[]){0x48}, 1, 0},
  {0xB1, (uint8_t[]){0x00}, 1, 0},
  {0xB2, (uint8_t[]){0x09}, 1, 0},
  {0xB3, (uint8_t[]){0x02}, 1, 0},
  {0xB4, (uint8_t[]){0xD9}, 1, 0},
  {0xB5, (uint8_t[]){0x04}, 1, 0},
  {0xB6, (uint8_t[]){0x00}, 1, 0},
  {0xB7, (uint8_t[]){0x00}, 1, 0},
  {0xB8, (uint8_t[]){0x48}, 1, 0},
  {0xB9, (uint8_t[]){0x00}, 1, 0},
  {0xBA, (uint8_t[]){0x0B}, 1, 0},
  {0xBB, (uint8_t[]){0x02}, 1, 0},
  {0xBC, (uint8_t[]){0xDB}, 1, 0},
  {0xBD, (uint8_t[]){0x04}, 1, 0},
  {0xBE, (uint8_t[]){0x00}, 1, 0},
  {0xBF, (uint8_t[]){0x00}, 1, 0},
  {0xC0, (uint8_t[]){0x10}, 1, 0},
  {0xC1, (uint8_t[]){0x47}, 1, 0},
  {0xC2, (uint8_t[]){0x56}, 1, 0},
  {0xC3, (uint8_t[]){0x65}, 1, 0},
  {0xC4, (uint8_t[]){0x74}, 1, 0},
  {0xC5, (uint8_t[]){0x88}, 1, 0},
  {0xC6, (uint8_t[]){0x99}, 1, 0},
  {0xC7, (uint8_t[]){0x01}, 1, 0},
  {0xC8, (uint8_t[]){0xBB}, 1, 0},
  {0xC9, (uint8_t[]){0xAA}, 1, 0},
  {0xD0, (uint8_t[]){0x10}, 1, 0},
  {0xD1, (uint8_t[]){0x47}, 1, 0},
  {0xD2, (uint8_t[]){0x56}, 1, 0},
  {0xD3, (uint8_t[]){0x65}, 1, 0},
  {0xD4, (uint8_t[]){0x74}, 1, 0},
  {0xD5, (uint8_t[]){0x88}, 1, 0},
  {0xD6, (uint8_t[]){0x99}, 1, 0},
  {0xD7, (uint8_t[]){0x01}, 1, 0},
  {0xD8, (uint8_t[]){0xBB}, 1, 0},
  {0xD9, (uint8_t[]){0xAA}, 1, 0},
  {0xF3, (uint8_t[]){0x01}, 1, 0},
  {0xF0, (uint8_t[]){0x00}, 1, 0},
  {0x21, (uint8_t[]){0x00}, 1, 0},
  {0x11, (uint8_t[]){0x00}, 1, 120},
  {0x29, (uint8_t[]){0x00}, 1, 0},
  {0x36, (uint8_t[]){0x08}, 1, 0}, // MADCTL: Définir le bit BGR (0x08) pour corriger l'inversion Rouge/Bleu
};

static bool verrouiller_lvgl(int timeout_ms);
static void deverrouiller_lvgl(void);
static void incrementer_tick_lvgl(void *arg);
static bool notifier_lvgl_flush_pret(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx);
static void rappel_flush_lvgl(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map);
void rappel_arrondi_lvgl(struct _lv_disp_drv_t *disp_drv, lv_area_t *area);
static void rappel_tactile_lvgl(lv_indev_drv_t *drv, lv_indev_data_t *data);

// Initialise le bus d'affichage, LVGL et le gestionnaire des entrées tactiles en DMA.
void lcd_lvgl_Init(void)
{
  static lv_disp_draw_buf_t tampon_disp; // Contient le(s) tampon(s) graphique(s) interne(s) appelé(s) tampon(s) de dessin
  static lv_disp_drv_t pilote_disp;      // Contient les fonctions de rappel

  const spi_bus_config_t config_bus = SH8601_PANEL_BUS_QSPI_CONFIG(EXAMPLE_PIN_NUM_LCD_PCLK,
                                                               EXAMPLE_PIN_NUM_LCD_DATA0,
                                                               EXAMPLE_PIN_NUM_LCD_DATA1,
                                                               EXAMPLE_PIN_NUM_LCD_DATA2,
                                                               EXAMPLE_PIN_NUM_LCD_DATA3,
                                                               ECRAN_RES_LARG * ECRAN_RES_HAUT * LCD_BIT_PER_PIXEL / 8);
  ESP_ERROR_CHECK_WITHOUT_ABORT(spi_bus_initialize(LCD_HOST, &config_bus, SPI_DMA_CH_AUTO));
  esp_lcd_panel_io_handle_t gestionnaire_es = NULL;

  const esp_lcd_panel_io_spi_config_t config_es = SH8601_PANEL_IO_QSPI_CONFIG(EXAMPLE_PIN_NUM_LCD_CS,
                                                                              example_notify_lvgl_flush_ready,
                                                                              &pilote_disp);

  sh8601_vendor_config_t config_fournisseur = 
  {
    .init_cmds = lcd_init_cmds,
    .init_cmds_size = sizeof(lcd_init_cmds) / sizeof(lcd_init_cmds[0]),
    .flags = 
    {
      .use_qspi_interface = 1,
    },
  };
  ESP_ERROR_CHECK_WITHOUT_ABORT(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &config_es, &gestionnaire_es));
  amoled_panel_io_handle = gestionnaire_es;
  esp_lcd_panel_handle_t gestionnaire_panneau = NULL;
  const esp_lcd_panel_dev_config_t config_panneau = 
  {
    .reset_gpio_num = EXAMPLE_PIN_NUM_LCD_RST,
    .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB, // Essai RGB pour corriger Swap R-B
    .bits_per_pixel = LCD_BIT_PER_PIXEL,
    .vendor_config = &config_fournisseur,
  };
  ESP_ERROR_CHECK_WITHOUT_ABORT(nouveau_panneau_sh8601_esp_lcd(gestionnaire_es, &config_panneau, &gestionnaire_panneau));
  ESP_ERROR_CHECK_WITHOUT_ABORT(esp_lcd_panel_reset(gestionnaire_panneau));
  ESP_ERROR_CHECK_WITHOUT_ABORT(esp_lcd_panel_init(gestionnaire_panneau));
  //ESP_ERROR_CHECK_WITHOUT_ABORT(esp_lcd_panel_disp_on_off(gestionnaire_panneau, true));

  lv_init();
  lv_color_t *buf1 = heap_caps_malloc(ECRAN_RES_LARG * EXAMPLE_LVGL_BUF_HEIGHT * sizeof(lv_color_t), MALLOC_CAP_DMA);
  assert(buf1);
  lv_color_t *buf2 = heap_caps_malloc(ECRAN_RES_LARG * EXAMPLE_LVGL_BUF_HEIGHT * sizeof(lv_color_t), MALLOC_CAP_DMA);
  assert(buf2);
  lv_disp_draw_buf_init(&tampon_disp, buf1, buf2, ECRAN_RES_LARG * EXAMPLE_LVGL_BUF_HEIGHT);
  lv_disp_drv_init(&pilote_disp);
  pilote_disp.hor_res = ECRAN_RES_LARG;
  pilote_disp.ver_res = ECRAN_RES_HAUT;
  pilote_disp.flush_cb = rappel_flush_lvgl;
  pilote_disp.rounder_cb = rappel_arrondi_lvgl;
  pilote_disp.draw_buf = &tampon_disp;
  pilote_disp.user_data = gestionnaire_panneau;
  lv_disp_t *disp = lv_disp_drv_register(&pilote_disp);

  static lv_indev_drv_t pilote_indev;    // Pilote de périphérique d'entrée (Tactile)
  lv_indev_drv_init(&pilote_indev);
  pilote_indev.type = LV_INDEV_TYPE_POINTER;
  pilote_indev.disp = disp;
  pilote_indev.read_cb = rappel_tactile_lvgl;
  lv_indev_drv_register(&pilote_indev);

  const esp_timer_create_args_t arguments_minuterie_tick = 
  {
    .callback = &incrementer_tick_lvgl,
    .name = "lvgl_tick"
  };
  esp_timer_handle_t minuterie_tick_lvgl = NULL;
  ESP_ERROR_CHECK(esp_timer_create(&arguments_minuterie_tick, &minuterie_tick_lvgl));
  ESP_ERROR_CHECK(esp_timer_start_periodic(minuterie_tick_lvgl, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000));

  lvgl_mux = xSemaphoreCreateMutex(); //sémaphores mutex
  assert(lvgl_mux);
  
  if (example_lvgl_lock(-1)) 
  {   
    // ui_init();     /* Un exemple de widgets */
    // Libère le mutex
    example_lvgl_unlock();
  }
}

// Bloque et acquiert le mutex LVGL avec un timeout pour modifier l'interface en sécurité.
static bool verrouiller_lvgl(int timeout_ms)
{
  assert(lvgl_mux && "bsp_display_start must be called first");

  const TickType_t timeout_ticks = (timeout_ms == -1) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
  return xSemaphoreTake(lvgl_mux, timeout_ticks) == pdTRUE;
}

// Libère le mutex LVGL après avoir manipulé des widgets (FreeRTOS).
static void deverrouiller_lvgl(void)
{
  assert(lvgl_mux && "bsp_display_start must be called first");
  xSemaphoreGive(lvgl_mux);
}

// Incrémente le compteur de temps interne de LVGL (tick).
static void incrementer_tick_lvgl(void *arg)
{
  lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}
// Callback interne notifiant LVGL qu'un transfert d'image (flush) est prêt.
static bool notifier_lvgl_flush_pret(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
  lv_disp_drv_t *pilote_disp = (lv_disp_drv_t *)user_ctx;
  lv_disp_flush_ready(pilote_disp);
  return false;
}
// Callback LVGL appelé pour envoyer les pixels dans la zone désignée de l'écran.
static void rappel_flush_lvgl(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
  esp_lcd_panel_handle_t gestionnaire_panneau = (esp_lcd_panel_handle_t) drv->user_data;
  const int offsetx1 = area->x1;
  const int offsetx2 = area->x2;
  const int offsety1 = area->y1;
  const int offsety2 = area->y2;

  esp_lcd_panel_draw_bitmap(gestionnaire_panneau, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);
}
// Callback LVGL appelé pour ajuster/arrondir les coordonnées selon l'exigence matérielle (pair).
void rappel_arrondi_lvgl(struct _lv_disp_drv_t *disp_drv, lv_area_t *area)
{
  uint16_t x1 = area->x1;
  uint16_t x2 = area->x2;

  uint16_t y1 = area->y1;
  uint16_t y2 = area->y2;

  // Arrondit le début de la coordonnée à l'inferieur vers le multiple de 2 le plus proche
  area->x1 = (x1 >> 1) << 1;
  area->y1 = (y1 >> 1) << 1;
  // Arrondit la fin de la coordonnée au supérieur vers le nombre impair (2N+1) le plus proche
  area->x2 = ((x2 >> 1) << 1) + 1;
  area->y2 = ((y2 >> 1) << 1) + 1;
}
// Callback LVGL pour lire la position du doigt sur l'écran tactile.
static void rappel_tactile_lvgl(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
  uint16_t tp_x,tp_y;
  uint8_t win = obtenir_tactile(&tp_x,&tp_y);
  if (win)
  {
    #ifdef EXAMPLE_Rotate_90
      data->point.x = tp_y;
      data->point.y = (ECRAN_RES_HAUT - tp_x);
    #else
      data->point.x = tp_x;
      data->point.y = tp_y;
    #endif
    if(data->point.x > ECRAN_RES_LARG)
    data->point.x = ECRAN_RES_LARG;
    if(data->point.y > ECRAN_RES_HAUT)
    data->point.y = ECRAN_RES_HAUT;
    data->state = LV_INDEV_STATE_PRESSED;
    //ESP_LOGE("TP","(%d,%d)",data->point.x,data->point.y);
  }
  else
  {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}
#else
// ------------------------------------
// PILOTE FICTIF POUR ESP32 STANDARD (Non-S3)
// ------------------------------------
#include "esp_log.h"
#include "driver/gpio.h"

// Initialisation minimale LVGL pour traiter la logique/les rappels mais sans sortie écran
static void dummy_flush_cb(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_p) {
    lv_disp_flush_ready(drv);
}

void lcd_lvgl_Init(void)
{
    printf("LCD: Mock Init for Standard ESP32 (No driver)\n");
    lv_init();
    
    // Crée un petit tampon
    static lv_disp_draw_buf_t disp_buf;
    static lv_color_t buf[240 * 10]; 
    lv_disp_draw_buf_init(&disp_buf, buf, NULL, 240 * 10);
    
    // Enregistre le pilote fictif
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf = &disp_buf;
    disp_drv.hor_res = 240;
    disp_drv.ver_res = 240;
    disp_drv.flush_cb = dummy_flush_cb;
    lv_disp_drv_register(&disp_drv);
}

// Initialisation tactile fictive - RETIRÉ pour éviter les conflits avec cst816.cpp
// void initialisation_tactile(){
//    printf("Touch: Mock Init\n");
// }
#endif
