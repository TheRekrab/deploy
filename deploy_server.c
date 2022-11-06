#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

// socket-related imports:
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

// local imports:
#include "defs.h"

int do_server(void);
void load_saved_data(struct user_data*);

int main(int argc, char** argv) {
	
	int pid = fork();
	// I fork because with fork() I can run in the background and be unobtrusive.

	if (pid == 0) {
		// we're the child!
		do_server();
	} else {
		// we're the parent!
		printf("[ SERVER ] Server is active and listening at localhost on port %d. PID is %d\n", PORT, pid);
	}

	return EXIT_SUCCESS;
}

int do_server(void) {
	// create socket info struct
	struct sockaddr_in server_info = {0};	

	server_info.sin_family = AF_INET;
	server_info.sin_port = htons(PORT);

	socklen_t server_info_len = sizeof(server_info);

	struct sockaddr client_info = {0};
	socklen_t client_info_len = sizeof(client_info);

	// create socket
	int sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd < 0) {
		perror("deploy server : socket");
		return -1;
	}

	// bind
	if (0 > bind(sfd, (struct sockaddr*)&server_info, server_info_len)) {
		perror("deploy server : bind");
		return -1;
	}

	// listen
	if (0 > listen(sfd, 0)) {
		perror("deploy server : listen");
		return -1;
	}


	// accept
	int cfd = accept(sfd, &client_info, &client_info_len);
	if (0 > cfd) {
		perror("deploy server : accept");
		return -1;
	}
	
	// get # of bytes in filename
	int filename_size;
	recv(cfd, &filename_size, sizeof(int), 0);

	// get filename
	char* filename = malloc(filename_size * sizeof(char)); // TODO: Don't forget to free the 'filename', b/c I used malloc (heap not stack)
	recv(cfd, filename, filename_size*sizeof(char), 0);

	// get # of bytes:
	long file_size;
	recv(cfd, &file_size, sizeof(long), 0);

	// open the file
	FILE* newfile = fopen(filename, "wb");
	
	for (long i = 0;i<file_size;i++) {
		// recieve the byte:
		int byte;
		recv(cfd, &byte, sizeof(int), 0);
		fputc(byte, newfile);
	}

	// close the file
	fclose(newfile);

	// verification time!
	long sum = md5hashit(filename);
	send(cfd, &sum, sizeof(long), 0);

	bool is_corrupt;
	recv(cfd, &is_corrupt, sizeof(bool), 0);

	if (is_corrupt) {
		remove(filename); // the transaction did not go well, the file is wrong.
	}
	
	// close the cfd.
	close(cfd);

	// free filename
	free(filename);

	return 0;

}

