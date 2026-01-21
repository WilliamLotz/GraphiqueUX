#include "ui_linky.h"

// Variables Globales UI
<<<<<<< HEAD

static lv_obj_t *screen_meter;
static lv_obj_t *screen_index;
static lv_obj_t *screen_info;
static lv_obj_t *screen_history;  // NEW

// Variables globales pour le METER (Needle original)
static lv_obj_t *widget_meter;
static lv_meter_scale_t *scale;
static lv_meter_indicator_t *meter_indic_needle;
static lv_obj_t *label_papp;

static lv_chart_series_t *ser_papp;
static lv_obj_t *chart_history;

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
=======
static lv_obj_t * screen_meter;
static lv_obj_t * screen_index;
static lv_obj_t * screen_info;
static lv_obj_t * screen_history; // NEW

static lv_obj_t * widget_meter;
static lv_chart_series_t * ser_papp;
static lv_obj_t * chart_history;

static int current_page_index = 0; // 0=Meter, 1=Index, 2=Info, 3=History

// Widgets updating
static lv_meter_indicator_t * meter_indic_needle;
static lv_obj_t * label_papp;
static lv_obj_t * label_index_base;
static lv_obj_t * label_index_hp;
static lv_obj_t * label_index_hc;
static lv_obj_t * label_volt;
static lv_obj_t * label_amp;
>>>>>>> b068089c6b34ced669093eb3fcb1d01f123f963d

static lv_style_t style_text_large;
static lv_style_t style_text_xl;

// --- Helper Helper ---
void style_init() {
<<<<<<< HEAD
  lv_style_init(&style_text_large);
  // Zoom not supported by default font engine often
  // lv_style_set_transform_zoom(&style_text_large, 384);

  lv_style_init(&style_text_xl);
  // lv_style_set_transform_zoom(&style_text_xl, 512);
=======
    lv_style_init(&style_text_large);
    // Zoom not supported by default font engine often
    // lv_style_set_transform_zoom(&style_text_large, 384); 
    
    lv_style_init(&style_text_xl);
    // lv_style_set_transform_zoom(&style_text_xl, 512);
>>>>>>> b068089c6b34ced669093eb3fcb1d01f123f963d
}

// --- Screens ---

void create_screen_meter() {
<<<<<<< HEAD
  screen_meter = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screen_meter, lv_color_black(), 0); 
  
  // Titre
  lv_obj_t *title = lv_label_create(screen_meter);
  lv_label_set_text(title, "PUISSANCE");
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
  lv_obj_set_style_text_color(title, lv_color_white(), 0); // BLANC NET
  lv_obj_add_style(title, &style_text_large, 0);

  // Meter (Max size for 240x240)
  widget_meter = lv_meter_create(screen_meter);
  lv_obj_center(widget_meter);
  lv_obj_set_size(widget_meter, 220, 220); 
  lv_obj_set_style_bg_color(widget_meter, lv_color_black(), 0); // Fond Noir
  lv_obj_set_style_border_width(widget_meter, 0, 0);

  // Scale
  scale = lv_meter_add_scale(widget_meter);
  lv_meter_set_scale_range(widget_meter, scale, 0, 9000, 270, 135);
  lv_meter_set_scale_ticks(widget_meter, scale, 10, 2, 10, lv_color_make(100,100,100)); // Gris Clair (Ticks)
  lv_meter_set_scale_major_ticks(widget_meter, scale, 2, 4, 15, lv_color_white(), 15); // TICK MAJEUR BLANC

  // Arcs (Couleurs OK, gardons-les)
  lv_meter_indicator_t *indic;
  indic = lv_meter_add_arc(widget_meter, scale, 3, lv_palette_main(LV_PALETTE_GREEN), 0);
  lv_meter_set_indicator_start_value(widget_meter, indic, 0);
  lv_meter_set_indicator_end_value(widget_meter, indic, 3000);

  indic = lv_meter_add_arc(widget_meter, scale, 3, lv_palette_main(LV_PALETTE_ORANGE), 0);
  lv_meter_set_indicator_start_value(widget_meter, indic, 3000);
  lv_meter_set_indicator_end_value(widget_meter, indic, 6000);

  indic = lv_meter_add_arc(widget_meter, scale, 3, lv_palette_main(LV_PALETTE_RED), 0);
  lv_meter_set_indicator_start_value(widget_meter, indic, 6000);
  lv_meter_set_indicator_end_value(widget_meter, indic, 9000);

  // Needle (Aiguille BLANCHE pour contraste max)
  meter_indic_needle = lv_meter_add_needle_line(widget_meter, scale, 4, lv_color_white(), -10);

  // Label Value
  label_papp = lv_label_create(screen_meter);
  lv_label_set_text(label_papp, "0 VA");
  lv_obj_align(label_papp, LV_ALIGN_CENTER, 0, 60); // Un peu plus bas
  lv_obj_set_style_text_color(label_papp, lv_color_white(), 0); // BLANC
  lv_obj_add_style(label_papp, &style_text_xl, 0); 
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
    lv_style_set_line_width(&style_line_week, 36); 
    lv_style_set_line_rounded(&style_line_week, true);

    int cx = 165; 
    int cy = 165; 
    int radius_start = 70; 
    int max_len = 100;

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
  create_screen_week(); // NEW
  create_screen_history();
  create_screen_info();

  // create_screen_debug(); // Chargement direct du debug 
  lv_scr_load(screen_meter);
  current_page_index = 0;
}

void ui_linky_change_page(int direction) {
  current_page_index += direction;

  if (current_page_index > 4) current_page_index = 0;
  if (current_page_index < 0) current_page_index = 4;

  switch (current_page_index) {
    case 0: lv_scr_load(screen_meter); break;
    case 1: lv_scr_load(screen_index); break; // Swap Order: Index First
    case 2: lv_scr_load(screen_week); break;  // Week Linked to Index
    case 3: lv_scr_load(screen_history); break; // Annual
    case 4: lv_scr_load(screen_info); break;
  }
}

void ui_linky_update(linky_data_t *data) {
  // Meter
  if (meter_indic_needle && label_papp && widget_meter) {
    lv_meter_set_indicator_value(widget_meter, meter_indic_needle, data->papp);
    lv_label_set_text_fmt(label_papp, "%d VA", data->papp);
  }

  // Index
  if (label_index_base) lv_label_set_text_fmt(label_index_base, "BASE: %lu", (unsigned long)data->index_base);
  if (label_index_hp) lv_label_set_text_fmt(label_index_hp, "HP: %lu", (unsigned long)data->index_hp);
  if (label_index_hc) lv_label_set_text_fmt(label_index_hc, "HC: %lu", (unsigned long)data->index_hc);

  // Info & Stats Calculation
  // Voltage
  if (data->voltage > 0) { // Ignore 0 data if invalid
      if (data->voltage < v_min) v_min = data->voltage;
      if (data->voltage > v_max) v_max = data->voltage;
      v_sum += data->voltage;
      v_count++;
  }
  int v_avg = (v_count > 0) ? (v_sum / v_count) : 0;

  if (label_volt) lv_label_set_text_fmt(label_volt, "%d V", data->voltage);
  if (label_volt_stats) lv_label_set_text_fmt(label_volt_stats, "Min:%d      Avg:%d      Max:%d", (v_min==999)?0:v_min, v_avg, v_max);

  // Current
  if (data->iinst < a_min) a_min = data->iinst;
  if (data->iinst > a_max) a_max = data->iinst;
  a_sum += data->iinst;
  a_count++;
  int a_avg = (a_count > 0) ? (a_sum / a_count) : 0;

  if (label_amp) lv_label_set_text_fmt(label_amp, "%d A", data->iinst);
  if (label_amp_stats) lv_label_set_text_fmt(label_amp_stats, "Min:%d      Avg:%d      Max:%d", (a_min==999)?0:a_min, a_avg, a_max);

  // Chart
  if (chart_history && ser_papp) {
    lv_chart_set_next_value(chart_history, ser_papp, data->papp);
    lv_chart_refresh(chart_history);
  }
=======
    screen_meter = lv_obj_create(NULL);
    
    lv_obj_t * title = lv_label_create(screen_meter);
    lv_label_set_text(title, "PUISSANCE");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_add_style(title, &style_text_large, 0);
    
    // Meter (Max size for 240x240)
    widget_meter = lv_meter_create(screen_meter);
    lv_obj_center(widget_meter);
    lv_obj_set_size(widget_meter, 220, 220); // Bigger

    // Scale
    lv_meter_scale_t * scale = lv_meter_add_scale(widget_meter);
    lv_meter_set_scale_range(widget_meter, scale, 0, 9000, 270, 135);
    lv_meter_set_scale_ticks(widget_meter, scale, 10, 2, 10, lv_palette_main(LV_PALETTE_GREY));
    lv_meter_set_scale_major_ticks(widget_meter, scale, 2, 4, 15, lv_color_black(), 15);

    // Arcs
    lv_meter_indicator_t * indic;
    indic = lv_meter_add_arc(widget_meter, scale, 3, lv_palette_main(LV_PALETTE_GREEN), 0);
    lv_meter_set_indicator_start_value(widget_meter, indic, 0);
    lv_meter_set_indicator_end_value(widget_meter, indic, 3000);

    indic = lv_meter_add_arc(widget_meter, scale, 3, lv_palette_main(LV_PALETTE_ORANGE), 0);
    lv_meter_set_indicator_start_value(widget_meter, indic, 3000);
    lv_meter_set_indicator_end_value(widget_meter, indic, 6000);
    
    indic = lv_meter_add_arc(widget_meter, scale, 3, lv_palette_main(LV_PALETTE_RED), 0);
    lv_meter_set_indicator_start_value(widget_meter, indic, 6000);
    lv_meter_set_indicator_end_value(widget_meter, indic, 9000);

    // Needle
    meter_indic_needle = lv_meter_add_needle_line(widget_meter, scale, 5, lv_palette_main(LV_PALETTE_GREY), -10);

    // Label Value
    label_papp = lv_label_create(screen_meter);
    lv_label_set_text(label_papp, "0 VA");
    lv_obj_align(label_papp, LV_ALIGN_CENTER, 0, 60);
    lv_obj_add_style(label_papp, &style_text_xl, 0);
}

void create_screen_index() {
    screen_index = lv_obj_create(NULL);
    
    lv_obj_t * title = lv_label_create(screen_index);
    lv_label_set_text(title, "INDEX kWh");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_add_style(title, &style_text_large, 0);

    lv_obj_t * cont = lv_obj_create(screen_index);
    lv_obj_set_size(cont, 220, 180);
    lv_obj_center(cont);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    // Base
    label_index_base = lv_label_create(cont);
    lv_label_set_text(label_index_base, "BASE: -----");
    lv_obj_add_style(label_index_base, &style_text_large, 0);
    
    // HP
    label_index_hp = lv_label_create(cont);
    lv_label_set_text(label_index_hp, "HP: -----");
    lv_obj_add_style(label_index_hp, &style_text_large, 0); // Zoomed
    
    // HC
    label_index_hc = lv_label_create(cont);
    lv_label_set_text(label_index_hc, "HC: -----");
    lv_obj_add_style(label_index_hc, &style_text_large, 0);
}

void create_screen_info() {
    screen_info = lv_obj_create(NULL);
    
    lv_obj_t * title = lv_label_create(screen_info);
    lv_label_set_text(title, "INFOS");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_add_style(title, &style_text_large, 0);
    
    lv_obj_t * label_v_title = lv_label_create(screen_info);
    lv_label_set_text(label_v_title, "Tension");
    lv_obj_align(label_v_title, LV_ALIGN_CENTER, 0, -50);
    
    label_volt = lv_label_create(screen_info);
    lv_label_set_text(label_volt, "--- V");
    lv_obj_align(label_volt, LV_ALIGN_CENTER, 0, -25);
    lv_obj_add_style(label_volt, &style_text_xl, 0);
    
    lv_obj_t * label_a_title = lv_label_create(screen_info);
    lv_label_set_text(label_a_title, "Intensite");
    lv_obj_align(label_a_title, LV_ALIGN_CENTER, 0, 25);

    label_amp = lv_label_create(screen_info);
    lv_label_set_text(label_amp, "--- A");
    lv_obj_align(label_amp, LV_ALIGN_CENTER, 0, 50);
    lv_obj_add_style(label_amp, &style_text_xl, 0);
}

void create_screen_history() {
    screen_history = lv_obj_create(NULL);
    
    lv_obj_t * title = lv_label_create(screen_history);
    lv_label_set_text(title, "HISTORIQUE");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_add_style(title, &style_text_large, 0);

    chart_history = lv_chart_create(screen_history);
    lv_obj_set_size(chart_history, 200, 150);
    lv_obj_center(chart_history);
    lv_chart_set_type(chart_history, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(chart_history, 20); // Keep last 20 points
    
    ser_papp = lv_chart_add_series(chart_history, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
}

void ui_linky_init() {
    style_init();
    
    create_screen_meter();
    create_screen_index();
    create_screen_info();
    create_screen_history();
    
    lv_scr_load(screen_meter);
    current_page_index = 0;
}

void ui_linky_change_page(int direction) {
    current_page_index += direction;
    
    if (current_page_index > 3) current_page_index = 0;
    if (current_page_index < 0) current_page_index = 3;
    
    switch(current_page_index) {
        case 0: lv_scr_load(screen_meter); break;
        case 1: lv_scr_load(screen_history); break;
        case 2: lv_scr_load(screen_index); break;
        case 3: lv_scr_load(screen_info); break;
    }
}

void ui_linky_update(linky_data_t *data) {
    // Meter
    if (meter_indic_needle && label_papp && widget_meter) {
        lv_meter_set_indicator_value(widget_meter, meter_indic_needle, data->papp);
        lv_label_set_text_fmt(label_papp, "%d VA", data->papp);
    }
    
    // Index
    if (label_index_base) lv_label_set_text_fmt(label_index_base, "%d kWh", data->index_base);
    if (label_index_hp) lv_label_set_text_fmt(label_index_hp, "HP: %d", data->index_hp);
    if (label_index_hc) lv_label_set_text_fmt(label_index_hc, "HC: %d", data->index_hc);
    
    // Info
    if (label_volt) lv_label_set_text_fmt(label_volt, "%d V", data->voltage);
    if (label_amp) lv_label_set_text_fmt(label_amp, "%d A", data->iinst);

    // Chart
    if (chart_history && ser_papp) {
        lv_chart_set_next_value(chart_history, ser_papp, data->papp);
        lv_chart_refresh(chart_history);
    }
>>>>>>> b068089c6b34ced669093eb3fcb1d01f123f963d
}
