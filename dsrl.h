/*
 * Dark Shadows - The Roguelike
 *
 * Copyright 2011 Rolf Klausen
 */
#ifndef _DS_H
#define _DS_H

#define DS_VERSION_MAJ 0
#define DS_VERSION_MIN 0
#define DS_VERSION_REV 1

#define GAME_NAME "Dark Shadows RL"

#define MAIN_DATA_FILE "data/data.cfg"

#define DEVELOPMENT_MODE

typedef enum {
    state_unknown = 0,
    state_playing,
    state_menu,
    state_whatever,
} state_e;

typedef struct {                              // ds_config_t
        int  mind, maxd;
        int  dxsize, dysize;
        int  compress_savefile;               // compress the savefile?
        char autopickup[10];
        bool ap[10];                          // adjust later, match object type
        int  fontsize;
        int  rows, cols;                      // screen "resolution"
} ds_config_t;

typedef struct {
        int x, y, w, h;
        TCOD_console_t c;
} win_t;

typedef struct {                              // game_t
        char           version[20];
        short          width, height;           // width, height of screen
        short          mapw, maph;              // width, height of map window
        int            mapcx, mapcy;
        bool           dead;                    // is the game/player dead?
        state_e        state;
        short          context;                 // which context are we in? see CONTEXT_ defines
        short          currentlevel;            // what's the current level?
        int            turn;                    // count turns
        long long      tick;
        unsigned int   seed;                    // random seed
        short          monsterdefs;             // number of monster definitions
        short          objdefs;                 // number of object definitions
        short          createdareas;         // number of areas which have been created
        int            num_objects;             // number of spawned objects
        int            num_monsters;            // number of spawned monsters
        int            num_npcs;
        bool           wizardmode;              // yay!
        char           savefile[255];           // filename of the save file for this game
        obj_t         *objects[2000];
        dstime         t;
        dstime         total;
        win_t          map, messages, left, right;
        //int            vpx, vpy, vpw, vph;      // viewport x,y,w,h
} game_t;

typedef struct {                              // message_t
        TCOD_color_t color;
        char text[250];
} message_t;

struct actionqueue {                          // struct actionqueue
        struct actionqueue *head;
        struct actionqueue *next;
        int action;
        int num;
};

typedef struct coord {
        int y;
        int x;
} co;

typedef struct action {
        int        action;
        int        tick;
        monster_t *monster;
        obj_t     *object;
        actor_t   *actor;
        int        gain;                      // for temporary effects
} action_t;

#define MAXACT 100

#define ACTION_FREESLOT          -577
#define ACTION_NOTHING            0
#define ACTION_PLAYER_MOVE_LEFT   1
#define ACTION_PLAYER_MOVE_RIGHT  2
#define ACTION_PLAYER_MOVE_UP     3
#define ACTION_PLAYER_MOVE_DOWN   4
#define ACTION_PLAYER_MOVE_NW     5
#define ACTION_PLAYER_MOVE_NE     6
#define ACTION_PLAYER_MOVE_SW     7
#define ACTION_PLAYER_MOVE_SE     8
#define ACTION_PICKUP             9
#define ACTION_ATTACK            10
#define ACTION_MOVE_MONSTERS     11
#define ACTION_ENTER_DUNGEON     12
#define ACTION_GO_DOWN_STAIRS    13
#define ACTION_GO_UP_STAIRS      14
#define ACTION_FIX_VIEW          15
#define ACTION_WIELDWEAR         16
#define ACTION_UNWIELDWEAR       17
#define ACTION_HEAL_PLAYER       18
#define ACTION_DROP              20
#define ACTION_USE_EXIT          21
#define ACTION_MOVE_MONSTER      22
#define ACTION_PLAYER_NEXTMOVE   23
#define ACTION_MOVE_NPC          24

#define TICKS_MOVEMENT  1000
#define TICKS_ATTACK    1000
#define TICKS_WIELDWEAR  333

#define CONTEXT_OUTSIDE 0
#define CONTEXT_INSIDE 1

#define FALSE 0
#define TRUE 1
// #define MAX_MESSAGES 100
#define ENDOFLIST -577
#define PLAYER_ID -577

// define some shortcuts
#define ply player->y
#define plx player->x
#define ppx player->px
#define ppy player->py
#define pyxt world->cmap[player->y][player->x].type

#define round(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))

// global variables
extern world_t *world;
extern monster_t *monsterdefs;
extern obj_t *objdefs;
extern game_t *game;
extern monster_t *monsterdefs;
extern obj_t *objdefs;
extern actor_t *player;
extern int mapcx, mapcy;
extern FILE *messagefile;
extern message_t messages[500];
extern int currmess, maxmess;
extern struct actionqueue *aq;
extern ds_config_t dsconfig;
extern int tempxsize, tempysize;

/*extern WINDOW *wall;
extern WINDOW *wstat;
extern WINDOW *winfo;
extern WINDOW *wmap;*/


/* function prototypes */


void queuemany(actor_t *actor, int first, ...);
void schedule_actionx(int num, int action, actor_t *actor);
void unschedule_all_monsters();
void schedule_monster(monster_t *m);
void schedule_npc(actor_t *m);
void unschedule_action(int index);
int schedule_action_immediately(int action, actor_t *actor);
int schedule_action_delayed(int action, actor_t *actor, obj_t *object, int delay);
int schedule_action(int action, actor_t *actor);

void process_player_input();

bool do_action(action_t *aqe);
void shutdown_ds();

#endif
// vim: fdm=syntax 
