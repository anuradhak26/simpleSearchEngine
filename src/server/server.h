#ifndef SERVER_H_
#define SERVER_H_

#include <stdio.h>
#include <stdint.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <netinet/in.h>

#include "network.h"

#define FILENAME "master.txt" //file to write master ip and port

typedef struct count_node
{
	char *word;
	int lengthOfWord;
	int count;
	struct count_node* next;
}CountNode;

int removeNonAlphaNumericChars(int length, char* string);
int run_command(int connfd);

#endif
