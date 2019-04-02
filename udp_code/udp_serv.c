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
#include <string.h>
#include <pthread.h>

#define MAX_NUM 10000
#define MAX_NODE 32
int len[MAX_NODE];
int temp[MAX_NODE];
int pack_num[MAX_NODE];
int pack_num_temp[MAX_NODE];
int repeat_num[MAX_NODE];
int lost_num[MAX_NODE];
int lost_num_temp[MAX_NODE];
int error_packet[MAX_NODE];
int late[MAX_NODE];

void show_count()
{
int i = 0;
	for(; i < MAX_NODE; i++){
		if (len[i]!=0){
printf(" #recv node[%d] total byte:%d rate:%fM/s total_num:%d  %d/5s  repeat_num:%d , lost_num:%d  %d/5s error packet:%d  late:%d\n",i, len[i],
		 (len[i]-temp[i])*8.0/(5*1024*1024), pack_num[i], pack_num[i]-pack_num_temp[i],
		repeat_num[i], lost_num[i],lost_num[i]-lost_num_temp[i],error_packet[i],late[i]);
	
			temp[i] = len[i];
			pack_num_temp[i] = pack_num[i];
			lost_num_temp[i] = lost_num[i];
		}
	}
}

int determin_id(char *address,int n)
{	
	int id;
	const char *tem=".";
	char *s=strtok(address,tem);
	s=strtok(NULL,tem);
	s=strtok(NULL,tem);
	if(strlen(s) == 2)
		{
	id=10*((int)(s[0]-48))+(s[1]-48);
		}
	else if(strlen(s) == 1)
		{id=(int)(s[0]-48);}
	else{
	id=100*((int)(s[0]-48))+10*(s[1]-48)+(s[2]-48);
	printf("#error,the id of node is bigger than 32!!!\n");
	}
	if(id == 0)
	{
		s=strtok(NULL,tem);
		if(strlen(s) == 2)
		{
	id=10*((int)(s[0]-48))+ (s[1]-48);
		}
	else if(strlen(s) == 1)
		{id=(int)(s[0]-48);}
	else{
	id=100*((int)(s[0]-48))+10*(s[1]-48)+(s[2]-48);
	printf("#error,the id of node is bigger than 32!!!\n");
	}
	}
	return id;
}

int main(int argc , char *argv[])
{
	int sockfd,length_temp=0 ,str_len ,id ,ran,old,differ;
	int emap=0,past=0;
	char *ip_address;
struct itimerval timer_conf={
	{5,0},
	{1,0}
	};
struct sockaddr_in servaddr;
struct sockaddr_in cliaddr;
int addr_len = sizeof(struct sockaddr_in);
	char sendline[MAX_NUM];
	char seq[MAX_NODE];   /* expect to receive packet */
	int i = 0;
	for(; i < MAX_NODE; i++){
		seq[i] = 1;  
		pack_num[i] = 0;
		pack_num_temp[i] = 0;
		len[i] = 0;
		temp[i] = 0;
		repeat_num[i] = 0;
		lost_num[i] = 0;
		error_packet[i]=0;
		lost_num_temp[i]=0;
		late[i] = 0;
		}
		/* Set the receiving address parameter */
		bzero(&servaddr, sizeof(servaddr));
		sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(9877);
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		bind(sockfd, (struct sockaddr *)&servaddr,sizeof(servaddr));
	
		if(errno == EINTR)
			printf("			###connect is interupted\n");
		if(errno == ETIMEDOUT)
			printf("			###connect is interupted2\n");
		
		signal(SIGALRM, show_count);
		setitimer(ITIMER_REAL, &timer_conf, NULL);

while(1)
	{
	int i,j;
	length_temp = recvfrom(sockfd, sendline, MAX_NUM, 0, (struct sockaddr *)&cliaddr, &addr_len);
	ip_address = inet_ntoa(cliaddr.sin_addr);
	str_len = sizeof(ip_address);
	id = determin_id(ip_address , str_len);
	len[id] += length_temp;
	
	/* determine the num of error_pacoket */
	if(length_temp >= 100){
	int x=0;
	for(;x<length_temp/100;x++)
		{
		srand(time(0));
		ran= rand() % 100;
	if(sendline[x*100+ran]!=sendline[0])
		break;
	}
	if(x!=length_temp/100)
		{
		printf("#error_packet packet=%d length_temp=%d real_line=%d\n",*sendline,length_temp,x);
	error_packet[id]++;
	}
	}
	else
	{
	int x=0;
	for(;x<length_temp/10;x++)
	{
	srand(time(0));
	ran= rand() % 10;
	if(sendline[x*10+ran]!=sendline[0])
		break;
	}
	if(x!=length_temp/10)
	{
	printf("#error_packet packet=%d length_temp=%d real_line=%d\n",*sendline,length_temp,x);
	error_packet[id]++;
	}}


	if( seq[id] == *sendline)  //若头部数据在传输时被修改，将会导致统计错误
	{
	int y = 1;
	for(;y <= 5; y++)
	{
	if(!(emap & (1 << (y-1))))
		break;
	}
	old =seq[id];
	seq[id] = seq[id] + y;
	//更新past
	past <<= y;
	int i =1;
	for(;i<=y;i++)
	{
	past |= (1<<(i - 1));
	}
	past &= 31;
	emap >>= y;   //更新emap
	}
	else{
	if((*sendline > seq[id])&&(((*sendline - seq[id]) <= 5)))
		{
		differ = *sendline - seq[id];
		if(emap & (1<<(differ - 1)))
			repeat_num[id]++;
		else
			emap |= (1<<(differ -1));
		}
	else if((*sendline > seq[id])&&(((*sendline - seq[id]) <= 10)))
		{
		int tmp =*sendline - 5;
		differ = tmp -seq[id];
		for(;differ <= 5; differ++)
			{if(!(emap & (1<<(differ-1))))
				break;
			}
		seq[id] = seq[id] + differ;
			//更新past
			past <<= differ;
		int i = 1;
		for(;i<=(differ-1);i++)
			{
			if(!(emap & (1<<(i-1))))
				lost_num[id]++;
			else
				past |= (1<<(differ -i -1));
			}
		past &= 31;
		lost_num[id]++;
		 //更新emap
		 emap >>= differ;
		 int x= *sendline -seq[id];
		 emap |= (1<<(x-1));
		}
	else if((*sendline > seq[id])&&(((*sendline - seq[id]) <=100)))
		{
		old = seq[id];
		seq[id] = *sendline -5;
		differ = seq[id] - old;
		//更新past
		past <<= differ;
		int i =1;
		for(;i <= differ;i++)
			{
			if(((seq[id]-i) <= (old +5))&&((seq[id]-i)>old) )
				{
				if(emap &(1<<(seq[id] -i -old -1)))
					{if(i<=20)
						past |= (1<<(i-1));
					}
				else
				lost_num[id]++;
				}
			else
				lost_num[id]++;
			}
		past &= 31;
		 //更新emap
		emap >>= differ;
		int x= *sendline -seq[id];
		emap |= (1<<(x-1));
		}
	else if((*sendline > seq[id])&&(((*sendline - seq[id]) >100)))
		{
		differ = (seq[id] -*sendline +120)%120;
		if(past & (1<<(differ-1)))
			repeat_num[id]++;
		else
			{
			late[id]++;
			past |= (1<<(differ-1));
			}
		past &= 31;
		}
	else if((*sendline < seq[id])&&((seq[id]-*sendline)<=5))
		{
		if(past & (1<<(seq[id]-*sendline -1)))
			repeat_num[id]++;
		else
			{
			late[id]++;
			past |= (1<<(seq[id]-*sendline -1));
			}
		past &= 31;
		}
	else if((*sendline < seq[id])&&((seq[id] - *sendline) > 100))
		{
		differ = (*sendline - seq[id] + 120) % 120;
		if(emap & (1<<(differ - 1)))
			repeat_num[id]++;
		else
			emap |= (1<<(differ -1));
		}
	else{
	late[id]++;}
	}
	if(emap >= 16)
	{
	int y = 1;
	for(;y <= 5; y++)
	{
	if(!(emap & (1 << (y-1))))
		break;
	}
	old = seq[id];
	seq[id] = seq[id] + y;
	//更新past
	past <<= y;
	int i = 1;
	for(; i <= (y-1) ;i++)
		{
		if(emap & (1<<(seq[id]-i-old-1)))
			past |= (1<<(i-1));
		else
			lost_num[id]++;
		}
	lost_num[id]++;
	past &= 31;
	emap >>= y;
	}
	if(seq[id]>120)
		{
		seq[id] = seq[id] - 120;
		}	
	past &= 31;
	emap &= 31;
	pack_num[id]++;
		}
show_count();
	exit(0);
}


