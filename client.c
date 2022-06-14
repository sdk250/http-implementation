// Version 1.0
#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <strings.h>
#include <netdb.h> // gethostbyname()
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#define SERVERPORT 8080
#define BUFFERSIZE 100
#define RESPONSESIZE 102400
#define SERVERIP "127.0.0.1"

void recvmsgs(void *);
void sendmsgs(void *);
void header(char*, char*);

int main(int argc, char **argv)
{
	struct sockaddr_in server_addr;
	struct hostent *host;
	int *sockfd = (int *)malloc(sizeof(int));
	long int sendbyte, recvbyte, length;
	char *msg = (char *)malloc(100 * sizeof(char)),
	*method = (char *)malloc(6 * sizeof(char)),
	*index = (char *)malloc(30 * sizeof(char)),
	*user_agent = (char *)malloc(50 * sizeof(char)),
	*host_req = (char *)malloc(32 * sizeof(char)),
	*content_type = (char *)malloc(30 * sizeof(char)),
	*range = (char *)malloc(20 * sizeof(char)),
	*connection = (char *)malloc(15 * sizeof(char)),
	*body = (char *)malloc(102400 * sizeof(char));
	char *ip = SERVERIP;
	unsigned int port = SERVERPORT;
	FILE *fp;
	pthread_t id;

	if (argc < 2)
	{
		if ((host = gethostbyname(SERVERIP)) == NULL)
		{
			fprintf(stderr, "Get host error.\n");
			exit(EXIT_FAILURE);
		}
		puts("Get host success.");
	} else if (argc >= 2) {
		ip = argv[1];
		host = gethostbyname(argv[1]);
		port = atoi(argv[2]);
	} else {
		fprintf(stderr, "Argc Error.\n");
		exit(EXIT_FAILURE);
	}
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr = *((struct in_addr*)host->h_addr);
	bzero(&server_addr.sin_zero, sizeof(struct sockaddr));
	if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr, "Socket create fail.\n");
		exit(EXIT_FAILURE);
	}
	puts("Socket create success.");
	if (connect(*sockfd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) < 0)
	{
		fprintf(stderr, "Connect to %s fail.\n", ip);
		exit(EXIT_FAILURE);
	}
	puts("Connect success.");

	header(method, "GET");
	header(index, "/favicon.ico");
	header(user_agent, "C/Socket");
	header(host_req, "127.0.0.1");
	header(content_type, "text/html");
	header(range, "0-");
	header(connection, "Keep-Alive");
	sprintf(msg, "%s %s HTTP/1.1\n"
		"Content-Type: %s\n"
		"User-Agent: %s\n"
		"Host: %s\n"
		"Range: bytes=%s\n"
		"Connection: %s\n\n",
		method,
		index,
		content_type,
		user_agent,
		host_req,
		range,
		connection
	);
	printf("enter: %s\33[1;31m-\33[0m\nstrlen: %zd\n", msg, strlen(msg));
	if ((sendbyte = send(*sockfd, msg, strlen(msg), 0)) < 0)
	{
		fprintf(stderr, "Msg send fail.");
		exit(-1);
	}
	printf("Send bytes: %ld\n", sendbyte);
	fp = fopen("icon.ico", "a+");
	while ((recvbyte = recv(*sockfd, body, RESPONSESIZE - 1, 0)) > 0)
	{
		static unsigned long int recvbytes = 0;
		recvbytes += recvbyte;
		char *content_t, *pre;
		pre = body;
		if ((content_t = strstr(body, "Content-Length:")))
		{
			if ((content_t = strchr(content_t, ' ')))
			{
				content_t += 1;
				length = atoi(content_t);
				if ((body = strstr(body, "\r\n\r\n")))
					body += 4;
				recvbyte += (pre - body);
			}
		}
		printf("Response: \33[1;36m%ld\33[0m bytes and allow \33[1;35m%ld\33[0m bytes\n", recvbyte, recvbytes);
		if ((fwrite(body, sizeof(char), recvbyte, fp)) < 0)
			fprintf(stderr, "Write error.");
		if (recvbytes > length)
			break;
	}

	// pthread_create(&id, NULL, (void *)recvmsgs, (void *)sockfd);
	// pthread_create(&id, NULL, (void *)sendmsgs, (void *)sockfd);

	// pthread_join(id, NULL);
	fclose(fp);
	close(*sockfd);
	free(sockfd);
	free(body);
	free(method);
	free(index);
	free(user_agent);
	free(host_req);
	free(content_type);
	free(range);
	free(connection);
	puts("\33[1;32mDone.\33[0m");
	return 0;
}

void header(char *type, char *arg)
{
	printf("Enter %s:", arg);
	if ((type = fgets(type, BUFFERSIZE, stdin)))
	{
		if (*type == '\n')
		{
			*type = '\0';
			strcat(type, arg);
		}
		if (*(type + strlen(type) - 1) == '\n')
			*(type + strlen(type) - 1)= '\0';
	}
}

void recvmsgs(void *clientfd)
{
	int recvbyte;
	char buffer[BUFFERSIZE];
	puts("Waiting for messages....");
	while ((recvbyte = recv(*((int *)clientfd), buffer, BUFFERSIZE, 0)) > 0)
	{
		buffer[recvbyte] = '\0';
		printf("\n\33[33mResponse: \33[0m\33[1;32m%s\33[0m\t\33[1;36msize: %d\33[0m\n\33[35mEnter: \33[0m", buffer, recvbyte);
	}
}

void sendmsgs(void *clientfd)
{
	char *msg = (char *)malloc(100 * sizeof(char)), *ch;
	printf("\33[35mEnter: \33[0m");
	while ((msg = fgets(msg, BUFFERSIZE, stdin)) != NULL)
	{
		int sendbyte;
		if ((ch = strchr(msg, '\n')) != NULL)
			*ch = '\0';
		if (msg[0] == 27)
			break;
		puts("Send msg....");
		if ((sendbyte = send(*((int *)clientfd), msg, strlen(msg) > 100 ? BUFFERSIZE : strlen(msg), 0)) < 0)
		{
			fprintf(stderr, "Send msg fail.\n");
			exit(EXIT_FAILURE);
		}
		puts("Send success.");
		printf("\33[35mEnter: \33[0m");
	}
	free(msg);
}