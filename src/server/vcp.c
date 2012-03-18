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
    strcat(response, VAL_ADDS_STR);
    return TRUE;
}

int brew(pot_struct *pot, char* adds) {
    int event = EVENT_BREW;

    if (!validate_adds(adds)) {
        return E_INVALID_ADDS;
    } else if (pot->states[pot->current_state][event].error) {
        return pot->states[pot->current_state][event].error;
    }

    pot->adds = adds;
    pot->states[pot->current_state][event].action(pot);
    pot->current_state = pot->states[pot->current_state]
            [event].next_state;

    return SUCCESS;
}

int get(pot_struct *pot, char* adds, char* response) {
    int event;

    if (pot->current_state == STATE_OFF) {
        return E_OFF;
    }

    if (adds == NULL)
        event = EVENT_COLLECT;
    else
        event = EVENT_BREW;

    switch (event) {
        case EVENT_BREW:
            return brew(pot, adds);
            break;
        case EVENT_COLLECT:
            if (pot->states[pot->current_state][event].error) {
                return pot->states[pot->current_state][event].error;
            } else if (difftime(time(NULL), pot->brew_end_time)
                    >= (BREWING_TIME + T_TO_COLD)) {
                return E_CUP_COLD;
            } else if (pot->adds != NULL) {
                return E_WAITING_ADDS;
            } else {
                pot->states[pot->current_state][event].action(pot);
                init_pot(pot, pot->pot_id);
                strcat(response, BEVERAGE);
            }
            break;
        default:
            break;
    }
    return SUCCESS;
}

int pour(pot_struct *pot) {
    int event = EVENT_POUR;

    if (pot->states[pot->current_state][event].error) {
        return pot->states[pot->current_state][event].error;
    }

    pot->states[pot->current_state][event].action(pot);
    pot->current_state = pot->states[pot->current_state]
            [event].next_state;
    pot->adds = NULL;

    return SUCCESS;
}

int when(pot_struct *pot) {
    int event = EVENT_STOP;

    if (pot->states[pot->current_state][event].error) {
        return pot->states[pot->current_state][event].error;
    }

    pot->states[pot->current_state][event].action(pot);
    pot->current_state = pot->states[pot->current_state]
            [event].next_state;

    return SUCCESS;
}

void brewing_action(pot_struct *pot) {
    signal(SIGALRM, brew_alarm);
    alarm(BREWING_TIME);
    pot->brew_end_time = time(NULL) + BREWING_TIME;
    sprintf(buf, "Brewing on Pot %d...", pot->pot_id);
    log(buf);
}

void pouring_action(pot_struct *pot) {
    signal(SIGALRM, pour_alarm);
    alarm(POURING_TIME);
    pot->pour_end_time = time(NULL) + POURING_TIME;
    sprintf(buf, "Pouring on Pot %d...", pot->pot_id);
    log(buf);
}

void waiting_action(pot_struct *pot) {
    if (pot->adds == NULL) {
        sprintf(buf, "Coffee waiting for collection on pot %d", pot->pot_id);
    } else {
        sprintf(buf, "Cup waiting to pour on pot %d", pot->pot_id);
    }
    log(buf);
}

void ready_action(pot_struct *pot) {
    sprintf(buf, "Pot %d order complete.", pot->pot_id);
    log(buf);
}

void off_action(pot_struct *pot) {
    sprintf(buf, "Switching off");
    log(buf);
}

void null_action() {
    //do nothing
}

void brew_alarm(int sig) {
    log("Brewing finished");
    int event = EVENT_STOP;

    for (int i = 0; i < NUM_POTS; ++i) {
        if (difftime(time(NULL), pots[i].brew_end_time)
                <= BREWING_TIME) {
            if (pots[i].current_state == STATE_BREWING
                    || pots[i].current_state == STATE_POURING) {

                pots[i].states[pots[i].current_state][event].action(&pots[i]);
                pots[i].current_state = pots[i].states[pots[i].current_state]
                        [event].next_state;
            }
        }
    }
}

void pour_alarm(int sig) {
    log("Cup overflowed");
    int event = EVENT_STOP;
}

int validate_adds(char* adds) {
    fprintf(stderr, "%s\n", adds);

    const char delimiters[] = " &";
    char *header, *add = NULL, *type = NULL, *quant = NULL;
    char *adds_arr[MAX_ADDS];
    int i = 0;

    header = strtok(adds, delimiters);
    add = strtok(NULL, delimiters);
    adds_arr[i++] = add;

    while (add != NULL) {
        printf("add = \"%s\"\n", add);
        add = strtok(NULL, delimiters);

        if (i == MAX_ADDS - 1) {
            return FALSE;
        }
        adds_arr[i++] = add;
    }

    for (int i = 0; i < MAX_ADDS; i++) {
        if (adds_arr[i] == NULL) break;
        fprintf(stderr, "add: %s\n", adds_arr[i]);
        type = strtok(adds_arr[i], "=");
        quant = strtok(NULL, "=");

        if (!valid_add(type) || !valid_add(quant)) {
            return FALSE;
        }

        fprintf(stderr, "type: %s, quant: %s\n", type, quant);
    }

    return TRUE;
}

int valid_add(char* add) {
    char* val_adds[] = VAL_ADDS_ARR;

    for (int i = 0; i < VAL_ADDS_ARR_LEN; i++) {
        if (strcasecmp(add, val_adds[i]) == 0) {
            return TRUE;
        }
    }
    return FALSE;
}

void init_pot(pot_struct *pot, int id) {

    pot->pot_id = id;
    pot->current_thread = 0;
    pot->current_state = STATE_READY;
    pot->adds = NULL;

    for (int j = 0; j < NUM_STATES; ++j) {
        for (int k = 0; k < NUM_EVENTS; ++k) {
            pot->states[j][k].action = null_action;
            pot->states[j][k].error = FALSE;
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

    char buf[20];
    sprintf(buf, "Pot %d ready.", pot->pot_id);
    log(buf);
}

void calc_etc(char* response, pot_struct* pot) {
    time_t etc = pot->brew_end_time - time(NULL);
    sprintf(buf, "ETC: %d seconds\r\n", (int) etc);
    strcat(response, buf);
}
