#include "cst816.h"
#include "esp_err.h"
#include "lcd_config.h"

#define PORT_I2C_TEST I2C_NUM_0

// Écrit un buffer de données sur le bus I2C vers le registre spécifié.
uint8_t I2C_ecrire_tampon(uint8_t addr,uint8_t reg,uint8_t *buf,uint8_t len)
{
  uint8_t ret;
  uint8_t *ptampon = (uint8_t*)malloc(len+1);
  ptampon[0] = reg;
  for(uint8_t i = 0; i<len; i++)
  {
    ptampon[i+1] = buf[i];
  }
  ret = i2c_master_write_to_device(PORT_I2C_TEST,addr,ptampon,len+1,1000);
  free(ptampon);
  ptampon = NULL;
  return ret;
}
// Lit un buffer de données depuis le bus I2C à partir du registre spécifié.
uint8_t I2C_lire_tampon(uint8_t addr,uint8_t reg,uint8_t *buf,uint8_t len)
{
  uint8_t ret;
  ret = i2c_master_write_read_device(PORT_I2C_TEST,addr,&reg,1,buf,len,1000);
  return ret;
}
// Effectue une séquence d'écriture puis de lecture sur le bus I2C.
uint8_t I2C_maitre_ecrire_lire_appareil(uint8_t addr,uint8_t *writeBuf,uint8_t writeLen,uint8_t *readBuf,uint8_t readLen)
{
  uint8_t ret;
  ret = i2c_master_write_read_device(PORT_I2C_TEST,addr,writeBuf,writeLen,readBuf,readLen,1000);
  return ret;
}
// Initialise la communication I2C et configure le mode normal du contrôleur tactile CST816.
void initialisation_tactile(void)
{
  i2c_config_t conf = 
  {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = EXAMPLE_PIN_NUM_TOUCH_SDA,
    .scl_io_num = EXAMPLE_PIN_NUM_TOUCH_SCL,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master = {.clk_speed = 300 * 1000,},
    .clk_flags = 0,
  };
  ESP_ERROR_CHECK(i2c_param_config(PORT_I2C_TEST, &conf));
  ESP_ERROR_CHECK(i2c_driver_install(PORT_I2C_TEST, conf.mode,0,0,0));

  uint8_t donnees = 0x00;
  I2C_ecrire_tampon(EXAMPLE_TOUCH_ADDR,0x00,&donnees,1); // Passer en mode normal
}
// Lit les données tactiles I2C et retourne 1 si un point est détecté, en renseignant x et y.
uint8_t obtenir_tactile(uint16_t *x,uint16_t *y)
{
  uint8_t obtenir_num = 0;
  uint8_t donnees[7] = {0};
  I2C_lire_tampon(EXAMPLE_TOUCH_ADDR,0x00,donnees,7);
  obtenir_num = donnees[2];
  if(obtenir_num)
  {
    *x = ((uint16_t)(donnees[3] & 0x0f)<<8) + (uint16_t)donnees[4];
    *y = ((uint16_t)(donnees[5] & 0x0f)<<8) + (uint16_t)donnees[6];
    return 1;
  }
  return 0;
}


