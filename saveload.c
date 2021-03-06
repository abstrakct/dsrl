/*
 * Dark Shadows - The Roguelike
 *
 * Saving and loading!
 *
 * Copyright 2011 Rolf Klausen
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "npc-names.h"
#include "objects.h"
#include "actor.h"
#include "monsters.h"
#include "utils.h"
#include "world.h"
#include "datafiles.h"
#include "display.h"
#include "debug.h"
#include "saveload.h"
#include "dstime.h"
#include "dsrl.h"

/*
 * write one object
 */
void save_object(obj_t *o, FILE *f)
{
        if(o) {
                fwrite(" OBJECT", sizeof(char), 7, f);
                fwrite(o, sizeof(obj_t), 1, f);
        } else {
                fwrite("NOBJECT", sizeof(char), 7, f);
        }
}

/*
 * write one list of objects
 */
void save_inventory(inv_t *i, FILE *f)
{
        int j;

        fwrite("INVENTORY", sizeof(char), 9, f);
        fwrite(&i->num_used, sizeof(short), 1, f);
        fwrite(&i->gold, sizeof(int), 1, f);

        for(j = 0; j < 52; j++)
                save_object(i->object[j], f);
}

/* 
 * write one monster
 */
void save_monster(monster_t *m, FILE *f)
{
        int tmp;

        fwrite("MONSTER",    sizeof(char),  7, f);     // is this necessary?
        fwrite(&m->id,       sizeof(short), 1, f);     // write the monsterdef ID, then any vars which might have changed
        fwrite(&m->mid,      sizeof(int),   1, f);
        fwrite(&m->y,        sizeof(short), 1, f);
        fwrite(&m->x,        sizeof(short), 1, f);
        fwrite(&m->hp,       sizeof(int),   1, f);
        fwrite(&m->goalx,    sizeof(short), 1, f);
        fwrite(&m->goaly,    sizeof(short), 1, f);
        if(m->attacker && m->attacker == player)
                tmp = 1;
        else
                tmp = 0;

        fwrite(&tmp,         sizeof(int), 1, f);

        // add saving inventory etc here later when implemented
        // add saving attacker, wear_t, etc.
}

/*
 * Write one cell_t  to file f, at current file position
 */
void save_cell(cell_t *c, FILE *f)
{
        bool flag;

        fwrite(&c->type,    sizeof(char), 1, f);
        fwrite(&c->flags,   sizeof(int), 1, f);
        fwrite(&c->desty,   sizeof(short), 1, f);
        fwrite(&c->destx,   sizeof(short), 1, f);
        //fwrite(&c->color,   sizeof(short), 1, f);
        fwrite(&c->visible, sizeof(bool), 1, f);

        flag = c->monster ? true : false;
        fwrite(&flag, sizeof(bool), 1, f);
        if(flag)
                save_monster(c->monster, f);

        flag = c->inventory ? true : false;
        fwrite(&flag, sizeof(bool), 1, f);
        if(flag)
                save_inventory(c->inventory, f);

}

/*
 * Write one level_t to file f, at current file position
 */
void save_level(level_t *l, FILE *f)
{
        int y, x;

        fwrite("LEVEL", sizeof(char), 5, f);
        fwrite(&l->xsize, sizeof(short), 1, f);
        fwrite(&l->ysize, sizeof(short), 1, f);
        for(y = 0; y < l->ysize; y++)
                for(x = 0; x < l->xsize; x++)
                        save_cell(&l->c[y][x], f);
}

/*
 * Write one monsterdef
 */
void save_monsterdef(monster_t *m, FILE *f)
{
        struct monsterdef_save_struct s;

        memset(&s, 0, sizeof(struct monsterdef_save_struct));
        s.id = m->id;
        strcpy(s.name, m->name);
        s.attr = m->attr;
        s.c = m->c;
        s.level = m->level;
        s.hp = m->hp;
        s.speed = m->speed;
        s.flags = m->flags;
        s.aitableindex = m->mid;
        s.viewradius = m->viewradius;

        fwrite("MONSTERDEF", sizeof(char), 10, f);
        fwrite(&s, sizeof(struct monsterdef_save_struct), 1, f);
        //dsprintf("saved monsterdef %s id=%d", s.name, s.id);
}

void save_objdef(obj_t *o, FILE *f)
{
        struct objdef_save_struct s;

        memset(&s, 0, sizeof(struct objdef_save_struct));
        s.id = o->id;
        s.fore = o->fore;
        s.back = o->back;
        s.type = o->type;
        s.flags = o->flags;
        s.attackmod = o->attackmod;
        s.damagemod = o->damagemod;
        strcpy(s.basename, o->basename);
        strcpy(s.truename, o->truename);
        strcpy(s.fullname, o->fullname);
        s.c = o->c;
        s.minlevel = o->minlevel;
        s.quantity = o->quantity;
        s.material = o->material;
        s.dice = o->dice;
        s.sides = o->sides;
        s.skill = o->skill;

        fwrite("OBJDEF", sizeof(char), 6, f);
        fwrite(&s, sizeof(struct objdef_save_struct), 1, f);
        //dsprintf("saved objdef %s", s.basename);
}

void save_player(actor_t *p, FILE *f)
{
        struct player_save_struct s;
        int i;

        s.x = p->x;
        s.y = p->y;
        s.oldx = p->oldx;
        s.oldy = p->oldy;
        s.px = p->px;
        s.py = p->py;
        s.viewradius = p->viewradius;
        strcpy(s.name, p->name);
        s.hp = p->hp;
        s.maxhp = p->maxhp;
        s.xp = p->xp;
        s.ac = p->ac;
        s.attr = p->attr;
        s.level = p->level;
        s.race = p->race;
        s.cla = p->cla;
        s.flags = p->flags;
        s.speed = p->speed;
        s.wvfactor = p->wvfactor;
        s.worldview = p->worldview;
        s.weapon = (p->weapon ? p->weapon->oid : 0);

        for(i = 0; i < WEAR_SLOTS; i++)
                s.w[i] = (p->w[i] ? p->w[i]->oid : 0);

        for(i = 0; i < MAX_SKILLS; i++)
                s.skill[i] = p->skill[i];

        fwrite("PLAYER", sizeof(char), 6, f);
        fwrite(&s, sizeof(struct player_save_struct), 1, f);

        save_inventory(player->inventory, f);
}

void generate_savefilename(char *filename)
{
        sprintf(filename, "%s/%d.dssave", SAVE_DIRECTORY, game->seed);
}

/*
 * Rough structure of the save file:
 *
 * - header struct
 * - dsconfig struct
 * - game struct
 * - the player (player_save_struct)
 * - the monsterdefs (monsterdef_save_struct)
 * - the objectdefs (objdef_save_struct)
 * - the levels
 */

bool save_game(char *filename)
{
        char cmd[260];
        int i;
        FILE *f;
        struct savefile_header header;
        //int i;
        monster_t *m;
        obj_t *o;

        dsprintf("Saving game to file %s", filename);
        f = fopen(filename, "w");

        header.magic = DS_SAVEFILE_MAGIC;
        header.version.major = DS_VERSION_MAJ;
        header.version.minor = DS_VERSION_MIN;
        header.version.revision = DS_VERSION_REV;

        fwrite(&header, sizeof(struct savefile_header), 1, f);
        fwrite(&dsconfig, sizeof(ds_config_t), 1, f);
        fwrite(game, sizeof(game_t), 1, f);

        /* then, let's save the player */
        save_player(player, f);

        m = monsterdefs->head;
        for(i=0; i <= game->monsterdefs; i++) {
                if(m)
                        save_monsterdef(m, f);
                m = m->next;
        }

        o = objdefs->head;
        for(i=0; i <= game->objdefs; i++) {
                if(o)
                        save_objdef(o, f);
                o = o->next;
        }

        /* then, let's save world and levels */
        save_level(world->area, f);

        for(i=1; i<=game->createdareas; i++)
                save_level(&world->area[i], f);

        fclose(f);

        if(dsconfig.compress_savefile) {
                sprintf(cmd, "xz %s", filename);
                system(cmd);
        }

        dsprintf("Saving successful!");
        return true;
}



/*
 * And now... loading!
 */

obj_t *load_object(FILE *f)
{
        char str[6];
        obj_t *o;

        fread(str, sizeof(char), 7, f);
        if(!strncmp(str, " OBJECT", 7)) {
                o = dsmalloc(sizeof(obj_t));
                fread(o, sizeof(obj_t), 1, f);
                add_to_master_object_list(o); 
        } else if(!strncmp(str, "NOBJECT", 7)) {
                o = NULL;
        }

        return o;
}

inv_t *load_inventory(FILE *f)
{
        char str[9];
        inv_t *i;
        int j;

        fread(str, sizeof(char), 9, f);
        if(strncmp(str, "INVENTORY", 9)) {
                fprintf(stderr, "Inventory not where expected! This won't end well!");
                return false;
        }

        i = init_inventory();

        fread(&i->num_used, sizeof(short), 1, f);
        fread(&i->gold, sizeof(int), 1, f);

        for(j = 0; j < 52; j++)
                i->object[j] = load_object(f);

        return i;
}

bool load_monster(monster_t *m, level_t *l, FILE *f)
{
        char str[7];
        int i;
        monster_t mdef;

        fread(str, sizeof(char), 7, f);
        if(strncmp(str, "MONSTER", 7)) {
                fprintf(stderr, "NO THIS IS NOT A MONSTER");
                fprintf(stderr, "load_monster! STR=%s", str);
                return false;
        }

        if(!m)
                m = dsmalloc(sizeof(monster_t));

        fread(&m->id,       sizeof(short), 1, f);
        fread(&m->mid,      sizeof(int),   1, f);
        fread(&m->y,        sizeof(short), 1, f);
        fread(&m->x,        sizeof(short), 1, f);
        fread(&m->hp,       sizeof(int),   1, f);
        fread(&m->goalx,    sizeof(short), 1, f);
        fread(&m->goaly,    sizeof(short), 1, f);
        fread(&i,           sizeof(int),   1, f);
        if(i)
                m->attacker = player;
        else
                m->attacker = NULL;

        mdef = get_monsterdef(m->id);

        m->oldx = mdef.oldx;
        m->oldy = mdef.oldy;
        m->viewradius = mdef.viewradius;
        strcpy(m->name, mdef.name);
        m->maxhp = mdef.maxhp;
        m->xp = mdef.xp;
        m->attr = mdef.attr;
        m->level = mdef.level;
        m->race = mdef.race;
        m->cla = mdef.cla;
        //m->w = mdef.w;          // implement this when monsters can wield and wear stuff!
        m->flags = mdef.flags;
        m->c = mdef.c;
        m->speed = mdef.speed;
        m->ai = mdef.ai;

        l->c[m->y][m->x].monster = m;

        m->prev = l->monsters;
        m->head = l->monsters;
        m->next = l->monsters->next;
        l->monsters->next = m;
        if(m->next)
                m->next->prev = m;

        // add loading inventory etc here later when implemented
        // add loading attacker, wear_t, etc.
        return true;
}

bool load_cell(cell_t *c, level_t *l, FILE *f)
{
        bool flag;

        fread(&c->type,    sizeof(char), 1, f);
        fread(&c->flags,   sizeof(int), 1, f);
        fread(&c->desty,   sizeof(short), 1, f);
        fread(&c->destx,   sizeof(short), 1, f);
        //fread(&c->color,   sizeof(short), 1, f);
        fread(&c->visible, sizeof(bool), 1, f);

        /* cell has monster? */
        fread(&flag, sizeof(bool), 1, f);
        if(flag) {
                c->monster = dsmalloc(sizeof(monster_t));
                load_monster(c->monster, l, f);
        }

        /* cell has inventory? */
        fread(&flag, sizeof(bool), 1, f);
        if(flag) {
                c->inventory = load_inventory(f);
                if(!c->inventory) {
                        printf("load_inventory failed!");
                        return false;
                }
        }

        return true;
}

bool load_level(level_t *l, FILE *f)
{
        int y, x;
        char str[5];

        fread(str, sizeof(char), 5, f);
        if(strcmp(str, "LEVEL")) {
                fprintf(stderr, "LEVEL header not were it's supposed to be!");
                return false;
        }
        fread(&l->xsize, sizeof(short), 1, f);
        fread(&l->ysize, sizeof(short), 1, f);
        if(!l->c)
                init_level(l);

        for(y = 0; y < l->ysize; y++)
                for(x = 0; x < l->xsize; x++)
                        if(!load_cell(&l->c[y][x], l, f)) {
                                printf("load_cell failed!");
                                return false;
                        }

        printf("loaded level!");
        return true;
}

/*
 * Load one monsterdef (into *m)
 */
bool load_monsterdef(monster_t *m, FILE *f)
{
        struct monsterdef_save_struct s;
        char str[10];

        fread(str, sizeof(char), 10, f);
        if(strncmp(str, "MONSTERDEF", 10)) {
                fprintf(stderr, "MONSTERDEF not where it should be! This won't end well! str=%s\n", str);
                return false;
        }
        fread(&s, sizeof(struct monsterdef_save_struct), 1, f);
        m->id = s.id;
        strcpy(m->name, s.name);
        m->attr = s.attr;
        m->c = s.c;
        m->level = s.level;
        m->hp = s.hp;
        m->speed = s.speed;
        m->flags = s.flags;
        m->ai = aitable[s.aitableindex];
        m->viewradius = s.viewradius;

        //printf("loaded monsterdef %s id=%d\n", m->name, m->id);
        return true;
}

bool load_objdef(obj_t *o, FILE *f)
{
        struct objdef_save_struct s;
        char str[6];

        fread(str, sizeof(char), 6, f);
        if(strncmp(str, "OBJDEF", 6)) {
                fprintf(stderr, "OBJDEF not where it should be! This won't end well! str=%s\n", str);
                return false;
        }

        fread(&s, sizeof(struct objdef_save_struct), 1, f);
        o->id = s.id;
        o->fore = s.fore;
        o->back = s.back;
        o->type = s.type;
        o->flags = s.flags;
        o->attackmod = s.attackmod;
        o->damagemod = s.damagemod;
        strcpy(o->basename, s.basename);
        strcpy(o->truename, s.truename);
        strcpy(o->fullname, s.fullname);
        o->c = s.c;
        o->minlevel = s.minlevel;
        o->quantity = s.quantity;
        o->material = s.material;
        o->dice = s.dice;
        o->sides = s.sides;
        o->skill = s.skill;
        //printf("loaded objdef %s\n", o->basename);
        return true;
}

bool load_player(actor_t *p, FILE *f)
{
        struct player_save_struct s;
        int i;
        char str[6];

        fread(str, sizeof(char), 6, f);
        if(strcmp(str, "PLAYER")) {
                fprintf(stderr, "DEBUG: %s:%d - str != PLAYER in load_player!\n", __FILE__, __LINE__);
                fprintf(stderr, "read %s instead of PLAYER?!\n", str);
                return false;
        }

        printf("loadplayerstart\n");
        fread(&s, sizeof(struct player_save_struct), 1, f);
        p->x = s.x;
        p->y = s.y;
        p->oldx = s.oldx;
        p->oldy = s.oldy;
        p->px = s.px;
        p->py = s.py;
        p->viewradius = s.viewradius;
        strcpy(p->name, s.name);
        p->hp = s.hp;
        p->maxhp = s.maxhp;
        p->xp = s.xp;
        p->ac = s.ac;
        p->attr = s.attr;
        p->level = s.level;
        p->race = s.race;
        p->cla = s.cla;
        p->flags = s.flags;
        p->speed = s.speed;
        p->worldview = s.worldview;
        p->wvfactor  = s.wvfactor;
        for(i = 0; i < MAX_SKILLS; i++)
                p->skill[i] = s.skill[i];
                

        p->inventory = load_inventory(f);

        if(!p->inventory) {
                printf("load_inventory in player failed!\n");
                return false;
        }

        if(s.weapon) {
                p->weapon = get_object_by_oid(p->inventory, s.weapon);
                if(!p->weapon) {
                        printf("get_object_by_oid failed!\n");
                        return false;
                }
        } else {
                p->weapon = NULL;
        }

        for(i = 0; i < WEAR_SLOTS; i++) {
                if(s.w[i]) {
                        p->w[i] = get_object_by_oid(p->inventory, s.w[i]);
                        if(!p->w[i]) {
                                printf("get_object_by_oid failed!\n");
                                return false;
                        }
                } else {
                        p->w[i] = NULL;
                }
        }

        //load_set_objlets(p);
        printf("loadplayerend\n");
        return true;
}

bool load_game(char *filename, int ingame)
{
        FILE *f;
        struct savefile_header header;
        monster_t *m;
        obj_t *o;
        int i;
        char cmd[100];

        printf("Loading game from %s", filename);
        
        if(filename[strlen(filename)-1] == 'z' && filename[strlen(filename)-2] == 'x') {               // it's pretty likely to be compressed with xz!
                sprintf(cmd, "xz -d %s", filename);
                system(cmd);
                filename[strlen(filename)-3] = '\0';  // strip off .xz
        }

        f = fopen(filename, "r");
        if(!f) {
                printf("Couldn't open file %s", filename);
                return false;
        }

        fread(&header, sizeof(struct savefile_header), 1, f);
        if(header.magic != DS_SAVEFILE_MAGIC) {
                fprintf(stderr, "This doesn't look like a GT savefile to me!");
                return false;
        }
        if(header.version.major != DS_VERSION_MAJ || header.version.minor != DS_VERSION_MIN || header.version.revision != DS_VERSION_REV)
                fprintf(stderr, "Warning: Save file doesn't match current version number. This may or may not work...!");

        fread(&dsconfig, sizeof(ds_config_t), 1, f);
        fread(game, sizeof(game_t), 1, f);
        clear_master_object_list();
        game->num_objects = 0;

        /* player */
        if(!load_player(player, f)) {
                fprintf(stderr, "DEBUG: %s:%d - loading failed in load_player\n", __FILE__, __LINE__);
                return false;
        }

        /* now, loading monsterdefs, linked lists, all that.. might be tricky! */
        for(i=0; i <= game->monsterdefs; i++) {
                if(!ingame)  // only alloc memory if we are loading at startup
                        m = dsmalloc(sizeof(monster_t));
                else
                        m = monsterdefs;
                if(!load_monsterdef(m, f)) {
                        fprintf(stderr, "DEBUG: %s:%d - loading failed in load_monsterdef\n", __FILE__, __LINE__);
                        return false;
                }
                
                m->next = monsterdefs->next;
                m->head = monsterdefs;
                m->prev = monsterdefs;
                if(monsterdefs->next)
                        monsterdefs->next->prev = m;
                monsterdefs->next = m;
}

        /* objdefs */
        for(i=0; i <= game->objdefs; i++) {
                if(!ingame)  // only alloc memory if we are loading at startup
                        o = dsmalloc(sizeof(obj_t));
                else
                        o = objdefs;
                if(!load_objdef(o, f)) {
                        fprintf(stderr, "DEBUG: %s:%d - loading failed in load_objdef %d\n", __FILE__, __LINE__, i);
                        return false;
                }

                o->next = objdefs->next;
                o->head = objdefs;
                /*o->prev = objdefs;
                if(objdefs->next)
                        objdefs->next->prev = o;*/
                objdefs->next = o;
        }

        /* world */
        if(!load_level(world->area, f)) {
                fprintf(stderr, "DEBUG: %s:%d - loading failed in load_level\n", __FILE__, __LINE__);
                return false;
        }
        for(i=1; i<=game->createdareas; i++) {
                if(!load_level(&world->area[i], f)) {
                        fprintf(stderr, "DEBUG: %s:%d - loading failed in load_level (level %d)\n", __FILE__, __LINE__, i);
                        return false;
                }
        }

        fclose(f);
        printf("Loading successful!");
        return true;
}
// vim: fdm=syntax 
