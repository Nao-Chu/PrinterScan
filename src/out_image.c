#include "../inc/out_image.h"
#include "../inc/usb.h"

#include <pthread.h>

void OutImage(libusb_device_handle *dev_handle)
{
	SendData(dev_handle,LockScanResource());
	RecvData(dev_handle);

 	SendData(dev_handle,SetDefaultScanJobSettings());
	RecvData(dev_handle);

	SendData(dev_handle,GetScanJobSettings());
	unsigned char *buffer;
	bzero(&buffer, sizeof(1024));
	buffer = RecvData(dev_handle);
	// 保存接收到的数据recvBuffer
	unsigned char recvBuffer[1024];
	for (int i = 0 ; i < 32; i++){
		recvBuffer[i] = buffer[i];
	}
	buffer = RecvData(dev_handle);

	SendData(dev_handle,SetScanJobSettingsHeader(recvBuffer));
	SendData(dev_handle,SetScanJobSettings(buffer));
	RecvData(dev_handle);

	SendData(dev_handle,StartScanJob());

	FILE *ofp = NULL;
	char path[1024] = "jpg/scan.jpg";
	ofp = fopen(path,"w");
	LogWrite(INFO,"%s %s","open file :",path);
	sendimagedone = 0;
	do {
		buffer = RecvData(dev_handle);
		WriteImageData(buffer,ofp);
	}while (!IsEnd(buffer));

	fclose(ofp);
	LogWrite(INFO,"%s %s","close file :",path);
	
	if (buffer != NULL)
		free(buffer);
	
	SendData(dev_handle,ReleaseScanResource());
	RecvData(dev_handle);
}

void SendData(libusb_device_handle *dev_handle,void* send_data)
{	
	scan_header* tmp = (scan_header *)send_data;
	if (IsScanHeader(tmp->Cookie)){
		Messages(tmp->Message,"send");
		PrintStatus(tmp->status);
	}

	unsigned char ibuf[1024];
    	bzero(ibuf, 1024);
    	memcpy(ibuf, send_data, sendLen);
	int num_bytes;
	 if (libusb_bulk_transfer(dev_handle, 0x02, ibuf, sendLen, &num_bytes, 1000) < 0) {
		 perror("send fail \n");
	 } 
	if (isdebug) {
		for (int i = 0; i < sendLen; i++){
			printf("%-x ",ibuf[i]);
			if ((i + 1) % 16 == 0) 
				printf("\n");
		}
	}
}

void* RecvData(libusb_device_handle *dev_handle)
{
	int num_bytes;
	unsigned char *obuf;
	obuf = malloc(1024*sizeof(unsigned char));
	bzero(obuf,1024);
	if(libusb_bulk_transfer(dev_handle,0x82, obuf, 1024, &recvLen, 0) < 0){
		perror("recv fail\n");
	} 

	CheckAbnormal(obuf,dev_handle);
	if (isdebug) {
		for (int i = 0; i < recvLen; i++){
			printf("%-x ",obuf[i]);
			if ((i + 1) % 16 == 0) 
				printf("\n");
		}
	}
	return obuf;
}

void CheckAbnormal(const unsigned char *buffer,libusb_device_handle *dev_handle)
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
		StatusProcess(data->status);

		if (Messages(data->Message,NULL) != eAbortscanjob){
			offset += 8;
			continue;
		}
		ReleaseScanJob(dev_handle);
	}
}

void ReleaseScanJob(libusb_device_handle *dev_handle)
{		
	SendData(dev_handle,ReleaseScanResource());
	RecvData(dev_handle);
	printf("scan job end...\n");
	LogWrite(INFO,"%s ","scan job end...");
        CloseDevice(dev_handle);
	extern pthread_t scan_id;
	if (scan_id > 0){
		LogWrite(INFO,"%s %d","Thread exited, id :",scan_id);
		pthread_cancel(scan_id);
	}
}

void StatusProcess(unsigned char status[4])
{
	char* response = PrintStatus(status);

	if (strcmp(response,"e_Busy") == 0) {
		LogWrite(ERROR,"%s ",response);
		extern pthread_t scan_id;
		if (scan_id > 0){
			LogWrite(INFO,"%s %d","Thread exited, id :",scan_id);
			pthread_cancel(scan_id);
		}
        }
}

// 判断是否开始发送图片数据
void WriteImageData(unsigned char *buffer,FILE *ofp)
{
	int offset = 0;
        int len = recvLen;
        int stride = 0;
        scan_header* data;
        
        do{
                offset += stride;
                if ((len -= stride) <= 0) break;  
                data = (scan_header*)&(buffer[offset]);
                        
                if (!IsScanHeader(data->Cookie))
                {
                        if (sendimagedone == 0){
                                stride = 4;
                                continue;
                        }
                        
                        if (sendimagedone == 1){
                                if (len < 24) {
                                        sendimagenumber = 6 - (len / 4);
                                        sendimagedone = 4;
                                        stride = len;
                                        continue;
                                }

                                stride = 24;
                                sendimagedone = 2;
                                continue;
                        }

                        if (sendimagedone == 2)
                        {
                                fwrite(&(buffer[offset]),1,len,ofp);
                                break;
                        }

                        if (sendimagedone == 3)
                        {
                                stride = 4;
                                if (--sendimagenumber == 0) 
                                        sendimagedone = 1;
                                continue;
                        }

                        if (sendimagedone == 4)
                        {
                                stride = 4;
                                if (--sendimagenumber == 0) 
                                        sendimagedone = 2;
                                continue;
                        }
                }
                
                stride = 32;

	        if(Messages(data->Message,NULL) == eScanImageData) {
                        sendimagedone = 1;
                        if (len >= 56){
                                stride += 24;
                                sendimagedone = 2;
                        }

                        if (len < stride){
                                sendimagenumber = 8 - (len / 4);
                                stride = len;
                                sendimagedone = 3;
                        }
                }
                
        }while (len >= 0);
}
