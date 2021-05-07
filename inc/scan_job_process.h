#ifndef  SCANIMAGE_SCAN_JOB_PROCESS_H_
#define SCANIMAGE_SCAN_JOB_PROCESS_H_

#include "../inc/scan_data_process.h"

void SendData(int sockfd,void* data);
void * RecvData(int sockfd);
void CheckAbnormal(const unsigned char *buffer,int sockfd);
void StatusProcess(unsigned char status[4],int sockfd);
void ReleaseScanJob(int sockfd);
void CancelScan(int sockfd);

#endif
