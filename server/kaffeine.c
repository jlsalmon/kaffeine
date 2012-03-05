/*
 * pot.c
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
#include "kaffeine.h"
#include "vcp.h"

#define USR_PORT 	60000 	/* the port users connect to */
#define MAX_Q_SIZE 	10 	/* max no. of pending connections in server queue */
#define MAX_DATA_SIZE 	200	/* max message size in bytes */

int main(void) {

	int connfd, sock;
	/* client's address info */
	struct sockaddr_in client_addr;
	/* size of address structure */
	socklen_t sin_size;
	/* holds ascii dot quad address */
	char clientAddr[20];

	/* Create an endpoint to listen on */
	if ((sock = create_tcp_endpoint(USR_PORT)) < 0) {
		fprintf(stderr, "Cannot create endpoint\n");
		exit(1);
	}

	init_sigchld_handler();

	init_pots();

	/* main accept() loop */
	while (1) {
		sin_size = sizeof(struct sockaddr_in);

		if ((connfd = accept(sock, (struct sockaddr *) &client_addr, &sin_size))
				== -1) {
			perror("Server accept");
			continue;
		}
		strcpy(clientAddr, inet_ntoa(client_addr.sin_addr));

		printf("Server: got connection from %s\n", clientAddr);

		/* the child process dealing with a client */
		if (!fork()) {
			char msg[MAX_DATA_SIZE];
			int numbytes;

			/* child does not need the listener */
			close(sock);
			/* no message yet, zero buffer */
			memset(&msg, 0, sizeof(msg));
			msg[0] = '\0';

			/* receive initial message */
			if ((numbytes = recv(connfd, msg, MAX_DATA_SIZE - 1, 0)) == -1) {
				perror("Server recv");
				exit(1);
			}

			while (strcmp(msg, "quit") != 0) {
				/* end of string */
				msg[numbytes] = '\0';
				fprintf(stderr, "Message received: %s\n", msg);

				strncpy(msg, "HTCPCP/1.0 200 OK", strlen(msg));

				if (send(connfd, msg, strlen(msg), 0) == -1) {
					perror("Server send");
					exit(1);
				}

				/* zero the message buffer */
				memset(&msg, 0, sizeof(msg));

				if ((numbytes = recv(connfd, msg, MAX_DATA_SIZE - 1, 0)) == -1) {
					perror("Server recv");
					exit(1);
				}
			}

			if (send(connfd, QUIT_MSG, strlen(QUIT_MSG), 0) == -1) {
				perror("Server send");
				exit(1); /* error end of child */
			}

			close(connfd);
			exit(0);
		}

		/* parent does not need the connection socket */
		close(connfd);
	}

	return 0;
}

char parse_request(const char request[]) {
	request = "HTCPCP/1.0 200 OK\r\n";
	return *request;
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
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("Server setsockopt");
		return -2;
	}

	/* zero the struct */
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	/* any server's IP addr */
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(port);

	if (bind(sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
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

void init_sigchld_handler() {
	// Signal handler stuff, deals with signals from dying children
	/*
	 struct sigaction sa;
	 sa.sa_handler = sigchld_handler; // reap all dead processes
	 sigemptyset(&sa.sa_mask);
	 sa.sa_flags = SA_RESTART;

	 if (sigaction(SIGCHLD, &sa, NULL) == -1) {
	 perror("Server sigaction");
	 exit(1);
	 }
	 */
}

/**
 * Signal handler for child fork exit
 */
void sigchld_handler(int s) {
	while (wait(NULL) > 0) {
		/* wait for any child to finish */
	}
}
