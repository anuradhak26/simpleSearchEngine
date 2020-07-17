
#ifndef MASTER_H_
#define MASTER_H_

#include <stdio.h>
#include <stdint.h> 
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>    
#include <sys/stat.h>
#include <pthread.h> //for threading , link with lpthread
#include <sys/socket.h>
#include <netinet/in.h>

#include "network.h"
#include "FreqHashTable.h"
#include "FileHashTable.h"

#define FILENAME "master.txt" //file to write master ip and port
#define FILENAME1 "server.txt" //file to write master ip and port

//struct connection_handler_args
//{
//	int connfd;
//	struct sockaddr_in* dest;
//};

typedef struct server_node
{
//	struct sockaddr_in* server;
	char* server_ip;
	char* server_port;
	struct server_node* next;
}ServerNode;

typedef struct ranking_node
{
	char* filename;
	int numKeyWordsInFile;
	int numInstancesKeywords;
	struct ranking_node* next;
}RankingNode;

//thread function
void *connection_handler(void *);

#endif
