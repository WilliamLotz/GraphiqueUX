#include "ui_linky.h"
#include <math.h>
#include <time.h>
#include <stdio.h>

#ifndef PI
#define PI 3.14159265358979323846
#endif

extern linky_data_t linky_data; // Ajout pour acceder aux données globales des historiques de la SD

// --- GLOBALS ---
static lv_obj_t *screen_meter;
static lv_obj_t *screen_index;
static lv_obj_t *screen_week;    // Page 2
static lv_obj_t *screen_history; // Page 3
static lv_obj_t *screen_info;    // Page 4
static lv_obj_t *screen_wifi;    // Page 5

static lv_obj_t *highlight_week = NULL;
static lv_obj_t *highlight_month = NULL;

static int current_page_index = 0; // 0=Meter, 1=Index, 2=Week, 3=History, 4=Info, 5=WiFi


// Styles
static lv_style_t style_text_large;
static lv_style_t style_text_xl;
static lv_style_t style_kb; // Style pour le clavier (touches)
static lv_style_t style_kb_main; // Style pour le clavier (conteneur)

// Meter Widgets
static lv_obj_t *label_time;
static lv_obj_t *label_date;
static lv_obj_t *label_papp_val; 
static lv_obj_t *label_papp_unit;
static lv_obj_t *meter_bars[6];
static lv_obj_t *meter_labels_val[6];
static lv_obj_t *meter_labels_time[6];
static int meter_values[6] = {0}; 

// Index Widgets
static lv_obj_t *label_index_base;
static lv_obj_t *label_index_hp;
static lv_obj_t *label_index_hc;

// Info Widgets
static lv_obj_t *label_volt;
static lv_obj_t *label_volt_stats;
static lv_obj_t *label_amp;
static lv_obj_t *label_amp_stats;
// Stats Data
static uint16_t v_min = 999, v_max = 0;
static uint32_t v_sum = 0, v_count = 0;
static uint16_t a_min = 999, a_max = 0;
static uint32_t a_sum = 0, a_count = 0;

// History/Week Radial Data
static lv_point_t history_points[12][2]; 
static lv_obj_t * history_lines[12];     
static lv_obj_t * label_months[12];      
const char* month_letters[12] = {"J", "F", "M", "A", "M", "J", "J", "A", "S", "O", "N", "D"};

static lv_point_t week_points[7][2];
static lv_obj_t * week_lines[7];
static lv_obj_t * label_days[7];
const char* day_names[7] = {"L", "M", "M", "J", "V", "S", "D"};

// WiFi Widgets
static lv_obj_t *ta_ssid;
static lv_obj_t *ta_pwd;
static lv_obj_t *kb;
static lv_obj_t *btn_connect;
static lv_obj_t *label_wifi_status;
char wifi_ssid[32] = {0};
char wifi_pwd[32] = {0};
volatile bool wifi_connect_requested = false;

// AZERTY Layout Maps (5 Rows for better spacing)
// AZERTY Layout Maps (Fixed with Shift Logic)
static const char * kb_map_azerty_lc[] = {
    "a", "z", "e", "r", "t", "y", "u", "i", "o", "p", "\n",
    "q", "s", "d", "f", "g", "h", "j", "k", "l", "m", "\n",
    " ", LV_SYMBOL_UP, "w", "x", "c", "v", "b", "n", LV_SYMBOL_BACKSPACE,"  ", "\n",
    "  ", "1#", " ", ".", "  ", "\n",
    ""
    
    
};

static const char * kb_map_azerty_uc[] = {
    "A", "Z", "E", "R", "T", "Y", "U","I", "O", "P", "\n",
    "Q", "S", "D", "F", "G", "H", "J", "K", "L", "M", "\n",
    " ", LV_SYMBOL_DOWN, "W", "X", "C", "V", "B", "N", LV_SYMBOL_BACKSPACE,"  ", "\n",
    "  ", "1#", " ", ".", "  ", "\n",
    ""
    
};

static const char * kb_map_spec[] = {
    "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "\n",
    "+", "-", "/", "*", "=", "%", "!", "?", "#", "<", ">", "\n",
    " ", "@", "$", "(", ")", "{", "}", "[", "]", ";", ":", " ", "\n",
    " ", "'", "_", "&", "^", "~", "|", "`", LV_SYMBOL_BACKSPACE, " ", "\n",
    "  ","abc", " ", ",", "  ", "\n",
    ""
    
    
};

static const lv_btnmatrix_ctrl_t kb_ctrl_azerty_lc_map[] = {
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    2, 4, 3, 3, 3, 3, 3, 3, 4, 2,
    2, 3, 10, 3, 2
    

};

static const lv_btnmatrix_ctrl_t kb_ctrl_azerty_uc_map[] = {
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    2, 4, 3, 3, 3, 3, 3, 3, 4, 2,
    2, 3, 10, 3, 2
};

static const lv_btnmatrix_ctrl_t kb_ctrl_spec_map[] = {
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1,
    2, 3, 3, 3, 3, 3, 3, 3, 5, 2,
    2, 4, 6, 3, 2
};


// --- INIT STYLES ---
void style_init() {
  lv_style_init(&style_text_large);
  // Zoom not always supported, rely on fonts if possible, or keep simple
  // lv_style_set_transform_zoom(&style_text_large, 384); 

  lv_style_init(&style_text_xl);
  // lv_style_set_transform_zoom(&style_text_xl, 512); 

  // Style Clavier (Touches)
  lv_style_init(&style_kb);
  lv_style_set_text_font(&style_kb, &lv_font_montserrat_16);
  lv_style_set_radius(&style_kb, 3); // Rayon plus petit

  // Style Clavier (Conteneur/Espacements)
  lv_style_init(&style_kb_main);
  lv_style_set_pad_row(&style_kb_main, 1);    // Petit espace (1px)
  lv_style_set_pad_column(&style_kb_main, 1); // Petit espace (1px)
  lv_style_set_pad_all(&style_kb_main, 1);    // Marge minime
  lv_style_set_pad_bottom(&style_kb_main, 10); // Espace vide en bas pour éviter la zone tactile difficile
  lv_style_set_radius(&style_kb_main, 0);
}

// --- SCREEN: METER (PAGE 0) ---
void create_screen_meter() {
  screen_meter = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screen_meter, lv_color_black(), 0); 
  
  // Time for Init
  time_t now;
  time(&now);
  struct tm *t = localtime(&now);

  // Header Time
  label_time = lv_label_create(screen_meter);
  lv_label_set_text_fmt(label_time, "%02d:%02d", t->tm_hour, t->tm_min);
  lv_obj_set_style_text_color(label_time, lv_color_white(), 0);
  lv_obj_set_style_text_font(label_time, &lv_font_montserrat_30, 0); 
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

  // Central Box
  lv_obj_t *box = lv_obj_create(screen_meter);
  lv_obj_set_size(box, 260, 90);
  lv_obj_align(box, LV_ALIGN_TOP_MID, 0, 110); 
  lv_obj_set_style_bg_color(box, lv_color_black(), 0); 
  lv_obj_set_style_border_color(box, lv_color_make(0, 0, 255), 0); // Hack Color
  lv_obj_set_style_border_width(box, 1, 0);
  lv_obj_set_style_radius(box, 20, 0);
  lv_obj_clear_flag(box, LV_OBJ_FLAG_SCROLLABLE);

  // Value
  label_papp_val = lv_label_create(box);
  lv_label_set_text(label_papp_val, "0.4");
  lv_obj_set_style_text_font(label_papp_val, &lv_font_montserrat_40, 0);
  lv_obj_set_style_text_color(label_papp_val, lv_color_white(), 0);
  lv_obj_align(label_papp_val, LV_ALIGN_CENTER, -10, -8); 

  // Unit
  label_papp_unit = lv_label_create(box);
  lv_label_set_text(label_papp_unit, "KW");
  lv_obj_set_style_text_font(label_papp_unit, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(label_papp_unit, lv_color_white(), 0);
  lv_obj_align_to(label_papp_unit, label_papp_val, LV_ALIGN_OUT_RIGHT_BOTTOM, 5, -5); 

  // Subtitle
  lv_obj_t *lbl_inst = lv_label_create(box);
  lv_label_set_text(lbl_inst, "INSTANT");
  lv_obj_set_style_text_color(lbl_inst, lv_color_white(), 0);
  lv_obj_set_style_text_font(lbl_inst, &lv_font_montserrat_16, 0); 
  lv_obj_align(lbl_inst, LV_ALIGN_BOTTOM_MID, 0, 11);

  // BARS
  int bar_w = 28;
  int gap = 16;
  int start_x = -((6 * bar_w + 5 * gap) / 2) + (bar_w/2); 

  // Mock Values
  int mock_vals[6] = {500, 1500, 3200, 800, 4500, 2100};
  for(int k=0; k<6; k++) meter_values[k] = mock_vals[k];

  for(int i=0; i<6; i++) {
      int val = meter_values[i];
      if(val > 9000) val = 9000;
      int h = (val * 100) / 9000;
      if(h < 5) h = 5;

      meter_bars[i] = lv_bar_create(screen_meter);
      lv_bar_set_range(meter_bars[i], 0, 100);
      lv_bar_set_value(meter_bars[i], 100, LV_ANIM_OFF);
      
      lv_obj_set_size(meter_bars[i], bar_w, h); // Dynamic Height
      lv_obj_set_style_radius(meter_bars[i], 2, LV_PART_MAIN);      
      lv_obj_set_style_radius(meter_bars[i], 2, LV_PART_INDICATOR); 
      
      // Cyan (0,255,255)
      lv_obj_set_style_bg_color(meter_bars[i], lv_color_make(0, 255, 255), LV_PART_INDICATOR);
      lv_obj_set_style_bg_grad_color(meter_bars[i], lv_color_make(0, 255, 255), LV_PART_INDICATOR); 
      lv_obj_set_style_bg_grad_dir(meter_bars[i], LV_GRAD_DIR_NONE, LV_PART_INDICATOR);
      lv_obj_set_style_bg_opa(meter_bars[i], LV_OPA_TRANSP, LV_PART_MAIN);

      int x_pos = start_x + i * (bar_w + gap);
      
      // Smile Effect
      int y_off = -15; 
      if(i==1 || i==4) y_off = 10; 
      if(i==2 || i==3) y_off = 25; 

      lv_obj_align(meter_bars[i], LV_ALIGN_BOTTOM_MID, x_pos, -50 + y_off); 

      // Label Value
      meter_labels_val[i] = lv_label_create(screen_meter);
      float kw_bar = val / 1000.0f;
      int i_b = (int)kw_bar; int d_b = (int)((kw_bar - i_b)*10);
      lv_label_set_text_fmt(meter_labels_val[i], "%d.%d", i_b, d_b);
      lv_obj_set_style_text_color(meter_labels_val[i], lv_color_white(), 0);
      lv_obj_align_to(meter_labels_val[i], meter_bars[i], LV_ALIGN_OUT_TOP_MID, 0, -5);

      // Label Time
      meter_labels_time[i] = lv_label_create(screen_meter);
      lv_label_set_text_fmt(meter_labels_time[i], "%d", (6-i)*10); 
      lv_obj_set_style_text_color(meter_labels_time[i], lv_color_white(), 0); 
      lv_obj_align_to(meter_labels_time[i], meter_bars[i], LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
  }
}

// --- SCREEN: INDEX (PAGE 1) ---
void create_screen_index() {
  screen_index = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screen_index, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(screen_index, LV_OPA_COVER, 0);

  lv_obj_t *title = lv_label_create(screen_index);
  lv_label_set_text(title, "INDEX kWh");
  lv_obj_set_style_text_color(title, lv_color_white(), 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 30);
  lv_obj_set_style_text_letter_space(title, 2, 0); 

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

// --- SCREEN: INFO (PAGE 4) ---
void create_screen_info() {
  screen_info = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screen_info, lv_color_black(), 0);
  
  lv_obj_t * cont = lv_obj_create(screen_info);
  lv_obj_set_size(cont, 240, 240);
  lv_obj_center(cont);
  lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(cont, 0, 0);
  lv_obj_set_style_pad_row(cont, 10, 0);
  lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *title = lv_label_create(cont);
  lv_label_set_text(title, "INFOS");
  lv_obj_set_style_text_color(title, lv_color_white(), 0);
  lv_obj_add_style(title, &style_text_large, 0);

  // Volts
  lv_obj_t *lbl_v = lv_label_create(cont);
  lv_label_set_text(lbl_v, "TENSION");
  lv_obj_set_style_text_color(lbl_v, lv_color_white(), 0);
  lv_obj_add_style(lbl_v, &style_text_large, 0);
  
  label_volt = lv_label_create(cont);
  lv_label_set_text(label_volt, "--- V");
  lv_obj_set_style_text_color(label_volt, lv_color_white(), 0);
  lv_obj_add_style(label_volt, &style_text_xl, 0);

  label_volt_stats = lv_label_create(cont);
  lv_label_set_text(label_volt_stats, "Min:-      Avg:-      Max:-");
  lv_obj_set_style_text_color(label_volt_stats, lv_color_white(), 0); 
  lv_obj_set_style_text_font(label_volt_stats, LV_FONT_DEFAULT, 0);
  lv_obj_set_style_pad_bottom(label_volt_stats, 10, 0);

  // Amps
  lv_obj_t *lbl_a = lv_label_create(cont);
  lv_label_set_text(lbl_a, "INTENSITE");
  lv_obj_set_style_text_color(lbl_a, lv_color_white(), 0);
  lv_obj_add_style(lbl_a, &style_text_large, 0);

  label_amp = lv_label_create(cont);
  lv_label_set_text(label_amp, "-- A");
  lv_obj_set_style_text_color(label_amp, lv_color_white(), 0);
  lv_obj_add_style(label_amp, &style_text_xl, 0);

  label_amp_stats = lv_label_create(cont);
  lv_label_set_text(label_amp_stats, "Min:-      Avg:-      Max:-");
  lv_obj_set_style_text_color(label_amp_stats, lv_color_white(), 0); 
  lv_obj_set_style_text_font(label_amp_stats, LV_FONT_DEFAULT, 0);
}

// --- SCREEN: WEEK (PAGE 2) ---
void create_screen_week() {
    screen_week = lv_obj_create(NULL);
    lv_obj_set_style_pad_all(screen_week, 0, 0); 
    lv_obj_set_style_bg_color(screen_week, lv_color_black(), 0);

    lv_obj_t * radial_area = lv_obj_create(screen_week);
    lv_obj_set_size(radial_area, 360, 360);
    lv_obj_center(radial_area);
    lv_obj_set_style_bg_opa(radial_area, LV_OPA_TRANSP, 0); 
    lv_obj_set_style_border_width(radial_area, 0, 0);
    lv_obj_clear_flag(radial_area, LV_OBJ_FLAG_SCROLLABLE);

    // Central Disc
    lv_obj_t * central_disc = lv_obj_create(screen_week);
    lv_obj_set_size(central_disc, 120, 120);
    lv_obj_set_pos(central_disc, 120, 120); // Manual centered pos (approx 360/2 - 60 = 120) -> But cx is 165 ? 
    // Wait, 120,120 is relative to screen (240x240 usually?) No 360x360 S3 screen.
    // If screen is 360x360 circle. Center is 180,180.
    // Disc 120x120. TopLeft = 180-60=120. So 120,120 is correct for absolute center of 360px screen.
    lv_obj_set_style_radius(central_disc, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(central_disc, lv_color_white(), 0);
    lv_obj_set_style_border_width(central_disc, 0, 0);

    lv_obj_t * title_shadow = lv_label_create(screen_week);
    lv_label_set_text(title_shadow, "CONSO\nSEMAINE\nkWh");
    lv_obj_set_style_text_align(title_shadow, LV_TEXT_ALIGN_CENTER, 0); 
    lv_obj_align(title_shadow, LV_ALIGN_CENTER, 0, 0); 
    lv_obj_set_style_text_color(title_shadow, lv_color_black(), 0); 

    // Styles
    static lv_style_t style_line_week;
    lv_style_init(&style_line_week);
    lv_style_set_line_width(&style_line_week, 30); 
    lv_style_set_line_rounded(&style_line_week, true);

    time_t now;
    time(&now);
    struct tm *t = localtime(&now);
    int current_day_idx = (t->tm_wday == 0) ? 6 : (t->tm_wday - 1); // 0=Lundi, 6=Dimanche

    int cx = 165; // Correct center for 360px screen
    int cy = 165; 
    int radius_start = 70; 
    int max_len = 70;

    // Create global highlight, initially hidden (creé AVANT les textes pour etre EN DESSOUS)
    highlight_week = lv_obj_create(radial_area);
    lv_obj_set_size(highlight_week, 24, 24);
    lv_obj_set_style_radius(highlight_week, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(highlight_week, lv_color_hex(0xFF0000), 0); 
    lv_obj_set_style_border_width(highlight_week, 0, 0);
    lv_obj_clear_flag(highlight_week, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(highlight_week, LV_OBJ_FLAG_HIDDEN);

    for(int i=0; i<7; i++) {
        float angle_deg = -90.0f + (i * 360.0f / 7.0f);
        float angle_rad = angle_deg * (PI / 180.0f);
        
        int value = linky_data.history_week[i]; // Utiliser la vraie donnee depuis la carte SD (kWh)

        // Limiter la valeur pour l'affichage (ex: max 60 kWh/jour)
        int max_val_scale = 60;
        int display_val = value;
        if(display_val > max_val_scale) display_val = max_val_scale;

        // Hack Color: Vert (Faible) -> Rouge (Fort) ! (rappel: ecran inverse RGB)
        // Ratio de consommation
        uint8_t ratio = (display_val * 255) / max_val_scale; 
        
        // On veut Green (Low) -> Red (High). 
        // Mais l'écran inverse les couleurs:
        // Envoyer Bleu (0,0,255) => Affiche Vert
        // Envoyer Vert (0,255,0) => Affiche Bleu
        // Envoyer Rouge (255,0,0) => Affiche Cyan ? Wait.
        // D'apres l'autre code: Green on screen is Blue(0,0,255). Red on screen is Green(0,255,0).
        lv_color_t color = lv_color_make(0, ratio, 255 - ratio);

        int bar_len = (display_val * max_len) / max_val_scale;
        
        week_points[i][0].x = cx + (int)(cos(angle_rad) * radius_start);
        week_points[i][0].y = cy + (int)(sin(angle_rad) * radius_start);
        week_points[i][1].x = cx + (int)(cos(angle_rad) * (radius_start + bar_len));
        week_points[i][1].y = cy + (int)(sin(angle_rad) * (radius_start + bar_len));

        week_lines[i] = lv_line_create(radial_area);
        lv_line_set_points(week_lines[i], week_points[i], 2);
        lv_obj_add_style(week_lines[i], &style_line_week, 0);
        lv_obj_set_style_line_color(week_lines[i], color, 0);

        int label_radius = 160;
        int lx = cx + (int)(cos(angle_rad) * label_radius);
        int ly = cy + (int)(sin(angle_rad) * label_radius);

        label_days[i] = lv_label_create(radial_area); 
        lv_label_set_text(label_days[i], day_names[i]);
        lv_obj_set_style_text_color(label_days[i], lv_color_white(), 0);
        lv_obj_set_pos(label_days[i], lx - 7, ly - 7);
        
        lv_obj_t * val_lbl = lv_label_create(radial_area);
        lv_label_set_text_fmt(val_lbl, "%d", value);  // En kW direct
        lv_obj_set_style_text_color(val_lbl, lv_color_black(), 0); 
        
        // Rapprochement Centre (Dans le disque blanc)
        int val_radius = 50; 
        int vx = cx + (int)(cos(angle_rad) * val_radius);
        int vy = cy + (int)(sin(angle_rad) * val_radius);
        lv_obj_set_pos(val_lbl, vx - 10, vy - 7);
    }
}

// --- SCREEN: HISTORY (PAGE 3) ---
void create_screen_history() {
    screen_history = lv_obj_create(NULL);
    lv_obj_set_style_pad_all(screen_history, 0, 0); 
    lv_obj_set_style_bg_color(screen_history, lv_color_black(), 0); // Important for background

    lv_obj_t * radial_area = lv_obj_create(screen_history);
    lv_obj_set_size(radial_area, 360, 360);
    lv_obj_center(radial_area);
    lv_obj_set_style_bg_opa(radial_area, LV_OPA_TRANSP, 0); 
    lv_obj_set_style_border_width(radial_area, 0, 0);
    lv_obj_clear_flag(radial_area, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * central_disc = lv_obj_create(screen_history);
    lv_obj_set_size(central_disc, 120, 120);
    lv_obj_center(central_disc);
    lv_obj_set_style_radius(central_disc, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(central_disc, lv_color_white(), 0);
    lv_obj_set_style_border_width(central_disc, 0, 0);

    lv_obj_t * title_shadow = lv_label_create(screen_history);
    lv_label_set_text(title_shadow, "CONSO\nANNUELLE\nkWh");
    lv_obj_set_style_text_align(title_shadow, LV_TEXT_ALIGN_CENTER, 0); 
    lv_obj_align(title_shadow, LV_ALIGN_CENTER, 0, 0); 
    lv_obj_set_style_text_color(title_shadow, lv_color_black(), 0); 
    lv_obj_add_style(title_shadow, &style_text_large, 0);

    static lv_style_t style_line;
    lv_style_init(&style_line);
    lv_style_set_line_width(&style_line, 28); 
    lv_style_set_line_rounded(&style_line, true);

    time_t now;
    time(&now);
    struct tm *t = localtime(&now);
    int current_month_idx = t->tm_mon; // 0=Janvier, 11=Decembre

    int cx = 165; 
    int cy = 165; 

    // Create global highlight, initially hidden (creé AVANT les textes pour etre EN DESSOUS)
    highlight_month = lv_obj_create(radial_area);
    lv_obj_set_size(highlight_month, 24, 24);
    lv_obj_set_style_radius(highlight_month, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(highlight_month, lv_color_hex(0xFF0000), 0); 
    lv_obj_set_style_border_width(highlight_month, 0, 0);
    lv_obj_clear_flag(highlight_month, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(highlight_month, LV_OBJ_FLAG_HIDDEN);
    
    for(int i=0; i<12; i++) {
        float angle_deg = (i + 1) * 30; 
        float angle_rad = angle_deg * (PI / 180.0f);

        int value_wh = linky_data.history_year[i]; 
        int value = value_wh / 1000; // Passage en kWh pour le graph
        
        int max_val_scale = 300; // max 300 kWh par mois
        int display_val = value;
        if(display_val > max_val_scale) display_val = max_val_scale;

        uint8_t ratio = (display_val * 255) / max_val_scale; 
        
        // Color hack (Vert a Rouge)
        lv_color_t bar_color = lv_color_make(0, ratio, 255 - ratio);

        int hole = 70; 
        // Normaliser la taille de la barre
        int bar_len = (display_val * 70) / max_val_scale;
        
        int x_start = cx + (int)(sin(angle_rad) * hole);
        int y_start = cy - (int)(cos(angle_rad) * hole);
        int x_end = cx + (int)(sin(angle_rad) * (hole + bar_len));
        int y_end = cy - (int)(cos(angle_rad) * (hole + bar_len));

        history_points[i][0].x = x_start;
        history_points[i][0].y = y_start;
        history_points[i][1].x = x_end;
        history_points[i][1].y = y_end;

        history_lines[i] = lv_line_create(radial_area);
        lv_line_set_points(history_lines[i], history_points[i], 2);
        lv_obj_add_style(history_lines[i], &style_line, 0);
        lv_obj_set_style_line_color(history_lines[i], bar_color, 0);
        
        int label_radius = 160; 
        int lx = cx + (int)(sin(angle_rad) * label_radius);
        int ly = cy - (int)(cos(angle_rad) * label_radius);

        label_months[i] = lv_label_create(radial_area); 
        lv_label_set_text(label_months[i], month_letters[i]);
        lv_obj_set_pos(label_months[i], lx - 6, ly - 9); 
        
        // Rapprochement Centre
        int val_radius = 50; 
        int vx = cx + (int)(sin(angle_rad) * val_radius);
        int vy = cy - (int)(cos(angle_rad) * val_radius);

        lv_obj_t * val_lbl = lv_label_create(radial_area);
        lv_label_set_text_fmt(val_lbl, "%d", value); // En kW direct
        lv_obj_set_style_text_color(val_lbl, lv_color_black(), 0); 
        lv_obj_set_pos(val_lbl, vx - 10, vy - 7); 
    }
}

// --- SCREEN: WIFI (PAGE 5) ---
static void ta_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    if(code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED) {
        if(kb != NULL) {
            lv_keyboard_set_textarea(kb, ta);
            lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
        }
    }
    if(code == LV_EVENT_DEFOCUSED) {
        if(kb != NULL) {
            lv_keyboard_set_textarea(kb, NULL);
            lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static void kb_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * kb_obj = lv_event_get_target(e);

    if(code == LV_EVENT_VALUE_CHANGED) {
        uint16_t btn_id = lv_btnmatrix_get_selected_btn(kb_obj);
        if(btn_id == LV_BTNMATRIX_BTN_NONE) return;

        const char * txt = lv_btnmatrix_get_btn_text(kb_obj, btn_id);
        if(txt == NULL) return;

        lv_obj_t * ta = lv_keyboard_get_textarea(kb_obj); // Get active Text Area

        if(strcmp(txt, LV_SYMBOL_UP) == 0) {
            if(ta) lv_textarea_del_char(ta); // Remove the inserted symbol
            lv_keyboard_set_mode(kb_obj, LV_KEYBOARD_MODE_TEXT_UPPER);
            lv_indev_wait_release(lv_indev_get_act()); // Prevent ghost click
        }
        else if(strcmp(txt, LV_SYMBOL_DOWN) == 0) {
            if(ta) lv_textarea_del_char(ta); // Remove the inserted symbol
            lv_keyboard_set_mode(kb_obj, LV_KEYBOARD_MODE_TEXT_LOWER);
            lv_obj_set_height(kb_obj, lv_pct(60)); // Back to normal height
           
        }
        else if(strcmp(txt, "1#") == 0) {
            if(ta) {
                lv_textarea_del_char(ta); // Remove #
                lv_textarea_del_char(ta); // Remove 1
            }
            lv_keyboard_set_mode(kb_obj, LV_KEYBOARD_MODE_SPECIAL);
            lv_obj_set_height(kb_obj, lv_pct(70)); // Taller for special chars
            lv_indev_wait_release(lv_indev_get_act()); // Prevent ghost click
        }
        else if(strcmp(txt, "abc") == 0) {
            if(ta) {
                lv_textarea_del_char(ta); // c
                lv_textarea_del_char(ta); // b
                lv_textarea_del_char(ta); // a
            }
            lv_keyboard_set_mode(kb_obj, LV_KEYBOARD_MODE_TEXT_LOWER);
            lv_obj_set_height(kb_obj, lv_pct(60)); // Back to normal height
            lv_indev_wait_release(lv_indev_get_act()); // Prevent ghost click
        }
    }
    else if(code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        lv_obj_add_flag(kb_obj, LV_OBJ_FLAG_HIDDEN);
    }
}

static void btn_connect_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        // Sauvegarde des credentials (simulation pour l'instant)
        const char * txt_ssid = lv_textarea_get_text(ta_ssid);
        const char * txt_pwd = lv_textarea_get_text(ta_pwd);
        snprintf(wifi_ssid, sizeof(wifi_ssid), "%s", txt_ssid);
        snprintf(wifi_pwd, sizeof(wifi_pwd), "%s", txt_pwd);
        
        if (label_wifi_status) {
            lv_label_set_text(label_wifi_status, "Connexion en cours...");
            lv_obj_set_style_text_color(label_wifi_status, lv_color_make(255, 165, 0), 0); // Orange
            wifi_connect_requested = true;
        }
        printf("WiFi Connect Request: %s / %s\n", wifi_ssid, wifi_pwd);
    }
}

// Fonction Helper pour le statut
void ui_linky_set_wifi_status(const char* msg, bool success) {
    if(label_wifi_status) {
        lv_label_set_text(label_wifi_status, msg);
        if(success) lv_obj_set_style_text_color(label_wifi_status, lv_color_make(0, 255, 0), 0); // Vert
        else lv_obj_set_style_text_color(label_wifi_status, lv_color_make(255, 0, 0), 0); // Rouge
    }
}

void create_screen_wifi() {
    screen_wifi = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen_wifi, lv_color_black(), 0);
    
    // Title
    lv_obj_t * title = lv_label_create(screen_wifi);
    lv_label_set_text(title, "CONFIGURATION WIFI");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

    // SSID Area
    ta_ssid = lv_textarea_create(screen_wifi);
    lv_obj_set_style_text_font(ta_ssid, &lv_font_montserrat_20, 0); // Police agrandie
    lv_textarea_set_placeholder_text(ta_ssid, "Nom du reseau");
    lv_obj_set_width(ta_ssid, 220);
    lv_obj_align(ta_ssid, LV_ALIGN_TOP_MID, 0, 60);
    lv_obj_add_event_cb(ta_ssid, ta_event_cb, LV_EVENT_ALL, NULL);

    // Password Area
    ta_pwd = lv_textarea_create(screen_wifi);
    lv_obj_set_style_text_font(ta_pwd, &lv_font_montserrat_20, 0); // Police agrandie
    lv_textarea_set_placeholder_text(ta_pwd, "Mot de passe");
    lv_textarea_set_password_mode(ta_pwd, false);
    lv_obj_set_width(ta_pwd, 220);
    lv_obj_align(ta_pwd, LV_ALIGN_TOP_MID, 0, 110);
    lv_obj_add_event_cb(ta_pwd, ta_event_cb, LV_EVENT_ALL, NULL);

    // Connect Button
    btn_connect = lv_btn_create(screen_wifi);
    lv_obj_set_width(btn_connect, 120);
    lv_obj_align(btn_connect, LV_ALIGN_TOP_MID, 0, 160);
    lv_obj_add_event_cb(btn_connect, btn_connect_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_color(btn_connect, lv_color_make(0, 100, 255), 0);

    lv_obj_t * btn_label = lv_label_create(btn_connect);
    lv_label_set_text(btn_label, "Connexion");
    lv_obj_center(btn_label);

    // Status Label
    label_wifi_status = lv_label_create(screen_wifi);
    lv_label_set_text(label_wifi_status, "Pret");
    lv_obj_set_style_text_color(label_wifi_status, lv_color_make(0, 255, 0), 0);
    lv_obj_align(label_wifi_status, LV_ALIGN_TOP_MID, 0, 200);

    // Keyboard (Initially Hidden)
    kb = lv_keyboard_create(screen_wifi);
    lv_obj_set_width(kb, lv_pct(100)); 
    lv_obj_set_height(kb, lv_pct(70));
    lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, -20); // Remonte de 20px pour eviter la zone difficile
    
    // Set AZERTY Map
    lv_keyboard_set_map(kb, LV_KEYBOARD_MODE_TEXT_LOWER, kb_map_azerty_lc, kb_ctrl_azerty_lc_map);
    lv_keyboard_set_map(kb, LV_KEYBOARD_MODE_TEXT_UPPER, kb_map_azerty_uc, kb_ctrl_azerty_uc_map);
    lv_keyboard_set_map(kb, LV_KEYBOARD_MODE_SPECIAL, kb_map_spec, kb_ctrl_spec_map);
    lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_TEXT_LOWER);

    // Apply Styles
    lv_obj_add_style(kb, &style_kb, LV_PART_ITEMS);
    lv_obj_add_style(kb, &style_kb_main, LV_PART_MAIN);

    lv_obj_add_event_cb(kb, kb_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
}

// --- MAIN INIT ---
void ui_linky_init() {
  style_init();

  create_screen_meter();
  create_screen_index();
  create_screen_week();
  create_screen_history();
  create_screen_info();
  create_screen_wifi(); // Init de la page WiFi

  lv_scr_load(screen_meter); // Load Meter first
  current_page_index = 0;
}

// --- PAGE NAVIGATION ---
void ui_linky_change_page(int direction) {
  printf("UI PAGE CHANGE CMD: %d\n", direction);
  current_page_index += direction;

  if (current_page_index > 5) current_page_index = 0;
  if (current_page_index < 0) current_page_index = 5;

  switch (current_page_index) {
    case 0: lv_scr_load_anim(screen_meter, LV_SCR_LOAD_ANIM_NONE, 0, 0, false); break;
    case 1: lv_scr_load_anim(screen_index, LV_SCR_LOAD_ANIM_NONE, 0, 0, false); break;
    case 2: lv_scr_load_anim(screen_week, LV_SCR_LOAD_ANIM_NONE, 0, 0, false); break;
    case 3: lv_scr_load_anim(screen_history, LV_SCR_LOAD_ANIM_NONE, 0, 0, false); break;
    case 4: lv_scr_load_anim(screen_info, LV_SCR_LOAD_ANIM_NONE, 0, 0, false); break;
    case 5: lv_scr_load_anim(screen_wifi, LV_SCR_LOAD_ANIM_NONE, 0, 0, false); break;
  }
} // End change_page (added wifi case)

// --- UPDATE LOOP ---
void ui_linky_update(linky_data_t * data) {
  // Time Update
  time_t now;
  time(&now);
  struct tm *t = localtime(&now);

  char buf_time[16];
  char buf_date[16];
  
  if (t->tm_year >= 120) { // Time valid (>= 2020)
      snprintf(buf_time, sizeof(buf_time), "%02d:%02d", t->tm_hour, t->tm_min);
      snprintf(buf_date, sizeof(buf_date), "%02d/%02d/%04d", t->tm_mday, t->tm_mon+1, t->tm_year+1900);
      
      // Update dynamic highlights
      if (highlight_week) {
          int wday = (t->tm_wday == 0) ? 6 : (t->tm_wday - 1);
          float angle_deg = -90.0f + (wday * 360.0f / 7.0f);
          float angle_rad = angle_deg * (PI / 180.0f);
          int lx = 165 + (int)(cos(angle_rad) * 160);
          int ly = 165 + (int)(sin(angle_rad) * 160);
          lv_obj_set_pos(highlight_week, lx - 12, ly - 13);
          lv_obj_clear_flag(highlight_week, LV_OBJ_FLAG_HIDDEN);
      }
      
      if (highlight_month) {
          int mon = t->tm_mon;
          float angle_deg = (mon + 1) * 30; 
          float angle_rad = angle_deg * (PI / 180.0f);
          int lx = 165 + (int)(sin(angle_rad) * 160);
          int ly = 165 - (int)(cos(angle_rad) * 160);
          lv_obj_set_pos(highlight_month, lx - 12, ly - 13);
          lv_obj_clear_flag(highlight_month, LV_OBJ_FLAG_HIDDEN);
      }
      
  } else {
      snprintf(buf_time, sizeof(buf_time), "--:--");
      snprintf(buf_date, sizeof(buf_date), "--/--/----");
  }
  
  if (label_time) {
      lv_obj_set_style_text_color(label_time, lv_color_white(), 0); 
      lv_label_set_text(label_time, buf_time);
  }

  if (label_date) {
      lv_obj_set_style_text_color(label_date, lv_color_white(), 0);
      const char* week_days[] = {"DIMANCHE", "LUNDI", "MARDI", "MERCREDI", "JEUDI", "VENDREDI", "SAMEDI"};
      const char* months[] = {"JANVIER", "FEVRIER", "MARS", "AVRIL", "MAI", "JUIN", "JUILLET", "AOUT", "SEPTEMBRE", "OCTOBRE", "NOVEMBRE", "DECEMBRE"};
      lv_label_set_text_fmt(label_date, "%s %d %s\n%d", week_days[t->tm_wday], t->tm_mday, months[t->tm_mon], 1900 + t->tm_year);
  }

  // Value Update
  if (label_papp_val) {
      float kw = data->papp / 1000.0f;
      int i_kw = (int)kw;
      int d_kw = (int)((kw - i_kw) * 10);
      lv_label_set_text_fmt(label_papp_val, "%d.%d", i_kw, d_kw);
      lv_obj_align_to(label_papp_unit, label_papp_val, LV_ALIGN_OUT_RIGHT_BOTTOM, 5, -3);
  }

  // Index
  if (label_index_base) lv_label_set_text_fmt(label_index_base, "BASE: %lu", (unsigned long)data->index_base);
  if (label_index_hp) lv_label_set_text_fmt(label_index_hp, "HP: %lu", (unsigned long)data->index_hp);
  if (label_index_hc) lv_label_set_text_fmt(label_index_hc, "HC: %lu", (unsigned long)data->index_hc);

  // Info Stats
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

    // Refresh WEEK page if needed (redessiner les lignes si de nouvelles données sont recoltées par la SD)
    // Seules quelques redessins sont necessaires (la boucle est lourde donc optionnelle 
    // ou on la met a jour seulement a minuit, mais bon on update ici)
    // Actually, to avoid overloading the CPU, since SD data only updates at midnight,
    // we don't necessarily need to update the 7*12 lines 10 times a second here.
    // The initial create screens have already grabbed the global SD data !



  // Bars Update Logic (Simulated)
  static uint32_t last_chart_upd = 0;
  static bool first_chart = true; 
  
  if (meter_bars[0]) {
       if (first_chart || (lv_tick_get() - last_chart_upd > 10 * 60 * 1000)) {
           // Shift & Add
           for(int i=0; i<5; i++) meter_values[i] = meter_values[i+1];
           meter_values[5] = data->papp; 
           last_chart_upd = lv_tick_get();
           first_chart = false;
           
           for(int i=0; i<6; i++) {
               int val = meter_values[i];
               if(val > 9000) val = 9000;
               int h = (val * 100) / 9000;
               if(h < 5) h = 5;

               lv_obj_set_height(meter_bars[i], h);
               
               float kw_bar = val / 1000.0f;
               if (meter_labels_val[i]) {
                   int i_b = (int)kw_bar; int d_b = (int)((kw_bar - i_b)*10);
                   lv_label_set_text_fmt(meter_labels_val[i], "%d.%d", i_b, d_b);
                   lv_obj_align_to(meter_labels_val[i], meter_bars[i], LV_ALIGN_OUT_TOP_MID, 0, -5);
               }
           }
       }
  }
}
