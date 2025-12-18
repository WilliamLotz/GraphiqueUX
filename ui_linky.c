#include "ui_linky.h"

// Variables Globales UI
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

static lv_style_t style_text_large;
static lv_style_t style_text_xl;

// --- Helper Helper ---
void style_init() {
    lv_style_init(&style_text_large);
    // Zoom not supported by default font engine often
    // lv_style_set_transform_zoom(&style_text_large, 384); 
    
    lv_style_init(&style_text_xl);
    // lv_style_set_transform_zoom(&style_text_xl, 512);
}

// --- Screens ---

void create_screen_meter() {
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
}
