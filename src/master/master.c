#include "master.h"

static ServerNode* head = NULL;
static ServerNode* last = NULL;
static ServerNode* current = NULL;

static fHashTable* frequencyHT = NULL;

static fileHashTable* fileHT = NULL;

int run_master(int sockfd)
{
	int connfd;
	printf("Hello");

	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	pthread_t thread_id;

	do
	{
//		struct sockaddr_in* dest = (struct sockaddr_in*) malloc(sizeof(struct sockaddr_in));
//		int sockaddrSize = sizeof(struct sockaddr);
//		connfd = accept(sockfd, (struct sockaddr*) dest, &sockaddrSize);
		connfd = accept(sockfd, NULL, NULL);
		if (connfd <= 0)
		{
//			free(dest);
			break;
		}
		puts("Connection accepted");

//		struct connection_handler_args* args = (struct connection_handler_args*) malloc(sizeof(struct connection_handler_args));
//		args->connfd = connfd;
//		args->dest = dest;
		if (pthread_create(&thread_id, NULL, connection_handler, (void*) &connfd) < 0)
		{
//			free(args);
//			free(dest);
			perror("could not create thread");
			return 1;
		}
		else
		{
			pthread_detach(thread_id);
		}

		//Now join the thread , so that we dont terminate before the thread
		//pthread_join( thread_id , NULL);
		puts("Handler assigned");
	} while (connfd != 0);

	if (connfd < 0)
	{
		perror("accept failed");
		return 1;
	}

	return 0;
}

int write_master_ip(char *addrstr, char *port)
{
	FILE *ptr_file;

	ptr_file = fopen(FILENAME, "w");
	if (!ptr_file)
	{
		perror("server: open file to write");
		return -1;
	}
	fprintf(ptr_file, "%s,%s", addrstr, port);
	fclose(ptr_file);
	return 0;
}

int readServerIP(char *server_ip, char *server_port)
{
	// read IP address and port from file define in header.h
	FILE *ptr_file;
	char buf[100];

	// read the global file
	ptr_file = fopen(FILENAME1, "r");
	if (!ptr_file)
	{
		return 0;
		printf("no file open");
	}
	if (fgets(buf, 1000, ptr_file) != NULL)
	{
		// ip address
		strcpy(server_ip, strtok(buf, ","));

		// port number
		strcpy(server_port, strtok(NULL, ","));
	}

	if (server_ip && server_port)
		printf("portmapper ip and port is %s %s\n", server_ip, server_port);

	fclose(ptr_file);

	return 1;
}

/*
 * This will handle connection for each client
 * */
//void *connection_handler(void *void_arguments)
void *connection_handler(void* connectionFD)
{
//	struct connection_handler_args* args = (struct connection_handler_args*) void_arguments;
	//Get the socket descriptor
//	int connfd = args->connfd;
	int connfd = *(int*)connectionFD;
	int task1 = 0, task = 0;
	char *buffer;
//	char master_ip[100], master_port[100];
	int serverfd;

//	send(connfd, "Hello Client send the Data to search", sizeof("Hello Client send the Data to search"), 0);
	// Get the task ID
	printf("\n hello receive \n");
	recv(connfd, &task, sizeof(int), 0);
	task1 = ntohl(task);

	printf("\n TASK %d\n ", task1);
	switch (task1)
	{
	case 1:
		{
			printf("\n Send Request to the server for indexing the the document\n");

			//Get the file size
			int size = 0;
			char* fileName;
			FILE* requestedDocument;
			int remaining;
			printf("\n Se\n");

			//get document name length
			recv(connfd, &size, sizeof(int), 0);
			size = ntohl(size);
			printf("\n Size received %d \n", size);

			//get document name
			fileName = (char*) malloc((size+1) * sizeof(char));
			memset(fileName, 0, (size+1) * sizeof(char));
			recv(connfd, fileName, (size+1) * sizeof(char), 0);
			printf("\n document NAME %s\n", fileName);
			requestedDocument = fopen(fileName, "w+");

			//get document size
			recv(connfd, &size, sizeof(int), 0);
			size = ntohl(size);
			printf("\n document SIZe %d\n", size);

			//get document
			remaining = size;
			buffer = (char*) malloc(101 * sizeof(char));
			while (remaining > 0)
			{
				memset(buffer, 0, 101 * sizeof(char));
				int received = recv(connfd, buffer, 100 * sizeof(char), 0);
//				printf("\n inside buffer %s\n", buffer);

				if (received == -1)
				{
					puts("ERROR\n");
					continue;
				}
				fprintf(requestedDocument, "%s", buffer);
				printf("\n Received %d (%d remains) with buffer: %s\n", received, remaining-received, buffer);

				fflush(requestedDocument);
				remaining -= received;
			}
//			printf("\n outside buffer %s\n", buffer);

			free(buffer);

			printf("\nReceived entire file, %s.\n", fileName);

			//connecting to server
			// read master ip and port through file
			//TODO needs to change to round robin style
//			if (!readServerIP(master_ip, master_port))
//				return 0;
//			printf("\nConnection Details for Server is %s %s\n", master_ip,
//					master_port);
//			// open socket to connect to master
//			if ((serverfd = connectTCP_server(master_ip, master_port)) == 0)
//				return 0;
			printf("Connecting to server at %s : %s", current->server_ip, current->server_port);
//			if ((serverfd = connectTCPWithInfo(current->server)) == -1)
			if((serverfd = connectTCP_server(current->server_ip, current->server_port)) == -1)
			{
				puts("Error connecting to server.");
				return 0;
			}
			file_ht_set(fileHT, fileName, current->server_ip, current->server_port);
			if (current->next == NULL)
			{
				current = head;
			}
			else
			{
				current = current->next;
			}

			// SEND THE FILE DATA TO SERVER

//			printf(" \nTASk is %d\n", task1);
			puts("Sending task to server");
			int task2 = htonl(task1);
			send(serverfd, &task2, sizeof(int), 0);

			//sending size of name and name to server
			size = strlen(fileName);
			printf("\nSize %d\n", size);

			size = htonl(size);
			send(serverfd, &size, sizeof(int), 0);
			send(serverfd, fileName, sizeof(char) * (strlen(fileName) + 1), 0);
			printf("\nSize %s\n", fileName);

			//get file size
			struct stat st;
			stat(fileName, &st);
			size = st.st_size;

			//send file size
			size = htonl(size);
			send(serverfd, &size, sizeof(int), 0);

			//read and send file
			fseek(requestedDocument, 0, SEEK_SET);
			remaining = st.st_size;
			buffer = (char*) malloc(st.st_size + sizeof(char));
			memset(buffer, 0, st.st_size + sizeof(char));
			fread(buffer, sizeof(char), st.st_size, requestedDocument);
			while (remaining > 0)
			{
				int sent = send(serverfd, buffer, remaining, 0);
				if (sent == -1)
				{
					puts("ERROR\n");
					continue;
				}
				remaining -= sent;
			}
			free(buffer);

			//delete temporary holding file
			fclose(requestedDocument);
			remove(fileName);
			//receive the indexing back
			//receive number of keyword entries
			recv(serverfd, &remaining, sizeof(int), 0);
			remaining = ntohl(remaining);
			printf("\nNo. of words %d\n",remaining);
			//receive keywords
			while (remaining > 0)
			{
				//receive length of keyword
				recv(serverfd, &size, sizeof(int), 0);
				size = ntohl(size);
				printf("\nLength of keyword %d\n",size);
			
				//receive keyword
				char* keyword = (char*) malloc(sizeof(char) * (size + 1));
				memset(keyword, 0, sizeof(char) * (size + 1));
				recv(serverfd, keyword, sizeof(char) * (size + 1), 0);
				printf("\nkeyword is \"%s\"\n",keyword);
			
				//receive count
				int count = 0;
				recv(serverfd, &count, sizeof(int), 0);
				
				count = ntohl(count);
				printf("\nNo. of words %d\n",count);
			
				printf(" \nInsert data in the hash table: keyword %s, fileName %s, count %d\n", keyword, fileName, count);
				//put it in the hash table
				freq_ht_set(frequencyHT, keyword, fileName, count);
				//ffNode* node=freq_ht_get(frequencyHT,keyword);
				//printf("data in the hash table");
				
				free(keyword);
				remaining--;
			}

			//return success to client

			close(serverfd);
			free(fileName);
			break;
		}
			
			//Request list of documents based on keyword
		case 2:
		{
			//get the keywords
			int maxNumKeywords = 0;
			char** keywords;
			int numWords = 0;
			recv(connfd, &numWords, sizeof(int), 0);
			numWords = ntohl(numWords);
			keywords = (char**)malloc(numWords*sizeof(char*));

			ffNode** keywordFreqHeadLists = (ffNode**)malloc(numWords*sizeof(ffNode*));
			int i;
			for(i = 0; i < numWords; i++)
			{
				int length = 0;
				recv(connfd, &length, sizeof(int), 0);
				length = ntohl(length);

				keywords[i] = (char*)malloc((length+1)*sizeof(char));
				memset(keywords[i], 0, (length+1)*sizeof(char));
				recv(connfd, keywords[i], (length+1)*sizeof(char), 0);

				ffNode* node = freq_ht_get(frequencyHT, keywords[i]);
				if(node == NULL)
				{
					printf("Keyword %s not found in any document.\n", keywords[i]);
				}
				maxNumKeywords++;
				keywordFreqHeadLists[i] = node;
			}

			//count keyword frequencies
			RankingNode* allHead = NULL;
			int totalNumFiles = 0;
			for(i = 0; i < numWords; i++)
			{
				if(keywordFreqHeadLists[i] == NULL)
				{
					continue;
				}

				if(allHead == NULL)
				{
					allHead = (RankingNode*)malloc(sizeof(RankingNode));
					memset(allHead, 0, sizeof(RankingNode));
					allHead->filename = keywordFreqHeadLists[i]->filename;
					allHead->numKeyWordsInFile = 1;
					allHead->numInstancesKeywords = keywordFreqHeadLists[i]->frequency;
					allHead->next = NULL;
					keywordFreqHeadLists[i] = keywordFreqHeadLists[i]->next;
					totalNumFiles++;
				}

				while(keywordFreqHeadLists[i] != NULL)
				{
					RankingNode* previous = NULL;
					RankingNode* findRankPosition = allHead;
					while(findRankPosition != NULL)
					{
						if(strcmp(findRankPosition->filename, keywordFreqHeadLists[i]->filename) == 0)
						{
							break;
						}
						previous = findRankPosition;
						findRankPosition = findRankPosition->next;
					}

					if(findRankPosition != NULL)
					{
						findRankPosition->numKeyWordsInFile++;
						findRankPosition->numInstancesKeywords+=keywordFreqHeadLists[i]->frequency;
					}
					else
					{
						findRankPosition = (RankingNode*)malloc(sizeof(RankingNode));
						findRankPosition->filename = keywordFreqHeadLists[i]->filename;
						findRankPosition->numKeyWordsInFile = 1;
						findRankPosition->numInstancesKeywords = keywordFreqHeadLists[i]->frequency;
						if(previous != NULL)
						{
							if(previous->next != NULL)
							{
								findRankPosition->next = previous->next->next;
							}
							previous->next = findRankPosition;
						}
						else
						{
							allHead = findRankPosition;
						}
						totalNumFiles++;
					}

					keywordFreqHeadLists[i] = keywordFreqHeadLists[i]->next;
				}
			}
			free(keywordFreqHeadLists);

			//sort ranking nodes
			int group;
			RankingNode* fullRankingHead = NULL;
			RankingNode* fullRankingLast = NULL;
			for(group = 1; group <= maxNumKeywords; group++)
			{
				RankingNode *iterator2 = allHead;
				RankingNode *previous = NULL;
				RankingNode *groupHeader = NULL;
				RankingNode *groupLast = NULL;
				while(iterator2 != NULL)
				{
					if(iterator2->numKeyWordsInFile == group)
					{
						RankingNode* newGroupMember = (RankingNode*)malloc(sizeof(RankingNode));
						memcpy(newGroupMember, iterator2, sizeof(RankingNode));
						newGroupMember->next = NULL;
						if(groupHeader == NULL)
						{
							groupHeader = newGroupMember;
							groupLast = newGroupMember;
						}
						else
						{
							//insertion sort
							RankingNode* groupIterator = groupHeader;
							RankingNode* groupPrevious = NULL;
							while(groupIterator != NULL && newGroupMember->numInstancesKeywords < groupIterator->numInstancesKeywords)
							{
								groupPrevious = groupIterator;
								groupIterator = groupIterator->next;
							}
							if(groupIterator == NULL)
							{
								groupLast->next = newGroupMember;
								groupLast = newGroupMember;
							}
							else
							{
								if(groupPrevious != NULL)
								{
									groupPrevious->next = newGroupMember;
								}
								else
								{
									groupHeader = newGroupMember;
								}
								newGroupMember->next = groupIterator;
							}
						}
						if(previous != NULL)
						{
							previous->next = iterator2->next;
						}
						RankingNode* toFree = iterator2;
						iterator2 = iterator2->next;
						free(toFree);
					}
					else
					{
						previous = iterator2;
						iterator2 = iterator2->next;
					}
				}

				if(fullRankingHead == NULL)
				{
					fullRankingHead = groupHeader;
				}
				else
				{
					fullRankingLast->next = groupHeader;
				}
				fullRankingLast = groupLast;
			}

			//send ranked filenames to client
			totalNumFiles = htonl(totalNumFiles);
			send(connfd, &totalNumFiles, sizeof(int), 0);

			RankingNode* iterator = fullRankingHead;
			while(iterator != NULL)
			{
				//send filename length
				int fileNameLength = strlen(iterator->filename);
				int length = htonl(fileNameLength);
				send(connfd, &length, sizeof(int), 0);

				//send filename
				send(connfd, iterator->filename, (fileNameLength+1)*sizeof(char), 0);

				RankingNode* toFree = iterator;
				iterator = iterator->next;
				free(toFree);
			}

			break;
		}
			//Request a document based on its filename
		case 3:
		{
			//get filename length
			int filenameLength = 0;
			recv(connfd, &filenameLength, sizeof(int), 0);
			filenameLength = ntohl(filenameLength);

			//get filename
			char* filename = (char*)malloc((filenameLength+1)*sizeof(char));
			memset(filename, 0, (filenameLength+1)*sizeof(char));
			recv(connfd, filename, (filenameLength+1)*sizeof(char), 0);

			//get corresponding server details and connect to that server
			ipValueNode* serverDetails = file_ht_get(fileHT, filename);
			int serverfd = 0;
			if((serverfd = connectTCP_server(serverDetails->ip, serverDetails->port)) == -1)
			{
				puts("Error connecting to server.");
				return 0;
			}

			//send the server the get command
			int getCommand = 2;
			getCommand = htonl(getCommand);
			send(serverfd, &getCommand, sizeof(int), 0);

			//send the server the filename
			send(serverfd, &filenameLength, sizeof(int), 0);
			send(serverfd, filename, (filenameLength+1)*sizeof(char), 0);

			FILE* requestedDocument = fopen(filename,"w+");

			//get document size
			int size = 0;
			recv(serverfd, &size, sizeof(int), 0);
			size = ntohl(size);
			printf("\n document SIZe %d\n", size);

			//get document
			int remaining = size;
			char* buffer = (char*) malloc(101 * sizeof(char));
			while (remaining > 0)
			{
				memset(buffer, 0, 101 * sizeof(char));
				int received = recv(serverfd, buffer, 100 * sizeof(char), 0);
//				printf("\n inside buffer %s\n", buffer);

				if (received == -1)
				{
					puts("ERROR\n");
					continue;
				}
				fprintf(requestedDocument, "%s", buffer);
				printf("\n Received %d (%d remains) with buffer: %s\n", received, remaining-received, buffer);

				fflush(requestedDocument);
				remaining -= received;
			}
//			printf("\n outside buffer %s\n", buffer);

			free(buffer);
			close(serverfd);

			//send file size
			int fileSize = size;
			size = htonl(size);
			send(connfd, &size, sizeof(int), 0);

			//read and send file
			fseek(requestedDocument, 0, SEEK_SET);
			remaining = fileSize;
			buffer = (char*) malloc(fileSize + sizeof(char));
			memset(buffer, 0, fileSize + sizeof(char));
			fread(buffer, sizeof(char), fileSize, requestedDocument);
			while (remaining > 0)
			{
				int sent = send(connfd, buffer, remaining, 0);
				if (sent == -1)
				{
					printf("ERROR (%d remaining).\n", remaining);
					continue;
				}
				remaining -= sent;
			}
			free(buffer);

			//delete temporary holding file
			fclose(requestedDocument);
			remove(filename);

			printf("\nSent entire file, %s, to client.\n", filename);

			free(filename);
			break;
		}

			//server wants to register
		case 1000:
		{
			//store values of server ip and port.
			//TODO: Server should send ip and port information
			ServerNode* newNode;
			newNode = (ServerNode*) malloc(sizeof(ServerNode));
			memset(newNode, 0, sizeof(ServerNode));
//			newNode->server = (struct sockaddr_in*) malloc(sizeof(struct sockaddr_in));
//			memset(newNode->server, 0, sizeof(struct sockaddr_in));
//			memcpy(newNode->server, args->dest, sizeof(struct sockaddr_in));
//			newNode->next = NULL;

			//get address
			int length = 0;
			recv(connfd, &length, sizeof(int), 0);
			length = ntohl(length);
			char* buffer = malloc((length+1)*sizeof(char));
			memset(buffer, 0, (length+1)*sizeof(char));
			recv(connfd, buffer, (length+1)*sizeof(char), 0);
//			inet_pton(AF_INET, buffer, &(newNode->server->sin_addr));
//			inet_aton(buffer, &(newNode->server->sin_addr));
			newNode->server_ip = buffer;
//			free(buffer);

			//get port
			recv(connfd, &length, sizeof(int), 0);
			length = ntohl(length);
			buffer = malloc((length+1)*sizeof(char));
			recv(connfd, buffer, (length+1)*sizeof(char), 0);
//			newNode->server->sin_port = strtol(buffer, NULL, 10);
			newNode->server_port = buffer;
//			free(buffer);

			if (head == NULL)
			{
				head = newNode;
				current = newNode;
			}
			else
			{
				last->next = newNode;
			}
			last = newNode;
			//return success
			int success1 = htonl(0);
			send(connfd, &success1, sizeof(int), 0);

//			buffer = (char*)malloc(100*sizeof(char));
//			memset(buffer, 0, 100*sizeof(char));
//			inet_ntop(AF_INET, &(newNode->server->sin_addr), buffer, sizeof(newNode->server->sin_addr));

			printf("Success on registering %s: %s\n", newNode->server_ip, newNode->server_port);

			break;
		}

			//server wants to deregister
//		case 1001:
//		{
//			//find server info
//			ServerNode* iterator = head;
//			ServerNode* previous = NULL;
//			while (iterator != NULL)
//			{
////				comparison->next = iterator->next;
//				if (memcmp(args->dest, iterator->server,
//						sizeof(struct sockaddr_in)) == 0)
//				{
//					break;
//				}
//				previous = iterator;
//				iterator = iterator->next;
//			}
//
//			//remove node
//			if (iterator != NULL)
//			{
//				if (previous != NULL)
//				{
//					previous->next = iterator->next;
//				}
//				else
//				{
//					head = head->next;
//				}
//				free(iterator->server);
//				free(iterator);
//			}
//
//			int success2 = htonl(0);
//			send(connfd, &success2, sizeof(int), 0);
//
//			break;
//
//		}
	}
//	free(args->dest);
//	free(args);
	close(connfd);
	return 0;
}

int main()
{
	int sockfd;
	char master_ip[100], master_port[100];
	frequencyHT = createFreqHashTable(500000);
	fileHT = createFileHashTable(100000);

#ifdef _DEBUG
	setvbuf(stdout, NULL, _IONBF, 0); // this disables buffering for stdout.
#endif

	//create TCP socket 
	if ((sockfd = createTCP_server(master_ip, master_port)) == 0)
	{
		return -1;
	}

	printf("Server IP and Port is %s %s\n", master_ip, master_port);

	// Write the Master IP and Port Details in a file
	write_master_ip(master_ip, master_port);

	//Run the Master 
	run_master(sockfd);

	//close the connection
	close(sockfd);
}
