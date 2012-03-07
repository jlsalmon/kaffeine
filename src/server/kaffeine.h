/* 
 * File:   kaffeine.h
 * Author: jussy
 *
 * Created on 28 February 2012, 22:16
 */

#ifndef KAFFEINE_H
#define	KAFFEINE_H

#define HTCPCP_VERSION  "HTCPCP/1.0 "
#define METHOD_BREW     "BREW"
#define METHOD_POST     "POST"
#define METHOD_GET      "GET"
#define METHOD_WHEN     "WHEN"
#define METHOD_PROPFIND "PROPFIND"
#define CONTENT_TYPE    "Content-Type: message/coffeepot\r\n\r\n"

#define C_200           "200 OK\r\n"
#define C_406           "406 Not Acceptable\r\n"
#define C_407           "407 Pot Turned Off\r\n"
#define C_408           "408 Pot In Use\r\n"
#define C_418           "418 I'm A Teapot\r\n"
#define C_504           "504 Cup Overflow\r\n"
#define C_505           "505 Cup Gone Cold\r\n"

#define M_200_START     "Your coffee is brewing. ETC (Estimated Time to Caffeination) = 20s\r\n"
#define M_200_END       "Your additions were added successfully.\r\n"
#define M_407           "The requested pot is not turned on.\r\n"
#define M_408           "The requested pot is in use by another client.\r\n"
#define M_418           "The requested pot is not capable of brewing coffee. Please use a weaker protocol.\r\n"
#define M_504           "Out of time: your cup has overflowed.\r\n"
#define M_505           "Out of time: your coffee has gone cold.\r\n"

#define QUIT_MSG	"Goodbye!\r\n"
#define TRUE 		1
#define FALSE		0

int parse_request(char*, char*);
int propfind_request(char*, char*);
int brew_request(char*, char*, char*);
int get_request(char*, char*, char*);
int when_request(char*, char*);

int create_tcp_endpoint();
void init_sigchld_handler();
void sigchld_handler();

#endif	/* KAFFEINE_H */

