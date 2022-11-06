#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <openssl/md5.h>

// socket-related imports:
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netdb.h>

// local imports:
#include "defs.h"



void prog(struct file_transfer*);
int do_client(const struct user_data*); // I use a pointer to save memory
int load_saved_data(struct user_data*);
void save_data(struct user_data*); // Same here

int main(int argc, char** argv) {
	struct user_data u_data;

	int data_result = load_saved_data(&u_data);
	if (0 > data_result && argc == 4) {
		u_data.server_addr = argv[1];
		u_data.filename = argv[2];
		u_data.new_filename = argv[3];
	} else if (0 > data_result) {
		// this will happen if there is no settings file, and also not the proper command syntax.
		printf("Usage: %s  <SERVER IP>  <FILENAME>  <NEW FILENAME>\n", argv[0]);
		return EXIT_FAILURE;
	}
	printf("Server IP:\t%s\n", u_data.server_addr);
	printf("Filename:\t%s\n", u_data.filename);
	printf("New Filename:\t%s\n", u_data.new_filename);

	int result = do_client(&u_data);

	if (result == 0) {
		save_data(&u_data);
		printf("Current settings saved to " SETTINGS_FILE ", and will be used automatically.\n");
	} // I don't want to save any settings that may be faulty.

	free(u_data.server_addr);
	free(u_data.filename);
	free(u_data.new_filename);
	
	if (result != 0) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void prog(struct file_transfer* transfer) {
	int to_do = PROGBAR_LEN * transfer->percent_done;

	printf("[");
	for (int i = 0;i < to_do;i ++) {
		printf("=");
	}
	for (int i = PROGBAR_LEN;i > to_do;i --) {
		printf(" ");
	}
	printf("] Sent: %f megabytes\r", (double)transfer->bytes_done/1000000);
	fflush(stdout);
	if (transfer->percent_done == 1.00) {
		printf("\n");
	}
}

int do_client(const struct user_data* data) {

	const char* server_addr = data->server_addr;
	const char* filename = data->filename;
	const char* new_filename = data->new_filename;
		

	// create server info

	struct sockaddr_in server_info = {0};
	server_info.sin_family = AF_INET;

	// do stuff with ip address:
	if (0 >= inet_pton(AF_INET, server_addr, &server_info.sin_addr)) {
		perror("inet_pton");
		return -1;
	}

	server_info.sin_port = htons(PORT);

	socklen_t server_info_len = sizeof(server_info);


	// create socket
	int sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (0 > sfd) {
		perror("socket");
		return -1;
	}

	// connect
	if (0 > connect(sfd, (struct sockaddr*)&server_info, server_info_len)) {
		perror("connect");
		return -1;
	}

	// send the # of bytes in filename
	int filename_size = strlen(new_filename) + 1; // + one for the null byte terminator. Don't want to forget that!
	send(sfd, &filename_size, sizeof(int), 0);

	// send the (new) filename
	send(sfd, new_filename, filename_size, 0);

	// setup:
	struct stat s;
	int result = stat(filename, &s);

	if (result == -1) {
		perror("stat");
		return -1;
	}

	long size = s.st_size; // note that st_size is already in bytes, so we're okay.
	size_t size_size = sizeof(long);

	// send the # of bytes
	ssize_t bytes_sent = send(sfd, &size, size_size, 0);
	if (0 > bytes_sent) {
		perror("send");
		return -1;
	}

	// send the actual data:
	
	struct file_transfer transfer = {size, 0, 0.00};

	FILE* file = fopen(filename, "rb");
	int this_byte = fgetc(file);
	while (this_byte != EOF) {
		bytes_sent = send(sfd, &this_byte, (ssize_t)sizeof(int), 0);
		if (0 > bytes_sent) {
			perror("send");
			return -1;
		}
		this_byte = fgetc(file);
		transfer.bytes_done++;
		transfer.percent_done = (long double)transfer.bytes_done / (long double)transfer.total_bytes;
		prog(&transfer);
	}
	// once this loop is done, we will be done with the file:
	fclose(file);

	// recieve hash for the file from the server (verification)
	long server_sum;
	recv(sfd, &server_sum, sizeof(long), 0);

	long client_sum = md5hashit(filename);

	bool is_corrupt = (client_sum != server_sum);

	send(sfd, &is_corrupt, sizeof(bool), 0);
	
	// we're finally done with the socket!
	close(sfd);

	if (is_corrupt) {
		printf("NETWORK ERROR: File is corrupt, checksums do not match.");
	}

	return 0;
}

int load_saved_data(struct user_data* u_data) {
	FILE* fptr = fopen(SETTINGS_FILE, "r");

	if (fptr == NULL) {
		return -1;
	}

	// get file size
	struct stat s;
	stat(SETTINGS_FILE, &s);
	long size = s.st_size;

	// get server ip
	char* got_server_addr = malloc(20 * sizeof(char));
	fgets(got_server_addr, 20, fptr);
	got_server_addr[strcspn(got_server_addr, "\n")] = 0x00;

	// get the original filename
	char* got_filename = malloc((size - 20) * sizeof(char));
	fgets(got_filename, size - 20, fptr);
	got_filename[strcspn(got_filename, "\n")] = 0x00;

	// get the new filename
	char* got_new_filename = malloc((size - 20) * sizeof(char));
	fgets(got_new_filename, size - 20, fptr);
	got_new_filename[strcspn(got_new_filename, "\n")] = 0x00;

	u_data->server_addr = got_server_addr;
	u_data->filename = got_filename;
	u_data->new_filename = got_new_filename;

	fclose(fptr);
	return 0;
}

void save_data(struct user_data* u_data) {
	FILE* fptr = fopen(SETTINGS_FILE, "w");
	if (fptr == NULL) {
		perror("fopen");
		return;
	}
	fprintf(fptr, "%s\n%s\n%s\n", u_data->server_addr, u_data->filename, u_data->new_filename);
	fclose(fptr);
}
