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

    if (pot->current_state == STATE_OFF) {
        return E_OFF;
    } else if (pot->current_state == STATE_BREWING
            || pot->current_state == STATE_POURING
            || pot->current_state == STATE_WAITING) {
        return E_BUSY;
    } else if (pot->current_state == STATE_READY) {
        pot->states[pot->current_state][event].action(pot);
        pot->current_state = pot->states[pot->current_state]
                [event].next_state;
    }
    return TRUE;
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
            if (pot->current_state == STATE_BREWING) {
                return E_BUSY;
            } else if (pot->current_state == STATE_READY) {
                pot->states[pot->current_state][event].action(pot->pot_id);
                pot->current_state = pot->states
                        [pot->current_state][event].next_state;
            }
            break;
        case EVENT_COLLECT:
            if (pot->current_state == STATE_BREWING) {

                if (difftime(time(NULL), pot->brew_end_time)
                        >= (BREWING_TIME + T_TO_COLD)) {
                    return E_CUP_COLD;
                } else if (difftime(time(NULL), pot->brew_end_time)
                        >= BREWING_TIME) {
                    pot->states[pot->current_state][event].action(pot->pot_id);
                    pot->current_state = pot->states
                            [pot->current_state][event].next_state;
                    strcat(response, BEVERAGE);
                } else {
                    return E_STILL_BREWING;
                }
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

            pot->states[j][k].next_state = STATE_OFF;
            pot->states[j][k].action = null_action;
        }
    }

    pot->states[STATE_READY][EVENT_BREW].next_state = STATE_BREWING;
    pot->states[STATE_READY][EVENT_BREW].action = brewing_action;

    pot->states[STATE_BREWING][EVENT_STOP].next_state = STATE_WAITING;
    pot->states[STATE_BREWING][EVENT_STOP].action = waiting_action;

    pot->states[STATE_WAITING][EVENT_POUR].next_state = STATE_POURING;
    pot->states[STATE_WAITING][EVENT_POUR].action = pouring_action;
    pot->states[STATE_WAITING][EVENT_COLLECT].next_state = STATE_READY;
    pot->states[STATE_WAITING][EVENT_COLLECT].action = ready_action;

    pot->states[STATE_POURING][EVENT_STOP].next_state = STATE_WAITING;
    pot->states[STATE_POURING][EVENT_STOP].action = waiting_action;
}

void brewing_action(pot_struct *pot) {
    signal(SIGALRM, catch_alarm);
    alarm(BREWING_TIME);
    pot->brew_end_time = time(NULL) + BREWING_TIME;
    printf("Brewing on Pot %d...\n", pot->pot_id);
}

void pouring_action() {

    printf("Pouring...\n");
}

void waiting_action(pthread_t tid) {
    for (int i = 0; i < NUM_POTS; ++i) {
        if (difftime(time(NULL), pots[i].brew_end_time)
                <= BREWING_TIME) {
            pots[i].current_state = STATE_WAITING;
            printf("Coffee waiting for collection on pot %d\n", i);
        }
    }

}

void ready_action(pot_struct *pot) {

    printf("Pot %d finished brewing.\n", pot->pot_id);
    printf("Ready\n");
}

void off_action() {
    printf("Switching off\n");
}

void null_action() {
    //do nothing
}

void catch_alarm(int sig) {
    puts("signal caught");
    waiting_action(pthread_self());
    signal(sig, catch_alarm);
}