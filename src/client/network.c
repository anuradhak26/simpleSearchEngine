#include "network.h"

/********************************* TCP socket function ***************************************************/
int createTCP_server(char *addrstr, char *port)
{
	int sockfd, len;  // listen on sock_fd, new connection on new_fd    
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_in local;
	void *ptr;
	char hostname[1024];
	int rv, yes = 1;

	hostname[1023] = '\0';
	gethostname(hostname, 1023);

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP
	if ((rv = getaddrinfo(hostname, NULL, &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 0;
	}

	// loop through all the results and bind to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol))
				== -1)
		{
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))
				== -1)
		{
			perror("setsockopt: reuse address");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(sockfd);
			perror("server: bind");
			continue;
		}
		break;
	}

	if (p == NULL)
	{
		fprintf(stderr, "server: failed to bind\n");
		return 0;
	}

	/*get ip address and port*/
	//get ip
	switch (p->ai_family)
	{
		case AF_INET:
			ptr = &((struct sockaddr_in *) p->ai_addr)->sin_addr;
			break;
		case AF_INET6:
			ptr = &((struct sockaddr_in6 *) p->ai_addr)->sin6_addr;
			break;
	}
	inet_ntop(p->ai_family, ptr, addrstr, 100);

	// get port number
	len = sizeof(local);
	getsockname(sockfd, (struct sockaddr *) &local, &len);
	sprintf(port, "%d", htons(local.sin_port));
	/* done get ip and port */
	freeaddrinfo(servinfo); // all done with this structure
	if (listen(sockfd, 10) == -1)
	{
		perror("listen");
		return 0;
	}

	return sockfd;
}

int connectTCP_server(char *Server_ip, char *port)
{
	int sockfd, status;
	struct addrinfo hints, *servinfo, *p;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if ((status = getaddrinfo(Server_ip, port, &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return 0;
	}

	// loop through all the results and connect to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol))
				== -1)
		{
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(sockfd);
			perror("TCP client: connect");
			continue;
		}
		else
		{
			printf("Connecting.... Master... ");

		}

		break;
	}
	if (p == NULL)
	{
		fprintf(stderr, "client: failed to connect\n");
		return 0;
	}

	freeaddrinfo(servinfo); // all done with this structure

	return sockfd;
}

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in*) sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

/********************************* END TCP socket function ************************************************/

