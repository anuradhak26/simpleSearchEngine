/*
 * FreqHashTable.h
 *
 *  Created on: Dec 11, 2015
 *      Author: ongnathan
 */

#ifndef FREQHASHTABLE_H_
#define FREQHASHTABLE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

struct freqFileNode{
	int frequency;
	char* filename;
	struct freqFileNode *next;
};
typedef struct freqFileNode ffNode;

struct keywordEntry{
	char* key;
	ffNode* value;
	struct keywordEntry *next;
};
typedef struct keywordEntry kEntry;

struct freqHashTable{
	int size;
	kEntry** table;
};
typedef struct freqHashTable fHashTable;

fHashTable* createFreqHashTable(int size);
void freq_ht_set(fHashTable* hashtable, char *key, char* fileName, int frequency);
ffNode* freq_ht_get(fHashTable* hashtable, char *key);
ffNode* getHighFreqDoc(ffNode* value);

#endif /* FREQHASHTABLE_H_ */
