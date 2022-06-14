// Version 1.0
#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
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
void header(char *, char *);
int recvb(int, void *, long int, FILE *);

int main(int argc, char **argv)
{
	struct sockaddr_in server_addr;
	struct hostent *host;
	int *sockfd = (int *)malloc(sizeof(int));
	long int sendbyte, recvbyte, length, ranges = 0;
	char *msg = (char *)malloc(2000 * sizeof(char)), *body = (char *)malloc(1024000 * sizeof(char));
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
	bzero(&server_addr.sin_zero, 8);
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
	strcpy(msg, "GET /FlowerDance.flac HTTP/1.1\n");
	strcat(msg, "Content-Type: text/html\n");
	strcat(msg, "Host: dark-1258224056.cos.ap-chengdu.myqcloud.com\n");
	strcat(msg, "Range: bytes=0-\n");
	strcat(msg, "Connection: Keep-Alive\n");
	strcat(msg, "Origin: https://www.aliyundrive.com/\n");
	strcat(msg, "Referer: https://www.aliyundrive.com/\n\n");
	printf("\33[1;33mRequest header\33[0m: %s\33[1;31m----\33[0m\nrequest header \33[1;33msize: %zd\33[0m\n", msg, strlen(msg));
	if ((sendbyte = send(*sockfd, msg, strlen(msg), 0)) < 0)
	{
		fprintf(stderr, "Msg send fail.\n");
		exit(-1);
	}
	printf("Send bytes: %ld\n", sendbyte);
	fp = fopen("icon.ico", "w");
	// ranges = atoi(range);
	printf("\33[31mRanges:\33[0m %ld\n", ranges);
	if ((recvbyte = recvb(*sockfd, body, ranges, fp)) > 0)
	{
		printf("All: %ld Bytes\tDownloaded: %ld Bytes\n", recvbyte, (ranges == 0) ? recvbyte : ranges);
		// if ((ranges) || (ranges < ))
	}

	// pthread_create(&id, NULL, (void *)recvmsgs, (void *)sockfd);
	// pthread_create(&id, NULL, (void *)sendmsgs, (void *)sockfd);

	// pthread_join(id, NULL);
	fclose(fp);
	close(*sockfd);
	free(msg);
	free(sockfd);
	free(body);
	puts("\33[1;32mDone.\33[0m");
	return 0;
}

int recvb(int __sockfd, void *buf, long int range, FILE *fp/* , long int length */)
{
	long int recvbyte, length;
	while ((recvbyte = recv(__sockfd, buf, RESPONSESIZE - 1, 0)) > 0)
	{
		static unsigned long int recvbytes = 0;
		recvbytes += recvbyte;
		char *content_t, *pre = (char *)buf;
		if ((content_t = strstr(buf, "Content-Range:")) || (content_t = strstr(buf, "Content-Length:")))
		{
			if ((content_t = strchr(content_t, '/')) || (content_t = strchr(content_t, ' ')))
			{
				length = atoi(content_t + 1);
				(range == 0) ? range = length : range;
				if ((pre = strstr(buf, "\r\n\r\n")))
					recvbyte += ((char *)buf - (pre += 4));
				puts(buf);
			}
		}
		printf("Response: \33[1;36m%ld\33[0m bytes and allow \33[1;35m%ld\33[0m bytes. Length: %ld\n", recvbyte, recvbytes, length);
		if ((fwrite(pre, sizeof(char), recvbyte, fp)) < 0)
			fprintf(stderr, "Write error.");
		if (range < recvbytes)
			break;
	}
	return length;
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