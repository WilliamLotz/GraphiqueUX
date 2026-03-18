#ifndef CST816_H
#define CST816_H
#include "driver/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif 

// Initialise le contrôleur tactile I2C CST816.
void initialisation_tactile(void);

// Récupère les coordonnées (x,y) du dernier point de contact s'il existe (retourne 1 si détecté).
uint8_t obtenir_tactile(uint16_t *x,uint16_t *y);

#ifdef __cplusplus
}
#endif
#endif