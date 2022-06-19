/* struct is beautiful
 * by sdk250
 */
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h> // gethostbyname()
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#define ERRORMSG "\33[1;31mError: \33[0m"
#define SERVERIP "192.168.20.35"
#define SERVERPORT 8080
#define MSGSIZE 500

void sendmsgs(int *);
void recvmsgs(int *);

int main(int argc, char **argv) {
	struct sockaddr_in server_addr;
	struct hostent *host;
	char *ip = SERVERIP;
	int sockfd, port = SERVERPORT;
	pthread_t id;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "%sSocket create fail, socket: %d\n", ERRORMSG, sockfd);
		exit(EXIT_FAILURE);
	}
	printf("Socket create success. sockfd: %d\n", sockfd);

	bzero(&server_addr, sizeof(struct sockaddr));
	server_addr.sin_family = AF_INET;
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
	printf("Get IP: %s:%u of %s success!\n", inet_ntoa(server_addr.sin_addr), port, ip);

	if ((connect(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))) < 0) {
		fprintf(stderr, "%sConnect to IP: %s:%u of %s fail.\n", ERRORMSG, inet_ntoa(server_addr.sin_addr), port, ip);
		exit(EXIT_FAILURE);
	}
	printf("Connect to IP: %s:%u of %s success.\n", inet_ntoa(server_addr.sin_addr), port, ip);

	if (pthread_create(&id, NULL, (void *)recvmsgs, (void *)&sockfd) != 0) {
		fprintf(stderr, "%sCreate \'recvmsg\' function fail\n", ERRORMSG);
		exit(EXIT_FAILURE);
	}
	if (pthread_create(&id, NULL, (void *)sendmsgs, (void *)&sockfd) != 0) {
		fprintf(stderr, "%sCreate \'sendmsg\' function fail.\n", ERRORMSG);
		exit(EXIT_FAILURE);
	}

	pthread_join(id, NULL);
	close(sockfd);
	puts("\33[1;32mDone.\33[0m");
	return 0;
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