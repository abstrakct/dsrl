/*
 * Dark Shadows - The Roguelike
 *
 * Copyright 2011 Rolf Klausen
 */
#ifndef _DS_UTILS_H
#define _DS_UTILS_H

#ifdef DS_USE_NCURSES
#include <curses.h>
#else
#include <libtcod/libtcod.h>
#endif

#define clearbit(a, b) ((a) &= ~(b))
#define   setbit(a, b) ((a) |=  (b))
#define   hasbit(a, b) ((a) &   (b))

#define SGN(a) (((a)<0) ? -1 : 1)

#define MAX_GARBAGE 100000
// d(a, b)
// "throw" a b-sided dices without any modifiers
#define d(a,b) dice(a,b,0)

#define min(a, b) (a > b ? b : a)
#define max(a, b) (a > b ? a : b)

// ri(a, b)
// pick a random number in the range [a, b]
//#define ri(a,b) (a + (rand() % (b-a+1)))

void get_version_string(char *s);
void die(char *m, ...);

void  *dsmalloc(size_t size);
void  *dscalloc(size_t num, size_t size);
void **dsmalloc2d(int y, int x, size_t size);
void   dsfree(void *ptr);

int dice(int num, int sides, signed int modifier);
int perc(int i);
int ri(int a, int b);

void you(char *fmt, ...);
void youc(int color, char *fmt, ...);
void yousee(char *fmt, ...);
void dsprintf(char *fmt, ...);
void dsprintfc(int color, char *fmt, ...);
#ifdef DS_USE_NCURSES
void dsprintfwc(WINDOW *win, int color, char *fmt, ...);
char ask_char(char *question);
char ask_for_hand();
#else
TCOD_key_t ask_char(char *question);
TCOD_key_t ask_for_hand();
#endif

bool yesno(char *fmt, ...);
void more();

char *a_an(char *s);
char *Upper(char *s);

extern int garbageindex;
extern void *garbage[MAX_GARBAGE];
#endif
