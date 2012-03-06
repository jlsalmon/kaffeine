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

#define VALID_ADDITIONS "Milk types: Cream, Half-and-half, Whole-milk, Part-skim, Skim, Non-dairy\nSyrup types: Vanilla, Almond, Raspberry\nSweetener types: White-sugar, Sweetener, Raw-cane, Honey\nSpice types: Cinnamon, Cardamom\nAlcohol types: Brandy, Rum, Whiskey, Aquavit, Kahlua\nVolume units: [1-5] or dash, splash, little, medium, lots\n"
#define BEVERAGE        "              ) (\n               (    )\n              ____(___ \n          _|`--------`| \n         (C|          |__ \n       /` `\\          /  `\\ \n       \\    `========`    / \n        `'--------------'`\n"

void off_action();
void brewing_action();
void pouring_action();
void ready_action();
void null_action();
void init_pots();

typedef void (*tfp) (void);

typedef struct {
    int next_state;
    tfp action;
} pot_state_table;

typedef struct {
    int pot_id;
    int current_state;
    pot_state_table states[NUM_STATES][NUM_EVENTS];
} pot_struct;

#endif /* VCP_H_ */
