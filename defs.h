#define PORT 1337
#define PROGBAR_LEN 30
#define CHUNKS 1024
#define SETTINGS_FILE ".deploy_settings"

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


