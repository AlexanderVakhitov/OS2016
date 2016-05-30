#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>
#include <netinet/in.h>

#define PORT 5001

#define WIDTH 10
#define HEIGHT 10

char field[WIDTH][HEIGHT] = {
	"..........",
	"..........",
	".....*....",
	"......*...",
	"....***...",
	"..........",
	"..........",
	"..........",
	"..........",
	".........."
};

void start_server() {
	int sockfd, newsockfd, portno, clilen;
	struct sockaddr_in serv_addr, cli_addr;
	int pid;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0) {
		printf("Error! Can't open socket!\n");
		exit(1);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = PORT;

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		printf("Error! Can't build socket!\n");
		exit(1);
	}


	listen(sockfd, 5);
	clilen = sizeof(cli_addr);

	while (1) {
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0) {
			printf("Error! Can't accept socket!\n");
			exit(1);
		}
		pid = fork();
		if (pid < 0) {
			printf("Error! Can't fork!\n");
			exit(1);
		}
		if (pid == 0) {
			close(sockfd);
			sendMessage(newsockfd);
			exit(0);
		} else {
			close(newsockfd);
		}
	}
}

void sendMessage(int socket) {
	int n;
	n = write(socket, field, WIDTH * HEIGHT);
	if (n < 0) {
		printf("Error! Can't write in socket!\n");
		exit(1);
	}
}

int get_count(int x, int y) {
	int i, j, count = 0;
	int _dx[] = {-1, 0, 1};
	int _dy[] = {-1, 0, 1};

	for (i = 0; i < 3; ++i) {
		for (j = 0; j < 3; ++j) {
			int dx = x + _dx[i];
			int dy = y + _dy[j];
			if (dx >= 0 &&
				dx < WIDTH &&
				dy >= 0 &&
				dy < HEIGHT &&
				(x != dx || y != dy)) {
				if (field[dx][dy] == '*') {
					++count;
				}
			}
		}
	}

	return count;
}

void next_condition() {
	int i, j;
	char tmp[WIDTH][HEIGHT];

	for (i = 0; i < WIDTH; ++i) {
		for (j = 0; j < HEIGHT; ++j) {
			switch (get_count(i, j)) {
				case 2:
					tmp[i][j] = field[i][j];
					break;
				case 3:
					tmp[i][j] = '*';
					break;
				default:
					tmp[i][j] = '.';
					break;
			}
		}
	}

	for (i = 0; i < WIDTH; ++i) {
		for (j = 0; j < HEIGHT; ++j) {
			field[i][j] = tmp[i][j];
		}
	}
}

int main(int argc, char * argv[]) {
	int error, status;
	pthread_t serv_list_thread, next_cond_thread;

	error = pthread_create(&serv_list_thread, NULL, &start_server, NULL);
	if (error != 0) {
		printf("Error! Can't create thread!\n");
		exit(1);
	}

	while (1) {
		error = pthread_create(&next_cond_thread, NULL, &next_condition, NULL);
		if (error != 0) {
			printf("Error! Can't create thread!\n");
			exit(1);
		}
		sleep(1);
		status = pthread_kill(next_cond_thread, NULL);
		if(status == 0) {
			printf("Error! Too much time!\n");
			pthread_cancel(next_cond_thread);
		}
	}

	return 0;
}
