#include <sys/socket.h>
#include <netinet/in.h>
#include <assert.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>

#include <pthread.h>

#define MAX_NUM 10000
int len = 0;
int temp = 0;
int pack_num = 0;
int pack_num_temp = 0;

void show_count()
{

	printf("recv total:%d rate:%fM/s\n total_num:%d %d/5s\n", len,
	 (len-temp)*8.0/(5*1024*1024), pack_num, pack_num-pack_num_temp);
	temp = len;
	pack_num_temp = pack_num;
}

int main(int argc, char **argv)
{
	struct itimerval timer_conf={
		{5,0},
		{1,0}
	};
	int listenfd,connfd;
	int length;
	struct sockaddr_in servaddr;
	struct sockaddr cliaddr;
	socklen_t cliaddr_len;

	char sendline[MAX_NUM];

	if(argc != 2)
		printf("usage: test_serv <IP addresss>\n");

	bzero(&servaddr, sizeof(servaddr));

	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(9877);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(listenfd, (struct sockaddr *)&servaddr,sizeof(servaddr));
	listen(listenfd, 5);

	if(errno == EINTR)
		printf("connect is interupted\n");
	if(errno == ETIMEDOUT)
		printf("connect is interupted2\n");
	
	signal(SIGALRM, show_count);
	setitimer(ITIMER_REAL, &timer_conf, NULL);

	connfd = accept(listenfd, &cliaddr, &cliaddr_len);
	printf("connect success\n");
	while(1)
	{

		len += read(connfd, sendline, MAX_NUM);
		pack_num ++;
	//	printf("%s\n", sendline);
	}


	exit(0);
}

