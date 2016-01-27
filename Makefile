CFLAGS = -O2
C = gcc $(CFLAGS)

all: chatroom_client.c chatroom_server.c
	$(C) chatroom_client.c -o chatroom_client
	$(C) chatroom_server.c -o chatroom_server

clean:
	rm -f chatroom_client chatroom_server
