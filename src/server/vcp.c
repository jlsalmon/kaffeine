/*
 * vcp.c
 *
 *  Created on: Feb 27, 2012
 *      Author: jl2-salmon
 * Description: Virtual Coffee Pot
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include "vcp.h"

int propfind(pot_struct *pot, char* response) {
    char val_adds[30];
    sprintf(val_adds, "Valid additions for Pot %d: \n\n", pot->pot_id);
    strcat(response, val_adds);
    strcat(response, VALID_ADDITIONS);
    return TRUE;
}

int brew(pot_struct *pot, char* adds) {
    int event = EVENT_BREW;

    pot->states[pot->current_state][event].action(pot);
    pot->current_state = pot->states[pot->current_state]
            [event].next_state;
    return pot->states[pot->current_state][event].error;
}

int get(pot_struct *pot, char* adds, char* response) {
    int event = EVENT_COLLECT;

    if (pot->current_state == STATE_OFF) {
        return E_OFF;
    }

    /*
        if (adds == NULL)
            event = EVENT_READY;
        else
            event = EVENT_BREW;
     */

    switch (event) {
        case EVENT_BREW:
                pot->states[pot->current_state][event].action(pot->pot_id);
                pot->current_state = pot->states
                        [pot->current_state][event].next_state;
                return pot->states[pot->current_state][event].error;

        case EVENT_COLLECT:

            switch (pot->current_state) {
                case STATE_BREWING:
                    return E_STILL_BREWING;
                    break;

                case STATE_WAITING:

                    if (difftime(time(NULL), pot->brew_end_time)
                            >= (BREWING_TIME + T_TO_COLD)) {
                        return E_CUP_COLD;
                    } else {

                        pot->states[pot->current_state][event].action(pot);
                        pot->current_state = pot->states
                                [pot->current_state][event].next_state;
                        strcat(response, BEVERAGE);
                    }
                    break;
            }
            break;
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
            pot->states[j][k].action = null_action;
            pot->states[j][k].error = NULL;
        }
    }

    pot->states[STATE_READY][EVENT_BREW].next_state = STATE_BREWING;
    pot->states[STATE_READY][EVENT_BREW].action = brewing_action;
    pot->states[STATE_READY][EVENT_STOP].error = E_NOT_POURING;
    pot->states[STATE_READY][EVENT_POUR].error = E_NO_CUP;
    pot->states[STATE_READY][EVENT_COLLECT].error = E_NO_CUP;

    pot->states[STATE_BREWING][EVENT_STOP].next_state = STATE_WAITING;
    pot->states[STATE_BREWING][EVENT_STOP].action = waiting_action;
    pot->states[STATE_BREWING][EVENT_BREW].error = E_ALRDY_BREWING;
    pot->states[STATE_BREWING][EVENT_POUR].error = E_STILL_BREWING;
    pot->states[STATE_BREWING][EVENT_COLLECT].error = E_STILL_BREWING;

    pot->states[STATE_WAITING][EVENT_POUR].next_state = STATE_POURING;
    pot->states[STATE_WAITING][EVENT_POUR].action = pouring_action;
    pot->states[STATE_WAITING][EVENT_COLLECT].next_state = STATE_READY;
    pot->states[STATE_WAITING][EVENT_COLLECT].action = ready_action;
    pot->states[STATE_WAITING][EVENT_BREW].error = E_CUP_WAITING;
    pot->states[STATE_WAITING][EVENT_STOP].error = E_NOT_POURING;

    pot->states[STATE_POURING][EVENT_STOP].next_state = STATE_WAITING;
    pot->states[STATE_POURING][EVENT_STOP].action = waiting_action;
    pot->states[STATE_POURING][EVENT_BREW].error = E_BUSY;
    pot->states[STATE_POURING][EVENT_POUR].error = E_ALRDY_POURING;
    pot->states[STATE_POURING][EVENT_COLLECT].error = E_STILL_POURING;
}

void brewing_action(pot_struct *pot) {
    signal(SIGALRM, catch_alarm);
    alarm(BREWING_TIME);
    pot->brew_end_time = time(NULL) + BREWING_TIME;
    printf("Brewing on Pot %d...\n", pot->pot_id);
}

void pouring_action(pot_struct *pot) {

    printf("Pouring...\n");
}

void waiting_action(pot_struct *pot) {


}

void ready_action(pot_struct *pot) {

    printf("Pot %d finished brewing.\n", pot->pot_id);
    printf("Ready\n");
}

void off_action(pot_struct *pot) {
    printf("Switching off\n");
}

void null_action() {
    //do nothing
}

void catch_alarm(int sig) {
    puts("signal caught");
    int event = EVENT_STOP;

    for (int i = 0; i < NUM_POTS; ++i) {
        if (difftime(time(NULL), pots[i].brew_end_time)
                <= BREWING_TIME) {
            if (pots[i].current_state == STATE_BREWING
                    || pots[i].current_state == STATE_POURING) {

                pots[i].states[pots[i].current_state][event].action(&pots[i]);
                pots[i].current_state = pots[i].states[pots[i].current_state]
                        [event].next_state;

                printf("Coffee waiting for collection on pot %d\n", i);
            }
        }
    }


    signal(sig, catch_alarm);
}