#include "client.h"

//Read the Master Address details
int readMasterIP(char *server_ip, char *server_port)
{
	// read IP address and port from file define in header.h
	FILE *ptr_file;
	char buf[100];

	// read the global file
	ptr_file = fopen(FILENAME, "r");
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

//Client 
int main(int argc, char *argv[])
{
#ifdef _DEBUG
	setvbuf(stdout, NULL, _IONBF, 0); // this disables buffering for stdout.
#endif

	char master_ip[100], master_port[100];
	int sockfd, task, i;
	struct dirent *pDirent;
	DIR *pDir;
	struct stat statbuf;

	// read master ip and port through file
	if (!readMasterIP(master_ip, master_port))
		return 0;
	printf("\nConnection Details is %s %s\n", master_ip, master_port);

	printf("Select the task to carry out\n");
	printf("1. Indexing a Document\n");
	printf("2. Search Query\n");
	printf("3. Get Document\n");
	scanf("%d", &task);
	printf(" TASk is %d", task);
	int taskVal = htonl(task);
	stat(argv[1], &statbuf);
	if (S_ISDIR(statbuf.st_mode))
	{
		task = 4;
		printf("directory %d\n", task);
	}

	//

	// open socket to connect to master
	/* if ((sockfd = connectTCP_server(master_ip, master_port)) == 0)
	 return 0;*/
//	printf("yes");
	printf(" TASk is %d\n", task);

	switch (task)
	{
		case 1:
		{
			printf("Send Request for Indexing the Document\n");
			for (i = 1; i < argc; i++)
			{

				//printf("Please enter the filename");
				// open socket to connect to master
				if ((sockfd = connectTCP_server(master_ip, master_port)) == 0)
					return 0;

				//send the task to the master
				send(sockfd, &taskVal, sizeof(int), 0);

				//send the file name and the size of the file
				char* fileName = argv[i];
//				int nameSize = sizeof(fileName);
				int filenameLength = strlen(fileName);
				int networkNameLength = htonl(filenameLength);
				printf(" fileName : %s, nameSize = %d", fileName, filenameLength);
				send(sockfd, &networkNameLength, sizeof(int), 0);
//				printf(" nameSize = %d", nameSize);
				send(sockfd, fileName, (filenameLength + 1) * sizeof(char), 0);
//				printf(" fileName : %s", fileName);

				//get file size
				struct stat st;
				stat(fileName, &st);
				int size = st.st_size;
				printf("File Size : %d", size);

				//send file size
				size = htonl(size);
				send(sockfd, &size, sizeof(int), 0);

				//read and send file
				FILE* requestedDocument = fopen(fileName, "r");
				int remaining = st.st_size;
				char* buffer = (char*) malloc(st.st_size + sizeof(char));
				memset(buffer, 0, st.st_size + sizeof(char));
				fread(buffer, sizeof(char), st.st_size, requestedDocument);
				while (remaining > 0)
				{
					int sent = send(sockfd, buffer, remaining, 0);
					if (sent == -1)
					{
						puts("ERROR\n");
						continue;
					}
					remaining -= sent;
				}

				free(buffer);
				close(sockfd);
			}

			break;
		}
			// keyword search
		case 2:
		{
			// open socket to connect to master
			if ((sockfd = connectTCP_server(master_ip, master_port)) == 0)
				return 0;

			//send the task to the master
			send(sockfd, &taskVal, sizeof(int), 0);

			//search for the keyword
			printf("\nSearch the keyword\n");
			char keywords[2001];
			memset(keywords, 0, 2001 * sizeof(char));
			int i;
			printf("Enter the keywords separated by spaces to be searched for\n");
			scanf("%2000s", keywords);

			int length = strlen(keywords);
			int numSpaces = 0;
			for (i = 0; i < length; i++)
			{
				if (keywords[i] == ' ')
				{
					numSpaces++;
				}
			}

			int numWords = numSpaces + 1;

			//send number of keywords over
			numWords = htonl(numWords);
			send(sockfd, &numWords, sizeof(int), 0);

			//send all keywords one at a time
			char* keyword = strtok(keywords, " ");
			while (keyword != NULL)
			{
				int keywordLength = htonl(strlen(keyword));
				send(sockfd, &keywordLength, sizeof(int), 0);
				send(sockfd, keyword, sizeof(char) * (strlen(keyword) + 1), 0);
				keyword = strtok(NULL, " ");
			}

			//receive number of files
			int numFiles;
			recv(sockfd, &numFiles, sizeof(int), 0);
			numFiles = ntohl(numFiles);

			printf("%d files contain your keywords.\n", numFiles);

			//receive filenames
			int counter = 1;
			for (; numFiles > 0; numFiles--)
			{
				int filenameSize = 0;
				char filename[256];
				memset(filename, 0, 256 * sizeof(char));

				//get filename size
				recv(sockfd, &filenameSize, sizeof(int), 0);
				filenameSize = ntohl(filenameSize);

				//get filename
				recv(sockfd, filename, (filenameSize + 1) * sizeof(char), 0);
				printf("%d .  %s\n", counter, filename);
				counter++;
			}

			close(sockfd);

			break;
		}
		case 3:
		{
			// open socket to connect to master
			if ((sockfd = connectTCP_server(master_ip, master_port)) == 0)
				return 0;

			//send the task to the master
			send(sockfd, &taskVal, sizeof(int), 0);

			char filename[256];
			memset(filename, 0, 256 * sizeof(char));
			printf("Enter the file name\n");
			scanf("%255s", filename);

			//send filename length and filename
			int filenameLength = htonl(strlen(filename));
			send(sockfd, &filenameLength, sizeof(int), 0);
			send(sockfd, filename, sizeof(char) * (strlen(filename) + 1), 0);

			//receive file size
			int fileSize = 0;
			recv(sockfd, &fileSize, sizeof(int), 0);
			fileSize = ntohl(fileSize);

			//open and prepare file
			FILE* incomingFile = fopen(filename, "w");

			//receive the file
			int remaining = fileSize;
			char* buffer = (char*) malloc(100 * sizeof(char));
			while (remaining > 0)
			{
				memset(buffer, 0, 100 * sizeof(char));
				int received = recv(sockfd, buffer, 100 * sizeof(char), 0);
				if (received == -1)
				{
					puts("ERROR\n");
					continue;
				}
				fprintf(incomingFile, buffer);
				fflush(incomingFile);
				remaining -= received;
			}
			free(buffer);
			fclose(incomingFile);
			close(sockfd);
			break;
		}
		case 4:
		{

//			int count = 0;
			printf("Send Request for Indexing the Directory\n");
			if (argc < 2)
			{
				printf("Usage: testprog <dirname>\n");
			}
			pDir = opendir(argv[1]);
			if (pDir == NULL)
			{
				printf("Cannot open directory '%s'\n", argv[1]);
			}

			while ((pDirent = readdir(pDir)) != NULL)
			{
				if (!strcmp(pDirent->d_name, ".") || !strcmp(pDirent->d_name, ".."))
				{
					printf("[%s]\n", pDirent->d_name);
				}
				else
				{
					printf("%s\n", pDirent->d_name);

					// open socket to connect to master
					if ((sockfd = connectTCP_server(master_ip, master_port)) == 0)
						return 0;
					printf("%s\n", pDirent->d_name);

					printf("%d\n", taskVal);

					//send the task to the master
					send(sockfd, &taskVal, sizeof(int), 0);

					//send the file name and the size of the file
					char* fileName = strdup(pDirent->d_name);
					printf("%s\n", fileName);

//					int nameSize = sizeof(fileName);
					int filenameLength = strlen(fileName);
					int networkNameLength = htonl(filenameLength);
					printf(" fileName : %s, nameSize = %d\n", fileName, filenameLength);
					send(sockfd, &networkNameLength, sizeof(int), 0);
					send(sockfd, fileName, (filenameLength + 1) * sizeof(char),0);

					char* filenameAndDir = strdup(argv[1]);
					strcat(filenameAndDir,"/");
					strcat(filenameAndDir, fileName);
					printf("finding \"%s\" file size\n", filenameAndDir);

					//get file size
					struct stat st;
					stat(filenameAndDir, &st);
					int size = st.st_size;
					printf("File Size : %d\n", size);
					free(filenameAndDir);

					//send file size
					int networksize = htonl(size);
					send(sockfd, &networksize, sizeof(int), 0);

					//read and send file CHECK
					FILE* requestedDocument = fopen(filenameAndDir, "r");
					int remaining = size;
					char* buffer = (char*) malloc(size + sizeof(char));
					memset(buffer, 0, size + sizeof(char));
					fread(buffer, sizeof(char), size, requestedDocument);
					while (remaining > 0)
					{
						int sent = send(sockfd, buffer, remaining, 0);
						printf("Sent %d over\n", sent);
						if (sent == -1)
						{
							puts("ERROR\n");
							continue;
						}
						remaining -= sent;
					}
					free(buffer);
					close(sockfd);
				}
			}
			break;
		}
		default:
		{
			break;
		}
	}
}
