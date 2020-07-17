#ifndef NETWORK_H_
#define NETWORK_H_

#include <stdio.h>
#include <ctype.h> 
#include <stdarg.h> 
#include <inttypes.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
/********************************************************/
int createTCP_server(char *addrstr, char *port);
int connectTCP_server(char *Server_ip, char *port);
void *get_in_addr(struct sockaddr *sa);
//int connectTCPWithInfo(struct sockaddr_in* dest);

/********************************************************/

#endif
