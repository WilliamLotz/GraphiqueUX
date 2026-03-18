#include "ui_linky.h"
#include <math.h>
#include <time.h>
#include <stdio.h>

#ifndef PI
#define PI 3.14159265358979323846
#endif

extern donnees_linky_t donnees_linky; // Ajout pour acceder aux données globales des historiques de la SD

// --- GLOBALES ---
static lv_obj_t *ecran_jauge;
static lv_obj_t *ecran_index;
static lv_obj_t *ecran_semaine;    // Page 2
static lv_obj_t *ecran_historique; // Page 3
static lv_obj_t *ecran_info;    // Page 4
static lv_obj_t *ecran_wifi;    // Page 5

static lv_obj_t *surbrillance_semaine = NULL;
static lv_obj_t *surbrillance_mois = NULL;

static int index_page_actuelle = 0; // 0=Jauge, 1=Index, 2=Semaine, 3=Historique, 4=Info, 5=WiFi

// Styles
static lv_style_t style_texte_grand;
static lv_style_t style_texte_tg;
static lv_style_t style_clavier; // Style pour le clavier (touches)
static lv_style_t style_clavier_principal; // Style pour le clavier (conteneur)

// Meter Widgets
static lv_obj_t *etiquette_heure;
static lv_obj_t *etiquette_date;
static lv_obj_t *etiquette_papp_valeur; 
static lv_obj_t *etiquette_papp_unite;
static lv_obj_t *barres_jauge[6];
static lv_obj_t *etiquettes_jauge_valeur[6];
static lv_obj_t *etiquettes_jauge_temps[6];
static int valeurs_jauge[6] = {0}; 

// Widgets d'index
static lv_obj_t *etiquette_index_base;
static lv_obj_t *etiquette_index_hp;
static lv_obj_t *etiquette_index_hc;

// Widgets d'information
static lv_obj_t *etiquette_tension;
static lv_obj_t *etiquette_stats_tension;
static lv_obj_t *etiquette_intensite;
static lv_obj_t *etiquette_stats_intensite;
// Données statistiques
static uint16_t tension_min = 999, tension_max = 0;
static uint32_t tension_somme = 0, tension_compteur = 0;
static uint16_t intensite_min = 999, intensite_max = 0;
static uint32_t intensite_somme = 0, intensite_compteur = 0;

// Historique
static lv_point_t points_historique[12][2]; 
static lv_obj_t * lignes_historique[12];     
static lv_obj_t * etiquettes_mois[12];      
const char* lettres_mois[12] = {"J", "F", "M", "A", "M", "J", "J", "A", "S", "O", "N", "D"};

// Semaine
static lv_point_t points_semaine[7][2];
static lv_obj_t * lignes_semaine[7];
static lv_obj_t * etiquettes_jours[7];
const char* noms_jours[7] = {"L", "M", "M", "J", "V", "S", "D"};

// Widgets WiFi
static lv_obj_t *zone_texte_reseau;
static lv_obj_t *zone_texte_mdp;
static lv_obj_t *clavier_virtuel;
static lv_obj_t *bouton_connexion;
static lv_obj_t *etiquette_statut_wifi;
char wifi_reseau[32] = {0};
char wifi_mdp[32] = {0};
volatile bool connexion_wifi_demandee = false;

// Cartes de disposition AZERTY (5 lignes pour un meilleur espacement)
// Cartes de disposition AZERTY (Corrigé avec la logique de majuscule)
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


// Initialise les styles globaux appliqués aux textes et éléments du clavier virtuel.
void style_init() {
  lv_style_init(&style_texte_grand);
  // Le zoom n'est pas toujours supporté, s'appuyer sur les polices si possible, ou garder simple
  // lv_style_set_transform_zoom(&style_texte_grand, 384); 

  lv_style_init(&style_texte_tg);
  // lv_style_set_transform_zoom(&style_texte_tg, 512); 

  // Style Clavier (Touches)
  lv_style_init(&style_clavier);
  lv_style_set_text_font(&style_clavier, &lv_font_montserrat_16);
  lv_style_set_radius(&style_clavier, 3); // Rayon plus petit

  // Style Clavier (Conteneur/Espacements)
  lv_style_init(&style_clavier_principal);
  lv_style_set_pad_row(&style_clavier_principal, 1);    // Petit espace (1px)
  lv_style_set_pad_column(&style_clavier_principal, 1); // Petit espace (1px)
  lv_style_set_pad_all(&style_clavier_principal, 1);    // Marge minime
  lv_style_set_pad_bottom(&style_clavier_principal, 10); // Espace vide en bas pour éviter la zone tactile difficile
  lv_style_set_radius(&style_clavier_principal, 0);
}

// Crée l'écran principal (Page 0) affichant la jauge, l'heure, la date et la puissance instantanée.
void create_screen_meter() {
  ecran_jauge = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(ecran_jauge, lv_color_black(), 0); 
  
  // Heure d'initialisation
  time_t maintenant;
  time(&maintenant);
  struct tm *temps = localtime(&maintenant);

  // Heure dans l'en-tête
  etiquette_heure = lv_label_create(ecran_jauge);
  lv_label_set_text_fmt(etiquette_heure, "%02d:%02d", temps->tm_hour, temps->tm_min);
  lv_obj_set_style_text_color(etiquette_heure, lv_color_white(), 0);
  lv_obj_set_style_text_font(etiquette_heure, &lv_font_montserrat_30, 0); 
  lv_obj_align(etiquette_heure, LV_ALIGN_TOP_MID, 0, 40);

  // Date
  etiquette_date = lv_label_create(ecran_jauge);
  const char* jours_semaine[] = {"DIMANCHE", "LUNDI", "MARDI", "MERCREDI", "JEUDI", "VENDREDI", "SAMEDI"};
  const char* mois_annee[] = {"JANVIER", "FEVRIER", "MARS", "AVRIL", "MAI", "JUIN", "JUILLET", "AOUT", "SEPTEMBRE", "OCTOBRE", "NOVEMBRE", "DECEMBRE"};
  lv_label_set_text_fmt(etiquette_date, "%s %d %s\n%d", jours_semaine[temps->tm_wday], temps->tm_mday, mois_annee[temps->tm_mon], 1900 + temps->tm_year);
  
  lv_obj_set_style_text_align(etiquette_date, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_color(etiquette_date, lv_color_white(), 0); 
  lv_obj_set_style_text_font(etiquette_heure, &lv_font_montserrat_20, 0);
  lv_obj_align_to(etiquette_date, etiquette_heure, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

  // Boîte centrale
  lv_obj_t *boite = lv_obj_create(ecran_jauge);
  lv_obj_set_size(boite, 260, 90);
  lv_obj_align(boite, LV_ALIGN_TOP_MID, 0, 110); 
  lv_obj_set_style_bg_color(boite, lv_color_black(), 0); 
  lv_obj_set_style_border_color(boite, lv_color_make(0, 0, 255), 0); // Ajustement temporaire de la couleur
  lv_obj_set_style_border_width(boite, 1, 0);
  lv_obj_set_style_radius(boite, 20, 0);
  lv_obj_clear_flag(boite, LV_OBJ_FLAG_SCROLLABLE);

  // Valeur
  etiquette_papp_valeur = lv_label_create(boite);
  lv_label_set_text(etiquette_papp_valeur, "0.4");
  lv_obj_set_style_text_font(etiquette_papp_valeur, &lv_font_montserrat_40, 0);
  lv_obj_set_style_text_color(etiquette_papp_valeur, lv_color_white(), 0);
  lv_obj_align(etiquette_papp_valeur, LV_ALIGN_CENTER, -10, -8); 

  // Unité
  etiquette_papp_unite = lv_label_create(boite);
  lv_label_set_text(etiquette_papp_unite, "KW");
  lv_obj_set_style_text_font(etiquette_papp_unite, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(etiquette_papp_unite, lv_color_white(), 0);
  lv_obj_align_to(etiquette_papp_unite, etiquette_papp_valeur, LV_ALIGN_OUT_RIGHT_BOTTOM, 5, -5); 

  // Sous-titre
  lv_obj_t *etiquette_inst = lv_label_create(boite);
  lv_label_set_text(etiquette_inst, "INSTANT");
  lv_obj_set_style_text_color(etiquette_inst, lv_color_white(), 0);
  lv_obj_set_style_text_font(etiquette_inst, &lv_font_montserrat_16, 0); 
  lv_obj_align(etiquette_inst, LV_ALIGN_BOTTOM_MID, 0, 11);

  // BARRES
  int largeur_barre = 28;
  int espacement = 16;
  int x_depart = -((6 * largeur_barre + 5 * espacement) / 2) + (largeur_barre/2); 

  // Valeurs fictives
  int valeurs_simulees[6] = {500, 1500, 3200, 800, 4500, 2100};
  for(int k=0; k<6; k++) valeurs_jauge[k] = valeurs_simulees[k];

  for(int i=0; i<6; i++) {
      int val = valeurs_jauge[i];
      if(val > 9000) val = 9000;
      int hauteur = (val * 100) / 9000;
      if(hauteur < 5) hauteur = 5;

      barres_jauge[i] = lv_bar_create(ecran_jauge);
      lv_bar_set_range(barres_jauge[i], 0, 100);
      lv_bar_set_value(barres_jauge[i], 100, LV_ANIM_OFF);
      
      lv_obj_set_size(barres_jauge[i], largeur_barre, hauteur); // Hauteur dynamique
      lv_obj_set_style_radius(barres_jauge[i], 2, LV_PART_MAIN);      
      lv_obj_set_style_radius(barres_jauge[i], 2, LV_PART_INDICATOR); 
      
      // Cyan (0,255,255)
      lv_obj_set_style_bg_color(barres_jauge[i], lv_color_make(0, 255, 255), LV_PART_INDICATOR);
      lv_obj_set_style_bg_grad_color(barres_jauge[i], lv_color_make(0, 255, 255), LV_PART_INDICATOR); 
      lv_obj_set_style_bg_grad_dir(barres_jauge[i], LV_GRAD_DIR_NONE, LV_PART_INDICATOR);
      lv_obj_set_style_bg_opa(barres_jauge[i], LV_OPA_TRANSP, LV_PART_MAIN);

      int position_x = x_depart + i * (largeur_barre + espacement);
      
      // Effet de sourire
      int decalage_y = -15; 
      if(i==1 || i==4) decalage_y = 10; 
      if(i==2 || i==3) decalage_y = 25; 

      lv_obj_align(barres_jauge[i], LV_ALIGN_BOTTOM_MID, position_x, -50 + decalage_y); 

      // Étiquette de valeur
      etiquettes_jauge_valeur[i] = lv_label_create(ecran_jauge);
      float kw_bar = val / 1000.0f;
      int i_b = (int)kw_bar; int d_b = (int)((kw_bar - i_b)*10);
      lv_label_set_text_fmt(etiquettes_jauge_valeur[i], "%d.%d", i_b, d_b);
      lv_obj_set_style_text_color(etiquettes_jauge_valeur[i], lv_color_white(), 0);
      lv_obj_align_to(etiquettes_jauge_valeur[i], barres_jauge[i], LV_ALIGN_OUT_TOP_MID, 0, -5);

      // Étiquette de temps
      etiquettes_jauge_temps[i] = lv_label_create(ecran_jauge);
      lv_label_set_text_fmt(etiquettes_jauge_temps[i], "%d", (6-i)*10); 
      lv_obj_set_style_text_color(etiquettes_jauge_temps[i], lv_color_white(), 0); 
      lv_obj_align_to(etiquettes_jauge_temps[i], barres_jauge[i], LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
  }
}

// Crée l'écran des compteurs (Page 1) affichant les index Base, Heures Pleines et Heures Creuses.
void create_screen_index() {
  ecran_index = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(ecran_index, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(ecran_index, LV_OPA_COVER, 0);

  lv_obj_t *titre = lv_label_create(ecran_index);
  lv_label_set_text(titre, "INDEX kWh");
  lv_obj_set_style_text_color(titre, lv_color_white(), 0);
  lv_obj_align(titre, LV_ALIGN_TOP_MID, 0, 30);
  lv_obj_set_style_text_letter_space(titre, 2, 0); 

  // Base
  etiquette_index_base = lv_label_create(ecran_index);
  lv_label_set_text(etiquette_index_base, "BASE: -----");
  lv_obj_set_style_text_color(etiquette_index_base, lv_color_white(), 0);
  lv_obj_align(etiquette_index_base, LV_ALIGN_CENTER, 0, -40); 
  lv_obj_set_style_text_letter_space(etiquette_index_base, 1, 0);

  // Heures Pleines
  etiquette_index_hp = lv_label_create(ecran_index);
  lv_label_set_text(etiquette_index_hp, "HP: -----");
  lv_obj_set_style_text_color(etiquette_index_hp, lv_color_white(), 0);
  lv_obj_align(etiquette_index_hp, LV_ALIGN_CENTER, 0, 0); 
  lv_obj_set_style_text_letter_space(etiquette_index_hp, 1, 0);

  // Heures Creuses
  etiquette_index_hc = lv_label_create(ecran_index);
  lv_label_set_text(etiquette_index_hc, "HC: -----");
  lv_obj_set_style_text_color(etiquette_index_hc, lv_color_white(), 0);
  lv_obj_align(etiquette_index_hc, LV_ALIGN_CENTER, 0, 40); 
  lv_obj_set_style_text_letter_space(etiquette_index_hc, 1, 0);
}

// Crée l'écran d'informations (Page 4) affichant les statistiques de tension et d'intensité.
void create_screen_info() {
  ecran_info = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(ecran_info, lv_color_black(), 0);
  
  lv_obj_t * conteneur = lv_obj_create(ecran_info);
  lv_obj_set_size(conteneur, 240, 240);
  lv_obj_center(conteneur);
  lv_obj_set_flex_flow(conteneur, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(conteneur, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_bg_opa(conteneur, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(conteneur, 0, 0);
  lv_obj_set_style_pad_row(conteneur, 10, 0);
  lv_obj_clear_flag(conteneur, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *titre = lv_label_create(conteneur);
  lv_label_set_text(titre, "INFOS");
  lv_obj_set_style_text_color(titre, lv_color_white(), 0);
  lv_obj_add_style(titre, &style_texte_grand, 0);

  // Volts
  lv_obj_t *etiquette_v = lv_label_create(conteneur);
  lv_label_set_text(etiquette_v, "TENSION");
  lv_obj_set_style_text_color(etiquette_v, lv_color_white(), 0);
  lv_obj_add_style(etiquette_v, &style_texte_grand, 0);
  
  etiquette_tension = lv_label_create(conteneur);
  lv_label_set_text(etiquette_tension, "--- V");
  lv_obj_set_style_text_color(etiquette_tension, lv_color_white(), 0);
  lv_obj_add_style(etiquette_tension, &style_texte_tg, 0);

  etiquette_stats_tension = lv_label_create(conteneur);
  lv_label_set_text(etiquette_stats_tension, "Min:-      Avg:-      Max:-");
  lv_obj_set_style_text_color(etiquette_stats_tension, lv_color_white(), 0); 
  lv_obj_set_style_text_font(etiquette_stats_tension, LV_FONT_DEFAULT, 0);
  lv_obj_set_style_pad_bottom(etiquette_stats_tension, 10, 0);

  // Ampères
  lv_obj_t *etiquette_a = lv_label_create(conteneur);
  lv_label_set_text(etiquette_a, "INTENSITE");
  lv_obj_set_style_text_color(etiquette_a, lv_color_white(), 0);
  lv_obj_add_style(etiquette_a, &style_texte_grand, 0);

  etiquette_intensite = lv_label_create(conteneur);
  lv_label_set_text(etiquette_intensite, "-- A");
  lv_obj_set_style_text_color(etiquette_intensite, lv_color_white(), 0);
  lv_obj_add_style(etiquette_intensite, &style_texte_tg, 0);

  etiquette_stats_intensite = lv_label_create(conteneur);
  lv_label_set_text(etiquette_stats_intensite, "Min:-      Avg:-      Max:-");
  lv_obj_set_style_text_color(etiquette_stats_intensite, lv_color_white(), 0); 
  lv_obj_set_style_text_font(etiquette_stats_intensite, LV_FONT_DEFAULT, 0);
}

// Crée l'écran de la semaine (Page 2) avec un graphique circulaire sur les 7 derniers jours.
void create_screen_week() {
    ecran_semaine = lv_obj_create(NULL);
    lv_obj_set_style_pad_all(ecran_semaine, 0, 0); 
    lv_obj_set_style_bg_color(ecran_semaine, lv_color_black(), 0);

    lv_obj_t * zone_radiale = lv_obj_create(ecran_semaine);
    lv_obj_set_size(zone_radiale, 360, 360);
    lv_obj_center(zone_radiale);
    lv_obj_set_style_bg_opa(zone_radiale, LV_OPA_TRANSP, 0); 
    lv_obj_set_style_border_width(zone_radiale, 0, 0);
    lv_obj_clear_flag(zone_radiale, LV_OBJ_FLAG_SCROLLABLE);

    // Disque central
    lv_obj_t * disque_central = lv_obj_create(ecran_semaine);
    lv_obj_set_size(disque_central, 120, 120);
    lv_obj_set_pos(disque_central, 120, 120); // Position manuelle centrée (environ 360/2 - 60 = 120) -> Mais cx est 165 ? 
    // Attendez, 120,120 est relatif à l'écran (240x240 d'habitude ?) Non, écran S3 360x360.
    // Si l'écran est un cercle 360x360. Le centre est 180,180.
    // Disque 120x120. HautGauche = 180-60=120. Donc 120,120 est correct pour le centre absolu de l'écran 360px.
    lv_obj_set_style_radius(disque_central, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(disque_central, lv_color_white(), 0);
    lv_obj_set_style_border_width(disque_central, 0, 0);

    lv_obj_t * ombre_titre = lv_label_create(ecran_semaine);
    lv_label_set_text(ombre_titre, "CONSO\nSEMAINE\nkWh");
    lv_obj_set_style_text_align(ombre_titre, LV_TEXT_ALIGN_CENTER, 0); 
    lv_obj_align(ombre_titre, LV_ALIGN_CENTER, 0, 0); 
    lv_obj_set_style_text_color(ombre_titre, lv_color_black(), 0); 

    // Styles
    static lv_style_t style_ligne_semaine;
    lv_style_init(&style_ligne_semaine);
    lv_style_set_line_width(&style_ligne_semaine, 30); 
    lv_style_set_line_rounded(&style_ligne_semaine, true);

    time_t maintenant_semaine;
    time(&maintenant_semaine);
    struct tm *temps_semaine = localtime(&maintenant_semaine);
    int index_jour_actuel = (temps_semaine->tm_wday == 0) ? 6 : (temps_semaine->tm_wday - 1); // 0=Lundi, 6=Dimanche

    int centre_x = 165; // Centre correct pour l'écran 360px
    int centre_y = 165; 
    int rayon_depart = 70; 
    int longueur_max = 70;

    // Create global highlight, initially hidden (creé AVANT les textes pour etre EN DESSOUS)
    surbrillance_semaine = lv_obj_create(zone_radiale);
    lv_obj_set_size(surbrillance_semaine, 24, 24);
    lv_obj_set_style_radius(surbrillance_semaine, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(surbrillance_semaine, lv_color_hex(0xFF0000), 0); 
    lv_obj_set_style_border_width(surbrillance_semaine, 0, 0);
    lv_obj_clear_flag(surbrillance_semaine, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(surbrillance_semaine, LV_OBJ_FLAG_HIDDEN);

    for(int i=0; i<7; i++) {
        float angle_deg = -90.0f + (i * 360.0f / 7.0f);
        float angle_rad = angle_deg * (PI / 180.0f);
        
        int valeur = donnees_linky.historique_semaine[i]; // Utiliser la vraie donnee depuis la carte SD (kWh)

        // Limiter la valeur pour l'affichage (ex: max 60 kWh/jour)
        int echelle_val_max = 60;
        int val_affichage = valeur;
        if(val_affichage > echelle_val_max) val_affichage = echelle_val_max;

        // Ajustement de couleur: Vert (Faible) -> Rouge (Fort) ! (rappel: ecran inverse RGB)
        // Ratio de consommation
        uint8_t ratio = (val_affichage * 255) / echelle_val_max; 
        
        // Nous voulons Vert (Faible) -> Rouge (Fort). 
        // Mais l'écran inverse les couleurs:
        // Envoyer Bleu (0,0,255) => Affiche Vert
        // Envoyer Vert (0,255,0) => Affiche Bleu
        // Envoyer Rouge (255,0,0) => Affiche Cyan ? Attendez.
        // D'après l'autre code : Le vert sur l'écran est Bleu (0,0,255). Le rouge sur l'écran est Vert (0,255,0).
        lv_color_t couleur = lv_color_make(0, ratio, 255 - ratio);

        int longueur_barre = (val_affichage * longueur_max) / echelle_val_max;
        
        points_semaine[i][0].x = centre_x + (int)(cos(angle_rad) * rayon_depart);
        points_semaine[i][0].y = centre_y + (int)(sin(angle_rad) * rayon_depart);
        points_semaine[i][1].x = centre_x + (int)(cos(angle_rad) * (rayon_depart + longueur_barre));
        points_semaine[i][1].y = centre_y + (int)(sin(angle_rad) * (rayon_depart + longueur_barre));

        lignes_semaine[i] = lv_line_create(zone_radiale);
        lv_line_set_points(lignes_semaine[i], points_semaine[i], 2);
        lv_obj_add_style(lignes_semaine[i], &style_ligne_semaine, 0);
        lv_obj_set_style_line_color(lignes_semaine[i], couleur, 0);

        int rayon_etiquette = 160;
        int coord_x = centre_x + (int)(cos(angle_rad) * rayon_etiquette);
        int coord_y = centre_y + (int)(sin(angle_rad) * rayon_etiquette);

        etiquettes_jours[i] = lv_label_create(zone_radiale); 
        lv_label_set_text(etiquettes_jours[i], noms_jours[i]);
        lv_obj_set_style_text_color(etiquettes_jours[i], lv_color_white(), 0);
        lv_obj_set_pos(etiquettes_jours[i], coord_x - 7, coord_y - 7);
        
        lv_obj_t * etiquette_val = lv_label_create(zone_radiale);
        lv_label_set_text_fmt(etiquette_val, "%d", valeur);  // En kW direct
        lv_obj_set_style_text_color(etiquette_val, lv_color_black(), 0); 
        
        // Rapprochement Centre (Dans le disque blanc)
        int rayon_valeur = 50; 
        int val_x = centre_x + (int)(cos(angle_rad) * rayon_valeur);
        int val_y = centre_y + (int)(sin(angle_rad) * rayon_valeur);
        lv_obj_set_pos(etiquette_val, val_x - 10, val_y - 7);
    }
}

// Crée l'écran de l'historique annuel (Page 3) avec un graphique circulaire des 12 derniers mois.
void create_screen_history() {
    ecran_historique = lv_obj_create(NULL);
    lv_obj_set_style_pad_all(ecran_historique, 0, 0); 
    lv_obj_set_style_bg_color(ecran_historique, lv_color_black(), 0); // Important pour l'arrière-plan

    lv_obj_t * zone_radiale = lv_obj_create(ecran_historique);
    lv_obj_set_size(zone_radiale, 360, 360);
    lv_obj_center(zone_radiale);
    lv_obj_set_style_bg_opa(zone_radiale, LV_OPA_TRANSP, 0); 
    lv_obj_set_style_border_width(zone_radiale, 0, 0);
    lv_obj_clear_flag(zone_radiale, LV_OBJ_FLAG_SCROLLABLE);

    // Disque central
    lv_obj_t * disque_central = lv_obj_create(ecran_historique);
    lv_obj_set_size(disque_central, 120, 120);
    lv_obj_center(disque_central);
    lv_obj_set_style_radius(disque_central, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(disque_central, lv_color_white(), 0);
    lv_obj_set_style_border_width(disque_central, 0, 0);

    lv_obj_t * ombre_titre = lv_label_create(ecran_historique);
    lv_label_set_text(ombre_titre, "CONSO\nANNUELLE\nkWh");
    lv_obj_set_style_text_align(ombre_titre, LV_TEXT_ALIGN_CENTER, 0); 
    lv_obj_align(ombre_titre, LV_ALIGN_CENTER, 0, 0); 
    lv_obj_set_style_text_color(ombre_titre, lv_color_black(), 0); 
    lv_obj_add_style(ombre_titre, &style_texte_grand, 0);

    static lv_style_t style_ligne;
    lv_style_init(&style_ligne);
    lv_style_set_line_width(&style_ligne, 28); 
    lv_style_set_line_rounded(&style_ligne, true);

    time_t maintenant_annuel;
    time(&maintenant_annuel);
    struct tm *temps_annuel = localtime(&maintenant_annuel);
    int index_mois_actuel = temps_annuel->tm_mon; // 0=Janvier, 11=Decembre

    int centre_x = 165; 
    int centre_y = 165; 

    // Create global highlight, initially hidden (creé AVANT les textes pour etre EN DESSOUS)
    surbrillance_mois = lv_obj_create(zone_radiale);
    lv_obj_set_size(surbrillance_mois, 24, 24);
    lv_obj_set_style_radius(surbrillance_mois, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(surbrillance_mois, lv_color_hex(0xFF0000), 0); 
    lv_obj_set_style_border_width(surbrillance_mois, 0, 0);
    lv_obj_clear_flag(surbrillance_mois, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(surbrillance_mois, LV_OBJ_FLAG_HIDDEN);
    
    for(int i=0; i<12; i++) {
        float angle_deg = (i + 1) * 30; 
        float angle_rad = angle_deg * (PI / 180.0f);

        int valeur_wh = donnees_linky.historique_annee[i]; 
        int valeur = valeur_wh / 1000; // Passage en kWh pour le graph
        
        int echelle_max = 300; // max 300 kWh par mois
        int val_affichage = valeur;
        if(val_affichage > echelle_max) val_affichage = echelle_max;

        uint8_t ratio = (val_affichage * 255) / echelle_max; 
        
        // Ajustement de couleur (Vert a Rouge)
        lv_color_t couleur_barre = lv_color_make(0, ratio, 255 - ratio);

        int trou = 70; 
        // Normaliser la taille de la barre
        int longueur_barre = (val_affichage * 70) / echelle_max;
        
        int x_debut = centre_x + (int)(sin(angle_rad) * trou);
        int y_debut = centre_y - (int)(cos(angle_rad) * trou);
        int x_fin = centre_x + (int)(sin(angle_rad) * (trou + longueur_barre));
        int y_fin = centre_y - (int)(cos(angle_rad) * (trou + longueur_barre));

        points_historique[i][0].x = x_debut;
        points_historique[i][0].y = y_debut;
        points_historique[i][1].x = x_fin;
        points_historique[i][1].y = y_fin;

        lignes_historique[i] = lv_line_create(zone_radiale);
        lv_line_set_points(lignes_historique[i], points_historique[i], 2);
        lv_obj_add_style(lignes_historique[i], &style_ligne, 0);
        lv_obj_set_style_line_color(lignes_historique[i], couleur_barre, 0);
        
        int rayon_etiquette = 160; 
        int coord_x = centre_x + (int)(sin(angle_rad) * rayon_etiquette);
        int coord_y = centre_y - (int)(cos(angle_rad) * rayon_etiquette);

        etiquettes_mois[i] = lv_label_create(zone_radiale); 
        lv_label_set_text(etiquettes_mois[i], lettres_mois[i]);
        lv_obj_set_pos(etiquettes_mois[i], coord_x - 6, coord_y - 9); 
        
        // Rapprochement Centre
        int rayon_valeur = 50; 
        int val_x = centre_x + (int)(sin(angle_rad) * rayon_valeur);
        int val_y = centre_y - (int)(cos(angle_rad) * rayon_valeur);

        lv_obj_t * etiquette_val = lv_label_create(zone_radiale);
        lv_label_set_text_fmt(etiquette_val, "%d", valeur); // En kW direct
        lv_obj_set_style_text_color(etiquette_val, lv_color_black(), 0); 
        lv_obj_set_pos(etiquette_val, val_x - 10, val_y - 7); 
    }
}

// Gère les événements tactiles sur les zones de texte (ouverture/fermeture clavier).
static void rappel_evenement_ta(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * zt = lv_event_get_target(e);
    if(code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED) {
        if(clavier_virtuel != NULL) {
            lv_keyboard_set_textarea(clavier_virtuel, zt);
            lv_obj_clear_flag(clavier_virtuel, LV_OBJ_FLAG_HIDDEN);
        }
    }
    if(code == LV_EVENT_DEFOCUSED) {
        if(clavier_virtuel != NULL) {
            lv_keyboard_set_textarea(clavier_virtuel, NULL);
            lv_obj_add_flag(clavier_virtuel, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

// Gère les interactions et la saisie sur le clavier virtuel (changement de layout, validation).
static void rappel_evenement_clavier(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj_clavier = lv_event_get_target(e);

    if(code == LV_EVENT_VALUE_CHANGED) {
        uint16_t id_bouton = lv_btnmatrix_get_selected_btn(obj_clavier);
        if(id_bouton == LV_BTNMATRIX_BTN_NONE) return;

        const char * txt = lv_btnmatrix_get_btn_text(obj_clavier, id_bouton);
        if(txt == NULL) return;

        lv_obj_t * zt = lv_keyboard_get_textarea(obj_clavier); // Obtenir la zone de texte active

        if(strcmp(txt, LV_SYMBOL_UP) == 0) {
            if(zt) lv_textarea_del_char(zt); // Supprimer le symbole inséré
            lv_keyboard_set_mode(obj_clavier, LV_KEYBOARD_MODE_TEXT_UPPER);
            lv_indev_wait_release(lv_indev_get_act()); // Empêcher le clic fantôme
        }
        else if(strcmp(txt, LV_SYMBOL_DOWN) == 0) {
            if(zt) lv_textarea_del_char(zt); // Supprimer le symbole inséré
            lv_keyboard_set_mode(obj_clavier, LV_KEYBOARD_MODE_TEXT_LOWER);
            lv_obj_set_height(obj_clavier, lv_pct(60)); // Retourner à la hauteur normale
           
        }
        else if(strcmp(txt, "1#") == 0) {
            if(zt) {
                lv_textarea_del_char(zt); // Supprimer #
                lv_textarea_del_char(zt); // Supprimer 1
            }
            lv_keyboard_set_mode(obj_clavier, LV_KEYBOARD_MODE_SPECIAL);
            lv_obj_set_height(obj_clavier, lv_pct(70)); // Plus haut pour les caractères spéciaux
            lv_indev_wait_release(lv_indev_get_act()); // Empêcher le clic fantôme
        }
        else if(strcmp(txt, "abc") == 0) {
            if(zt) {
                lv_textarea_del_char(zt); // c
                lv_textarea_del_char(zt); // b
                lv_textarea_del_char(zt); // a
            }
            lv_keyboard_set_mode(obj_clavier, LV_KEYBOARD_MODE_TEXT_LOWER);
            lv_obj_set_height(obj_clavier, lv_pct(60)); // Retourner à la hauteur normale
            lv_indev_wait_release(lv_indev_get_act()); // Empêcher le clic fantôme
        }
    }
    else if(code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        lv_obj_add_flag(obj_clavier, LV_OBJ_FLAG_HIDDEN);
    }
}

// Gère le clic sur le bouton "Connexion" pour lancer la requête de connexion WiFi et l'animation d'état.
static void rappel_evenement_bouton_connexion(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        // Sauvegarde des credentials (simulation pour l'instant)
        const char * texte_ssid = lv_textarea_get_text(zone_texte_reseau);
        const char * texte_mdp = lv_textarea_get_text(zone_texte_mdp);
        snprintf(wifi_reseau, sizeof(wifi_reseau), "%s", texte_ssid);
        snprintf(wifi_mdp, sizeof(wifi_mdp), "%s", texte_mdp);
        
        if (etiquette_statut_wifi) {
            lv_label_set_text(etiquette_statut_wifi, "Connexion en cours...");
            lv_obj_set_style_text_color(etiquette_statut_wifi, lv_color_make(255, 165, 0), 0); // Orange
            connexion_wifi_demandee = true;
        }
        printf("WiFi Connect Request: %s / %s\n", wifi_reseau, wifi_mdp);
    }
}

// Modifie le texte et la couleur du label de statut WiFi sur la page de configuration.
void interface_linky_statut_wifi(const char* msg, bool success) {
    if(etiquette_statut_wifi) {
        lv_label_set_text(etiquette_statut_wifi, msg);
        if(success) lv_obj_set_style_text_color(etiquette_statut_wifi, lv_color_make(0, 255, 0), 0); // Vert
        else lv_obj_set_style_text_color(etiquette_statut_wifi, lv_color_make(255, 0, 0), 0); // Rouge
    }
}

// Crée l'écran de configuration réseau (Page 5) incluant la saisie du SSID et du mot de passe WiFi.
void create_screen_wifi() {
    ecran_wifi = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(ecran_wifi, lv_color_black(), 0);
    
    // Titre
    lv_obj_t * titre = lv_label_create(ecran_wifi);
    lv_label_set_text(titre, "CONFIGURATION WIFI");
    lv_obj_set_style_text_color(titre, lv_color_white(), 0);
    lv_obj_align(titre, LV_ALIGN_TOP_MID, 0, 20);

    // Zone SSID
    zone_texte_reseau = lv_textarea_create(ecran_wifi);
    lv_obj_set_style_text_font(zone_texte_reseau, &lv_font_montserrat_20, 0); // Police agrandie
    lv_textarea_set_placeholder_text(zone_texte_reseau, "Nom du reseau");
    lv_obj_set_width(zone_texte_reseau, 220);
    lv_obj_align(zone_texte_reseau, LV_ALIGN_TOP_MID, 0, 60);
    lv_obj_add_event_cb(zone_texte_reseau, rappel_evenement_ta, LV_EVENT_ALL, NULL);

    // Zone de mot de passe
    zone_texte_mdp = lv_textarea_create(ecran_wifi);
    lv_obj_set_style_text_font(zone_texte_mdp, &lv_font_montserrat_20, 0); // Police agrandie
    lv_textarea_set_placeholder_text(zone_texte_mdp, "Mot de passe");
    lv_textarea_set_password_mode(zone_texte_mdp, false);
    lv_obj_set_width(zone_texte_mdp, 220);
    lv_obj_align(zone_texte_mdp, LV_ALIGN_TOP_MID, 0, 110);
    lv_obj_add_event_cb(zone_texte_mdp, rappel_evenement_ta, LV_EVENT_ALL, NULL);

    // Bouton de connexion
    bouton_connexion = lv_btn_create(ecran_wifi);
    lv_obj_set_width(bouton_connexion, 120);
    lv_obj_align(bouton_connexion, LV_ALIGN_TOP_MID, 0, 160);
    lv_obj_add_event_cb(bouton_connexion, rappel_evenement_bouton_connexion, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_color(bouton_connexion, lv_color_make(0, 100, 255), 0);

    lv_obj_t * etiquette_bouton = lv_label_create(bouton_connexion);
    lv_label_set_text(etiquette_bouton, "Connexion");
    lv_obj_center(etiquette_bouton);

    // Étiquette de statut
    etiquette_statut_wifi = lv_label_create(ecran_wifi);
    lv_label_set_text(etiquette_statut_wifi, "Pret");
    lv_obj_set_style_text_color(etiquette_statut_wifi, lv_color_make(0, 255, 0), 0);
    lv_obj_align(etiquette_statut_wifi, LV_ALIGN_TOP_MID, 0, 200);

    // Clavier (initialement masqué)
    clavier_virtuel = lv_keyboard_create(ecran_wifi);
    lv_obj_set_width(clavier_virtuel, lv_pct(100)); 
    lv_obj_set_height(clavier_virtuel, lv_pct(70));
    lv_obj_align(clavier_virtuel, LV_ALIGN_BOTTOM_MID, 0, -20); // Remonte de 20px pour eviter la zone difficile
    
    // Définir la configuration AZERTY
    lv_keyboard_set_map(clavier_virtuel, LV_KEYBOARD_MODE_TEXT_LOWER, kb_map_azerty_lc, kb_ctrl_azerty_lc_map);
    lv_keyboard_set_map(clavier_virtuel, LV_KEYBOARD_MODE_TEXT_UPPER, kb_map_azerty_uc, kb_ctrl_azerty_uc_map);
    lv_keyboard_set_map(clavier_virtuel, LV_KEYBOARD_MODE_SPECIAL, kb_map_spec, kb_ctrl_spec_map);
    lv_keyboard_set_mode(clavier_virtuel, LV_KEYBOARD_MODE_TEXT_LOWER);

    // Appliquer les styles
    lv_obj_add_style(clavier_virtuel, &style_clavier, LV_PART_ITEMS);
    lv_obj_add_style(clavier_virtuel, &style_clavier_principal, LV_PART_MAIN);

    lv_obj_add_event_cb(clavier_virtuel, rappel_evenement_clavier, LV_EVENT_ALL, NULL);
    lv_obj_add_flag(clavier_virtuel, LV_OBJ_FLAG_HIDDEN);
}

static void rappel_glissement_ecran(lv_event_t * e) {
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    if(dir == LV_DIR_LEFT) {
        interface_linky_changer_page(1);
    } else if(dir == LV_DIR_RIGHT) {
        interface_linky_changer_page(-1);
    }
}

// Initialise toutes les pages de l'interface graphique Linky et charge l'écran principal.
void interface_linky_initialisation() {
  style_init();

  create_screen_meter();
  create_screen_index();
  create_screen_week();
  create_screen_history();
  create_screen_info();
  create_screen_wifi(); // Initialisation de la page WiFi

  lv_obj_add_event_cb(ecran_jauge, rappel_glissement_ecran, LV_EVENT_GESTURE, NULL);
  lv_obj_clear_flag(ecran_jauge, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_add_event_cb(ecran_index, rappel_glissement_ecran, LV_EVENT_GESTURE, NULL);
  lv_obj_clear_flag(ecran_index, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_add_event_cb(ecran_semaine, rappel_glissement_ecran, LV_EVENT_GESTURE, NULL);
  lv_obj_clear_flag(ecran_semaine, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_add_event_cb(ecran_historique, rappel_glissement_ecran, LV_EVENT_GESTURE, NULL);
  lv_obj_clear_flag(ecran_historique, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_add_event_cb(ecran_info, rappel_glissement_ecran, LV_EVENT_GESTURE, NULL);
  lv_obj_clear_flag(ecran_info, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_add_event_cb(ecran_wifi, rappel_glissement_ecran, LV_EVENT_GESTURE, NULL);
  lv_obj_clear_flag(ecran_wifi, LV_OBJ_FLAG_SCROLLABLE);

  lv_scr_load(ecran_jauge); // Charger le compteur en premier
  index_page_actuelle = 0;
}

// Change la page affichée à l'écran (direction = +1 ou -1) selon la navigation.
void interface_linky_changer_page(int direction) {
  printf("UI PAGE CHANGE CMD: %d\n", direction);
  index_page_actuelle += direction;

  if (index_page_actuelle > 5) index_page_actuelle = 0;
  if (index_page_actuelle < 0) index_page_actuelle = 5;

  lv_scr_load_anim_t anim_type = (direction > 0) ? LV_SCR_LOAD_ANIM_MOVE_LEFT : LV_SCR_LOAD_ANIM_MOVE_RIGHT;
  if (direction == 0) anim_type = LV_SCR_LOAD_ANIM_NONE;

  switch (index_page_actuelle) {
    case 0: lv_scr_load_anim(ecran_jauge, anim_type, 300, 0, false); break;
    case 1: lv_scr_load_anim(ecran_index, anim_type, 300, 0, false); break;
    case 2: lv_scr_load_anim(ecran_semaine, anim_type, 300, 0, false); break;
    case 3: lv_scr_load_anim(ecran_historique, anim_type, 300, 0, false); break;
    case 4: lv_scr_load_anim(ecran_info, anim_type, 300, 0, false); break;
    case 5: lv_scr_load_anim(ecran_wifi, anim_type, 300, 0, false); break;
  }
} // Fin de change_page (cas wifi ajouté)

// Met à jour les éléments graphiques et historiques de l'UI avec les dernières données Linky.
void interface_linky_actualiser(donnees_linky_t * data) {
  // Mise à jour de l'heure
  time_t maintenant;
  time(&maintenant);
  struct tm *temps = localtime(&maintenant);

  char tampon_heure[16];
  char tampon_date[16];
  
  if (temps->tm_year >= 120) { // Heure valide (>= 2020)
      snprintf(tampon_heure, sizeof(tampon_heure), "%02d:%02d", temps->tm_hour, temps->tm_min);
      snprintf(tampon_date, sizeof(tampon_date), "%02d/%02d/%04d", temps->tm_mday, temps->tm_mon+1, temps->tm_year+1900);
      
      // Mettre à jour les surbrillances dynamiques
      if (surbrillance_semaine) {
          int jour_v = (temps->tm_wday == 0) ? 6 : (temps->tm_wday - 1);
          float angle_deg = -90.0f + (jour_v * 360.0f / 7.0f);
          float angle_rad = angle_deg * (PI / 180.0f);
          int coord_x = 165 + (int)(cos(angle_rad) * 160);
          int coord_y = 165 + (int)(sin(angle_rad) * 160);
          lv_obj_set_pos(surbrillance_semaine, coord_x - 12, coord_y - 13);
          lv_obj_clear_flag(surbrillance_semaine, LV_OBJ_FLAG_HIDDEN);
      }
      
      if (surbrillance_mois) {
          int mois_v = temps->tm_mon;
          float angle_deg = (mois_v + 1) * 30; 
          float angle_rad = angle_deg * (PI / 180.0f);
          int coord_x = 165 + (int)(sin(angle_rad) * 160);
          int coord_y = 165 - (int)(cos(angle_rad) * 160);
          lv_obj_set_pos(surbrillance_mois, coord_x - 12, coord_y - 13);
          lv_obj_clear_flag(surbrillance_mois, LV_OBJ_FLAG_HIDDEN);
      }
      
  } else {
      snprintf(tampon_heure, sizeof(tampon_heure), "--:--");
      snprintf(tampon_date, sizeof(tampon_date), "--/--/----");
  }
  
  if (etiquette_heure) {
      lv_obj_set_style_text_color(etiquette_heure, lv_color_white(), 0); 
      lv_label_set_text(etiquette_heure, tampon_heure);
  }

  if (etiquette_date) {
      lv_obj_set_style_text_color(etiquette_date, lv_color_white(), 0);
      const char* jours_semaine[] = {"DIMANCHE", "LUNDI", "MARDI", "MERCREDI", "JEUDI", "VENDREDI", "SAMEDI"};
      const char* mois_annee[] = {"JANVIER", "FEVRIER", "MARS", "AVRIL", "MAI", "JUIN", "JUILLET", "AOUT", "SEPTEMBRE", "OCTOBRE", "NOVEMBRE", "DECEMBRE"};
      lv_label_set_text_fmt(etiquette_date, "%s %d %s\n%d", jours_semaine[temps->tm_wday], temps->tm_mday, mois_annee[temps->tm_mon], 1900 + temps->tm_year);
  }

  // Mise à jour de la valeur
  if (etiquette_papp_valeur) {
      float kw = data->papp / 1000.0f;
      int i_kw = (int)kw;
      int d_kw = (int)((kw - i_kw) * 10);
      lv_label_set_text_fmt(etiquette_papp_valeur, "%d.%d", i_kw, d_kw);
      lv_obj_align_to(etiquette_papp_unite, etiquette_papp_valeur, LV_ALIGN_OUT_RIGHT_BOTTOM, 5, -3);
  }

  // Index
  if (etiquette_index_base) lv_label_set_text_fmt(etiquette_index_base, "BASE: %lu", (unsigned long)data->index_base);
  if (etiquette_index_hp) lv_label_set_text_fmt(etiquette_index_hp, "HP: %lu", (unsigned long)data->index_hp);
  if (etiquette_index_hc) lv_label_set_text_fmt(etiquette_index_hc, "HC: %lu", (unsigned long)data->index_hc);

  // Statistiques d'informations
  if (data->tension > 0) { 
      if (data->tension < tension_min) tension_min = data->tension;
      if (data->tension > tension_max) tension_max = data->tension;
      tension_somme += data->tension;
      tension_compteur++;
  }
  int v_avg = (tension_compteur > 0) ? (tension_somme / tension_compteur) : 0;
  if (etiquette_tension) lv_label_set_text_fmt(etiquette_tension, "%d V", data->tension);
  if (etiquette_stats_tension) lv_label_set_text_fmt(etiquette_stats_tension, "Min:%d      Avg:%d      Max:%d", (tension_min==999)?0:tension_min, v_avg, tension_max);

  if (data->iinst < intensite_min) intensite_min = data->iinst;
  if (data->iinst > intensite_max) intensite_max = data->iinst;
  intensite_somme += data->iinst;
  intensite_compteur++;
  int a_avg = (intensite_compteur > 0) ? (intensite_somme / intensite_compteur) : 0;
  if (etiquette_intensite) lv_label_set_text_fmt(etiquette_intensite, "%d A", data->iinst);
  if (etiquette_stats_intensite) lv_label_set_text_fmt(etiquette_stats_intensite, "Min:%d      Avg:%d      Max:%d", (intensite_min==999)?0:intensite_min, a_avg, intensite_max);

    // Refresh WEEK page if needed (redessiner les lignes si de nouvelles données sont recoltées par la SD)
    // Seules quelques redessins sont necessaires (la boucle est lourde donc optionnelle 
    // ou on la met a jour seulement a minuit, mais bon on update ici)
    // En fait, pour éviter de surcharger le processeur, puisque les données SD ne se mettent à jour qu'à minuit,
    // il n'est pas nécessaire de mettre à jour les 7*12 lignes 10 fois par seconde ici.
    // Les écrans de création initiaux ont déjà récupéré les données SD globales !



  // Logique de mise à jour des barres (Simulée)
  static uint32_t derniere_maj_graph = 0;
  static bool premier_graph = true; 
  
  if (barres_jauge[0]) {
       if (premier_graph || (lv_tick_get() - derniere_maj_graph > 10 * 60 * 1000)) {
           // Décaler & Ajouter
           for(int i=0; i<5; i++) valeurs_jauge[i] = valeurs_jauge[i+1];
           valeurs_jauge[5] = data->papp; 
           derniere_maj_graph = lv_tick_get();
           premier_graph = false;
           
           for(int i=0; i<6; i++) {
               int val = valeurs_jauge[i];
               if(val > 9000) val = 9000;
               int h = (val * 100) / 9000;
               if(h < 5) h = 5;

               lv_obj_set_height(barres_jauge[i], h);
               
               float barre_kw = val / 1000.0f;
               if (etiquettes_jauge_valeur[i]) {
                   int i_b_v = (int)barre_kw; int d_b_v = (int)((barre_kw - i_b_v)*10);
                   lv_label_set_text_fmt(etiquettes_jauge_valeur[i], "%d.%d", i_b_v, d_b_v);
                   lv_obj_align_to(etiquettes_jauge_valeur[i], barres_jauge[i], LV_ALIGN_OUT_TOP_MID, 0, -5);
               }
           }
       }
  }
}
