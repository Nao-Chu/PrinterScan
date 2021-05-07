#include "../inc/scan_job_process.h"
#include "../inc/log.h"

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
		LogWrite(ERROR,"%s %s","send msg error:",strerror(errno));
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
		LogWrite(ERROR,"%s %s","recv msg error:",strerror(errno));
		close(sockfd);
		pthread_exit(0);
	} 
	CheckAbnormal(recvbuffer,sockfd);
	return recvbuffer;
}

void ReleaseScanJob(int sockfd)
{		
	SendData(sockfd,ReleaseScanResource());
	RecvData(sockfd);
	printf("scan job end...\n");
	LogWrite(INFO,"%s ","scan job end...");
	close(sockfd);
	extern pthread_t scan_id;
	if (scan_id > 0){
		LogWrite(INFO,"%s %d","Thread exited, id :",scan_id);
		pthread_cancel(scan_id);
	}
}

void CancelScan(int sockfd)
{
	LogWrite(INFO,"%s","Cancel Scaning ...");
	SendData(sockfd,CancelScanJob());
}

void CheckAbnormal(const unsigned char *buffer,int sockfd)
{
	int offset = 0;
	scan_header *data;
	while (offset < recvLen) {
		data = (scan_header*)&(buffer[offset]);
		if (!IsScanHeader(data->Cookie)){
			offset += 4;
			continue;
		}
		Messages(data->Message,"recv");
		StatusProcess(data->status,sockfd);

		if (Messages(data->Message,NULL) != eAbortscanjob){
			offset += 8;
			continue;
		}
		ReleaseScanJob(sockfd);
	}
}

void StatusProcess(unsigned char status[4],int sockfd)
{
	char* response = PrintStatus(status);

	if (strcmp(response,"e_Busy") == 0) {
		LogWrite(ERROR,"%s ",response);
		extern pthread_t scan_id;
                close(sockfd);
		LogWrite(INFO,"%s %d","Thread exited, id :",scan_id);
		pthread_cancel(scan_id);
        }

	if (strcmp(response,"e_Success") != 0) {
		LogWrite(ERROR,"%s ",response);
                ReleaseScanJob(sockfd);
        }		
}