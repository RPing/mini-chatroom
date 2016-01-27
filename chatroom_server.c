#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
int alpha(const char *test);
int search(char name[][50], const char *test, int maxi);
ssize_t Writen(int fd, const void *vptr, size_t n);
static ssize_t my_read(int fd, char *ptr);
ssize_t	readlinebuf(void **vptrptr);
ssize_t Readline(int fd, void *vptr, size_t maxlen);
static int read_cnt;
static char *read_ptr;
static char read_buf[1000];
#define MAXLINE 1000
#define proc_error(x) \
    do \
    { \
        perror(#x); \
        return -1; \
    }while(0)

int main(int argc, char **argv)
{
    if (argc != 2) {
        printf("usage: ./chatroom_server <port number>\n");
        return 0;
    }
    int	i, j, k, maxi, maxfd, listenfd, connfd, sockfd;
    int nready, client[FD_SETSIZE];
    char name[FD_SETSIZE][50];
    char str[100];
    ssize_t	n;
    fd_set rset, allset;
    char line[MAXLINE];
    char reg[MAXLINE];
    char name_reg[50];
    char *message, *chatwith;
    char *p;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr, temp;

	memset(line, 0, sizeof(line));

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

	memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons( atoi(argv[1]) );
	bind(listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr));

	listen(listenfd, 20);

	maxfd = listenfd;/* initialize */
	maxi = -1;/* index into client[] array */
	for (i = 0; i < FD_SETSIZE; i++){
		client[i] = -1;/* -1 indicates available entry */
		strcpy(name[i], "anonymous");
	}
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);
	for (;;) {
        rset = allset;          /* structure assignment */
        nready = select(maxfd+1, &rset, NULL, NULL, NULL);
        if (FD_ISSET(listenfd, &rset)) {        /* new client connection */
            clilen = sizeof(cliaddr);
            connfd = accept(listenfd, NULL, NULL);
#ifdef  NOTDEF
            printf("new client: %s, port %d\n",
                            inet_ntop(AF_INET, &cliaddr.sin_addr, 4, NULL),
                            ntohs(cliaddr.sin_port));
#endif
            for (i = 0; i < FD_SETSIZE; i++){
                if (client[i] < 0) {
                    client[i] = connfd;     /* save descriptor */
                    break;
                }
			}
            if (i == FD_SETSIZE)
                proc_error("too many clients");
			FD_SET(connfd, &allset);        /* add new descriptor to set */
            if (connfd > maxfd)
                maxfd = connfd;                 /* for select */
            if (i > maxi)
                maxi = i;                               /* max index in client[] array */
            socklen_t len = sizeof(temp);
			getpeername(client[i], (struct sockaddr *) &temp, &len);
            sprintf(line, "[Server] Hello, anonymous! From: %s/%d\n", inet_ntop(AF_INET, &temp.sin_addr, str, sizeof(str)), ntohs(temp.sin_port));
            Writen(client[i], line, sizeof(line));
            memset(line, 0, sizeof(line));

			for (i = 0; i <= maxi; i++) {
				if(client[i] == connfd || client[i] < 0)
					continue;
				sprintf(line, "[Server] Someone is coming!\n");
            	Writen(client[i], line, sizeof(line));
            	memset(line, 0, sizeof(line));
			}
            if (--nready <= 0)
                continue;                               /* no more readable descriptors */
        }
		for (i = 0; i <= maxi; i++) {   /* check all clients for data */
            if ( (sockfd = client[i]) < 0)
                    continue;
            if (FD_ISSET(sockfd, &rset)) {
                if ( (n = Readline(sockfd, line, MAXLINE)) == 0) {
                    /* connection closed by client */
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                    sprintf(line, "[Server] %s is offline.\n", name[i]);
                    for (j = 0; j <= maxi; j++) {
						if(client[j] == sockfd || client[j] < 0)
							continue;
                    	Writen(client[j], line, sizeof(line));
					}
					memset(line, 0, sizeof(line));
					strcpy(name[i], "anonymous");
                } else {
					p = strtok(line, " \n");
					if(!p){
						sprintf(line, "[Server] ERROR: Error command.\n");
                    	Writen(sockfd, line, sizeof(line));
                    	memset(line, 0, sizeof(line));
					}
					else if(!strcmp(p, "yell")){
						message = strtok(NULL, "\n");
						if(!message){
							sprintf(line, "[Server] ERROR: Error command.\n");
                    		Writen(sockfd, line, sizeof(line));
                    		memset(line, 0, sizeof(line));
                    		if (--nready <= 0)
                    			break;
                    		else
                    			continue;
						}
						strcpy(reg, message);
						sprintf(line, "[Server] %s yell %s\n", name[i], reg);
						for (k = 0; k <= maxi; k++) {
							if(client[k] < 0)
								continue;
	                        Writen(client[k], line, sizeof(line));
						}
						memset(line, 0, sizeof(line));
					} else if(!strcmp(p, "name")){
						message = strtok(NULL, "\n");
						if(!message){
							sprintf(line, "[Server] ERROR: Error command.\n");
                    		Writen(sockfd, line, sizeof(line));
                    		memset(line, 0, sizeof(line));
                    		if (--nready <= 0)
                    			break;
                    		else
                    			continue;
						}
						strcpy(reg, message);
						if(!strcmp(reg, "anonymous")){
							sprintf(line, "[Server] ERROR: Username cannot be anonymous.\n");
							Writen(sockfd, line, sizeof(line));
						} else if(strlen(reg)<2 || strlen(reg)>12){
							sprintf(line, "[Server] ERROR: Username can only consists of 2~12 English letters.\n");
							Writen(sockfd, line, sizeof(line));
						} else if(!alpha(reg)){
							sprintf(line, "[Server] ERROR: Username can only consists of 2~12 English letters.\n");
							Writen(sockfd, line, sizeof(line));
						} else if( search(name, reg, maxi) >= 0 && search(name, reg, maxi) != i){
							sprintf(line, "[Server] ERROR: %s has been used by others.\n", reg);
							Writen(sockfd, line, sizeof(line));
						} else {
							for (j = 0; j <= maxi; j++) {
								if(client[j] < 0){
									continue;
								} else if(client[j] == sockfd) {
									sprintf(line, "[Server] You're now known as %s.\n", reg);
									Writen(client[j], line, sizeof(line));
									memset(line, 0, sizeof(line));
								} else {
									sprintf(line, "[Server] %s is now known as %s.\n", name[i], reg);
									Writen(client[j], line, sizeof(line));
									memset(line, 0, sizeof(line));
								}
							}
							strcpy(name[i], reg);
						}
						memset(line, 0, sizeof(line));
					} else if(!strcmp(p, "tell")){
						chatwith = strtok(NULL, " ");
						message = strtok(NULL, "\n");
						if(!message || !chatwith){
							sprintf(line, "[Server] ERROR: Error command.\n");
                    		Writen(sockfd, line, sizeof(line));
                    		memset(line, 0, sizeof(line));
                    		if (--nready <= 0)
                    			break;
                    		else
                    			continue;
						}
						strcpy(name_reg, chatwith);
						strcpy(reg, message);

						if(!strcmp(name[i], "anonymous")){
							sprintf(line, "[Server] ERROR: You are anonymous.\n");
							Writen(sockfd, line, sizeof(line));
						} else if(!strcmp(chatwith, "anonymous")){
							sprintf(line, "[Server] ERROR: The client to which you sent is anonymous.\n");
							Writen(sockfd, line, sizeof(line));
						} else if( search(name, name_reg, maxi) < 0){
							sprintf(line, "[Server] ERROR: The receiver doesn't exist.\n");
							Writen(sockfd, line, sizeof(line));
						} else {
							sprintf(line, "[Server] %s tell you %s\n", name[i], reg);
							Writen(client[search(name, name_reg, maxi)], line, sizeof(line));
							memset(line, 0, sizeof(line));

							sprintf(line, "[Server] SUCCESS: Your message has been sent.\n");
							Writen(sockfd, line, sizeof(line));
						}
						memset(line, 0, sizeof(line));
					} else if(!strcmp(p, "who")){
						for (j = 0; j <= maxi; j++) {
							if(client[j] < 0){
								continue;
							} else if(client[j] == sockfd) {
								socklen_t len = sizeof(temp);
								getpeername(client[j], (struct sockaddr *) &temp, &len);
        						sprintf(line, "[Server] %s %s/%d ->me\n", name[i], inet_ntop(AF_INET, &temp.sin_addr, str, sizeof(str)), ntohs(temp.sin_port));
        						Writen(sockfd, line, sizeof(line));
        						memset(line, 0, sizeof(line));
							} else {
								socklen_t len = sizeof(temp);
								getpeername(client[j], (struct sockaddr *) &temp, &len);
        						sprintf(line, "[Server] %s %s/%d\n", name[j], inet_ntop(AF_INET, &temp.sin_addr, str, sizeof(str)), ntohs(temp.sin_port));
        						Writen(sockfd, line, sizeof(line));
        						memset(line, 0, sizeof(line));
							}
						}
					} else {
						sprintf(line, "[Server] ERROR: Error command.\n");
                    	Writen(sockfd, line, sizeof(line));
                    	memset(line, 0, sizeof(line));
					}
				}
                if (--nready <= 0)
                    break;                          /* no more readable descriptors */
            }
        }
    }
}

int search(char name[][50], const char *test, int maxi)
{
	int i;
	for(i = 0; i <= maxi; i++){
		if(!strcmp(name[i], test))
			return i;
	}
	return -1;
}
int alpha(const char *test) //return true or false
{
	int i;
	for(i = 0;i < strlen(test);i++){
		if(!isalpha(test[i]))
			return 0;
	}
	return 1;
}
ssize_t Writen(int fd, const void *vptr, size_t n)
{
	size_t nleft;
	ssize_t nwritten;
	const char *ptr;

	ptr = vptr;
	nleft = n;
	while(nleft > 0){
		if( (nwritten = write(fd, ptr, nleft)) <= 0){
			if(nwritten < 0 && errno == EINTR)
				nwritten = 0;
			else
				return(-1);
		}
		nleft -= nwritten;
		ptr += nwritten;
	}
	return (n);
}
static ssize_t
my_read(int fd, char *ptr)
{
    if (read_cnt <= 0) {
      again:
        if ( (read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
            if (errno == EINTR)
                goto again;
            return (-1);
        } else if (read_cnt == 0)
            return (0);
        read_ptr = read_buf;
    }

    read_cnt--;
    *ptr = *read_ptr++;
    return (1);
}
ssize_t
Readline(int fd, void *vptr, size_t maxlen)
{
    ssize_t n, rc;
    char    c, *ptr;

    ptr = vptr;
    for (n = 1; n < maxlen; n++) {
        if ( (rc = my_read(fd, &c)) == 1) {
            *ptr++ = c;
            if (c  == '\n')
                break;          /* newline is stored, like fgets() */
        } else if (rc == 0) {
            *ptr = 0;
            return (n - 1);     /* EOF, n - 1 bytes were read */
        } else
            return (-1);        /* error, errno set by read() */
    }

    *ptr  = 0;                  /* null terminate like fgets() */
    return (n);
}
ssize_t
readlinebuf(void **vptrptr)
{
    if (read_cnt)
        *vptrptr = read_ptr;
    return (read_cnt);
}
