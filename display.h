/*
 * Dark Shadows - The Roguelike
 *
 * Copyright 2011 Rolf Klausen
 */
#ifndef _DISPALY_H
#define _DISPALY_H

#include "libtcod.h"

// Prototypes
void init_display();
void shutdown_display();
void draw_map(level_t *level);
void draw_wstat();

void dsmapaddch(int y, int x, TCOD_color_t color, TCOD_color_t backcolor, char c);
void update_screen();
void update_player();
void initial_update_screen();
TCOD_key_t dsgetch();
bool ds_checkforkeypress();

void domess();
void scrollmessages();
void mess(char *message);
void messc(TCOD_color_t color, char *message);
void delete_last_message();

void gtmsgbox(char *header, char *message);
void show_player_inventory();

bool blocks_light(level_t *l, int y, int x);
void newfov_initmap(level_t *l);

void fov_initmap(void *level);
void fov_updatemap(void *level);

#define MESSAGE_LINES   5
#define COLS  125                             // x
#define ROWS  (65 + MESSAGE_LINES)         // y

#define COLOR_PLAIN    C_BLACK_WHITE
#define COLOR_FOREST   C_BLACK_GREEN
#define COLOR_CITY     C_BLACK_YELLOW
#define COLOR_VILLAGE  COLOR_CITY
#define COLOR_DUNGEON  COLOR_CITY
#define COLOR_MOUNTAIN C_BLACK_YELLOW
#define COLOR_LAKE     C_BLACK_BLUE
#define COLOR_PLAYER   56

#define COLOR_WARNING C_BLACK_RED
#define COLOR_BAD     C_BLACK_RED
#define COLOR_GOOD    C_BLACK_GREEN
#define COLOR_NORMAL  C_BLACK_WHITE
#define COLOR_INFO    TCOD_yellow
#define COLOR_VISIBLE C_BLACK_YELLOW

#define COLOR_SHADE   63
#define COLOR_LIGHT   62

#define COLOR_INVISIBLE 63

// #define COLOR

#endif
// vim: fdm=syntax 
