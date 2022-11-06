CC=gcc
CFLAGS=-g -Wall -Wno-deprecated-declarations -lssl -lcrypto
CLIENT=deploy_client
SERVER=deploy_server
BIN=deploy
HDRS=defs.h

all: $(BIN)

$(BIN): $(CLIENT).o $(SERVER).o $(HDRS) $(BIN).o
	$(CC) $(CFLAGS) $(SERVER).o $(CLIENT).o $(BIN).o -o $(BIN)

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f a.out *.o $(CLIENT) $(SERVER) $(BIN)
