/*
 * vcp.h
 *
 *  Created on: Feb 28, 2012
 *      Author: jl2-salmon
 */

#ifndef VCP_H_
#define VCP_H_

#define STATE_OFF 	0
#define STATE_READY	1
#define STATE_BREWING   2
#define STATE_POURING   3
#define NUM_STATES 	4

#define EVENT_BREW	0
#define EVENT_STOP	1
#define EVENT_POUR	2
#define EVENT_READY	3
#define NUM_EVENTS	4

#define NUM_POTS 	5
#define TEAPOT          5

#define ERR_OFF         -1
#define ERR_BUSY        -2
#define ERR_CUP_COLD    -3
#define ERR_OVERFLOW    -4
#define ERR_TEAPOT      -5

#define VALID_ADDITIONS "Milk types:\tCream, Half-and-half, Whole-milk, Part-skim, Skim, Non-dairy\nSyrup types:\tVanilla, Almond, Raspberry\nSweeteners:\tWhite-sugar, Sweetener, Raw-cane, Honey\nSpice types:\tCinnamon, Cardamom\nAlcohol types:\tBrandy, Rum, Whiskey, Aquavit, Kahlua\nVolume units:\t[1-5], dash, splash, little, medium, lots\n"
#define BEVERAGE        "               ) (\n              (    )\n             ____(___ \n          _|`--------`| \n         (C|          |__ \n       /` `\\          /  `\\ \n       \\    `========`    / \n        `'--------------'`\n"

#define BREWING_TIME    20
#define T_TO_COLD       60
#define T_TO_OVERFLOW   10

#define TRUE            1
#define FALSE           0

typedef void (*tfp) ();

typedef struct {
    int next_state;
    tfp action;
} pot_state_table;

typedef struct {
    int pot_id;
    int current_state;
    pot_state_table states[NUM_STATES][NUM_EVENTS];
} pot_struct;

int propfind(pot_struct*, char*);
int brew(pot_struct*, char*);
int get(pot_struct*, char*);
int when(pot_struct*);

void off_action();
void brewing_action(pot_struct*);
void pouring_action();
void ready_action();
void null_action();
void init_pot(pot_struct*, int);

#endif /* VCP_H_ */
