#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <sys/types.h>

#include <sys/wait.h>

#include <unistd.h>



#define MAX_PROC 100

#define MAX_ARGC 100

#define MAX_LENGTH 100



typedef struct exec {

   char exec_name[100];

   char *exec_argv[MAX_ARGC];

   int exec_type;

};



int pid_count;

pid_t pid_list[MAX_PROC];



int exec_count;

struct exec exec_list[MAX_PROC];



int read_config(char * conf_name) {

	FILE *fd;

	char tmp_str[100];

	char *addr;



	fd = fopen (conf_name, "r");

	if (fd == NULL) {

		return -1;

	}



	exec_count = 0;

	while (1) {

		addr = fgets(tmp_str, sizeof(tmp_str), fd);

		if (addr == NULL) {

			if (feof (fd) != 0) {

				break;

			} else {

				return -1;

			}

		}



		char *separators = ": \n";

		char *istr;



		istr = strtok (tmp_str, separators);

		strcpy(exec_list[exec_count].exec_name, istr);

		exec_list[exec_count].exec_argv[0] = (char*) malloc(MAX_LENGTH);

		strcpy(exec_list[exec_count].exec_argv[0], istr);



		int argc = 1;

		while (1) {

			istr = strtok (NULL, separators);

			if (strcmp(istr, "wait") == 0) {

			    exec_list[exec_count].exec_argv[argc] = NULL;

				exec_list[exec_count].exec_type = 0;

				break;

			}

			if (strcmp(istr, "respawn") == 0) {

				exec_list[exec_count].exec_argv[argc] = NULL;

				exec_list[exec_count].exec_type = 1;

				break;

			}

			exec_list[exec_count].exec_argv[argc] = (char*) malloc(MAX_LENGTH);

			strcpy(exec_list[exec_count].exec_argv[argc], istr);

			++argc;

		}

		++exec_count;

	}



	if (fclose (fd) == EOF) {

		return -1;

	}



	return 0;

}



void delete_file(char *name, int i) {

	char folder[255];



	char *tmp = strchr(name, '/');

	while (tmp != NULL) {

		name = tmp + 1;

		tmp = strchr(name, '/');

	}

	sprintf(folder, "/tmp/%s%d.pid", name, i);



	remove(folder);

}



void write_at_log(char *name, int i, pid_t cpid) {

	char folder[255];



	char *tmp = strchr(name, '/');

	while (tmp != NULL) {

		name = tmp + 1;

		tmp = strchr(name, '/');

	}

	sprintf(folder, "/tmp/%s%d.pid", name, i);



	FILE *fp = fopen(folder, "w");

	if (fp != NULL) {

		fwrite(&cpid, sizeof(pid_t), 1, fp);

		fclose(fp);

	}

}



int fork_proccess(int i) {

	pid_t cpid = fork();

	switch (cpid) {

		case -1:

			return -1;

			break;

		case 0:

		    cpid = getpid();

		    execv(exec_list[i].exec_name, exec_list[i].exec_argv);

		    exit(0);

		default:

			pid_list[i] = cpid;

			write_at_log(exec_list[i].exec_name, i, pid_list[i]);

		    break;

	}

	return 0;

}



int run_fork_by_config() {

	int i, j;

	pid_t cpid;



	for (i = 0; i < exec_count; ++i) {

		if (fork_proccess(i) != 0) {

			return -1;

		}

		++pid_count;

	}



	while (pid_count) {

	    cpid = waitpid(-1, NULL, 0);

	    for (j = 0; j < exec_count; ++j) {

	        if(pid_list[j] == cpid) {

	            if (exec_list[j].exec_type == 0) {

	            	int k = 0;

	            	while (exec_list[j].exec_argv[k] != NULL) {

	            		free(exec_list[j].exec_argv[k]);

	            		++k;

	            	}

	            	delete_file(exec_list[j].exec_name, j);

	            	pid_list[j] = 0;

	            	--pid_count;

	            } else {

	            	fork_proccess(j);

	            	write_at_log(exec_list[j].exec_name, j, pid_list[j]);

	            }

	        }

		}

	}



	return 0;

}



int run_test(int argc, char * argv[]) {

	printf("Run test program!\n");

	int i;

	for (i = 0; i < argc; ++i) {

		printf("%d) %s\n", i, argv[i]);

		sleep(10);

	}

	printf("Stop test program!\n");

	return 0;

}



int main(int argc, char * argv[]) {

	if (strcmp(argv[1], "-c") == 0) {

		if (argc == 3) {

			if (read_config(argv[2]) == 0) {

				return run_fork_by_config();

			} else {

				printf("Error! Can't read config file!");

			}

		}

		printf("Error! You entered wrong parameters!");

	}

	return run_test(argc, argv);

}