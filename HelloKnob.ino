#include "lcd_bsp.h"
#include "cst816.h"
#include "lcd_bl_pwm_bsp.h"
#include "lvgl.h"
#include "bidi_switch_knob.h" // Knob Driver

#include "mock_data.h"
#include "ui_linky.h"

// Configuration
#define BACKLIGHT_BRIGHTNESS 150 
#define EXAMPLE_ENCODER_ECA_PIN    8
#define EXAMPLE_ENCODER_ECB_PIN    7

// Knob Handle
static knob_handle_t s_knob = NULL;

// Linky Data Global Instance
linky_data_t linky_data = {
    .papp = 0,
    .iinst = 0,
    .index_base = 12345,
    .index_hp = 5600,
    .index_hc = 4200,
    .isousc = 45,
    .option_tarif = "BASE",
    .mot_etat = "PAS D'ETAT",
    .voltage = 230
};

// Mock Update Function
void update_mock_data() {
    // Variation aléatoire de la puissance
    int delta = (rand() % 401) - 200; // -200 to 200
    int new_papp = (int)linky_data.papp + delta;
    
    // Bornes 0 - 9000 VA
    if (new_papp < 0) new_papp = 100;
    if (new_papp > 9000) new_papp = 9000;
    
    linky_data.papp = (uint16_t)new_papp;
    
    // Calcul IINST
    linky_data.iinst = linky_data.papp / 230;
    if(linky_data.iinst == 0 && linky_data.papp > 0) linky_data.iinst = 1;

    // Simulation tension 220-240V
    linky_data.voltage = 225 + (rand() % 18);
    
    // Incrémentation lente de l'index
    if ((rand() % 100) < 5) { // 5% de chance
        linky_data.index_base++;
    }
}

// Callbacks for Knob
static void _knob_left_cb(void *arg, void *data)
{
    // Important: LVGL input should ideally be handled in the main loop or thread safe
    // But for simplicity in this Arduino sketch, we just signal a flag or call directly if simple
    // Since we are running single threaded in loop(), we can use a flag.
    ui_linky_change_page(-1);
    Serial.println("Knob Left");
}

static void _knob_right_cb(void *arg, void *data)
{
    ui_linky_change_page(1);
    Serial.println("Knob Right");
}


void setup() {
  Serial.begin(115200);
  delay(1000); 
  Serial.println("Starting LinkyMock...");

  // 1. Init Drivers
  Touch_Init();
  lcd_lvgl_Init();
  lcd_bl_pwm_bsp_init(BACKLIGHT_BRIGHTNESS);

  // 2. Init Knob
  knob_config_t cfg = {
    .gpio_encoder_a = EXAMPLE_ENCODER_ECA_PIN,
    .gpio_encoder_b = EXAMPLE_ENCODER_ECB_PIN,
  };
  s_knob = iot_knob_create(&cfg);
  iot_knob_register_cb(s_knob, KNOB_LEFT, _knob_left_cb, NULL);
  iot_knob_register_cb(s_knob, KNOB_RIGHT, _knob_right_cb, NULL);
  
  // 3. Init UI
  ui_linky_init();
  
  Serial.println("Setup done.");
}

void loop() {
  static uint32_t last_update = 0;
  
  // LVGL Task
  lv_timer_handler();
  
  // Mock Data Update every 1s
  if (millis() - last_update > 1000) {
      update_mock_data();
      ui_linky_update(&linky_data);
      last_update = millis();
  }
  
  delay(5);
}
