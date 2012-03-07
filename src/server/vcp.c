/*
 * vcp.c
 *
 *  Created on: Feb 27, 2012
 *      Author: jl2-salmon
 * Description: Virtual Coffee Pot
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "vcp.h"

int last_state = STATE_OFF;

int propfind(pot_struct *pot, char* response) {

    if (pot->pot_id == TEAPOT) {
        return ERR_TEAPOT;
    } else {
        char val_adds[30];
        sprintf(val_adds, "Valid additions for Pot %d: \n\n", pot->pot_id);
        strcat(response, val_adds);
        strcat(response, VALID_ADDITIONS);
        return TRUE;
    }
}

int brew(pot_struct *pot, char* adds) {
    int event = EVENT_BREW;

    if (pot->current_state == STATE_OFF) {
        return ERR_OFF;
    } else if (pot->current_state == STATE_BREWING
            || pot->current_state == STATE_POURING) {
        return ERR_BUSY;
    } else if (pot->pot_id == TEAPOT) {
        return ERR_TEAPOT;
    } else if (pot->current_state == STATE_READY) {

        pot->states[last_state][event].action(pot);
        pot->current_state = pot->states[last_state][event].next_state;
    }

    return TRUE;
}

int get(pot_struct *pot, char* adds) {
    int event;

    if (adds == NULL) {
        event = EVENT_READY;
    } else {
        event = EVENT_BREW;
    }

    if (pot->current_state == STATE_OFF) {
        return ERR_OFF;
    } else if (pot->current_state == STATE_BREWING
            || pot->current_state == STATE_POURING) {
        return ERR_BUSY;
    } else if (pot->pot_id == TEAPOT) {
        return ERR_TEAPOT;
    } else if (pot->current_state == STATE_READY) {

        pot->states[last_state][event].action(pot->pot_id);
        pot->current_state = pot->states[last_state][event].next_state;
    }

    return TRUE;
}

int when(pot_struct *pot) {
    return TRUE;
}

void init_pot(pot_struct *pot, int id) {

    pot->pot_id = id;
    pot->current_state = STATE_READY;

    for (int j = 0; j < NUM_STATES; ++j) {
        for (int k = 0; k < NUM_EVENTS; ++k) {
            pot->states[j][k].next_state = STATE_OFF;
            pot->states[j][k].action = null_action;
        }
    }

    /* From STATE_OFF, only STATE_READY and STATE_BREWING are valid */
    pot->states[STATE_OFF][EVENT_BREW].next_state = STATE_BREWING;
    pot->states[STATE_OFF][EVENT_BREW].action = brewing_action;
    pot->states[STATE_OFF][EVENT_READY].next_state = STATE_READY;
    pot->states[STATE_OFF][EVENT_READY].action = ready_action;
    /* From STATE_READY, valid states are STATE_OFF and STATE_BREWING */
    pot->states[STATE_READY][EVENT_STOP].next_state = STATE_OFF;
    pot->states[STATE_READY][EVENT_STOP].action = off_action;
    pot->states[STATE_READY][EVENT_BREW].next_state = STATE_BREWING;
    pot->states[STATE_READY][EVENT_BREW].action = brewing_action;
    /* From STATE_BREWING, both EVENT_OFF and EVENT_POURING are valid */
    pot->states[STATE_BREWING][EVENT_STOP].next_state = STATE_OFF;
    pot->states[STATE_BREWING][EVENT_STOP].action = off_action;
    pot->states[STATE_BREWING][EVENT_POUR].next_state = STATE_POURING;
    pot->states[STATE_BREWING][EVENT_POUR].action = pouring_action;
    /* From STATE_POURING, only STATE_READY is valid */
    pot->states[STATE_POURING][EVENT_READY].next_state = STATE_READY;
    pot->states[STATE_POURING][EVENT_READY].action = ready_action;

}

void off_action() {
    printf("Switching off\n");
}

void brewing_action(pot_struct *pot) {
    printf("Brewing on Pot %d...\n", pot->pot_id);
    sleep(BREWING_TIME);
    printf("Pot %d finished brewing.\n", pot->pot_id);
    pot->current_state = STATE_READY;
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
