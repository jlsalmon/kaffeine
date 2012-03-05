/* 
 * File:   kaffeine.h
 * Author: jussy
 *
 * Created on 28 February 2012, 22:16
 */

#ifndef KAFFEINE_H
#define	KAFFEINE_H

#define QUIT_MSG        "Goodbye!\n"

int create_tcp_endpoint();
void init_sigchld_handler();
void sigchld_handler();
char parse_request(const char *);

#endif	/* KAFFEINE_H */

