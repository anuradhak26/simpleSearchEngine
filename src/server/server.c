#include "server.h"

int run_server(int sockfd)
{
	int connfd;
	puts("Waiting for connection");

	while (1)
	{
		connfd = accept(sockfd, NULL, NULL);

		printf("\n master connecting %d\n", connfd);
		if (connfd == -1)
		{
			perror("server: accept");
		}
		int returnVal = run_command(connfd);
		if (returnVal == 1)
		{
			puts("Success in running command!\n");
		}
		close(connfd);
	}
}

int run_command(int connfd)
{
	int command = -1;
	int size = 0;
	char* buffer;
	char* fileName;
	FILE* requestedDocument;
	int remaining;

	//command number should be network order
	recv(connfd, &command, sizeof(int), 0);
	command = ntohl(command);
	printf("\n%d\n", command);
	switch (command)
	{
		case 0:
			break;

			//store command
		case 1:
			//get document name length
			recv(connfd, &size, sizeof(int), 0);
			size = ntohl(size);
			printf("\nSIZE %d\n", size);

			//get document name
			fileName = (char*) malloc((size + 1) * sizeof(char));
			memset(fileName, 0, (size + 1) * sizeof(char));
			recv(connfd, fileName, (size + 1) * sizeof(char), 0);
			printf("\nSIZE %s\n", fileName);

			//set up file
			printf("Creating new file %s.\n", fileName);
			requestedDocument = fopen(fileName, "w+");

			//get document size
			recv(connfd, &size, sizeof(int), 0);
			size = ntohl(size);
			printf("\n Document size %d \n", size);

			//get document
			remaining = size;
			buffer = (char*) malloc(100 * sizeof(char));
			while (remaining > 0)
			{
				memset(buffer, 0, 100 * sizeof(char));
				int received = recv(connfd, buffer, 100 * sizeof(char), 0);
				if (received == -1)
				{
					puts("ERROR\n");
					continue;
				}
				fprintf(requestedDocument, buffer);
				fflush(requestedDocument);
				remaining -= received;
			}

			printf("\n Received entire file, %s.\n", fileName);

			free(buffer);

			//index document
			fseek(requestedDocument, 0, SEEK_SET);
			char* wordBuffer = (char *) malloc(10 * (1909 + 1) * sizeof(char)); // longer than the length of the longest word in English
			CountNode* head = NULL;
			int wordListSize = 0;
			fscanf(requestedDocument, "%s", wordBuffer);
			while (feof(requestedDocument) == 0)
			{
				int lengthOfWord = strlen(wordBuffer);
				lengthOfWord = removeNonAlphaNumericChars(lengthOfWord,	wordBuffer);
				if (head == NULL)
				{
					char* word = (char *) malloc(lengthOfWord * sizeof(char));
					strcpy(word, wordBuffer);
					head = (CountNode*) malloc(sizeof(CountNode));
					head->word = word;
					head->lengthOfWord = lengthOfWord;
					head->count = 1;
					head->next = NULL;
					wordListSize++;
				}
				else
				{
					CountNode* current = head;
					CountNode* previous = NULL;
					while (current != NULL)
					{
						if (strcmp(current->word, wordBuffer) == 0)
						{
							break;
						}
						previous = current;
						current = current->next;
					}
					if (current != NULL)
					{
						current->count++;
					}
					else
					{
						char* word = (char *) malloc(
								lengthOfWord * sizeof(char));
						strcpy(word, wordBuffer);
						CountNode* newNode = (CountNode*) malloc(
								sizeof(CountNode));
						newNode->word = word;
						newNode->lengthOfWord = lengthOfWord;
						newNode->count = 1;
						newNode->next = NULL;
						previous->next = newNode;
						wordListSize++;
					}
				}
				fscanf(requestedDocument, "%s", wordBuffer);
			}

			//send over the size of the list
			wordListSize = htonl(wordListSize);
			send(connfd, &wordListSize, sizeof(int), 0);

			CountNode* current = head;
			while (current != NULL)
			{
				//send length of the word
				size = htonl(current->lengthOfWord);
				send(connfd, &size, sizeof(int), 0);

				//send the word
				send(connfd, current->word,
						sizeof(char) * (current->lengthOfWord + 1), 0);

				//send the indexing count
				int count = htonl(current->count);
				send(connfd, &count, sizeof(int), 0);

				//once done sending, we don't need the node anymore.  We can just free them here.
				free(current->word);
				CountNode* toFree = current;
				current = current->next;
				free(toFree);
			}
			printf("DONE");
			free(wordBuffer);
			free(fileName);
			fclose(requestedDocument);
			break;

			//get command
		case 2:
			//get document name length
			recv(connfd, &size, sizeof(int), 0);
			size = ntohl(size);

			//get document name
			fileName = (char*) malloc((size + 1) * sizeof(char));
			memset(fileName, 0, (size + 1) * sizeof(char));
			recv(connfd, fileName, (size + 1) * sizeof(char), 0);

			//check if the file exists
			if (access(fileName, F_OK) == -1)
			{
				//if not, return -1 and terminate
				size = htonl(-1);
				send(connfd, &size, sizeof(int), 0);
				printf("Requested %s, but file not found.\n", fileName);
				break;
			}

			//get file size
			struct stat st;
			stat(fileName, &st);
			size = st.st_size;

			//send file size
			size = htonl(size);
			send(connfd, &size, sizeof(int), 0);

			//read and send file
			requestedDocument = fopen(fileName, "r");
			remaining = st.st_size;
			buffer = (char*) malloc(st.st_size + sizeof(char));
			memset(buffer, 0, st.st_size + sizeof(char));
			fread(buffer, sizeof(char), st.st_size, requestedDocument);
			while (remaining > 0)
			{
				int sent = send(connfd, buffer, remaining, 0);
				if (sent == -1)
				{
					puts("ERROR\n");
					continue;
				}
				remaining -= sent;
			}

			printf("File %s found and sent.\n", fileName);

			free(buffer);
			free(fileName);
			fclose(requestedDocument);
			break;

		default:
			break;
	}

	return 1;
}

int removeNonAlphaNumericChars(int length, char* string)
{
	int i = 0;
	int replace = 0;
	for (i = 0; i + replace < length; i++)
	{
		if (!isalpha(string[i + replace]) && !isdigit(string[i + replace])
				&& !(string[i + replace] == '-'))
		{
			replace++;
			i--;
			continue;
		}
		string[i] = tolower(string[i + replace]); //also converts to lowercase
	}
	string[i] = '\0';
	printf("Actual value %s\n",string);
	return i;
}

int write_server_ip(char *addrstr, char *port)
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

//Read the Master Address details
int readMasterIP(char *master_ip, char *master_port)
{
	// read IP address and port from file define in header.h
	FILE *ptr_file;
	char buf[100];

	// read the global file
	ptr_file = fopen(FILENAME, "r");
	if (!ptr_file)
	{
		return -1;
		printf("no file open");
	}
	if (fgets(buf, 1000, ptr_file) != NULL)
	{
		// ip address
		strcpy(master_ip, strtok(buf, ","));

		// port number
		strcpy(master_port, strtok(NULL, ","));
	}

	if (master_ip && master_port)
		printf("portmapper ip and port is %s %s\n", master_ip, master_port);

	fclose(ptr_file);

	return 0;
}

int main()
{
	int sockfd, masterfd;
	char server_ip[100], server_port[100];
	char master_ip[100], master_port[100];
//	char buffer[512];

	memset(server_ip, 0, 100*sizeof(char));
	memset(server_port, 0, 100*sizeof(char));

	if ((sockfd = createTCP_server(server_ip, server_port)) == 0)
	{
		return -1;
	}

#ifdef _DEBUG
	setvbuf(stdout, NULL, _IONBF, 0); // this disables buffering for stdout.
#endif

	printf("Server IP and Port is %s %s\n", server_ip, server_port);

	//register with master
	readMasterIP(master_ip, master_port);
	if ((masterfd = connectTCP_server(master_ip, master_port)) == 0)
	{
		fputs("Error, master is not online", stderr);
		return -1;
	}
//	recv(masterfd, buffer, sizeof(buffer), 0);
	int command = htonl(1000);
	send(masterfd, &command, sizeof(int), 0);

	//send address
	int length = strlen(server_ip);
	int networkLength = htonl(length);
	send(masterfd, &networkLength, sizeof(int), 0);
	send(masterfd, server_ip, (length+1)*sizeof(char), 0);

	//send port number
	length = strlen(server_port);
	networkLength = htonl(length);
	send(masterfd, &networkLength, sizeof(int), 0);
	send(masterfd, server_port, (length+1)*sizeof(char), 0);

	recv(masterfd, &command, sizeof(int), 0);
	puts("Received register acknowledgement");
	command = ntohl(command);
	close(masterfd);
	if(command != 0)
	{
		fputs("Error in registering with master", stderr);
		return -1;
	}

	// Write the server IP and Port Details in a file
//	write_server_ip(server_ip, server_port);

	//Run the server
	//create TCP socket

	run_server(sockfd);

	puts("Unregistering from the server");

	//deregister the server
	if ((masterfd = connectTCP_server(master_ip, master_port)) == 0)
	{
		fputs("Error, master is not online", stderr);
		return -1;
	}
//	recv(masterfd, buffer, sizeof(buffer), 0);
	command = htonl(1001);
	send(masterfd, &command, sizeof(int), 0);
	recv(masterfd, &command, sizeof(int), 0);
	command = ntohl(command);
	close(masterfd);
	if(command != 0)
	{
		fputs("Error in deregistering from master", stderr);
		return -1;
	}

	close(sockfd);
}
