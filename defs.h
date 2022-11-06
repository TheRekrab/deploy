#define PORT 1337
#define PROGBAR_LEN 30
#define CHUNKS 1024
#define SETTINGS_FILE ".deploy_settings"

#include <openssl/md5.h>

struct user_data {
	char* server_addr;
	char* filename;
	char* new_filename;
};

struct file_transfer {
	long total_bytes;
	long bytes_done;
	double percent_done;
};

// the hashing function that both programs will use:

long md5hashit(const char* fname) {
	unsigned char c[MD5_DIGEST_LENGTH];
	FILE* inFile = fopen(fname, "rb");
	MD5_CTX mdc;
	int bytes;
	unsigned char data[CHUNKS];

	if (inFile == NULL) {
		return -1;
	}

	MD5_Init(&mdc);
	while ((bytes = fread(data, 1, CHUNKS, inFile)) != 0) {
		MD5_Update(&mdc, data, bytes);
	}
	MD5_Final(c, &mdc);
	fclose(inFile);

	long sum = 0;
	for(int i = 0;i < MD5_DIGEST_LENGTH;i ++) {
		sum += (long int)c[i];
	}
	return sum;
}
