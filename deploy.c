#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>

#include "defs.h"

int server_main(int, char**);
int client_main(int, char**);
long md5hashit(const char*);


int main(int argc, char** argv) {
	if (argc == 2) {
		if (strcmp(argv[1], "server") == 0) {
			return server_main(argc, argv);
		} else if (strcmp(argv[1], "-h") * strcmp(argv[1], "--help") == 0) {
			printf("Usage: %s <SERVER IP>  <FILENAME>  <NEW FILENAME>\n", argv[0]);
			printf("\nA program to deploy code to another computer easily.\nOn the server, run:\n");
			printf("%s server\n", argv[0]);
			printf("That will listen for any incoming connections.\nTo deploy code, simply give values for all of the parameters:\n");
			printf("  SERVER IP\tThe IP address that the server is listening on.\n");
			printf("  FILENAME\tThe name of the file that you want to deploy to the server\n");
			printf("  NEW FILENAME\tThe name that you want the file to be when it is on the server\n");
			printf("\nOnce you have supplied values once and ran the program, the values will be automatically saved, and you will only have to type:\n");
			printf("%s\n", argv[0]);
		}
	}
	return client_main(argc, argv);
}
