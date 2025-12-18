#ifndef UI_LINKY_H
#define UI_LINKY_H

#include "lvgl.h"
#include "mock_data.h"

#ifdef __cplusplus
extern "C" {
#endif

// Initialisation de l'UI
void ui_linky_init();

// Appelé quand le knob tourne (+1 ou -1)
void ui_linky_change_page(int direction);

// Appelé périodiquement pour mettre à jour les valeurs
void ui_linky_update(linky_data_t *data);

#ifdef __cplusplus
}
#endif

#endif
