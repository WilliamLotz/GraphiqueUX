#include "ui_linky.h"
#include <math.h>
#include <time.h>
#include <stdio.h>

#ifndef PI
#define PI 3.14159265358979323846
#endif

// Variables Globales UI

static lv_obj_t *screen_meter;
static lv_obj_t *screen_index;
static lv_obj_t *screen_info;
static lv_obj_t *screen_history;  // NEW

// Variables globales pour le METER (Premium Design)
static lv_obj_t *label_time;
static lv_obj_t *label_date;
static lv_obj_t *label_papp_val; // "125.7"
static lv_obj_t *label_papp_unit; // "kW"

static lv_obj_t *meter_bars[6];
static lv_obj_t *meter_labels_val[6];
static lv_obj_t *meter_labels_time[6];
static int meter_values[6] = {0}; 
// Removed meter_lines, points.

static int current_page_index = 0;  // 0=Meter, 1=Index, 2=Info, 3=History

// Widgets updating
static lv_obj_t *label_index_base;
static lv_obj_t *label_index_hp;
static lv_obj_t *label_index_hc;
static lv_obj_t *label_volt;
static lv_obj_t *label_volt_stats; // NEW
static lv_obj_t *label_amp;
static lv_obj_t *label_amp_stats;  // NEW

// Stats Variables
static uint16_t v_min = 999, v_max = 0;
static uint32_t v_sum = 0, v_count = 0;
static uint16_t a_min = 999, a_max = 0;
static uint32_t a_sum = 0, a_count = 0;

static lv_style_t style_text_large;
static lv_style_t style_text_xl;

// --- Helper Helper ---
void style_init() {
  lv_style_init(&style_text_large);
  // lv_style_set_transform_zoom(&style_text_large, 384); // x1.5 (Not supported?)

  lv_style_init(&style_text_xl);
  // lv_style_set_transform_zoom(&style_text_xl, 512); // x2
}

// --- Screens ---

void create_screen_meter() {
  screen_meter = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screen_meter, lv_color_black(), 0); 
  
  // Get System Time for Init
  time_t now;
  time(&now);
  struct tm *t = localtime(&now);

  // 1. HEADER
  label_time = lv_label_create(screen_meter);
  lv_label_set_text_fmt(label_time, "%02d:%02d", t->tm_hour, t->tm_min);
  lv_obj_set_style_text_color(label_time, lv_color_white(), 0);
  lv_obj_set_style_text_font(label_time, &lv_font_montserrat_30, 0); // Font 30 Natif
  lv_obj_align(label_time, LV_ALIGN_TOP_MID, 0, 40);

  // Date
  label_date = lv_label_create(screen_meter);
  const char* week_days[] = {"DIMANCHE", "LUNDI", "MARDI", "MERCREDI", "JEUDI", "VENDREDI", "SAMEDI"};
  const char* months[] = {"JANVIER", "FEVRIER", "MARS", "AVRIL", "MAI", "JUIN", "JUILLET", "AOUT", "SEPTEMBRE", "OCTOBRE", "NOVEMBRE", "DECEMBRE"};
  lv_label_set_text_fmt(label_date, "%s %d %s\n%d", week_days[t->tm_wday], t->tm_mday, months[t->tm_mon], 1900 + t->tm_year);
  
  lv_obj_set_style_text_align(label_date, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_color(label_date, lv_color_white(), 0); 
  lv_obj_set_style_text_font(label_time, &lv_font_montserrat_20, 0);
  lv_obj_align_to(label_date, label_time, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

  // ... Box ...
  lv_obj_t *box = lv_obj_create(screen_meter);
  lv_obj_set_size(box, 260, 90);
  lv_obj_align(box, LV_ALIGN_TOP_MID, 0, 110); 
  lv_obj_set_style_bg_color(box, lv_color_black(), 0); // Noir pur 
  lv_obj_set_style_border_color(box, lv_color_make(0, 0, 255), 0); // Hack: Envoyer Bleu -> Obtenir Vert
  lv_obj_set_style_border_width(box, 1, 0);
  lv_obj_set_style_radius(box, 20, 0);
  lv_obj_clear_flag(box, LV_OBJ_FLAG_SCROLLABLE);

  // Value
  label_papp_val = lv_label_create(box);
  lv_label_set_text(label_papp_val, "0.4");
  lv_obj_set_style_text_font(label_papp_val, &lv_font_montserrat_40, 0); // Font 40 Natif
  lv_obj_set_style_text_color(label_papp_val, lv_color_white(), 0);
  lv_obj_align(label_papp_val, LV_ALIGN_CENTER, -10, -8); // Recentrer un peu (moins gauche)

  // Unit
  label_papp_unit = lv_label_create(box);
  lv_label_set_text(label_papp_unit, "KW");
  lv_obj_set_style_text_font(label_papp_unit, &lv_font_montserrat_20, 0); // Font 20 Natif
  lv_obj_set_style_text_color(label_papp_unit, lv_color_white(), 0);
  lv_obj_align_to(label_papp_unit, label_papp_val, LV_ALIGN_OUT_RIGHT_BOTTOM, 5, -5); // Baseline adjust (Up a bit)

  // Subtitle
  lv_obj_t *lbl_inst = lv_label_create(box);
  lv_label_set_text(lbl_inst, "INSTANT");
  lv_obj_set_style_text_color(lbl_inst, lv_color_white(), 0);
  lv_obj_set_style_text_font(lbl_inst, &lv_font_montserrat_16, 0); // Font 16 (Must receive compile)
  lv_obj_align(lbl_inst, LV_ALIGN_BOTTOM_MID, 0, 11);


  // 3. BARRES
  int bar_w = 28;
  int gap = 16;
  int start_x = -((6 * bar_w + 5 * gap) / 2) + (bar_w/2); 

  // Init Mock Values for Visualization
  int mock_vals[6] = {500, 1500, 3200, 800, 4500, 2100};
  for(int k=0; k<6; k++) meter_values[k] = mock_vals[k];

  for(int i=0; i<6; i++) {
      // Calc Height from Mock
      int val = meter_values[i];
      if(val > 9000) val = 9000;
      int h = (val * 100) / 9000;
      if(h < 5) h = 5;

      // Use proper Bar Widget
      meter_bars[i] = lv_bar_create(screen_meter);
      lv_bar_set_range(meter_bars[i], 0, 100);
      lv_bar_set_value(meter_bars[i], 100, LV_ANIM_OFF); // Always full, size varies
      
      lv_obj_set_size(meter_bars[i], bar_w, h); // Dynamic Height
      
      // Style Indicator (The actual bar)
      lv_obj_set_style_radius(meter_bars[i], 2, LV_PART_MAIN);      // Force Main Square
      lv_obj_set_style_radius(meter_bars[i], 2, LV_PART_INDICATOR); // Slight Rounding
      
      // Final Cyan (0,255,255) - No Artifacts with lv_bar!
      lv_obj_set_style_bg_color(meter_bars[i], lv_color_make(0, 255, 255), LV_PART_INDICATOR);
      lv_obj_set_style_bg_grad_color(meter_bars[i], lv_color_make(0, 255, 255), LV_PART_INDICATOR); // Same for safety
      lv_obj_set_style_bg_grad_dir(meter_bars[i], LV_GRAD_DIR_NONE, LV_PART_INDICATOR);
      
      // Hidden Background
      lv_obj_set_style_bg_opa(meter_bars[i], LV_OPA_TRANSP, LV_PART_MAIN);

      // Position X
      int x_pos = start_x + i * (bar_w + gap);
      
      // Position Y Curve Effect (Smile)
      int y_off = -15; // Remonter ENCORE les bords (60 et 10)
      if(i==1 || i==4) y_off = 10; 
      if(i==2 || i==3) y_off = 25; 

      lv_obj_align(meter_bars[i], LV_ALIGN_BOTTOM_MID, x_pos, -50 + y_off); 

      // Label Value (Au dessus)
      meter_labels_val[i] = lv_label_create(screen_meter);
      
      // Format text from Mock
      float kw_bar = val / 1000.0f;
      int i_b = (int)kw_bar; int d_b = (int)((kw_bar - i_b)*10);
      lv_label_set_text_fmt(meter_labels_val[i], "%d.%d", i_b, d_b);
      
      // Small Font via Zoom (14 -> ~10) -> Removed for sharpness
      // lv_obj_set_style_transform_zoom(meter_labels_val[i], 180, 0); 
      lv_obj_set_style_text_color(meter_labels_val[i], lv_color_white(), 0);
      lv_obj_align_to(meter_labels_val[i], meter_bars[i], LV_ALIGN_OUT_TOP_MID, 0, -5);

      // Label Time (Suit la barre)
      meter_labels_time[i] = lv_label_create(screen_meter);
      lv_label_set_text_fmt(meter_labels_time[i], "%d", (6-i)*10); 
      // lv_obj_set_style_transform_zoom(meter_labels_time[i], 180, 0); 
      lv_obj_set_style_text_color(meter_labels_time[i], lv_color_white(), 0); 
      
      lv_obj_align_to(meter_labels_time[i], meter_bars[i], LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
  }

}

void create_screen_index() {
  screen_index = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screen_index, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(screen_index, LV_OPA_COVER, 0); // Fond Noir 100% Opaque

  // Titre "Direct sur l'écran"
  lv_obj_t *title = lv_label_create(screen_index);
  lv_label_set_text(title, "INDEX kWh");
  lv_obj_set_style_text_color(title, lv_color_white(), 0); // Blanc Pur
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 30);
  lv_obj_set_style_text_letter_space(title, 2, 0); // Espacement pour look "Large"

  // Base
  label_index_base = lv_label_create(screen_index);
  lv_label_set_text(label_index_base, "BASE: -----");
  lv_obj_set_style_text_color(label_index_base, lv_color_white(), 0);
  lv_obj_align(label_index_base, LV_ALIGN_CENTER, 0, -40); 
  lv_obj_set_style_text_letter_space(label_index_base, 1, 0);

  // HP
  label_index_hp = lv_label_create(screen_index);
  lv_label_set_text(label_index_hp, "HP: -----");
  lv_obj_set_style_text_color(label_index_hp, lv_color_white(), 0);
  lv_obj_align(label_index_hp, LV_ALIGN_CENTER, 0, 0); 
  lv_obj_set_style_text_letter_space(label_index_hp, 1, 0);

  // HC
  label_index_hc = lv_label_create(screen_index);
  lv_label_set_text(label_index_hc, "HC: -----");
  lv_obj_set_style_text_color(label_index_hc, lv_color_white(), 0);
  lv_obj_align(label_index_hc, LV_ALIGN_CENTER, 0, 40); 
  lv_obj_set_style_text_letter_space(label_index_hc, 1, 0);
}

void create_screen_info() {
  screen_info = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screen_info, lv_color_black(), 0);
  
  // Conteneur Flex Centré (Plein Ecran)
  lv_obj_t * cont = lv_obj_create(screen_info);
  lv_obj_set_size(cont, 240, 240);
  lv_obj_center(cont);
  lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(cont, 0, 0);
  lv_obj_set_style_pad_row(cont, 10, 0); // Espacement entre éléments
  lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE); // SUPPRIMER BARRE DE DEFILEMENT

  // Titre
  lv_obj_t *title = lv_label_create(cont);
  lv_label_set_text(title, "INFOS");
  lv_obj_set_style_text_color(title, lv_color_white(), 0);
  lv_obj_add_style(title, &style_text_large, 0);
  // lv_obj_set_style_pad_bottom(title, 10, 0); 

  // --- TENSION ---
  lv_obj_t *lbl_v = lv_label_create(cont);
  lv_label_set_text(lbl_v, "TENSION");
  lv_obj_set_style_text_color(lbl_v, lv_color_white(), 0); // BLANC
  lv_obj_add_style(lbl_v, &style_text_large, 0); // Plus gros
  
  label_volt = lv_label_create(cont);
  lv_label_set_text(label_volt, "--- V");
  lv_obj_set_style_text_color(label_volt, lv_color_white(), 0);
  lv_obj_add_style(label_volt, &style_text_xl, 0); // TRES GROS

  label_volt_stats = lv_label_create(cont);
  lv_label_set_text(label_volt_stats, "Min:-      Avg:-      Max:-");
  lv_obj_set_style_text_color(label_volt_stats, lv_color_white(), 0); // BLANC
  lv_obj_set_style_text_font(label_volt_stats, LV_FONT_DEFAULT, 0);
  lv_obj_set_style_pad_bottom(label_volt_stats, 10, 0); // Séparation Groupes

  // --- COURANT ---
  lv_obj_t *lbl_a = lv_label_create(cont);
  lv_label_set_text(lbl_a, "INTENSITE");
  lv_obj_set_style_text_color(lbl_a, lv_color_white(), 0); // BLANC
  lv_obj_add_style(lbl_a, &style_text_large, 0); // Plus gros

  label_amp = lv_label_create(cont);
  lv_label_set_text(label_amp, "-- A");
  lv_obj_set_style_text_color(label_amp, lv_color_white(), 0);
  lv_obj_add_style(label_amp, &style_text_xl, 0); // TRES GROS

  label_amp_stats = lv_label_create(cont);
  lv_label_set_text(label_amp_stats, "Min:-      Avg:-      Max:-");
  lv_obj_set_style_text_color(label_amp_stats, lv_color_white(), 0); // BLANC
  lv_obj_set_style_text_font(label_amp_stats, LV_FONT_DEFAULT, 0);
}

// --- Variables Globales pour l'Historique Radial ---
static lv_point_t history_points[12][2]; // 12 lignes de 2 points
static lv_obj_t * history_lines[12];     // 12 objets lignes
static lv_obj_t * label_months[12];      // 12 labels (J, F, M...)
const char* month_letters[12] = {"J", "F", "M", "A", "M", "J", "J", "A", "S", "O", "N", "D"};

// Helper trigo simple (Angle en degrés 0..360, Retourne -32767..32767)
// Nous utilisons lv_trigo_sin / cos qui prend un angle (0..360)
// Mais attention, lv_trigo travaille souvent avec des entiers.
// Utilisons float standard pour la simplicité ici, l'ESP32 est puissant.
#include <math.h>
#ifndef PI
#define PI 3.14159265f
#endif

void create_screen_history() {
    screen_history = lv_obj_create(NULL);
    // Pas de padding sur l'écran racine
    lv_obj_set_style_pad_all(screen_history, 0, 0); 

    // 1. Conteneur Centré (La référence stable)
    lv_obj_t * radial_area = lv_obj_create(screen_history);
    lv_obj_set_size(radial_area, 360, 360); // On tente plus grand !
    lv_obj_center(radial_area);
    
    // --- STYLE FINAL (Transparent) ---
    lv_obj_set_style_bg_opa(radial_area, LV_OPA_TRANSP, 0); 
    lv_obj_set_style_border_width(radial_area, 0, 0);
    lv_obj_set_style_pad_all(radial_area, 0, 0); 
    lv_obj_clear_flag(radial_area, LV_OBJ_FLAG_SCROLLABLE);

    // Titre (Par dessus tout le monde)
    
    // Disque central BLANC pour contraste
    lv_obj_t * central_disc = lv_obj_create(screen_history);
    lv_obj_set_size(central_disc, 120, 120); // Taille du trou (60*2)
    lv_obj_center(central_disc);
    lv_obj_set_style_radius(central_disc, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(central_disc, lv_color_white(), 0);
    lv_obj_set_style_border_width(central_disc, 0, 0);
    lv_obj_clear_flag(central_disc, LV_OBJ_FLAG_SCROLLABLE);

    // Texte en NOIR (et gras via décalage)
    lv_obj_t * title_shadow = lv_label_create(screen_history);
    lv_label_set_text(title_shadow, "CONSO\nANNUELLE\nkWh");
    lv_obj_set_style_text_align(title_shadow, LV_TEXT_ALIGN_CENTER, 0); 
    lv_obj_align(title_shadow, LV_ALIGN_CENTER, 1, 1); 
    lv_obj_set_style_text_color(title_shadow, lv_color_black(), 0); // NOIR
    lv_obj_add_style(title_shadow, &style_text_large, 0);

    lv_obj_t * title = lv_label_create(screen_history);
    lv_label_set_text(title, "CONSO\nANNUELLE\nkWh");
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0); 
    lv_obj_align(title, LV_ALIGN_CENTER, 0, 0); 
    lv_obj_set_style_text_color(title, lv_color_black(), 0); // NOIR
    lv_obj_add_style(title, &style_text_large, 0);      
    // Pas de zoom pour netteté max
    static lv_style_t style_line;
    lv_style_init(&style_line);
    lv_style_set_line_width(&style_line, 28); // TRES Epais (Secteurs)
    lv_style_set_line_rounded(&style_line, true);

    // Centre relatif au conteneur (280x280) -> 140,140
    int cx = 180; 
    int cy = 180; 
    
    for(int i=0; i<12; i++) {
        // Angle
        float angle_deg = (i + 1) * 30; 
        float angle_rad = angle_deg * (PI / 180.0f);

        // Valeur
        int value = (rand() % 50) + 20; // Plus long
        
        // Dégradé Manuel "Hacké" pour écran Exotique
        // Observation : Envoi (R,0,0) -> Bleu. Envoi (0,G,0) -> Rouge.
        // Deduction : R->B, G->R, B->G (Rotation).
        // Solution :
        // Pour avoir ROUGE (Grand) : Envoyer dans le canal G.
        // Pour avoir VERT (Petit) : Envoyer dans le canal B.
        // Pour avoir JAUNE (R+G) : Envoyer dans G et B.
        
        // Calcul Gradient: Vert -> Rouge
        // BUG HARDWARE: R->B, G->R, B->G
        // On veut VISUEL Vert (Low) -> Rouge (High)
        // Donc on envoie BLEU (Low) -> VERT (High)
        
        uint8_t ratio = (value * 255) / 60; // 0..255
        if (ratio > 255) ratio = 255;
        
        // Low: Send Blue (0,0,255) -> Show Green
        // High: Send Green (0,255,0) -> Show Red
        // Mix: Send Cyan (0, 255, 255) -> Show Yellow
        
        uint8_t g_val_calc = ratio;
        uint8_t b_val_calc = 255 - ratio;
        
        // (R, G, B) sent to screen
        lv_color_t bar_color = lv_color_make(0, g_val_calc, b_val_calc);

        // Coordonnées (Offset par rapport au centre 140,140)
        int hole = 70; // Trou plus grand
        int x_start = cx + (int)(sin(angle_rad) * hole);
        int y_start = cy - (int)(cos(angle_rad) * hole);
        
        int x_end = cx + (int)(sin(angle_rad) * (hole + value));
        int y_end = cy - (int)(cos(angle_rad) * (hole + value));

        history_points[i][0].x = x_start;
        history_points[i][0].y = y_start;
        history_points[i][1].x = x_end;
        history_points[i][1].y = y_end;

        // Ligne -> Dans radial_area
        history_lines[i] = lv_line_create(radial_area);
        lv_line_set_points(history_lines[i], history_points[i], 2);
        lv_obj_add_style(history_lines[i], &style_line, 0);
        lv_obj_set_style_line_color(history_lines[i], bar_color, 0);
        
        // Label -> Dans radial_area
        int label_radius = 160; 
        int lx = cx + (int)(sin(angle_rad) * label_radius);
        int ly = cy - (int)(cos(angle_rad) * label_radius);

        // Surbrillance du Mois Actuel (Simulation : Janvier = Index 0)
        int current_month_idx = 0; 
        if (i == current_month_idx) {
             lv_obj_t * highlight = lv_obj_create(radial_area);
             lv_obj_set_size(highlight, 24, 24);
             lv_obj_set_style_radius(highlight, LV_RADIUS_CIRCLE, 0);
             // Si BGR : Bleu s'affiche Rouge. Si on veut Bleu, on envoie Rouge (FF0000).
             // Mais on a vu que RGB est R=>B. Donc pour Bleu on envoie Rouge.
             // Essayons Palette Blue standard, si c'est Rouge on changera.
             // Hack BGR confirmé : Pour avoir BLEU il faut envoyer ROUGE (0xFF0000).
             lv_obj_set_style_bg_color(highlight, lv_color_hex(0xFF0000), 0); 
             lv_obj_set_style_border_width(highlight, 0, 0);
             lv_obj_set_pos(highlight, lx - 12, ly - 13); // Centré sous la lettre
             lv_obj_clear_flag(highlight, LV_OBJ_FLAG_SCROLLABLE);
        }
        
        label_months[i] = lv_label_create(radial_area); 
        lv_label_set_text(label_months[i], month_letters[i]);
        lv_obj_set_pos(label_months[i], lx - 6, ly - 9); 

        // VALEUR (Petit Chiffre dans la barre)
        int val_radius = hole + 20; // Un peu après le début de la barre
        int vx = cx + (int)(sin(angle_rad) * val_radius);
        int vy = cy - (int)(cos(angle_rad) * val_radius);

        lv_obj_t * val_lbl = lv_label_create(radial_area);
        lv_label_set_text_fmt(val_lbl, "%d", value * 10); // x10 pour faire "kwh" réaliste
        lv_obj_set_style_text_color(val_lbl, lv_color_black(), 0); // Noir sur couleur vive
  lv_obj_set_pos(val_lbl, vx - 10, vy - 7); // Centrage approx -10,-7 
    }
}

// --- DEBUG SCREEN ---
void create_screen_debug() {
  lv_obj_t * screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
  
  // ROUGE Pur (Via Hack: Envoi Vert)
  lv_obj_t * rect_r = lv_obj_create(screen);
  lv_obj_set_size(rect_r, 50, 50);
  lv_obj_align(rect_r, LV_ALIGN_TOP_LEFT, 60, 60);
  lv_obj_set_style_bg_color(rect_r, lv_color_make(0, 255, 0), 0); 
  lv_obj_t * l_r = lv_label_create(rect_r);
  lv_label_set_text(l_r, "R");
  lv_obj_center(l_r);

  // VERT Pur (Via Hack: Envoi Bleu)
  lv_obj_t * rect_g = lv_obj_create(screen);
  lv_obj_set_size(rect_g, 50, 50);
  lv_obj_align(rect_g, LV_ALIGN_TOP_MID, 0, 60);
  lv_obj_set_style_bg_color(rect_g, lv_color_make(0, 0, 255), 0); 
  lv_obj_t * l_g = lv_label_create(rect_g);
  lv_label_set_text(l_g, "G");
  lv_obj_center(l_g);

  // BLEU Pur (Via Hack: Envoi Rouge)
  lv_obj_t * rect_b = lv_obj_create(screen);
  lv_obj_set_size(rect_b, 50, 50);
  lv_obj_align(rect_b, LV_ALIGN_TOP_RIGHT, -60, 60);
  lv_obj_set_style_bg_color(rect_b, lv_color_make(255, 0, 0), 0); 
  lv_obj_t * l_b = lv_label_create(rect_b);
  lv_label_set_text(l_b, "B");
  lv_obj_center(l_b);

  // Test Texte Blanc
  lv_obj_t * t3 = lv_label_create(screen);
  lv_label_set_text(t3, "BLANC PUR\n(Hack Local)");
  lv_obj_set_style_text_color(t3, lv_color_white(), 0);
  lv_obj_align(t3, LV_ALIGN_CENTER, 0, 60);

  lv_scr_load(screen);
}

// --- Variables Globales pour Semaine ---
static lv_obj_t *screen_week;
static lv_point_t week_points[7][2]; 
static lv_obj_t * week_lines[7];     
static lv_obj_t * label_days[7];      
const char* day_names[7] = {"L", "M", "M", "J", "V", "S", "D"};

void create_screen_week() {
    screen_week = lv_obj_create(NULL);
    lv_obj_set_style_pad_all(screen_week, 0, 0); 
    lv_obj_set_style_bg_color(screen_week, lv_color_black(), 0);

    // Conteneur 360x360
    lv_obj_t * radial_area = lv_obj_create(screen_week);
    lv_obj_set_size(radial_area, 360, 360);
    lv_obj_center(radial_area);
    lv_obj_set_style_bg_opa(radial_area, LV_OPA_TRANSP, 0); 
    lv_obj_set_style_border_width(radial_area, 0, 0);
    lv_obj_clear_flag(radial_area, LV_OBJ_FLAG_SCROLLABLE);

    // Disque Central (Blanc) - Position Absolue pour garantir alignement avec cx=170
    lv_obj_t * central_disc = lv_obj_create(screen_week);
    lv_obj_set_size(central_disc, 120, 120);
    // Centre désiré = 170. Rayon = 60. Pos = 110. (170-60)
    lv_obj_set_pos(central_disc, 120, 120); 
    lv_obj_set_style_radius(central_disc, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(central_disc, lv_color_white(), 0);
    lv_obj_set_style_border_width(central_disc, 0, 0);

    // Titre avec Ombre (Style History) - Décalé -10
    lv_obj_t * title_shadow = lv_label_create(screen_week);
    lv_label_set_text(title_shadow, "CONSO\nSEMAINE\nkWh");
    lv_obj_set_style_text_align(title_shadow, LV_TEXT_ALIGN_CENTER, 0); 
    lv_obj_align(title_shadow, LV_ALIGN_CENTER, -0, -0); 
    lv_obj_set_style_text_color(title_shadow, lv_color_black(), 0); 

    lv_obj_t * title = lv_label_create(screen_week);
    lv_label_set_text(title, "CONSO\nSEMAINE\nkWh");
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0); 
    lv_obj_align(title, LV_ALIGN_CENTER, -0, -0); 
    lv_obj_set_style_text_color(title, lv_color_black(), 0);

    // Styles
    static lv_style_t style_line_week;
    lv_style_init(&style_line_week);
    lv_style_set_line_width(&style_line_week, 30); 
    lv_style_set_line_rounded(&style_line_week, true);

    int cx = 165; 
    int cy = 165; 
    int radius_start = 70; 
    int max_len = 70;

    // 7 Jours
    for(int i=0; i<7; i++) {
        // Angle: 0 = Haut (-90 deg). Sens horaire.
        float angle_deg = -90.0f + (i * 360.0f / 7.0f);
        float angle_rad = angle_deg * (PI / 180.0f);
        
        // Simuler Données
        int value = rand() % 50 + 10; 

        // Couleur Hack
        uint8_t ratio = (value * 255) / 60; 
        if(ratio > 255) ratio = 255;
        lv_color_t color = lv_color_make(0, ratio, 255 - ratio);

        // Longueur
        int bar_len = (value * max_len) / 60;
        
        // Coordonnées
        week_points[i][0].x = cx + (int)(cos(angle_rad) * radius_start);
        week_points[i][0].y = cy + (int)(sin(angle_rad) * radius_start);
        week_points[i][1].x = cx + (int)(cos(angle_rad) * (radius_start + bar_len));
        week_points[i][1].y = cy + (int)(sin(angle_rad) * (radius_start + bar_len));

        // Create Line
        week_lines[i] = lv_line_create(radial_area);
        lv_line_set_points(week_lines[i], week_points[i], 2);
        lv_obj_add_style(week_lines[i], &style_line_week, 0);
        lv_obj_set_style_line_color(week_lines[i], color, 0);

        // Label Coords
        int label_radius = 160;
        int lx = cx + (int)(cos(angle_rad) * label_radius);
        int ly = cy + (int)(sin(angle_rad) * label_radius);

        // Highlight Current Day (e.g. Monday/Lun at i=0)
        if (i == 0) {
             lv_obj_t * highlight = lv_obj_create(radial_area);
             lv_obj_set_size(highlight, 24, 24);
             lv_obj_set_style_radius(highlight, LV_RADIUS_CIRCLE, 0);
             // Hack Color: Send RED (FF0000) to get BLUE? No, Send RED to get...
             // Debug Screen: Red(Send Vert), Green(Send Bleu), Blue(Send Rouge).
             // History uses 0xFF0000. Displayed as ??
             // If User liked History Color, I copy exact Hex 0xFF0000.
             lv_obj_set_style_bg_color(highlight, lv_color_hex(0xFF0000), 0); 
             lv_obj_set_style_border_width(highlight, 0, 0);
             lv_obj_set_pos(highlight, lx - 12, ly - 13);
             lv_obj_clear_flag(highlight, LV_OBJ_FLAG_SCROLLABLE);
        }

        label_days[i] = lv_label_create(radial_area); 
        lv_label_set_text(label_days[i], day_names[i]);
        lv_obj_set_style_text_color(label_days[i], lv_color_white(), 0);
        lv_obj_set_pos(label_days[i], lx - 7, ly - 7);
        
        // Valeur Textuelle
        lv_obj_t * val_lbl = lv_label_create(radial_area);
        lv_label_set_text_fmt(val_lbl, "%d", value * 10); 
        lv_obj_set_style_text_color(val_lbl, lv_color_black(), 0); 
        // Pos
        int val_radius = radius_start + 20; 
        int vx = cx + (int)(cos(angle_rad) * val_radius);
        int vy = cy + (int)(sin(angle_rad) * val_radius);
        lv_obj_set_pos(val_lbl, vx - 10, vy - 7);

    }
}

void ui_linky_init() {
  style_init();

  create_screen_meter();
  create_screen_index();
  create_screen_week();
  create_screen_history();
  create_screen_info();

  // create_screen_debug(); // Chargement direct du debug 
  lv_scr_load(screen_meter);
  current_page_index = 0;
}

void ui_linky_change_page(int direction) {
  printf("UI PAGE CHANGE CMD: %d\n", direction);
  current_page_index += direction;

  if (current_page_index > 4) current_page_index = 0;
  if (current_page_index < 0) current_page_index = 4;

  switch (current_page_index) {
    case 0: lv_scr_load_anim(screen_meter, LV_SCR_LOAD_ANIM_NONE, 0, 0, false); break;
    case 1: lv_scr_load_anim(screen_index, LV_SCR_LOAD_ANIM_NONE, 0, 0, false); break;
    case 2: lv_scr_load_anim(screen_week, LV_SCR_LOAD_ANIM_NONE, 0, 0, false); break;
    case 3: lv_scr_load_anim(screen_history, LV_SCR_LOAD_ANIM_NONE, 0, 0, false); break;
    case 4: lv_scr_load_anim(screen_info, LV_SCR_LOAD_ANIM_NONE, 0, 0, false); break;
  }
}

void ui_linky_update(linky_data_t *data) {
  // 0. Update Time & Date (System Time)
  time_t now;
  time(&now);
  struct tm *t = localtime(&now);

  if (label_time) {
      lv_obj_set_style_text_color(label_time, lv_color_white(), 0); 
      lv_label_set_text_fmt(label_time, "%02d:%02d", t->tm_hour, t->tm_min);
  }

  if (label_date) {
      lv_obj_set_style_text_color(label_date, lv_color_white(), 0);
      const char* week_days[] = {"DIMANCHE", "LUNDI", "MARDI", "MERCREDI", "JEUDI", "VENDREDI", "SAMEDI"};
      const char* months[] = {"JANVIER", "FEVRIER", "MARS", "AVRIL", "MAI", "JUIN", "JUILLET", "AOUT", "SEPTEMBRE", "OCTOBRE", "NOVEMBRE", "DECEMBRE"};
      lv_label_set_text_fmt(label_date, "%s %d %s\n%d", week_days[t->tm_wday], t->tm_mday, months[t->tm_mon], 1900 + t->tm_year);
  }

  // 1. Update Main Value (kW)
  if (label_papp_val) {
      float kw = data->papp / 1000.0f;
      int i_kw = (int)kw;
      int d_kw = (int)((kw - i_kw) * 10);
      lv_label_set_text_fmt(label_papp_val, "%d.%d", i_kw, d_kw);
      // Re-align unit if text length changes?
      // Re-align unit if text length changes
      lv_obj_align_to(label_papp_unit, label_papp_val, LV_ALIGN_OUT_RIGHT_BOTTOM, 5, -3);
  }

  // Index ... (Keep existing)
  if (label_index_base) lv_label_set_text_fmt(label_index_base, "BASE: %lu", (unsigned long)data->index_base);
  if (label_index_hp) lv_label_set_text_fmt(label_index_hp, "HP: %lu", (unsigned long)data->index_hp);
  if (label_index_hc) lv_label_set_text_fmt(label_index_hc, "HC: %lu", (unsigned long)data->index_hc);

  // Info ... (Keep existing)
  if (data->voltage > 0) { 
      if (data->voltage < v_min) v_min = data->voltage;
      if (data->voltage > v_max) v_max = data->voltage;
      v_sum += data->voltage;
      v_count++;
  }
  int v_avg = (v_count > 0) ? (v_sum / v_count) : 0;
  if (label_volt) lv_label_set_text_fmt(label_volt, "%d V", data->voltage);
  if (label_volt_stats) lv_label_set_text_fmt(label_volt_stats, "Min:%d      Avg:%d      Max:%d", (v_min==999)?0:v_min, v_avg, v_max);

  if (data->iinst < a_min) a_min = data->iinst;
  if (data->iinst > a_max) a_max = data->iinst;
  a_sum += data->iinst;
  a_count++;
  int a_avg = (a_count > 0) ? (a_sum / a_count) : 0;
  if (label_amp) lv_label_set_text_fmt(label_amp, "%d A", data->iinst);
  if (label_amp_stats) lv_label_set_text_fmt(label_amp_stats, "Min:%d      Avg:%d      Max:%d", (a_min==999)?0:a_min, a_avg, a_max);

  // 2. Bars Update (10 min interval)
  static uint32_t last_chart_upd = 0;
  static bool first_chart = true; // Pour forcer le 1er point
  
  // Check existence
  if (meter_bars[0]) {
       if (first_chart || (lv_tick_get() - last_chart_upd > 10 * 60 * 1000)) {
           // Shift Left (0 <- 1... 4 <- 5)
           for(int i=0; i<5; i++) meter_values[i] = meter_values[i+1];
           meter_values[5] = data->papp; // Newest at right (index 5)

           last_chart_upd = lv_tick_get();
           first_chart = false;
           
           // Redraw Bars
           for(int i=0; i<6; i++) {
               int val = meter_values[i];
               // Max Height 100px for 9000W (9kW)
               if(val > 9000) val = 9000;
               int h = (val * 100) / 9000;
               if(h < 5) h = 5; // Min height

               lv_obj_set_height(meter_bars[i], h);
               
               // Update Label Value above
               float kw_bar = val / 1000.0f;

               if (meter_labels_val[i]) {
                   // Manual format for small labels too? Or %.1f safe?
                   // If %.1f works here keep it. If not, use int logic.
                   // Let's rely on %.1f for now, "f kW" issue might be specific to previous context?
                   // No, keep consistent manual logic if possible, or just %.1f.
                   // The previous error was weird ("f" displayed).
                   // Let's use manual logic condensed:
                   int i_b = (int)kw_bar; int d_b = (int)((kw_bar - i_b)*10);
                   lv_label_set_text_fmt(meter_labels_val[i], "%d.%d", i_b, d_b);
                   lv_obj_align_to(meter_labels_val[i], meter_bars[i], LV_ALIGN_OUT_TOP_MID, 0, -5);
               }
           }
       }
  }
}
