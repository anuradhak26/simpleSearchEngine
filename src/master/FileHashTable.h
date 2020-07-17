/*
 * FileHashTable.h
 *
 *  Created on: Dec 12, 2015
 *      Author: ongnathan
 */

#ifndef FILEHASHTABLE_H_
#define FILEHASHTABLE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "network.h"

typedef struct ip_value{
	char* ip;
	char* port;
}ipValueNode;

typedef struct entry{
	char* key;
	ipValueNode* value;
	struct entry *next;
} fileEntry;

typedef struct hashTable{
	int size;
	fileEntry** table;
}fileHashTable;

fileHashTable* createFileHashTable(int size);
void file_ht_set(fileHashTable* hashtable, char *key, char* ip, char* port);
ipValueNode* file_ht_get(fileHashTable* hashtable, char *key);

#endif /* FILEHASHTABLE_H_ */
