#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_SIZE 1000

struct handler {
	int read;
	int write;
};

struct handler pipes[MAX_SIZE];

int amount;
long long numbers[MAX_SIZE];

void print_result() {
	int i, result, overall = 0;
	for (i = 0; i < amount; ++i) {
		FILE * fd = fdopen(pipes[i].read, "r");
		fscanf(fd, "%d", &result);
		fclose(fd);
		overall += result;
	}
	printf("Amount of prime numbers: %d\n", overall);
}

int isPrime(long long number) {
	int i;
	if (number % 2 == 0) {
		return 0;
	}
	for (i = 3; i <= number - 1 ; ++i) {
	      if (number % i == 0) {
	    	  return 0;
	      }
	}
	return 1;
}

void parallel_execute(struct handler p) {
	fd_set fds;
	struct timeval tv;
	int retval;

	FD_ZERO(&fds);
	FD_SET(p.read, &fds);

	tv.tv_sec = 5;
	tv.tv_usec = 0;

	retval = select(4000, &fds, NULL, NULL, &tv);
	if (retval == -1) {
		printf("Error! Select error!\n");
		exit(2);
	}

	FILE * fd;
	long long number;
	fd = fdopen(p.read, "r");
	fscanf(fd, "%lld", &number);
	fclose(fd);

	int result;
	result = isPrime(number);

	FD_ZERO(&fds);
	FD_SET(p.write, &fds);

	retval = select(4000, NULL, &fds, NULL, &tv);
	if (retval == -1) {
		printf("Error! Select error!\n");
		exit(2);
	}

	fd = fdopen(p.write, "w");
	fprintf(fd, "%d\n", result);
	fclose(fd);
}

void fork_to_execute(int index, long long number) {
	int fd[2];
	pipe(fd);

	pid_t childpid;
	if((childpid = fork()) == -1) {
		printf("Error! Can't fork!\n");
		exit(1);
	}

	struct handler pipe;
	pipe.read = fd[0];
	pipe.write = fd[1];

	if(childpid == 0) {
		parallel_execute(pipe);
		exit(0);
	} else {
		FILE *fd = fdopen(pipe.write, "w");
		fprintf(fd, "%lld\n", number);
		fclose(fd);

		pipes[index] = pipe;
	}
}

void read_file(char * filename) {
	amount = 0;
	FILE * fd = fopen(filename, "r");
	if (fd == NULL) {
		printf("Error! File don't exist!\n");
		exit(1);
	}
	while (fscanf(fd, "%lld", &numbers[amount]) == 1) {
	    ++amount;
	}
	fclose(fd);
}

void check_all_numbers() {
	int i;
	for (i = 0; i < amount; ++i) {
		fork_to_execute(i, numbers[i]);
	}
}

void wait_all_childs() {
	int count = amount;
	while (count) {
		wait(NULL);
		--count;
	}
}

int main(int argc, char * argv[]) {
	if (argc != 2) {
		printf("Error! You entered wrong parameters!\n");
		return 1;
	}

	read_file(argv[1]);
	check_all_numbers();
	wait_all_childs();
	print_result();

	return 0;
}
