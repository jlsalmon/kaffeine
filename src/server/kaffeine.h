/* 
 * File:   kaffeine.h
 * Author: jussy
 *
 * Created on 28 February 2012, 22:16
 */

#ifndef KAFFEINE_H
#define	KAFFEINE_H

#define HTCPCP_VERSION  "HTCPCP/1.0 "
#define C_200           "200 OK\r\n"
#define C_406           "406 Not Acceptable\r\n"
#define C_407           "407 Pot Turned Off\r\n"
#define C_408           "408 Pot In Use\r\n"
#define C_418           "418 I'm A Teapot\r\n"
#define C_504           "504 Cup Overflow\r\n"
#define C_505           "505 Cup Gone Cold\r\n"
#define QUIT_MSG	"Goodbye!\n"
#define TRUE 		1
#define FALSE		0

int create_tcp_endpoint();
void init_sigchld_handler();
void sigchld_handler();
int parse_request(char*, char*);

#endif	/* KAFFEINE_H */

