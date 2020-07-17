#include "FreqHashTable.h"

fHashTable* createFreqHashTable(int size)
{
	fHashTable* hashtable = NULL;
	int i = 0;
	if (size < 1)
		return NULL;

	if ((hashtable = (fHashTable*)malloc(sizeof(fHashTable))) == NULL)
		return NULL;
	if ((hashtable->table = (kEntry**)malloc(sizeof(kEntry*) * size)) == NULL)
		return NULL;
	for (i = 0; i < size; i++)
	{
		hashtable->table[i] = NULL;
	}
	hashtable->size = size;
	return hashtable;
}

ffNode* createFileFreqValueNode(char* filename, int frequency)
{
	ffNode* value = NULL;
	value = malloc(sizeof(ffNode));
	//strcpy(value->filename,filename);
	value->filename = strdup(filename);
	value->frequency = frequency;
	value->next = NULL;
	//printf("\nYo fn: %s",value->filename);
	//printf("\nYo freq: %d",value->frequency);
	return value;
}

/* Hash a string for a particular hash table. */
int freq_ht_hash(fHashTable* hashtable, char *key)
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
kEntry* freq_ht_newpair(char *key, ffNode* value)
{
	kEntry* newpair;

	if ((newpair = malloc(sizeof(kEntry))) == NULL)
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
void freq_ht_set(fHashTable* hashtable, char *key, char* fileName, int frequency)
{
	int bin = 0;
	kEntry* newpair = NULL;
	kEntry* findKeywordEntry = NULL;
	kEntry* lastKeywordEntry = NULL;
//	ffNode* current = NULL;

	ffNode* value = createFileFreqValueNode(fileName, frequency);

	bin = freq_ht_hash(hashtable, key);

	findKeywordEntry = hashtable->table[bin];

	while (findKeywordEntry != NULL && findKeywordEntry->key != NULL && strcmp(key, findKeywordEntry->key) != 0)
	{
		lastKeywordEntry = findKeywordEntry;
		findKeywordEntry = findKeywordEntry->next;
	}

	//did not find the keyword entry at all.  Need to add it to the hash table.
	if (findKeywordEntry == NULL)
	{
		newpair = freq_ht_newpair(key, value);
		printf(" \n data in the hash table: hash = %d, keyword \"%s\", fileName %s, count %d\n", bin, newpair->key, newpair->value->filename, newpair->value->frequency);

		//no entry exists yet for that bin.
		if (hashtable->table[bin] == NULL)
		{
//			newpair->next = findKeywordEntry;
			hashtable->table[bin] = newpair;
		}
		/* We're at the end of the linked list in this bin. */
//		else if (findKeywordEntry == NULL)
		else
		{
			lastKeywordEntry->next = newpair;
		}
//		else
//		{
//			newpair->next = findKeywordEntry;
//			lastKeywordEntry->next = newpair;
//		}
	}
//	if (findKeywordEntry != NULL && findKeywordEntry->key != NULL && strcmp(key, findKeywordEntry->key) == 0)
	else
	{
		//increase the frequency for the specified file if found
		//should change it if the same word has many files names and frequencies

		//	next->value->frequency = next->value->frequency+value->frequency;
		//	next->value->filename = value->filename;
		ffNode* current = findKeywordEntry->value;
		ffNode* last = NULL;
		while (current != NULL && strcmp(findKeywordEntry->value->filename, value->filename) != 0)
		{
			last = current;
			current = current->next;
		}

//		//no such filename, add it to the chain by insertion sort
//		if(current1 == NULL)
//		{
//			last = NULL;
//			current1 = findKeywordEntry->value;
//			while(current1 != NULL && value->frequency < current1->frequency)
//			{
//				last = current1;
//				current1 = current1->next;
//			}
//			if(last == NULL)
//			{
//				findKeywordEntry->value = value;
//			}
//			else
//			{
//				last->next = value;
//			}
//			value->next = current1;
//		}
		//same filename, just combine the frequencies
//		else if (strcmp(current1->filename, value->filename) == 0)
		if(current != NULL)
		{
			value->frequency = current->frequency + value->frequency;

			//remove node to prepare for reinsertion via insertion sort
			if(last != NULL)
			{
				last->next = current->next;
			}
			free(current);
		}

		//need to insert via insertion sort
		last = NULL;
		current = findKeywordEntry->value;
		while(current != NULL && value->frequency < current->frequency)
		{
			last = current;
			current = current->next;
		}
		if(last == NULL)
		{
			findKeywordEntry->value = value;
		}
		else
		{
			last->next = value;
		}
		value->next = current;
	}
}

/* Retrieve a key-value pair from a hash table. */
ffNode* freq_ht_get(fHashTable* hashtable, char *key)
{
	int bin = 0;
	kEntry* pair;

	bin = freq_ht_hash(hashtable, key);
	printf("Key \"%s\" has hash value = %d\n", key, bin);

	/* Step through the bin, looking for our value. */
	pair = hashtable->table[bin];
	while (pair != NULL && pair->key != NULL && strcmp(key, pair->key) != 0)
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

ffNode* getHighFreqDoc(ffNode* value)
{
	ffNode* maxValue = NULL;
	int max = 0;
	//Loops till the last node
	while (value->next != NULL)
	{
		if (value->frequency > max)
		{
			maxValue = value;
			max = value->frequency;
		}
		value = value->next;
	}
	if (value->frequency > max)
	{
		maxValue = value;
	}
	return maxValue;
}

//void main()
//{
//	fHashTable* hashtable = createFreqHashTable(65536);
//	ffNode* value1 = NULL;
//
////	if((value1 = malloc(sizeof(sValueNode)))==NULL)
////		printf("problem");
////	value1->filename = "YoYo.txt";
////	value1->frequency = 22;
//
//	ffNode* value2 = NULL;
////	if((value2 = malloc(sizeof(sValueNode)))==NULL)
////		printf("problem");
////	value2->filename = "YoYo.txt";
////	value2->frequency = 33;
//
//	ffNode* value3 = NULL;
////	if((value3 = malloc(sizeof(sValueNode)))==NULL)
////		printf("problem");
////	value3->filename = "Anu.txt";
////	value3->frequency = 44;
//	value1 = createFileFreqValueNode("Anu.txt", 8);
//	freq_ht_set(hashtable, "key1", createFileFreqValueNode("Sam.txt", 7));
//	freq_ht_set(hashtable, "key1", createFileFreqValueNode("Anu.txt", 8));
//	freq_ht_set(hashtable, "key1", createFileFreqValueNode("Anu.txt", 9));
//	freq_ht_set(hashtable, "key1", createFileFreqValueNode("Anku.txt", 7));
//
//	ffNode* getValue1 = NULL;
//	if ((getValue1 = malloc(sizeof(ffNode))) == NULL)
//		printf("problem");
//
//	getValue1 = freq_ht_get(hashtable, "key1");
//
//	value2 = getHighFreqDoc(getValue1);
//	printf("Highest frequency: %d  FileName: %s \n", value2->frequency,
//			value2->filename);
//
//	while (getValue1->next != NULL)
//	{
//		printf("key1 --> Frequency : %d, filename: %s\n", getValue1->frequency,
//				getValue1->filename);
//		getValue1 = getValue1->next;
//	}
//	printf("key1 --> Frequency : %d, filename: %s\n", getValue1->frequency,
//			getValue1->filename);
//
//	/*
//	 sValueNode getValue2 = NULL;
//	 if((getValue2 = malloc(sizeof(sValueNode)))==NULL)
//	 printf("problem");
//
//	 getValue2 = ht_get( hashtable, "key2" );
//	 printf( "key2 --> Frequency : %d, filename: %s\n",getValue2->frequency,getValue2->filename);
//	 */
//
//	printf("DONE");
//}
