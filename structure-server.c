/* struct is beautiful!
 * by sdk250
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#define ERRORMSG "\33[1;31mError: \33[0m"
#define SERVERIP "0.0.0.0"
#define SERVERPORT 9969
#define MSGSIZE 500
#define BACKLOG 10

void recvmsgs(int *);
void sendmsgs(int *);

int main(int argc, char **argv) {
	struct sockaddr_in server_addr, client_addr[BACKLOG];
	struct hostent *host;
	char *ip = SERVERIP;
	int sockfd, client_fd[BACKLOG], port = SERVERPORT, i, sin_size = sizeof(struct sockaddr), j = 0;
	pthread_t id;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "%sSocket create fail. socket: %d\n", ERRORMSG, sockfd);
		exit(EXIT_FAILURE);
	}
	printf("Socket create success. socket: %d\n", sockfd);

	bzero(&server_addr, sizeof(struct sockaddr));
	bzero(&client_addr, sizeof(struct sockaddr));
	server_addr.sin_family = AF_INET;
	// for (i = 0; i < BACKLOG; i++)
	// 	server_addr.sin_family = client_addr[i].sin_family = AF_INET;
	if (argc > 1)
		ip = argv[1];
	if (argc > 2)
		port = atoi(argv[2]);
	if (!(host = gethostbyname(ip))) {
		fprintf(stderr, "%sAnalyzing \'%s\' fail.\n", ERRORMSG, ip);
		exit(EXIT_FAILURE);
	}
	
	server_addr.sin_addr = *((struct in_addr *)host->h_addr_list[0]);
	server_addr.sin_port = htons(port);
	printf("Get IP:%s:%u of %s success!\n", inet_ntoa(server_addr.sin_addr), port, ip);

	if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0) {
		fprintf(stderr, "%sBind IP: %s:%u fail.\n", ERRORMSG, inet_ntoa(server_addr.sin_addr), port);
		exit(EXIT_FAILURE);
	}
	printf("Bind IP: %s:%u success.\n", inet_ntoa(server_addr.sin_addr), port);

	if (listen(sockfd, BACKLOG) < 0) {
		fprintf(stderr, "%sListen IP: %s:%u fail.\n", ERRORMSG, inet_ntoa(server_addr.sin_addr), port);
		exit(EXIT_FAILURE);
	}
	printf("Listening on IP: %s:%u....\n", inet_ntoa(server_addr.sin_addr), port);

	while ((client_fd[j] = accept(sockfd, (struct sockaddr *)&client_addr[j], (socklen_t *)&sin_size)) > 0) {
		printf("Accept \'%s:%u\' success!\n", inet_ntoa(client_addr[j].sin_addr), port);
		if (pthread_create(&id, NULL, (void *)recvmsgs, (void *)&client_fd[j]) != 0) {
			fprintf(stderr, "%sCreate \'recvmsg\' function fail\n", ERRORMSG);
			exit(EXIT_FAILURE);
		}
		if (pthread_create(&id, NULL, (void *)sendmsgs, (void *)&client_fd[j]) != 0) {
			fprintf(stderr, "%sCreate \'sendmsg\' function fail.\n", ERRORMSG);
			exit(EXIT_FAILURE);
		}
		j++;
		// fprintf(stderr, "%sAccpet \'%s:%u\' fail.\n", ERRORMSG, inet_ntoa(client_addr[0].sin_addr), port);
		// exit(EXIT_FAILURE);
	}
	

	

	pthread_join(id, NULL);
	close(sockfd);
	// close(client_fd);
	puts("\33[1;32mDone.\33[0m");
}

void sendmsgs(int *server_fd)
{
	char *msg = (char *)malloc(MSGSIZE * sizeof(char)), *ch;
	printf("\33[35mEnter: \33[0m");
	while ((msg = fgets(msg, MSGSIZE, stdin)))
	{
		int sendbyte;
		if ((ch = strchr(msg, '\n')))
			*ch = '\0';
		if (msg[0] == 27)
			break;
		puts("Send msg....");
		if ((sendbyte = send(*server_fd/* *((int *)server_fd) */, msg, (strlen(msg) > MSGSIZE) ? MSGSIZE : strlen(msg), 0)) < 0)
		{
			fprintf(stderr, "%sSend msg fail.\n", ERRORMSG);
			break;
		}
		puts("Send success.");
		fflush(stdout);
		printf("\33[35mEnter: \33[0m");
	}
	free(msg);
}

void recvmsgs(int *server_fd)
{
	int recvbyte;
	char buffer[MSGSIZE];
	puts("Waiting for messages....");
	while ((recvbyte = recv(*server_fd/* *((int *)server_fd) */, buffer, MSGSIZE, 0)) > 0)
	{
		buffer[recvbyte] = '\0';
		printf("\n\33[33mResponse: \33[0m\33[1;32m%s\33[0m\t\33[1;36msize: %d\33[0m\n\33[35mEnter: \33[0m", buffer, recvbyte);
	}
}
