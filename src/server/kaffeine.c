/**
 * File:        kaffeine.c
 * Author:      Justin Lewis Salmon
 * Student ID:  10000937
 * Created on:  27 February 2012
 * 
 * HTCPCP 1.0 compliant coffee-pot server. Handles HTCPCP client requests, 
 * allocates threads and sends HTCPCP responses based on the state of the
 * array of Virtual Coffee Pots (VCPs). 
 * 
 * Whilst this server maintains a connection with the client until either 
 * party terminates it due to error or successful completion, it is stateless.
 * All state is held within the VCP.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include "vcp.h"
#include "kaffeine.h"

int main(int argc, char *argv[]) {

    int sock, tmpsock, curr_thread;
    struct sockaddr_in client_addr;
    socklen_t sin_size;
    char cli_addr[20];

    /* Create an endpoint to listen on */
    if ((sock = create_tcp_endpoint(USR_PORT)) < 0) {
        log("Cannot create endpoint");
        exit(1);
    } else {
        log("TCP endpoint created.");
    }

    log("Initialising virtual coffee pots...");
    for (int i = 0; i < NUM_POTS; ++i) {
        init_pot(&pots[i], i);
    }
    log("Pots initialised.");
    sprintf(buf, "Accepting connections on port %d.", USR_PORT);
    log(buf);

    /* Main accept() loop */
    while (1) {
        curr_thread = 0;

        /* Find a free thread */
        while (threads[curr_thread].busy == TRUE && curr_thread < NUM_POTS) {
            curr_thread++;
        }

        sin_size = sizeof (struct sockaddr_in);

        if (curr_thread == NUM_POTS) {
            log("Connection limit reached");
            tmpsock = accept(sock, (struct sockaddr *) &client_addr, &sin_size);

            strcpy(buf, HTCPCP_VERSION);
            strcat(buf, C_503);
            strcat(buf, CONTENT_TYPE);
            strcat(buf, M_503);

            if (send(tmpsock, buf, strlen((char*) buf), 0) <= 0) {
                log(strerror(errno));
                exit(-1);
            }
        } else {
            /* Accept the connection, save sock in thread struct */
            if ((threads[curr_thread].sock = accept(sock,
                    (struct sockaddr *) &client_addr, &sin_size)) < 0) {
                log(strerror(errno));
                exit(-1);
            }

            strcpy(cli_addr, inet_ntoa(client_addr.sin_addr));
            sprintf(buf, "Connection established with %s", cli_addr);
            log(buf);

            /* Mark thread as busy */
            threads[curr_thread].busy = TRUE;
            /* Create pthread and run request handler */
            if (pthread_create(&threads[curr_thread].tid, NULL,
                    &handle_request, (void *) &threads[curr_thread]) != 0) {
                log(strerror(errno));
                exit(-2);
            }
        }
    }
}

/**
 * Function called from pthread_create() when receiving a client connection.
 * 
 * @param tptr
 *              pointer to this thread's thread_struct.
 * @return success, or error on send/recv failure.
 */
static void *handle_request(void *tptr) {
    thread_struct *thread;
    thread = (thread_struct *) tptr;
    char *request = malloc(MAX_DATA_SIZE + 1);
    char *response = malloc(MAX_DATA_SIZE + 1);
    int numbytes;

    sprintf(buf, "Created thread %d", (int) thread->tid);
    log(buf);

    memset(request, 0, sizeof (request));
    request[0] = '\0';

    /* Receive initial message */
    if ((numbytes = recv(thread->sock, request, MAX_DATA_SIZE - 1, 0)) == -1) {
        log(strerror(errno));
        close_thread(thread);
    }
    request[numbytes] = '\0';

    /* Sentinel value is "quit" */
    while (strcmp(request, "quit") != 0) {
        sprintf(buf, "Received: \n%s", request);
        log(buf);

        /* Only parse the request if it isn't null */
        if (strcmp(request, "\0") != 0) {
            parse_request(request, response);

            if (send(thread->sock, response, strlen(response), MSG_NOSIGNAL)
                    == -1) {
                log(strerror(errno));
                close_thread(thread);
            }
            sprintf(buf, "Sent: \n%s", response);
            log(buf);
        }
        /* Receive next message */
        if ((numbytes = recv(thread->sock, request, MAX_DATA_SIZE - 1, 0))
                == -1) {
            log(strerror(errno));
            close_thread(thread);
        }
        request[numbytes] = '\0';
    }

    /* Client closed connection */
    sprintf(buf, "Thread %d exiting.", (int) pthread_self());
    log(buf);
    close_thread(thread);
    return 0;
}

/**
 * Determines and validates the request method, and calls the appropriate
 * method in the VCP, then builds a response message. Otherwise, the 
 * appropriate error response will be built.
 * 
 * @param request 
 *              pointer to the client request string.
 * @param response 
 *              pointer to the response string to be built.
 */
void parse_request(char* request, char* response) {
    const char delimiters[] = " :/?";
    char *start_line, *header, *method, *pot_no, *adds = NULL, *protocol;
    char *rqcpy;
    int pot_id;

    strcpy(response, HTCPCP_VERSION);
    rqcpy = strdup(request);

    /* Extract and validate method */
    method = strtok(rqcpy, delimiters);
    if (!valid_method(method)) {
        strcat(response, C_400);
        strcat(response, CONTENT_TYPE);
        strcat(response, M_400);
        return;
    }

    /* Extract and validate pot number */
    pot_no = strtok(NULL, delimiters);
    pot_id = extract_pot_id(pot_no);
    if (pot_id > NUM_POTS || pot_id < 0) {
        strcat(response, C_404);
        strcat(response, CONTENT_TYPE);
        strcat(response, M_404);
        return;
    } else if (pot_id == TEAPOT) {
        strcat(response, C_418);
        strcat(response, CONTENT_TYPE);
        strcat(response, M_418);
        return;
    }

    /* Extract additions, if they exist. VCP will validate. */
    if (strstr(request, "?")) {
        adds = strtok(NULL, delimiters);
        strcat(adds, "\0");
    }

    protocol = strtok(NULL, delimiters);
    fprintf(stderr, "Method: %s, Pot: %d\n", method, pot_id);

    /*
     * Is the requested pot in use by another thread? 
     * If not, allocate it to this thread. Otherwise, return 420.
     */
    if (pots[pot_id].current_thread == 0) {
        pots[pot_id].current_thread = pthread_self();
    } else if (!pthread_equal(pots[pot_id].current_thread, pthread_self())) {
        strcat(response, C_420);
        strcat(response, CONTENT_TYPE);
        strcat(response, M_420);
        return;
    }

    /* Call appropriate function based on request method. */
    if (strcmp(method, METHOD_PROPFIND) == 0) {
        propfind_request(&pots[pot_id], response);

    } else if (strcmp(method, METHOD_BREW) == 0) {
        /* Get the Accept-Additions header. */
        start_line = strtok(request, "\r\n");
        header = strtok(NULL, "\r\n");
        /* Ignore the Content-Type header. */
        if (header != NULL && strstr(header, "Content-Type")) {
            header = NULL;
        }
        brew_request(&pots[pot_id], header, response);

    } else if (strcmp(method, METHOD_GET) == 0) {
        get_request(&pots[pot_id], adds, response);

    } else if (strcmp(method, METHOD_POUR) == 0) {
        pour_request(&pots[pot_id], response);

    } else if (strcmp(method, METHOD_WHEN) == 0) {
        when_request(&pots[pot_id], response);

    }
    return;
}

/**
 * Wrapper call for VCP propfind() method. The response buffer will be
 * filled with the string of valid additions for the specified pot.
 * 
 * @param pot
 *              the pot to find properties for.
 * @param response
 *              pointer to the response buffer.
 */
void propfind_request(pot_struct * pot, char* response) {
    strcat(response, C_200);
    strcat(response, CONTENT_TYPE);
    propfind(pot, response);
}

/**
 * Wrapper call for VCP brew() method. The pot state will be queried, 
 * and upon success will be transitioned into a brewing state. Upon
 * failure, the VCP will return an appropriate error code and the 
 * response buffer will be filled with the appropriate HTCPCP error
 * code and message.
 * 
 * @param pot
 *              the pot to brew on.
 * @param header
 *              the HTCPCP Accept-Additions header.
 * @param response
 *              pointer to the response buffer.
 */
void brew_request(pot_struct* pot, char* header, char* response) {
    int err;
    if ((err = brew(pot, header))) {
        build_err_response(response, pot, err);
    } else {
        strcat(response, C_200);
        strcat(response, CONTENT_TYPE);
        strcat(response, M_200_BREW);
    }
}

/**
 * Wrapper call for VCP get() method. Can either be used to brew coffee
 * or collect a brewed cup, depending on whether the additions passed are
 * NULL.
 * 
 * @param pot
 *              the pot to brew on or collect from.
 * @param adds
 *              the request URI additions string.
 * @param response
 *              pointer to the response buffer.
 */
void get_request(pot_struct* pot, char* adds, char* response) {
    int err;

    strcat(response, C_200);
    strcat(response, CONTENT_TYPE);
    strcat(response, SAFE_YES);

    if ((err = get(pot, adds, response))) {
        build_err_response(response, pot, err);
    } else if (adds != NULL) {
        strcat(response, M_200_BREW);
    }
}

/**
 * Wrapper for VCP pour() method. If an appropriate time to do so, the
 * VCP will transition to a pouring state. Otherwise, the response buffer
 * will be filled with the appropriate HTCPCP error code and message.
 * 
 * @param pot
 *              the pot to begin pouring on.
 * @param response
 *              pointer to the response buffer.
 */
void pour_request(pot_struct* pot, char* response) {
    int err;
    if ((err = pour(pot))) {
        build_err_response(response, pot, err);
    } else {
        strcat(response, C_200);
        strcat(response, CONTENT_TYPE);
        strcat(response, M_200_POUR);
    }
}

/**
 * Wrapper for VCP when() method. If an appropriate time to do so, the VCP
 * will stop pouring and wait for collection. Otherwise, an appropriate
 * response will be placed in the response buffer.
 * 
 * @param pot
 *              the pot to cease pouring on.
 * @param response
 *              pointer to the response buffer.
 */
void when_request(pot_struct* pot, char* response) {
    int err;
    if ((err = when(pot))) {
        build_err_response(response, pot, err);
    } else {
        strcat(response, C_200);
        strcat(response, CONTENT_TYPE);
        strcat(response, M_200_WHEN);
    }
}

/**
 * Constructs an HTCPCP server response message based on the given error
 * code.
 * 
 * @param response
 *              pointer to the response buffer.
 * @param pot
 *              pointer to the pot that generated the error.
 * @param err
 *              the error code that was generated.
 */
void build_err_response(char* response, pot_struct* pot, int err) {
    strcpy(response, HTCPCP_VERSION);

    switch (err) {
        case E_INVALID_ADDS:
            strcat(response, C_406);
            strcat(response, CONTENT_TYPE);
            strcat(response, M_406);
            /* Invalid additions? Send back list of valid ones.*/
            propfind(pot, response);
            break;
        case E_OFF:
            strcat(response, C_419);
            strcat(response, CONTENT_TYPE);
            strcat(response, M_419);
            break;
        case E_BUSY:
            strcat(response, C_420);
            strcat(response, CONTENT_TYPE);
            strcat(response, M_420);
            break;
        case E_STILL_BREWING:
            strcat(response, C_421);
            strcat(response, CONTENT_TYPE);
            strcat(response, M_421);
            /* GET too soon? Send back remaining time. */
            calc_etc(response, pot);
            break;
        case E_STILL_POURING:
            strcat(response, C_422);
            strcat(response, CONTENT_TYPE);
            strcat(response, M_422);
            break;
        case E_ALRDY_BREWING:
            strcat(response, C_423);
            strcat(response, CONTENT_TYPE);
            strcat(response, M_423);
            break;
        case E_ALRDY_POURING:
            strcat(response, C_424);
            strcat(response, CONTENT_TYPE);
            strcat(response, M_424);
            break;
        case E_NOT_POURING:
            strcat(response, C_425);
            strcat(response, CONTENT_TYPE);
            strcat(response, M_425);
            break;
        case E_CUP_WAITING:
            strcat(response, C_426);
            strcat(response, CONTENT_TYPE);
            strcat(response, M_426);
            break;
        case E_NO_CUP:
            strcat(response, C_427);
            strcat(response, CONTENT_TYPE);
            strcat(response, M_427);
            break;
        case E_WAITING_ADDS:
            strcat(response, C_428);
            strcat(response, CONTENT_TYPE);
            strcat(response, M_428);
            break;
        case E_OVERFLOW:
            strcat(response, C_504);
            strcat(response, CONTENT_TYPE);
            strcat(response, M_504);
            break;
        case E_CUP_COLD:
            strcat(response, C_505);
            strcat(response, CONTENT_TYPE);
            strcat(response, M_505);
            break;
        default:
            break;
    }
}

/**
 * Determines whether the supplied method is a valid HTCPCP method.
 * 
 * @param method
 *              the supplied method string.
 * @return true if the method is valid, false otherwise.
 */
int valid_method(char* method) {
    if ((strcmp(method, METHOD_BREW) != 0)
            && (strcmp(method, METHOD_POST) != 0)
            && (strcmp(method, METHOD_GET) != 0)
            && (strcmp(method, METHOD_POUR) != 0)
            && (strcmp(method, METHOD_WHEN) != 0)
            && (strcmp(method, METHOD_PROPFIND) != 0)) {
        return FALSE;
    }
    return TRUE;
}

/**
 * Takes in a pot designator string like "pot-2" and returns the integer
 * pot ID associated with it, i.e. integer 2.
 * 
 * @param pot_no
 *              the pot-designator string.
 * @return the integer pot ID.
 */
int extract_pot_id(char* pot_no) {
    const char delimiters[] = "-";
    char *head, *s_id;

    head = strtok(pot_no, delimiters);
    s_id = strtok(NULL, delimiters);
    return atoi(s_id);
}

/**
 * Useful function for creating a TCP socket to listen on.
 * 
 * @param port
 *              the port number to use.
 * @return the socket file descriptor.
 */
int create_tcp_endpoint(int port) {
    int sock, yes = 1;
    struct sockaddr_in server;

    /* make socket with TCP streams. Kernel choose a suitable protocol */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        log(strerror(errno));
        return -1;
    }

    /* Set Unix socket level to allow address reuse */
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (int)) == -1) {
        log(strerror(errno));
        return -2;
    }

    /* zero the struct */
    memset(&server, 0, sizeof (server));
    server.sin_family = AF_INET;
    /* any server's IP addr */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);
 
    /* Bind, then listen for client connections */
    if (bind(sock, (struct sockaddr *) &server, sizeof (server)) < 0) {
        log(strerror(errno));
        return -3;
    }
    if (listen(sock, MAX_Q_SIZE) == -1) {
        log(strerror(errno));
        return -4;
    }

    return sock;
}

/**
 * Deallocates the pot associated with the supplied thread struct, frees
 * the thread, closes the thread's socket and destroys the thread itself 
 * with pthread_exit().
 * 
 * @param thread
 *              the thread_struct to close.
 */
void close_thread(thread_struct* thread) {
    for (int i = 0; i < NUM_POTS; ++i) {
        if (pthread_equal(pots[i].current_thread, pthread_self())) {
            init_pot(&pots[i], i);
        }
    }
    thread->busy = FALSE;
    close((int) thread->sock);
    pthread_exit((void *) 1);
}

/**
 * Handy function for printing timestamped server log messages. Shouldn't
 * really be called log() because it overrides the built-in log() function,
 * but I couldn't think of a better name that was short.
 * 
 * @param msg
 *              pointer to the message buffer to be logged.
 */
void log(char* msg) {
    time_t timer;
    char buf[250];

    timer = time(NULL);
    strftime(buf, 250, "%Y:%m:%d %H:%M:%S", localtime(&timer));
    fprintf(stderr, "%s\t%s\n", buf, msg);
}
