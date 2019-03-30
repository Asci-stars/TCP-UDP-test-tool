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
int len = 0, temp = 0;
int pack_num = 0;
int pack_num_temp = 0;
void show_count()
{

	printf("send total:%d rate:%fM/s\n total_num:%d %d/5s\n", len,
	 (len-temp)*8.0/(5*1024*1024), pack_num, pack_num-pack_num_temp);
	temp = len;
	pack_num_temp = pack_num;
}

int main(int argc, char **argv)
{

	if (argc != 4)
	{
		printf("input:program ip_addr package_len send_delay(ms) \n");
	}
	struct itimerval timer_conf={
		{5,0},
		{1,0}
	};
	int connectfd, i, num, delay;
	struct sockaddr_in servaddr;
	char sendline[MAX_NUM];
	for (i = 0; i < MAX_NUM; ++i)
	{
		sendline[i] = 6;
		/* code */
	}
	
	num = atoi(argv[2]);
	delay = atoi(argv[3]);
	printf("package_len:%d, delay:%d\n", num, delay);

	bzero(&servaddr, sizeof(servaddr));

	connectfd = socket(AF_INET, SOCK_STREAM, 0);

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(9877);
	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	signal(SIGALRM, show_count);
	setitimer(ITIMER_REAL, &timer_conf, NULL);


	connect(connectfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
	printf("start send!!!\n");


	while(1)
	{
		usleep(delay*1000);
		len += write(connectfd, sendline, num);
		pack_num ++;
	}
		


	exit(0);
}
