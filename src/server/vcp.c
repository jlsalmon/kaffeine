/*
 * vcp.c
 *
 *  Created on: Feb 27, 2012
 *      Author: jl2-salmon
 */
#include <stdio.h>
#include <string.h>
#include "vcp.h"

typedef void (*tfp) (char*);

typedef struct {
    int next_state;
    tfp action;
} pot_state_table;

typedef struct {
    char pot_id[10];
    int current_state;
    pot_state_table states[NUM_STATES][NUM_EVENTS];
} pot_struct;

pot_struct pots[NUM_POTS];
int last_state = STATE_OFF;

int propfind(char* pot_id, char* response) {

    if (strcmp(pot_id, TEAPOT) == 0) {
        return ERR_TEAPOT;
    } else {
        char val_adds[30];
        sprintf(val_adds, "Valid additions for %s: \n\n", pot_id);
        strcat(response, val_adds);
        strcat(response, VALID_ADDITIONS);
        return TRUE;
    }
}

int brew(char* pot_id, char* adds) {
    int event = EVENT_BREW;

    for (int i = 0; i < NUM_POTS; ++i) {
        if (strcmp(pot_id, pots[i].pot_id) == 0) {

            if (pots[i].current_state == STATE_OFF) {
                return ERR_OFF;
            } else if (pots[i].current_state == STATE_BREWING
                    || pots[i].current_state == STATE_POURING) {
                return ERR_BUSY;
            } else if (strcmp(pot_id, TEAPOT) == 0) {
                return ERR_TEAPOT;
            } else if (pots[i].current_state == STATE_READY) {

                pots[i].states[last_state][event].action(pot_id);
                pots[i].current_state = pots[i].states[last_state][event].next_state;
            }
        }
    }
    return TRUE;
}

int get(char* pot_id, char* adds) {
    return TRUE;
}

int when(char* pot_id) {
    return TRUE;
}

void init_pots() {

    fprintf(stderr, "Initialising virtual coffee pots...\n");

    for (int i = 0; i < NUM_POTS; ++i) {

        snprintf(pots[i].pot_id, sizeof (pots[i].pot_id), "pot-%d", i);
        pots[i].current_state = STATE_READY;

        for (int j = 0; j < NUM_STATES; ++j) {
            for (int k = 0; k < NUM_EVENTS; ++k) {
                pots[i].states[j][k].next_state = STATE_OFF;
                pots[i].states[j][k].action = null_action;
            }
        }

        /* From STATE_OFF, only STATE_READY and STATE_BREWING are valid */
        pots[i].states[STATE_OFF][EVENT_BREW].next_state = STATE_BREWING;
        pots[i].states[STATE_OFF][EVENT_BREW].action = brewing_action;
        pots[i].states[STATE_OFF][EVENT_READY].next_state = STATE_READY;
        pots[i].states[STATE_OFF][EVENT_READY].action = ready_action;
        /* From STATE_READY, valid states are STATE_OFF and STATE_BREWING */
        pots[i].states[STATE_READY][EVENT_STOP].next_state = STATE_OFF;
        pots[i].states[STATE_READY][EVENT_STOP].action = off_action;
        pots[i].states[STATE_READY][EVENT_BREW].next_state = STATE_BREWING;
        pots[i].states[STATE_READY][EVENT_BREW].action = brewing_action;
        /* From STATE_BREWING, both EVENT_OFF and EVENT_POURING are valid */
        pots[i].states[STATE_BREWING][EVENT_STOP].next_state = STATE_OFF;
        pots[i].states[STATE_BREWING][EVENT_STOP].action = off_action;
        pots[i].states[STATE_BREWING][EVENT_POUR].next_state = STATE_POURING;
        pots[i].states[STATE_BREWING][EVENT_POUR].action = pouring_action;
        /* From STATE_POURING, only STATE_READY is valid */
        pots[i].states[STATE_POURING][EVENT_READY].next_state = STATE_READY;
        pots[i].states[STATE_POURING][EVENT_READY].action = ready_action;
    }
    fprintf(stderr, "Pots initialised.\n");
}

void off_action() {
    printf("Switching off\n");
}

void brewing_action(char* pot_id) {
    printf("Brewing on %s...\n", pot_id);
}

void pouring_action() {
    printf("Pouring...\n");
}

void ready_action() {
    printf("Ready\n");
}

void null_action() {
    //do nothing
}
