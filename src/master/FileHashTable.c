#include "FileHashTable.h"

fileHashTable* createFileHashTable(int size)
{
	fileHashTable* hashtable = NULL;
	int i = 0;
	if (size < 1)
		return NULL;

	if ((hashtable = malloc(sizeof(fileHashTable))) == NULL)
		return NULL;
	if ((hashtable->table = malloc(sizeof(fileEntry) * size)) == NULL)
		return NULL;
	for (i = 0; i < size; i++)
	{
		hashtable->table[i] = NULL;
	}
	hashtable->size = size;
	return hashtable;
}

/* Hash a string for a particular hash table. */
int file_ht_hash(fileHashTable* hashtable, char *key)
{
	unsigned long int hashval = 0;
	int i = 0;

	/* Convert our string to an integer */
	while (hashval < ULONG_MAX && i < strlen(key))
	{
		hashval = hashval << 8;
		hashval += key[i];
		i++;
	}

	return hashval % hashtable->size;
}

/* Create a key-value pair. */
fileEntry* file_ht_newpair(char *key, ipValueNode* value)
{
	fileEntry* newpair;

	if ((newpair = malloc(sizeof(fileEntry))) == NULL)
	{
		return NULL;
	}

	if ((newpair->key = strdup(key)) == NULL)
	{
		return NULL;
	}
	//doubt
	newpair->value = value;

	newpair->next = NULL;

	return newpair;
}

/* Insert a key-value pair into a hash table. */
void file_ht_set(fileHashTable* hashtable, char *key, char* ip, char* port)
{
	int bin = 0;
	fileEntry* newpair = NULL;
	fileEntry* next = NULL;
	fileEntry* last = NULL;

	ipValueNode* value = (ipValueNode*) malloc(sizeof(ipValueNode));
	value->ip = ip;
	value->port = port;

	bin = file_ht_hash(hashtable, key);

	next = hashtable->table[bin];

	while (next != NULL && next->key != NULL && strcmp(key, next->key) != 0)
	{
		last = next;
		next = next->next;
	}

	/* There's already a pair.  Let's replace that string. */
	if (next != NULL && next->key != NULL && strcmp(key, next->key) == 0)
	{
		//increase the frequency for the specified file if found
		//should change it if the same word has many files names and frequencies
		next->value = value;

		/* Nope, could't find it.  Time to grow a pair. */
	}
	else
	{
		newpair = file_ht_newpair(key, value);

		/* We're at the start of the linked list in this bin. */
		if (next == hashtable->table[bin])
		{
			newpair->next = next;
			hashtable->table[bin] = newpair;

			/* We're at the end of the linked list in this bin. */
		}
		else if (next == NULL)
		{
			last->next = newpair;

			/* We're in the middle of the list. */
		}
		else
		{
			newpair->next = next;
			last->next = newpair;
		}
	}
}

/* Retrieve a key-value pair from a hash table. */
ipValueNode* file_ht_get(fileHashTable* hashtable, char *key)
{
	int bin = 0;
	fileEntry* pair;

	bin = file_ht_hash(hashtable, key);

	/* Step through the bin, looking for our value. */
	pair = hashtable->table[bin];
	while (pair != NULL && pair->key != NULL && strcmp(key, pair->key) > 0)
	{
		pair = pair->next;
	}

	/* Did we actually find anything? */
	if (pair == NULL || pair->key == NULL || strcmp(key, pair->key) != 0)
	{
		return NULL;

	}
	else
	{
		return pair->value;
	}

}

//void main(){
//	sHashTable* hashtable = createHashTable(65536);
//	struct sockaddr_in *local= NULL;
//
//	ht_set( hashtable, "key1", local);
//
//	sValueNode* getValue1 = NULL;
//	printf("key1", ht_get(hashtable, "key1"));
//	printf("DONE");
//}
