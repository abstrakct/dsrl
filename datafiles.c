/*
 * Dark Shadows - The Roguelike
 *
 * Copyright 2011 Rolf Klausen
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libconfig.h>
#include <stdbool.h>

#include "npc-names.h"
#include "objects.h"
#include "o_effects.h"
#include "actor.h"
#include "monsters.h"
#include "utils.h"
#include "world.h"
#include "datafiles.h"
#include "dstime.h"
#include "dsrl.h"

config_t *cf;
int objid;             // to keep track of all parsed objects, to give each a unique ID
areadef_t r;

int parse_areadef_cfgfile(char *filename, int index)
{
        config_setting_t *cfg;
        int i, j, val;
        char sname[200];
        const char *value;

        cf = (config_t *) dsmalloc(sizeof(config_t));
        config_init(cf);

        if (!config_read_file(cf, filename)) {
                fprintf(stderr, "%s:%d - %s\n",
                                config_error_file(cf),
                                config_error_line(cf),
                                config_error_text(cf));
                config_destroy(cf);
                return(1);
        }

        cfg = config_lookup(cf, "config");
        j = config_setting_length(cfg);

        for(i = 0; i < j; i++) {
                int x;
                for(x = 0; x < 10; x++) {
                        sprintf(sname, "config.[%d].exit.[%d].location", i, x);
                        config_lookup_string(cf, sname, &value);
                        if(!strcmp(value, "outside collinwood"))
                                areadef[index].exit[x].location = AREA_OUTSIDE;
                        if(!strcmp(value, "upstairs collinwood"))
                                areadef[index].exit[x].location = AREA_COLLINWOOD_UPSTAIRS_HALL;
                        if(!strcmp(value, "roger study"))
                                areadef[index].exit[x].location = AREA_COLLINWOOD_STUDY;
                        if(!strcmp(value, "kitchen"))
                                areadef[index].exit[x].location = AREA_COLLINWOOD_KITCHEN;
                        if(!strcmp(value, "main floor collinwood"))
                                areadef[index].exit[x].location = AREA_COLLINWOOD_MAIN_FLOOR;
                        if(!strcmp(value, "bedrooms"))
                                areadef[index].exit[x].location = AREA_COLLINWOOD_UPSTAIRS;

                        sprintf(sname, "config.[%d].exit.[%d].type", i, x);
                        config_lookup_string(cf, sname, &value);
                        if(!strcmp(value, "exit"))
                                areadef[index].exit[x].type = ET_EXIT;
                        if(!strcmp(value, "stairs up"))
                                areadef[index].exit[x].type = ET_STAIRS_UP;
                        if(!strcmp(value, "stairs down"))
                                areadef[index].exit[x].type = ET_STAIRS_DOWN;
                        if(!strcmp(value, "door"))
                                areadef[index].exit[x].type = ET_DOOR;
                               
                        sprintf(sname, "config.[%d].exit.[%d].char", i, x);
                        config_lookup_string(cf, sname, &value);
                        areadef[index].exit[x].c = value[0];

                        sprintf(sname, "config.[%d].exit.[%d].corresponds_to", i, x);
                        if(config_lookup_int(cf, sname, &val) == CONFIG_TRUE)
                                areadef[index].exit[x].dest = val;
                        else
                                areadef[index].exit[x].dest = -1;

                        //printf("areadef[%d].exit[%d].dest = %d\n", index, x, areadef[index].exit[x].dest);
                }
        }

        config_destroy(cf);
        return 0;
}

int parse_areadef_file(char *filename, int index)
{
        FILE *f;
        int y, x, i, j, n;
        char c;
        char cfgfile[100];

        sprintf(cfgfile, "%s.cfg", filename);

        if(parse_areadef_cfgfile(cfgfile, index))
                printf("couldn't read %s - will try to continue anyway! cross your fingers!", cfgfile);

        f = fopen(filename, "r");
        if(!f)
                return 1;
        fscanf(f, "%d,%d\n", &y, &x);
        areadef[index].y = y;
        areadef[index].x = x;
        areadef[index].c = (cell_t **) dsmalloc2d(y, x, sizeof(cell_t));

        for(i = 0; i < y; i++) {
                for(j = 0; j < x; j++) {
                        fscanf(f, "%c", &c);
                        //printf("%c", c);
                        switch(c) {
                                case ' ': areadef[index].c[i][j].type = CELL_NOTHING; break;
                                case '#': areadef[index].c[i][j].type = CELL_WALL; break;
                                case '.': areadef[index].c[i][j].type = CELL_FLOOR; break;
                                case '+': areadef[index].c[i][j].type = CELL_FLOOR; 
                                          setbit(areadef[index].c[i][j].flags, CF_HAS_DOOR_CLOSED); break;
                                case '@': areadef[index].c[i][j].type = CELL_FLOOR; 
                                          setbit(areadef[index].c[i][j].flags, CF_IS_STARTING_POINT); break;
                                case '0':
                                case '1':
                                case '2':
                                case '3':
                                case '4':
                                case '5':
                                case '6':
                                case '7':
                                case '8':
                                case '9':
                                          n = c - '0';
                                          areadef[index].c[i][j].type = CELL_FLOOR;
                                          setbit(areadef[index].c[i][j].flags, CF_HAS_EXIT);
                                          areadef[index].c[i][j].exitindex = n;
                                          areadef[index].exit[n].x = j+1;
                                          areadef[index].exit[n].y = i+1;
                                          break;
                                case 'T':
                                          areadef[index].c[i][j].type = CELL_FLOOR;
                                          setbit(areadef[index].c[i][j].flags, CF_HAS_FURNITURE);
                                          setbit(areadef[index].c[i][j].flags, CF_HASF_TABLE);
                                          break;
                                case 'h':
                                          areadef[index].c[i][j].type = CELL_FLOOR;
                                          setbit(areadef[index].c[i][j].flags, CF_HAS_FURNITURE);
                                          setbit(areadef[index].c[i][j].flags, CF_HASF_CHAIR);
                                          break;
                                case 'f':
                                          areadef[index].c[i][j].type = CELL_FLOOR;
                                          setbit(areadef[index].c[i][j].flags, CF_HAS_FURNITURE);
                                          setbit(areadef[index].c[i][j].flags, CF_HASF_FIRE);
                                          break;
                                default: areadef[index].c[i][j].type = CELL_FLOOR; break;
                        }
                }
                fscanf(f, "\n");
                //printf("\n");
        }

        return 0;
}

int parse_areadef_files()
{
        int ret;
        ret = parse_areadef_file("data/area/collinwood.1", AREA_COLLINWOOD_MAIN_FLOOR);
        ret = parse_areadef_file("data/area/collinwood.2", AREA_COLLINWOOD_UPSTAIRS_HALL);
        ret = parse_areadef_file("data/area/collinwood.3", AREA_COLLINWOOD_STUDY);

        return ret;
}

int parse_monsters()
{
        config_setting_t *cfg_monsters;
        int i, j, boolval, tmp, id;
        char sname[100];
        const char *value;
        char string[100];

        cfg_monsters = config_lookup(cf, "monsters");
        i = config_setting_length(cfg_monsters);
        game->monsterdefs = i;
        //printf("Parsing monster file... We have %d monsters", i);

        /* 
         * main monster parsing loop 
         * goes through each possible setting, looks it up in the cfg file
         * and adds it to a monster structure which is then placed in the
         * linked list monsterdefs.
         */
        
        for(j=0;j<i;j++) {
                monster_t *m;

                m = (monster_t *) dsmalloc(sizeof(monster_t));
                id = j+1;

                sprintf(sname, "monsters.[%d].name", j);
                config_lookup_string(cf, sname, &value);
                strcpy(m->name, value);

                sprintf(sname, "monsters.[%d].str", j);
                config_lookup_int(cf, sname, &(m->attr.str));
                sprintf(sname, "monsters.[%d].phy", j);
                config_lookup_int(cf, sname, &(m->attr.phy));
                sprintf(sname, "monsters.[%d].intl", j);
                config_lookup_int(cf, sname, &(m->attr.intl));
                sprintf(sname, "monsters.[%d].wis", j);
                config_lookup_int(cf, sname, &(m->attr.wis));
                sprintf(sname, "monsters.[%d].dex", j);
                config_lookup_int(cf, sname, &(m->attr.dex));
                sprintf(sname, "monsters.[%d].cha", j);
                config_lookup_int(cf, sname, &(m->attr.cha));

                sprintf(sname, "monsters.[%d].appearance", j);
                config_lookup_string(cf, sname, &value);
                m->c = (char) *value;

                sprintf(sname, "monsters.[%d].level", j);
                config_lookup_int(cf, sname, &(m->level));

                sprintf(sname, "monsters.[%d].hp", j);
                config_lookup_int(cf, sname, &(m->hp));
                m->maxhp = m->hp;

                sprintf(sname, "monsters.[%d].speed", j);
                config_lookup_int(cf, sname, &(m->speed));

                sprintf(sname, "monsters.[%d].havegold", j);
                if(config_lookup_bool(cf, sname, &boolval))
                        if(boolval)
                                setbit(m->flags, MF_CANHAVEGOLD);

                sprintf(sname, "monsters.[%d].useweapon", j);
                if(config_lookup_bool(cf, sname, &boolval))
                        if(boolval)
                                setbit(m->flags, MF_CANUSEWEAPON);

                sprintf(sname, "monsters.[%d].usearmor", j);
                if(config_lookup_bool(cf, sname, &boolval))
                        if(boolval)
                                setbit(m->flags, MF_CANUSEARMOR);

                sprintf(sname, "monsters.[%d].usesimplesword", j);
                if(config_lookup_bool(cf, sname, &boolval))
                        if(boolval)
                                setbit(m->flags, MF_CANUSESIMPLESWORD);

                sprintf(sname, "monsters.[%d].aitype", j);
                config_lookup_int(cf, sname, &tmp);
                m->ai = aitable[tmp];
                m->mid = tmp;   // for monsterdefs, mid holds aitableindex
                m->id = id;

                m->viewradius = 12; // temporary solution?!

                // Let's give the monster a weapon!
                if(hasbit(m->flags, MF_CANUSEWEAPON)) {
                        int x;
                        obj_t *o;

                        sprintf(sname, "monsters.[%d].weapon", j);
                        config_lookup_string(cf, sname, &value);

                        strcpy(string, value);
                        x = get_objdef_by_name(string);

                        m->inventory = init_inventory();
                        o = spawn_object(x, 0);
                        m->inventory->object[0] = o;
                        m->weapon = o;
                }

                /*if(hasbit(m->flags, MF_CANUSEWEAPON) && !hasbit(m->flags, MF_CANUSESIMPLESWORD)) {
                        int x;
                        obj_t *o;

                        x = get_objdef_by_name("dagger");

                        m->inventory = init_inventory();
                        o = spawn_object(x, 0);
                        m->inventory->object[0] = o;
                        m->weapon = o;
                }*/

                if(hasbit(m->flags, MF_CANHAVEGOLD)) {
                        if(!m->inventory) {
                                m->inventory = init_inventory();
                                m->inventory->gold += ri(2, 20);
                        }
                }

                
//if(m->weapon)
//fprintf(stderr, "DEBUG: %s:%d - %s has the weapon called a %s\n", __FILE__, __LINE__, m->name, m->weapon->basename);
                        

                /*
                 * the following was written in one go, it's beautiful and seems totally bugfree!!
                 */
                m->head = monsterdefs->head;
                monsterdefs->next = m;
                m->next = NULL;
                m->prev = monsterdefs;
                monsterdefs = m;

                //printf("."); // simple "progress bar"
        }
        
        //printf(" OK\n");

        monsterdefs->head->x = i; // store number of monsters in x of head.
        return 0;
}

int parse_armor()
{
        config_setting_t *cfg;
        int i, j;
        char sname[100];
        const char *value;

        cfg = config_lookup(cf, "armor");
        i = config_setting_length(cfg);
        //printf("Parsing armor file... We have %d armors", i);
        for(j=0;j<i;j++) {
                obj_t *o;
                int x;

                o = (obj_t *) dsmalloc(sizeof(obj_t));

                sprintf(sname, "armor.[%d].name", j);
                config_lookup_string(cf, sname, &value);
                strcpy(o->basename, value);

                sprintf(sname, "armor.[%d].type", j);
                config_lookup_string(cf, sname, &value);
                if(!strcmp(value, "bodyarmor"))
                        o->flags |= OF_BODYARMOR;
                if(!strcmp(value, "headarmor"))
                        o->flags |= OF_HEADGEAR;
                if(!strcmp(value, "shield"))
                        o->flags |= OF_SHIELD;
                if(!strcmp(value, "gloves"))
                        o->flags |= OF_GLOVES;
                if(!strcmp(value, "footarmor"))
                        o->flags |= OF_FOOTWEAR;

                o->type   = OT_ARMOR;
                sprintf(sname, "armor.[%d].ac", j);
                config_lookup_int(cf, sname, &x);
                o->ac = x;

                o->id = objid; objid++;
                o->fore = TCOD_white;
                o->back = TCOD_black;

                o->head = objdefs->head;
                objdefs->next = o;
                o->next = NULL;
                objdefs = o;

                //printf(".");
        }

        //printf(" OK\n");
        objdefs->head->dice = i;
        game->objdefs = i;

        return 0;
}

int parse_weapons()
{
        config_setting_t *cfg;
        int i, j;
        char sname[100];
        const char *value;

        cfg = config_lookup(cf, "weapon");
        i = config_setting_length(cfg);
        //printf("Parsing weapon file... We have %d weapons", i);
        for(j=0;j<i;j++) {
                obj_t *o;
                int x;

                o = (obj_t *) dsmalloc(sizeof(obj_t));

                sprintf(sname, "weapon.[%d].name", j);
                config_lookup_string(cf, sname, &value);
                strcpy(o->basename, value);

                sprintf(sname, "weapon.[%d].type", j);
                config_lookup_string(cf, sname, &value);
                if(!strcmp(value, "sword")) {
                        o->flags |= OF_SWORD;
                        o->skill  = SKILL_SWORD;
                } else if(!strcmp(value, "axe")) {
                        o->flags |= OF_AXE;
                        o->skill  = SKILL_AXE;
                } else if(!strcmp(value, "knife")) {
                        o->flags |= OF_KNIFE;
                        o->skill  = SKILL_KNIFE;
                } else if(!strcmp(value, "stick")) {
                        o->flags |= OF_STICK;
                        o->skill  = SKILL_STICK;
                } else if(!strcmp(value, "mace")) {
                        o->flags |= OF_MACE;
                        o->skill  = SKILL_MACE;
                } if(!strcmp(value, "hammer")) {
                        o->flags |= OF_HAMMER;
                        o->skill  = SKILL_MACE;
                }

                o->type   = OT_WEAPON;

                sprintf(sname, "weapon.[%d].dice", j);
                config_lookup_int(cf, sname, &x);
                o->dice = x;
                sprintf(sname, "weapon.[%d].sides", j);
                config_lookup_int(cf, sname, &x);
                o->sides = x;

                sprintf(sname, "weapon.[%d].unique", j);
                config_lookup_bool(cf, sname, &x);
                if(x)
                        o->flags |= OF_UNIQUE;

                x = 0;
                sprintf(sname, "weapon.[%d].mod", j);
                config_lookup_int(cf, sname, &x);
                o->attackmod = o->damagemod = x;

                o->id = objid; objid++;
                o->fore = TCOD_white;
                o->back = TCOD_black;

                o->head = objdefs->head;
                objdefs->next = o;
                o->next = NULL;
                objdefs = o;

                game->objdefs++;
                //printf(".");
        }

        //printf(" OK\n");

        return 0;
}

int parse_amulet()
{
        config_setting_t *cfg;
        int i, j, material;
        char sname[100];
        const char *value;

        cfg = config_lookup(cf, "amulet");
        i = config_setting_length(cfg);
        //printf("Parsing jewelry file... We have %d amulets", i);
        material = 1;
        for(j=0;j<i;j++) {
                obj_t *o;
                int x;

                o = (obj_t *) dsmalloc(sizeof(obj_t));

                sprintf(sname, "amulet.[%d].name", j);
                config_lookup_string(cf, sname, &value);
                strcpy(o->basename, value);
                
                sprintf(sname, "amulet.[%d].brand", j);
                config_lookup_string(cf, sname, &value);
                if(!strcmp(value, "protection")) {                     // This means this amulet is some sort of protection
                        sprintf(sname, "amulet.[%d].type", j);
                        config_lookup_string(cf, sname, &value);

                        if(!strcmp(value, "life")) 
                                add_effect(o, OE_PROTECTION_LIFE);
                        if(!strcmp(value, "fire"))
                                add_effect(o, OE_PROTECTION_FIRE);
                }

                sprintf(sname, "amulet.[%d].unique", j);
                config_lookup_bool(cf, sname, &x);
                if(x)
                        setbit(o->flags, OF_UNIQUE);
                
                sprintf(sname, "amulet.[%d].obvious", j);
                config_lookup_bool(cf, sname, &x);
                if(x)
                        setbit(o->flags, OF_OBVIOUS);

                x = 0;
                /*sprintf(sname, "amulet.[%d].mod", j);
                config_lookup_int(cf, sname, &x);
                o->attackmod = o->damagemod = x;*/

                o->type = OT_AMULET;
                o->id = objid; objid++;

                o->fore = TCOD_white;
                o->back = TCOD_black;

                o->material = mats_amulets[material];
                material++;
                if(material > MATERIALS)
                        die("whoa! we ran out of material!");

                o->head = objdefs->head;
                objdefs->next = o;
                o->next = NULL;
                objdefs = o;

                game->objdefs++;
                //printf(".");
        }

        //printf(" OK\n");

        return 0;
}

int parse_bracelet()
{
        config_setting_t *cfg;
        int i, j, material;
        char sname[100];
        const char *value;

        cfg = config_lookup(cf, "bracelet");
        i = config_setting_length(cfg);
        //printf("Parsing jewelry file... We have %d bracelets", i);
        material = 1;
        for(j = 0; j < i; j++) {
                obj_t *o;
                int x;
                int y;

                o = (obj_t *) dsmalloc(sizeof(obj_t));

                sprintf(sname, "bracelet.[%d].name", j);
                config_lookup_string(cf, sname, &value);
                strcpy(o->basename, value);
                
                for(y = 0; y < 10; y++) {
                        sprintf(sname, "bracelet.[%d].effect.[%d].brand", j, y);
                        config_lookup_string(cf, sname, &value);
                        if(!strcmp(value, "stat")) {                     // This means this bracelet modifies a stat
                                sprintf(sname, "bracelet.[%d].effect.[%d].stat", j, y);
                                config_lookup_string(cf, sname, &value);

                                if(!strcmp(value, "strength")) 
                                        add_effect(o, OE_STRENGTH);
                                if(!strcmp(value, "physique"))
                                        add_effect(o, OE_PHYSIQUE);
                                if(!strcmp(value, "intelligence"))
                                        add_effect(o, OE_INTELLIGENCE);
                                if(!strcmp(value, "wisdom"))
                                        add_effect(o, OE_WISDOM);
                                if(!strcmp(value, "dexterity"))
                                        add_effect(o, OE_DEXTERITY);
                                if(!strcmp(value, "charisma"))
                                        add_effect(o, OE_CHARISMA);
                        }
                }

                sprintf(sname, "bracelet.[%d].unique", j);
                config_lookup_bool(cf, sname, &x);
                if(x)
                        setbit(o->flags, OF_UNIQUE);

                x = 0;
                sprintf(sname, "bracelet.[%d].obvious", j);
                config_lookup_bool(cf, sname, &x);
                if(x)
                        setbit(o->flags, OF_OBVIOUS);

                x = 0;
                sprintf(sname, "bracelet.[%d].mod", j);
                config_lookup_int(cf, sname, &x);
                o->attackmod = o->damagemod = x;

                o->type = OT_BRACELET;
                o->id = objid; objid++;
                o->fore = TCOD_white;
                o->back = TCOD_black;
                clearbit(o->flags, OF_IDENTIFIED);

                o->material = mats_bracelets[material];
                material++;
                if(material > MATERIALS)
                        die("whoa! we ran out of material!");

                o->head = objdefs->head;
                objdefs->next = o;
                o->next = NULL;
                objdefs = o;

                game->objdefs++;
                //printf(".");
        }

        //printf(" OK\n");

        return 0;
}

int parse_jewelry()
{
        int ret;

        ret = parse_bracelet();
        ret = parse_amulet();

        return ret;
}

int parse_objects()
{
        int ret;

        ret = parse_armor();
        ret = parse_weapons();
        ret = parse_jewelry();

        return ret;
}

int parse_configfile()
{
        config_setting_t *cfg;
        int i;
        char sname[100];
        const char *value;

        cfg = config_lookup(cf, "config");
        i = config_setting_length(cfg);
        //printf("Parsing configuration file...");
        if(i > 1) {
                printf("Something is wrong here...?\n");
                return 1;
        }

        sprintf(sname, "config.[0].dxsize");
        config_lookup_int(cf, sname, &dsconfig.dxsize);
        sprintf(sname, "config.[0].dysize");
        config_lookup_int(cf, sname, &dsconfig.dysize);
        sprintf(sname, "config.[0].compress_savefile");
        config_lookup_bool(cf, sname, &dsconfig.compress_savefile);
        sprintf(sname, "config.[0].autopickup");
        config_lookup_string(cf, sname, &value);
        sprintf(sname, "config.[0].fontsize");
        config_lookup_int(cf, sname, &dsconfig.fontsize);
        sprintf(sname, "config.[0].rows");
        config_lookup_int(cf, sname, &dsconfig.rows);
        sprintf(sname, "config.[0].cols");
        config_lookup_int(cf, sname, &dsconfig.cols);

        strcpy(dsconfig.autopickup, value);

        //printf(" OK\n");
        
        return 0;
}

int parse_data_files(int option)
{
        int ret;

        cf = (config_t *) dsmalloc(sizeof(config_t));
        config_init(cf);

        if (!config_read_file(cf, MAIN_DATA_FILE)) {
                fprintf(stderr, "%s:%d - %s\n",
                                config_error_file(cf),
                                config_error_line(cf),
                                config_error_text(cf));
                config_destroy(cf);
                return(1);
        }

        objid = 1;
        //printf("Reading %s\n", MAIN_DATA_FILE);

        if(option == ONLY_CONFIG) {
                ret = parse_configfile();
                config_destroy(cf);
                return ret;
        }

        /* TODO:
         * This return value stuff makes rather little sense!!
         */
        ret = parse_configfile();
        ret = parse_objects();
        ret = parse_monsters();

        config_destroy(cf);

        ret = parse_areadef_files();

        return ret;
}
// vim: fdm=syntax 
