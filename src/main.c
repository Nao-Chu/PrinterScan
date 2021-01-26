#include "../inc/scan_data_process.h"
#include "../inc/scan_job_process.h"
#include "../inc/log.h"

#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <pthread.h>

#define SCANHEADLEN 32

pthread_t scan_id;

int TcpConnect(char ** argv);
int TimeoutConnect(int __fd, __CONST_SOCKADDR_ARG __addr, socklen_t __len);
void *ScanJob(void *argv);

void PrintHelp();
void ReScan(char ** argv);
int sockfd;

int main(int argc,char ** argv)
{
	if (argc != 3){
		printf("user should input: ip port\n");
		exit(0);
	}

	//  创建scanJob线程
	if(pthread_create(&scan_id,NULL,ScanJob,(void *)argv) == -1) {
              fprintf(stderr,"pthread scanJob create error!\n");
        }
	int ch = 0;
	PrintHelp();
	
	while (1)
	{
		scanf("%d",&ch);
		switch (ch)
		{
		case 1:
			PrintHelp();
			break;
		case 2:
			ReScan(argv);
			break;
		case 3:
			CancelScan(sockfd);
			break;
		case 4:
			exit(0);
			break;
		default:
			break;
		}
		ch = 0;
	}
	
	return 0;
}

int  TcpConnect(char ** argv)
{	
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1) {
		printf("create socket error:%s(errno:%d)\n",strerror(errno),errno);
		return -1;
	}

	struct sockaddr_in servaddr;
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET,argv[1],&servaddr.sin_addr);
	
	// 改为非阻塞
	if (fcntl(sockfd,F_SETFL,fcntl(sockfd,F_GETFD) | O_NONBLOCK) == -1) {
		printf("set flags error:%s(errno:%d)\n",strerror(errno),errno);
		return -1;
	}
	
	int ret = TimeoutConnect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
	if (ret == -1){
		printf("connect end\n");
		close(sockfd);
		pthread_exit(0);
	}

	// 改回阻塞状态
	if (fcntl(sockfd,F_SETFL,fcntl(sockfd,F_GETFD) & (~O_NONBLOCK)) == -1) {
		printf("set flags error:%s(errno:%d)\n",strerror(errno),errno);
		return -1;
	}
	return ret;
}

int TimeoutConnect(int sockfd, __CONST_SOCKADDR_ARG __addr, socklen_t __len)
{
	int ret = connect(sockfd,__addr,__len);
	if (ret == 0) {
		if (fcntl(sockfd,F_SETFL,fcntl(sockfd,F_GETFD) | O_NONBLOCK) == -1) {
			printf("set flags error:%s(errno:%d)\n",strerror(errno),errno);
			return -1;
		}
		printf("reconnect success\n");
		return 0;
	} 

	if (errno != EINPROGRESS)
		return -1;
	
	int err = 0;
	int errlen = sizeof(err);
	fd_set fdw;
	FD_ZERO(&fdw);
	FD_SET(sockfd,&fdw);

	struct timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	//  10秒超时
	if (select(sockfd+1,NULL,&fdw,NULL,&timeout) <= 0) {
		printf("connect fail\n");
		return -1;
	}
	
	getsockopt(sockfd,SOL_SOCKET,SO_ERROR,&err,&errlen);
	if (err != 0) {
		printf("err = %d\n",err);
		printf("connect timeout: %s\n", strerror(errno));
		return -1;
	}
	// err =0   代表成功
	printf("connect success!\n");
	return 0;
}

void *ScanJob(void *argv)
{
	TcpConnect((char **)argv);

	unsigned char *buffer;
	bzero(&buffer, sizeof(1024));
	
	SendData(sockfd,LockScanResource());
	RecvData(sockfd);
	
	SendData(sockfd,SetDefaultScanJobSettings());
	RecvData(sockfd);

	SendData(sockfd,GetScanJobSettings());
	buffer = RecvData(sockfd);
	// 保存接收到的数据recvBuffer
	unsigned char recvBuffer[SCANHEADLEN];
	for (int i = 0 ; i < SCANHEADLEN; i++){
		recvBuffer[i] = buffer[i];
	}
	buffer = (recvLen == SCANHEADLEN) ? RecvData(sockfd) : &buffer[SCANHEADLEN];

	SendData(sockfd,SetScanJobSettingsHeader(recvBuffer));
	SendData(sockfd,SetScanJobSettings(buffer));
	RecvData(sockfd);

	SendData(sockfd,StartScanJob());

	FILE *ofp = NULL;
	char path[1024] = "jpg/scan.jpg";
	ofp = fopen(path,"w");
	sendimagedone = 0;
	sendimagenumber = 0;
	do {
		buffer = RecvData(sockfd);
		WriteImageData(buffer,ofp);
	}while (!IsEnd(buffer));
	
	if (buffer != NULL)
		free(buffer);

	if(ofp )
		fclose(ofp);

	ReleaseScanJob(sockfd);
}

void PrintHelp()
{
	printf("==================\n");
	printf("input '1': help\n");
	printf("input '2': reScan\n");
	printf("input '3': cancelScan\n");
	printf("input '4': exit\n");
	printf("==================\n");
}

void ReScan(char **argv)
{
	printf("reScan...\n");
	if(pthread_create(&scan_id,NULL,ScanJob,(void *)argv) == -1) {
              fprintf(stderr,"pthread scanJob create error!\n");
        }
}