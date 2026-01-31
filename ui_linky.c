#include "ui_linky.h"
#include <math.h>
#include <time.h>
#include <stdio.h>

#ifndef PI
#define PI 3.14159265358979323846
#endif

// --- GLOBALS ---
static lv_obj_t *screen_meter;
static lv_obj_t *screen_index;
static lv_obj_t *screen_week;    // Page 2
static lv_obj_t *screen_history; // Page 3
static lv_obj_t *screen_info;    // Page 4

static int current_page_index = 0; // 0=Meter, 1=Index, 2=Week, 3=History, 4=Info

// Styles
static lv_style_t style_text_large;
static lv_style_t style_text_xl;

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


// --- INIT STYLES ---
void style_init() {
  lv_style_init(&style_text_large);
  // Zoom not always supported, rely on fonts if possible, or keep simple
  // lv_style_set_transform_zoom(&style_text_large, 384); 

  lv_style_init(&style_text_xl);
  // lv_style_set_transform_zoom(&style_text_xl, 512); 
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

    int cx = 165; // Correct center for 360px screen
    int cy = 165; 
    int radius_start = 70; 
    int max_len = 70;

    for(int i=0; i<7; i++) {
        float angle_deg = -90.0f + (i * 360.0f / 7.0f);
        float angle_rad = angle_deg * (PI / 180.0f);
        
        int value = (rand() % 50) + 10; 

        // Hack Color: R->B, G->R, B->G
        uint8_t ratio = (value * 255) / 60; 
        if(ratio > 255) ratio = 255;
        // Want: Blue(Low) -> Green(High)
        // Send: Red -> Blue ?? 
        // Let's copy the Logic from History:
        // Low: Send Blue -> Show Green?? No, History said:
        // "Low: Send Blue (0,0,255) -> Show Green"
        // "High: Send Green (0,255,0) -> Show Red"
        // Let's use that (Blue->Green send).
        
        // CYAN FIXE
        lv_color_t color = lv_color_make(0, 255, 255);

        int bar_len = (value * max_len) / 60;
        
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

        if (i == 0) { // Active day highlight
             lv_obj_t * highlight = lv_obj_create(radial_area);
             lv_obj_set_size(highlight, 24, 24);
             lv_obj_set_style_radius(highlight, LV_RADIUS_CIRCLE, 0);
             // Hack: 0xFF0000 gives Blue on this screen?
             lv_obj_set_style_bg_color(highlight, lv_color_hex(0xFF0000), 0); 
             lv_obj_set_style_border_width(highlight, 0, 0);
             lv_obj_set_pos(highlight, lx - 12, ly - 13);
             lv_obj_clear_flag(highlight, LV_OBJ_FLAG_SCROLLABLE);
        }

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

    int cx = 165; 
    int cy = 165; 
    
    for(int i=0; i<12; i++) {
        float angle_deg = (i + 1) * 30; 
        float angle_rad = angle_deg * (PI / 180.0f);

        int value = (rand() % 50) + 20; 
        
        // Color Hack
        uint8_t ratio = (value * 255) / 60; 
        if (ratio > 255) ratio = 255;
        
        // CYAN FIXE
        lv_color_t bar_color = lv_color_make(0, 255, 255);

        int hole = 70; 
        int x_start = cx + (int)(sin(angle_rad) * hole);
        int y_start = cy - (int)(cos(angle_rad) * hole);
        int x_end = cx + (int)(sin(angle_rad) * (hole + value));
        int y_end = cy - (int)(cos(angle_rad) * (hole + value));

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

        if (i == 0) {
             lv_obj_t * highlight = lv_obj_create(radial_area);
             lv_obj_set_size(highlight, 24, 24);
             lv_obj_set_style_radius(highlight, LV_RADIUS_CIRCLE, 0);
             lv_obj_set_style_bg_color(highlight, lv_color_hex(0xFF0000), 0); 
             lv_obj_set_style_border_width(highlight, 0, 0);
             lv_obj_set_pos(highlight, lx - 12, ly - 13); 
             lv_obj_clear_flag(highlight, LV_OBJ_FLAG_SCROLLABLE);
        }
        
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

// --- MAIN INIT ---
void ui_linky_init() {
  style_init();

  create_screen_meter();
  create_screen_index();
  create_screen_week();
  create_screen_history();
  create_screen_info();

  lv_scr_load(screen_meter); // Load Meter first
  current_page_index = 0;
}

// --- PAGE NAVIGATION ---
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

// --- UPDATE LOOP ---
void ui_linky_update(linky_data_t *data) {
  // Time Update
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
