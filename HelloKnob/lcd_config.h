#ifndef LCD_CONFIG_H
#define LCD_CONFIG_H

#define ECRAN_RES_LARG              360
#define ECRAN_RES_HAUT              360

#define LCD_BIT_PER_PIXEL              16

#define EXAMPLE_PIN_NUM_LCD_CS      14
#define EXAMPLE_PIN_NUM_LCD_PCLK    13
#define EXAMPLE_PIN_NUM_LCD_DATA0   15
#define EXAMPLE_PIN_NUM_LCD_DATA1   16
#define EXAMPLE_PIN_NUM_LCD_DATA2   17
#define EXAMPLE_PIN_NUM_LCD_DATA3   18
#define EXAMPLE_PIN_NUM_LCD_RST     21
#define BROCHE_RETROECLAIRAGE    47

#define EXAMPLE_LVGL_BUF_HEIGHT        (ECRAN_RES_HAUT / 10)
#define EXAMPLE_LVGL_TICK_PERIOD_MS    2                          // Période d'horloge LVGL (ms)
#define EXAMPLE_LVGL_TASK_MAX_DELAY_MS 500                        // Délai max d'attente d'une tâche LVGL (ms)
#define EXAMPLE_LVGL_TASK_MIN_DELAY_MS 1                          // Délai min d'exécution d'une tâche LVGL (ms)
#define EXAMPLE_LVGL_TASK_STACK_SIZE   (4 * 1024)                 // Taille de pile allouée à LVGL (octets)
#define EXAMPLE_LVGL_TASK_PRIORITY     2                          // Priorité de la tâche LVGL

#define EXAMPLE_TOUCH_ADDR                0x15
#define EXAMPLE_PIN_NUM_TOUCH_SCL 12
#define EXAMPLE_PIN_NUM_TOUCH_SDA 11


//#define Backlight_Testing
//#define EXAMPLE_Rotate_90
#endif