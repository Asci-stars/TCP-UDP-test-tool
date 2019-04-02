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
#include <string.h>
#include <unistd.h>


int len, temp;
int pack_num = 0;
int pack_num_temp = 0;
void show_count()
{

	printf("    #send  total:%d rate:%fM/s\n total_num:%d %d/5s\n", len,
	 (len-temp)*8.0/(5*1024*1024), pack_num, pack_num-pack_num_temp);
	temp = len;
	pack_num_temp = pack_num;
}

int main(int argc,char *argv[])
{
		if (argc <4 )
	{
		printf("right input format : program  ip_addr  package_len  send_delay  package_num \n");
	}
	struct itimerval timer_conf={
		{5,0},
		{1,0}
	};
	char seq ;
	int pack_len , delay ,number ,connectfd;
	struct sockaddr_in servaddr;
	
	seq = 1;
	pack_len = atoi(argv[2]);
	delay = atoi(argv[3]);
	char sendline[pack_len];
	if(argc == 5){
		number = atoi(argv[4]);
	}else{
		number = -1;
	}
	if(number != -1 )
	{
	printf("package_len:%d, delay:%d, number:%d\n", pack_len, delay, number);
	}else{
	printf("package_len:%d, delay:%d, number: no restrictions \n", pack_len, delay);
	}
	/* Set the sending address parameter */
	bzero(&servaddr, sizeof(servaddr));
	connectfd = socket(AF_INET, SOCK_DGRAM, 0);
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(9877);
	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	signal(SIGALRM, show_count);
	setitimer(ITIMER_REAL, &timer_conf, NULL);


	printf("start send!!!\n");
	while(1)
	{
	memset(sendline , seq , pack_len);
	usleep(delay*1000);
	if(number == -1){
	len += sendto(connectfd, sendline, pack_len, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
		pack_num ++;
		}else
		{
		if(pack_num >= number){
			break;
		}else{
			len += sendto(connectfd, sendline, pack_len, 0, 
			(struct sockaddr *)&servaddr, sizeof(servaddr));
			pack_num ++;
		}
//		printf("seq = %d pack_num = %d\n",(int)(seq-0) , pack_num);
	}
	seq++;
	if(seq > 120){
		seq = 1;
	}
	}
	show_count();
		exit(0);
}


