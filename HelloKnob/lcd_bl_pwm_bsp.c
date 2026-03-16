#include <stdio.h>
#include "lcd_bl_pwm_bsp.h"
#include "esp_err.h"
#include "driver/ledc.h"
#include "driver/gpio.h"

#include "lcd_config.h"
// Initialise le timer et le canal PWM (LEDC) pour le rétroéclairage de l'écran.
void initialisation_pwm_lcd_bsp(uint16_t duty)
{ 
  ledc_timer_config_t config_minuteur = 
  {
    .speed_mode =  LEDC_LOW_SPEED_MODE,
    .duty_resolution = LEDC_TIMER_8_BIT, //256
    .timer_num =  LEDC_TIMER_3,
    .freq_hz = 50 * 1000,
    .clk_cfg = LEDC_SLOW_CLK_RC_FAST,
  };
  ledc_channel_config_t config_ledc = 
  {
    .gpio_num = BROCHE_RETROECLAIRAGE,
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .channel =  LEDC_CHANNEL_1,
    .intr_type =  LEDC_INTR_DISABLE,
    .timer_sel = LEDC_TIMER_3,
    .duty = duty,
    .hpoint = 0,
  };
  ESP_ERROR_CHECK_WITHOUT_ABORT(ledc_timer_config(&config_minuteur));
  ESP_ERROR_CHECK_WITHOUT_ABORT(ledc_channel_config(&config_ledc));
}

// Configure et applique le rapport cyclique PWM du rétroéclairage.
static void configurer_duty(uint16_t duty)
{
  ESP_ERROR_CHECK_WITHOUT_ABORT(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, duty));
  ESP_ERROR_CHECK_WITHOUT_ABORT(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1));
}
// Wrapper public pour ajuster la luminosité (rapport cyclique) du rétroéclairage.
void configurer_luminosite(uint16_t duty)
{
  configurer_duty(duty);
}


