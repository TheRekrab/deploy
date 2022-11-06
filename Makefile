CC=gcc
CFLAGS=-g -Wall -Wno-deprecated-declarations -lssl -lcrypto
CLIENT=deploy_client
SERVER=deploy_server
HDRS=defs.h

all: $(CLIENT) $(SERVER) $(HDRS)

%:%.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f a.out *.o $(CLIENT) $(SERVER)
