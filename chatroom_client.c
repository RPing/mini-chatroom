#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define DEFAULT_SERVER "127.0.0.1"
#define DEFAULT_PORT 8000
#define READ_BUFF_SIZE 1000

#define proc_error(x) \
    do \
    { \
        perror(#x); \
        return -1; \
    }while(0)

int connect_to(char address[], unsigned short port);

int main(int argc, char* argv[])
{
    char* server_ip = DEFAULT_SERVER;
    u_int16_t server_port = DEFAULT_PORT;
    int connect_fd;
    int max_sd;
    fd_set readfds;
    unsigned char read_buffer[READ_BUFF_SIZE];
    int flags;

	if(argc != 3){
        printf("usage: ./chatroom_client <server ip> <server port>\n");
		return 0;
	} else {
		server_port = atoi(argv[2]);
		server_ip = argv[1];
	}

    connect_fd = connect_to(server_ip, server_port);
    if(connect_fd < 0)
        proc_error("connect error");
/*
 * set read fds to non-blocking
 */
    if((flags = fcntl(connect_fd, F_GETFL, 0)) < 0)
        proc_error("get connect_fd flag");
    if(fcntl(connect_fd, F_SETFL, flags|O_NONBLOCK) < 0)
        proc_error("set connect_fd flag");

    if((flags = fcntl(STDIN_FILENO, F_GETFL, 0)) < 0)
        proc_error("get stdin flag");
    if(fcntl(STDIN_FILENO, F_SETFL, flags|O_NONBLOCK) < 0)
        proc_error("set stdin flag");
/*
 * use select to read input from stdin or socket
 */
    while(1)
    {
        int read_size = 0;
        int activity;
        FD_ZERO(&readfds);
        FD_SET(connect_fd, &readfds);
        FD_SET(STDIN_FILENO, &readfds);
        max_sd = connect_fd;
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);
        if ((activity < 0) && (errno!=EINTR))
        {
            shutdown(connect_fd, SHUT_RDWR);
            proc_error("select error");
        }
        if(FD_ISSET(connect_fd, &readfds))
        {/* receive data from server */
            int n = 0;
            ioctl(connect_fd, FIONREAD, &n);
            if(0 == n)
            {/* connection closed by server */
                shutdown(connect_fd, SHUT_RDWR);
                return 0;
            }
            while((read_size = read(connect_fd, read_buffer, sizeof(read_buffer))) > 0)
                write(STDOUT_FILENO, read_buffer, read_size);
        }
        if(FD_ISSET(STDIN_FILENO, &readfds))
        {
            while((read_size = read(STDIN_FILENO, read_buffer, sizeof(read_buffer))) > 0){
				if(strstr(read_buffer, "exit")){
					shutdown(connect_fd, SHUT_RDWR);
					return 0;
				} else {
					write(connect_fd, read_buffer, read_size);
				}
			}
        }
    }
    return 0;
}

int connect_to(char address[], unsigned short port)
{
	struct hostent *hptr;
	char str[100];
	char addr[100];
	char **pptr;

	hptr = gethostbyname(address);
	int sockfd = socket(AF_INET , SOCK_STREAM , 0);
 	struct sockaddr_in server;

    pptr=hptr->h_addr_list;
	inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str));

	server.sin_addr.s_addr = inet_addr( str );
	server.sin_family = AF_INET;
	server.sin_port = htons( port );

	if (connect(sockfd , (struct sockaddr *)&server , sizeof(server)) < 0)
        	proc_error("call to connect");

	return sockfd;
}
