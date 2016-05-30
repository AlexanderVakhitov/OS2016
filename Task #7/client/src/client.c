#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>

#define HOST "localhost"
#define PORT "5001"

#define WIDTH 10
#define HEIGHT 10

char field[WIDTH * HEIGHT];

int main(int argc, char * argv[]) {
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	portno = atoi(PORT);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0) {
		printf("Error! Can't open socket!\n");
		exit(1);
	}

	server = gethostbyname(HOST);

	if (server == NULL) {
		printf("Error! Can't find server!\n");
		exit(0);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);

	if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		printf("Error! Can't connect to server!\n");
		exit(1);
	}

	bzero(field, WIDTH * HEIGHT);
	n = read(sockfd, field, WIDTH * HEIGHT);

	if (n < 0) {
		printf("Error! Can't read from socket!\n");
		exit(1);
	}

	int i, j;
	for (i = 0; i < WIDTH; ++i) {
		for (j = 0; j < HEIGHT; ++j) {
			printf("%c", field[i * HEIGHT + j]);
		}
		printf("\n");
	}

	return 0;
}
