/**
 * File:        vcp.c
 * Author:      Justin Lewis Salmon
 * Student ID:  10000937
 * Created on:  27 February 2012
 * 
 * The Virtual Coffee Pot - controllable via the brew(), get(), pour(),
 * when() and propfind() methods. Attempts to emulate a real networked
 * coffee pot.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include "vcp.h"

/**
 * Finds a list of valid additions for the supplied pot.
 * 
 * @param pot
 *              the pot to find additions for.
 * @param response
 *              pointer to the response message buffer.
 * @return true, this method always succeeds.
 */
int propfind(pot_struct *pot, char* response) {
    char val_adds[30];
    char state[30];

    char *state_str = get_state_str(pot->current_state);
    sprintf(state, "Pot %d state: \t%s\n", pot->pot_id, state_str);
    sprintf(val_adds, "Valid additions for Pot %d: \n\n", pot->pot_id);
    strcat(response, state);
    strcat(response, val_adds);
    strcat(response, VAL_ADDS_STR);
    return TRUE;
}

/**
 * Begin brewing on the specified pot with the specified additions,
 * following addition validation and making sure that the pot is 
 * capable of brewing in its current state.
 * 
 * @param pot
 *              the pot to brew on.
 * @param adds
 *              the list of additions.
 * @return E_INVALID_ADDS if the supplied additions were invalid, 
 *         an appropriate error code if the pot cannot transition
 *         to a brewing state currently, SUCCESS otherwise.
 */
int brew(pot_struct *pot, char* adds) {
    int event = EVENT_BREW;

    if (!validate_adds(adds)) {
        return E_INVALID_ADDS;
    } else if (pot->states[pot->current_state][event].error) {
        return pot->states[pot->current_state][event].error;
    }

    pot->adds = adds;
    if (pot->adds != NULL
            && strstr(pot->adds, "unspecified") != NULL) {
        pot->waiting_adds = TRUE;
    }

    pot->states[pot->current_state][event].action(pot);
    pot->current_state = pot->states[pot->current_state]
            [event].next_state;

    return SUCCESS;
}

/**
 * Either return a brewed cup of coffee, or begin brewing a new cup
 * based on the value of the supplied additions. If the additions
 * are null, a collection request is assumed.
 * 
 * @param pot
 *              the pot to brew on, or to collect from.
 * @param adds
 *              the additions to (potentially) add.
 * @param response
 *              pointer to the response message buffer.
 * @return SUCCESS on brewing/collection, or an appropriate error
 *         code on failure.
 */
int get(pot_struct *pot, char* adds, char* response) {
    int event;

    if (pot->current_state == STATE_OFF) {
        return E_OFF;
    }

    if (adds == NULL) event = EVENT_COLLECT;
    else event = EVENT_BREW;

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
            } else if (pot->waiting_adds) {
                return E_WAITING_ADDS;
            } else {
                pot->states[pot->current_state][event].action(pot);
                strcat(response, BEVERAGE);

                if (pot->adds != NULL) {
                    format_adds(pot->adds);
                    strcat(response, pot->adds);
                } else {
                    strcat(response, "Your order: no additions");
                }
                init_pot(pot, pot->pot_id);
            }
            break;
    }
    return SUCCESS;
}

/**
 * Begin pouring additions on the specified pot.
 * 
 * @param pot
 *              the pot to begin pouring on.
 * @return SUCCESS on pouring start, or an appropriate error code
 *         if the pot cannot transition to a pouring state.
 */
int pour(pot_struct *pot) {
    int event = EVENT_POUR;

    if (pot->states[pot->current_state][event].error) {
        return pot->states[pot->current_state][event].error;
    }

    pot->states[pot->current_state][event].action(pot);
    pot->current_state = pot->states[pot->current_state]
            [event].next_state;

    return SUCCESS;
}

/**
 * Stop pouring on the specified pot.
 * 
 * @param pot
 *              the pot to stop pouring on.
 * @return SUCCESS on pouring stop, E_OVERFLOW if the pot has overflowed,
 *         or an appropriate error code if the pot is not pouring.
 */
int when(pot_struct *pot) {
    int event = EVENT_STOP;

    if (pot->states[pot->current_state][event].error) {
        return pot->states[pot->current_state][event].error;
    } else if (difftime(time(NULL), pot->pour_end_time) > 0) {
        return E_OVERFLOW;
    } else {
        pot->states[pot->current_state][event].action(pot);
        pot->current_state = pot->states[pot->current_state]
                [event].next_state;
    }
    return SUCCESS;
}

/**
 * Called automatically from a function pointer in the state table
 * of the specified pot when a brew request was successful.
 * 
 * @param pot
 *              the pot that just commenced brewing.
 */
void brewing_action(pot_struct *pot) {
    /* Set an alarm to finish brewing */
    signal(SIGALRM, brew_alarm);
    alarm(BREWING_TIME);
    pot->brew_end_time = time(NULL) + BREWING_TIME;
    sprintf(buf, "Brewing on Pot %d...", pot->pot_id);
    log(buf);
}

/**
 * Called automatically from a function pointer in the state table
 * of the specified pot when a pour request was successful.
 * 
 * @param pot
 *              the pot that just commenced pouring.
 */
void pouring_action(pot_struct *pot) {
    signal(SIGALRM, pour_alarm);
    alarm(POURING_TIME);
    pot->pour_end_time = time(NULL) + POURING_TIME;
    pot->waiting_adds = FALSE;
    sprintf(buf, "Pouring %s on Pot %d...", pot->adds, pot->pot_id);
    log(buf);
}

/**
 * Called automatically from a function pointer in the state table
 * of the specified pot when it has finished brewing.
 * 
 * @param pot
 *              the pot that just finished brewing.
 */
void waiting_action(pot_struct *pot) {
    if (pot->waiting_adds) {
        sprintf(buf, "Cup waiting to pour on pot %d", pot->pot_id);
    } else {
        sprintf(buf, "Coffee waiting for collection on pot %d"
                , pot->pot_id);
    }
    log(buf);
}

/**
 * Called automatically from a function pointer in the state table
 * of the specified pot when a cup is collected.
 * 
 * @param pot
 *              the pot that just had a cup collected.
 */
void ready_action(pot_struct *pot) {
    sprintf(buf, "Pot %d order complete.", pot->pot_id);
    log(buf);
}

/**
 * Called automatically from a function pointer in the state table
 * of the specified pot when it is switched off.
 * 
 * @param pot
 *              the pot that just switched off.
 */
void off_action(pot_struct *pot) {
    sprintf(buf, "Switching off");
    log(buf);
}

/**
 * Dummy action for an invalid state/event combination. Should never
 * be called.
 */
void null_action() {
    sprintf(buf, "Invalid event for current state");
    log(buf);
}

/**
 * Catches a SIGALRM and stops any pots that started brewing roughly
 * BREWING_TIME seconds ago.
 * 
 * @param sig
 *              the signal that was caught.
 */
void brew_alarm(int sig) {
    log("Brewing finished");
    int event = EVENT_STOP;

    for (int i = 0; i < NUM_POTS; ++i) {
        if (difftime(time(NULL), pots[i].brew_end_time)
                <= BREWING_TIME) {
            if (pots[i].current_state == STATE_BREWING) {
                pots[i].states[pots[i].current_state][event].action(&pots[i]);
                pots[i].current_state = pots[i].states[pots[i].current_state]
                        [event].next_state;
            }
        }
    }
}

/**
 * Catches a SIGALRM and simply logs the event.
 * 
 * @param sig
 *              the signal that was caught.
 */
void pour_alarm(int sig) {
    log("Pour alarm caught");
}

/**
 * Verifies that the supplied additions are valid HTCPCP additions.
 * 
 * @param adds
 *              the Accept-Additions header of an HTCPCP request.
 * @return true if the additions validated, false otherwise.
 */
int validate_adds(char* adds) {
    if (adds == NULL) {
        return TRUE;
    }

    const char delimiters[] = " &";
    char *header, *add = NULL, *type = NULL, *quant = NULL;
    char *adds_arr[MAX_ADDS], *addscpy;
    addscpy = strdup(adds);
    int i = 0;

    header = strtok(addscpy, delimiters);
    add = strtok(NULL, delimiters);
    adds_arr[i++] = add;

    while (add != NULL) {
        add = strtok(NULL, delimiters);

        if (i == MAX_ADDS - 1) {
            return FALSE;
        }
        adds_arr[i++] = add;
    }

    for (int i = 0; i < MAX_ADDS; i++) {
        if (adds_arr[i] == NULL) break;
        type = strtok(adds_arr[i], "=");
        quant = strtok(NULL, "=");

        if (!valid_add(type) || !valid_add(quant)) {
            return FALSE;
        }
    }

    return TRUE;
}

/**
 * Checks if a single addition is on the list of valid additions.
 * 
 * @param add
 *              the addition to validate
 * @return true if the addition is valid, false otherwise.
 */
int valid_add(char* add) {
    char* val_adds[] = VAL_ADDS_ARR;

    for (int i = 0; i < VAL_ADDS_ARR_LEN; i++) {
        if (strcasecmp(add, val_adds[i]) == 0) {
            return TRUE;
        }
    }
    return FALSE;
}

/**
 * Formats an HTCPCP addition-list into a readable state.
 * 
 * @param adds
 *              the additions-list to prettify.
 */
void format_adds(char* adds) {
    char *newstr = malloc(128);
    newstr = replace(adds, "=", ": ");
    newstr = replace(newstr, "&", ", ");
    newstr = replace(newstr, "unspecified", "custom amount");
    newstr = replace(newstr, "Accept-Additions: ", "");
    strcpy(adds, "Your order: ");
    strcat(adds, newstr);
}

/**
 * Replaces all occurences of "old" with "new" in "s".
 */
char *replace(char *s, char *old, char *new) {
    char *ret;
    int i, count = 0;
    size_t newlen = strlen(new);
    size_t oldlen = strlen(old);

    for (i = 0; s[i] != '\0'; i++) {
        if (strstr(&s[i], old) == &s[i]) {
            count++;
            i += oldlen - 1;
        }
    }

    ret = malloc(i + count * (newlen - oldlen));
    if (ret == NULL)
        exit(EXIT_FAILURE);

    i = 0;
    while (*s) {
        if (strstr(s, old) == s) {
            strcpy(&ret[i], new);
            i += newlen;
            s += oldlen;
        } else
            ret[i++] = *s++;
    }
    ret[i] = '\0';

    return ret;
}

/**
 * Returns the string representation of the given VCP state integer.
 * 
 * @param state
 *              the integer state to prettify
 * @return a pointer to the string representation.
 */
char *get_state_str(int state) {
    switch (state) {
        case STATE_OFF:
            return "Off";
        case STATE_READY:
            return "Ready";
        case STATE_WAITING:
            return "Waiting";
        case STATE_BREWING:
            return "Brewing";
        case STATE_POURING:
            return "Pouring";
        default:
            return "";
    }
}

/**
 * Initialises a single VCP to its default state, and defines its
 * state table.
 * 
 * @param pot
 *              the pot to initialise.
 * @param id
 *              the ID to assign to this pot.
 */
void init_pot(pot_struct *pot, int id) {

    pot->pot_id = id;
    pot->current_thread = 0;
    pot->current_state = STATE_READY;
    pot->adds = malloc(1024);
    pot->waiting_adds = FALSE;

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

    sprintf(buf, "Pot %d ready.", pot->pot_id);
    log(buf);
}

/**
 * Calculates the remaining time left to brew on the specified pot,
 * and concatenates it onto the given buffer.
 * 
 * @param response
 *              pointer to the response buffer.
 * @param pot
 *              the pot to calculate an ETC for.
 */
void calc_etc(char* response, pot_struct* pot) {
    time_t etc = pot->brew_end_time - time(NULL);
    sprintf(buf, "ETC: %d seconds\r\n", (int) etc);
    strcat(response, buf);
}
