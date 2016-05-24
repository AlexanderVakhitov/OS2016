#include <unistd.h>

#include <fcntl.h>

#include <stdio.h>

#include <stdlib.h>

#include <string.h>



const int END_BYTE = -1;

const unsigned int SPARSE_SIZE = 0x800;

const unsigned int BUFFER_SIZE = 0x800;



int create_sparse(char * f_name, int size) {

	int file_des = open(f_name,  O_WRONLY | O_TRUNC | O_CREAT , S_IRWXU);

	if (file_des == -1) {

		printf("Error! Can't open or create file: %s", f_name);

		return 1;

	}



	write(file_des, "Start sparse file!\n", 19);

	lseek (file_des, size, SEEK_CUR);

	write(file_des, "End sparse file!\n", 17);

	close(file_des);



	return 0;

}



int unzip_sparse(char * f_name) {

	int file_des = open(f_name,  O_WRONLY | O_TRUNC | O_CREAT , S_IRWXU);

	if (file_des == -1) {

		printf("Error! Can't open or create file: %s", f_name);

		return 1;

	}



	int n;

	char buffer[BUFFER_SIZE];

	while ((n = read(0, buffer, BUFFER_SIZE)) != 0) {

		int i;

		int skip = 0;

		for (i = 0; i < n; ++i) {

			if (buffer[i] == 0) {

				++skip;

			} else {

				if (skip != 0) {

					lseek (file_des, skip, SEEK_CUR);

				}

				write(file_des, &buffer[i], 1);

				skip = 0;

			}

		}

		if (skip != 0) {

			lseek (file_des, skip, SEEK_CUR);

		}

	}

	write(file_des, &END_BYTE, 1);

	close(file_des);



	return 0;

}



int main(int argc, char * argv[]) {

	if (strcmp(argv[1], "-c") == 0) {

		if (argc == 3) {

			return create_sparse(argv[2], SPARSE_SIZE);

		}

		if (argc == 4) {

			return create_sparse(argv[2], atoi(argv[3]));

		}

		printf("Error! You entered wrong parameters!");

	}

	if (strcmp(argv[1], "-u") == 0) {

		if (argc == 3) {

			return unzip_sparse(argv[2]);

		}

		printf("Error! You entered wrong parameters!");

	}

	return create_sparse(argv[1], SPARSE_SIZE);

}