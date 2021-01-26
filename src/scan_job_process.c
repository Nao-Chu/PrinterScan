#include "../inc/scan_job_process.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

void SendData(int sockfd,void* data)
{	
	scan_header* tmp = (scan_header *)data;
	if (IsScanHeader(tmp->Cookie)){
		Messages(tmp->Message,"send");
		PrintStatus(tmp->status);
	}
		
	unsigned char buffer[1024];
    	bzero(buffer, 1024);
    	memcpy(buffer, data, sendLen);
	if ( send(sockfd,buffer,sendLen,0) == -1){
		printf("send msg error:%s(errno:%d)\n",strerror(errno),errno);
		close(sockfd);
		pthread_exit(0);
	} 
}

void * RecvData(int sockfd)
{
	unsigned char* recvbuffer;
	recvbuffer = malloc(1024*sizeof(unsigned char));
	bzero(recvbuffer, 1024);
	if ((recvLen = recv(sockfd, recvbuffer, 1024, 0)) < 0){
		printf("recv msg error:%s(errno:%d)\n",strerror(errno),errno);
		close(sockfd);
		pthread_exit(0);
	} 
	CheckAbnormal(recvbuffer,sockfd);
	return recvbuffer;
}

void CheckAbnormal(unsigned char *buffer,int sockfd)
{
	do{
		if (buffer == NULL) {
			printf("buffer == NULL\n");
			break;
		}
		scan_header *data = (scan_header *)buffer;
		if (!IsScanHeader(data->Cookie))
			break;
		
		Messages(data->Message,"recv");
		StatusProcess(data->status,sockfd);
		
	} while (0);	
}

void ReleaseScanJob(int sockfd)
{		
	SendData(sockfd,ReleaseScanResource());
	RecvData(sockfd);
	printf("scan job end...\n");
	close(sockfd);
	extern pthread_t scan_id;
	if (scan_id > 0){
		pthread_cancel(scan_id);
	}
}

void CancelScan(int sockfd)
{
	extern pthread_t scan_id;
	pthread_cancel(scan_id);
	
	SendData(sockfd,CancelScanJob());
	RecvAbort(sockfd);
	ReleaseScanJob(sockfd);
	printf("cancel success\n");
}

void RecvAbort(int sockfd)
{
	unsigned char* buff;
	buff = malloc(1024*sizeof(unsigned char));
	bzero(buff, 1024);
	int isAbort = 0;
	while (1)
	{
		buff = RecvData(sockfd);
		extern int recvLen;
		int offset = 0;
		while (offset < recvLen)
		{
			scan_header *data = (scan_header*)&(buff[offset]);
			if (!IsScanHeader(data->Cookie)){
				offset += 4;
				continue;
			}
				
			if (Messages(data->Message,NULL) != eAbortscanjob){
				offset += 8;
				continue;
			}

			isAbort = 1;
			break;
		}
		if (isAbort)
			break;
	}
}

void StatusProcess(unsigned char status[4],int sockfd)
{
	char* response = PrintStatus(status);

	if (strcmp(response,"e_Busy") == 0) {
		extern pthread_t scan_id;
                close(sockfd);
		pthread_cancel(scan_id);
        }

	if (strcmp(response,"e_Success") != 0) {
                ReleaseScanJob(sockfd);
        }		
}