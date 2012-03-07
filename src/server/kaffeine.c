/*
 * kaffeine.c
 *
 *  Created on: Feb 27, 2012
 *      Author: jl2-salmon
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
#include "kaffeine.h"
#include "vcp.h"

typedef struct {
    pthread_t tid;
    int sock;
    int busy;
} thread_struct;

thread_struct threads[NUM_POTS];

int main(void) {

    int sock, tmpsock, curr_thread;
    struct sockaddr_in client_addr;
    socklen_t sin_size;
    char cli_addr[20], buf[MAX_DATA_SIZE];

    /* Create an endpoint to listen on */
    if ((sock = create_tcp_endpoint(USR_PORT)) < 0) {
        fprintf(stderr, "Cannot create endpoint\n");
        exit(1);
    } else {
        fprintf(stderr, "TCP endpoint created.\n");
    }

    init_pots();

    /* main accept() loop */
    while (1) {
        curr_thread = 0;

        while (threads[curr_thread].busy == TRUE && curr_thread < NUM_POTS) {
            curr_thread++;
        }

        sin_size = sizeof (struct sockaddr_in);

        if (curr_thread == NUM_POTS) {
            fprintf(stderr, "Connection limit reached\n");
            tmpsock = accept(sock, (struct sockaddr *) &client_addr, &sin_size);

            strcpy(buf, HTCPCP_VERSION);
            strcat(buf, C_503);
            strcat(buf, CONTENT_TYPE);
            strcat(buf, M_503);

            if (send(tmpsock, buf, strlen((char*) buf), 0) <= 0) {
                perror("write");
                exit(-1);
            }
        } else {

            if ((threads[curr_thread].sock = accept(
                    sock, (struct sockaddr *) &client_addr, &sin_size)) < 0) {
                perror("accept");
                exit(-1);
            }

            strcpy(cli_addr, inet_ntoa(client_addr.sin_addr));
            printf("\nConnection established with %s\n", cli_addr);

            threads[curr_thread].busy = TRUE;
            if (pthread_create(&threads[curr_thread].tid,
                    NULL,
                    &handle_request,
                    (void *) &threads[curr_thread])
                    != 0) {
                perror("pthread_create");
                exit(-2);
            }
        }

        /*
                sin_size = sizeof (struct sockaddr_in);

                if ((connfd = accept(sock, (struct sockaddr *) &client_addr, &sin_size))
                        == -1) {
                    perror("Server accept");
                    continue;
                }

                strcpy(clientAddr, inet_ntoa(client_addr.sin_addr));
                printf("\nConnection established with %s\n", clientAddr);

                // the child process dealing with a client 
                if (!(pid = fork())) {
                    fprintf(stderr, "Child process started.\n");
                    char msg[MAX_DATA_SIZE];
                    int numbytes;

                    // child does not need the listener 
                    close(sock);
                    // no message yet, zero buffer 
                    memset(&msg, 0, sizeof (msg));
                    msg[0] = '\0';

                    // receive initial message 
                    if ((numbytes = recv(connfd, msg, MAX_DATA_SIZE - 1, 0)) == -1) {
                        perror("Server recv");
                        exit(1);
                    }

                    while (strcmp(msg, "quit") != 0) {
                        // end of string 
                        msg[numbytes] = '\0';
                        fprintf(stderr, "Message received: %s\n", msg);

                        char response[MAX_DATA_SIZE];

                        parse_request(msg, response);

                        if (send(connfd, response, strlen(response), 0) == -1) {
                            perror("Server send");
                            exit(1);
                        }

                        // zero the message buffer 
                        memset(&msg, 0, sizeof (msg));

                        if ((numbytes = recv(connfd, msg, MAX_DATA_SIZE - 1, 0)) == -1) {
                            perror("Server recv");
                            exit(1);
                        }
                    }

                    if (send(connfd, QUIT_MSG, strlen(QUIT_MSG), 0) == -1) {
                        perror("Server send");
                        exit(1); // error end of child 
                    }

                    fprintf(stderr, "Child process finished.\n");
                    close(connfd);
                    exit(0);
                }

                // parent does not need the connection socket 
               close(connfd);
         */
    }
}

/**
 * 
 * @param tptr
 * @return 
 */
static void *handle_request(void *tptr) {
    thread_struct *thread;
    thread = (thread_struct *) tptr;
    char request[MAX_DATA_SIZE];
    char response[MAX_DATA_SIZE];
    int numbytes;

    printf("Created thread %d\n", (int) pthread_self());

    memset(&request, 0, sizeof (request));

    /* receive initial message */
    if ((numbytes = recv(thread->sock, request, MAX_DATA_SIZE - 1, 0)) == -1) {
        perror("recv");
        exit(1);
    }
    request[numbytes] = '\0';

    while (strcmp(request, "quit") != 0) {

        fprintf(stderr, "Message received: \n%s\n", request);

        if (strcmp(request, "\0") != 0) {
            parse_request(request, response);
        }

        if (send(thread->sock, response, strlen(response), 0) == -1) {
            perror("send");
            exit(1);
        }

        memset(&request, 0, sizeof (request));

        if ((numbytes = recv(thread->sock, request, MAX_DATA_SIZE - 1, 0)) == -1) {
            perror("recv");
            exit(1);
        }
        request[numbytes] = '\0';
    }

    if (send(thread->sock, QUIT_MSG, strlen(QUIT_MSG), 0) == -1) {
        perror("send");
        exit(1);
    }

    fprintf(stderr, "Thread %d exiting.\n", (int) pthread_self());
    close((int) thread->sock);
    thread->busy = FALSE;
}

void parse_request(char* request, char* response) {

    const char delimiters[] = " :/?";
    char *method, *scheme, *host, *pot_no, *start_line, *header;
    char *rqcpy;

    rqcpy = strdup(request);
    method = strtok(rqcpy, delimiters);
    scheme = strtok(NULL, delimiters);
    host = strtok(NULL, delimiters);
    pot_no = strtok(NULL, delimiters);

    fprintf(stderr, "Method: %s, Scheme: %s, Host: %s, Pot: %s\n"
            , method, scheme, host, pot_no);

    if (strcmp(method, METHOD_PROPFIND) == 0) {
        propfind_request(pot_no, response);

    } else if (strcmp(method, METHOD_BREW) == 0) {
        start_line = strtok(request, "\r\n");
        header = strtok(NULL, "\r\n");
        brew_request(pot_no, header, response);

    } else if (strcmp(method, METHOD_GET) == 0) {
        get_request(pot_no, request, response);

    } else if (strcmp(method, METHOD_WHEN) == 0) {
        when_request(pot_no, response);

    } else {
        strcpy(response, HTCPCP_VERSION);
        strcat(response, C_406);
    }

    return;
}

void propfind_request(char* pot_no, char* response) {

    strcpy(response, HTCPCP_VERSION);
    strcat(response, C_200);
    strcat(response, CONTENT_TYPE);

    if (propfind(pot_no, response) == ERR_TEAPOT) {
        strcpy(response, HTCPCP_VERSION);
        strcat(response, C_418);
        strcat(response, CONTENT_TYPE);
        strcat(response, M_418);
    }
}

void brew_request(char* pot_no, char* header, char* response) {

    strcpy(response, HTCPCP_VERSION);
    int err = brew(pot_no, header);

    switch (err) {
        case ERR_OFF:
            strcat(response, C_407);
            strcat(response, CONTENT_TYPE);
            strcat(response, M_407);
            break;
        case ERR_BUSY:
            strcat(response, C_408);
            strcat(response, CONTENT_TYPE);
            strcat(response, M_408);
            break;
        case ERR_TEAPOT:
            strcat(response, C_418);
            strcat(response, CONTENT_TYPE);
            strcat(response, M_418);
            break;
        default:
            strcat(response, C_200);
            strcat(response, CONTENT_TYPE);
            strcat(response, M_200_START);
            break;
    }
}

void get_request(char* pot_no, char* adds, char* response) {

    strcpy(response, HTCPCP_VERSION);
    strcat(response, C_200);
}

void when_request(char* pot_no, char* response) {

    strcpy(response, HTCPCP_VERSION);
    strcat(response, C_200);
}

/* Useful function to create server endpoint */
int create_tcp_endpoint(int port) {
    int sock, yes = 1;
    struct sockaddr_in server;

    /* make socket with TCP streams. Kernel choose a suitable protocol */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Server socket");
        return -1;
    }

    /* Set Unix socket level to allow address reuse */
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (int)) == -1) {
        perror("Server setsockopt");
        return -2;
    }

    /* zero the struct */
    memset(&server, 0, sizeof (server));
    server.sin_family = AF_INET;
    /* any server's IP addr */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    if (bind(sock, (struct sockaddr *) &server, sizeof (server)) < 0) {
        perror("Server bind");
        return -3;
    }

    /* Listen for client connections */
    if (listen(sock, MAX_Q_SIZE) == -1) {
        perror("Server listen");
        return -4;
    }

    return sock;
}