#include "../inc/scan_data_process.h"
#include "../inc/log.h"

#include <string.h>
#include <stdlib.h>

int sendimagedone = 0;
int sendimagenumber = 0;
int out = 0;
scan_header init()
{
	memset(&data, 0x0, sizeof(data));
        data.Cookie[0] = 0x41;
	data.Cookie[1] = 0x53;
	data.Cookie[2] = 0x50;
	data.Cookie[3] = 0x01;
	for (int i = 0; i < 4; i++){
		data.Message[i] = 0x00;
		data.Param1[i] = 0x00;
		data.Param2[i] = 0x00;
		data.status[i] = 0x00;
		data.DataLength[i] = 0x00;
		data.Reserved1[i] = 0x00;
		data.Reserved2[i] = 0x00;
	}
        sendLen = sizeof(data);
        return data;
}

void*  LockScanResource()
{
        data = init();
        return &data;
}

void*  SetDefaultScanJobSettings()
{
        data = init();
        data.Message[3] = 0x08;
        return &data;
}

void* GetScanJobSettings()
{
        data = init();
        data.Message[3] = 0x06;
        return &data;
}

void*  SetScanJobSettingsHeader(unsigned char* buffer)
{
        data = init();
        data.Message[3] = 0x07;
        for(int i = 0; i < 4;i++){
                data.DataLength[i] = buffer[i+20];
        }
        return &data;
}

void* SetScanJobSettings(unsigned char* buffer)
{    
        scan_job_settings* data = (scan_job_settings*) buffer;
        data->Resolution[2] = 0x00;
        data->Resolution[3] = 0x4b;
        //data->Resolution[2] = 0x00;
        //data->Resolution[3] = 0x96;
        //data->Resolution[2] = 0x01;
        //data->Resolution[3] = 0x2c;
        data->RemoteScan[0] = 0x01;
        data->Flags[2] = 0x01;
        data->ScanType[3] = 0x01;
        //data->ScanWindow[2] = data->ScanWindow[6] = 0x02;
        //data->ScanWindow[3] = data->ScanWindow[7] = 0x4e;
         sendLen = recvLen;
        return data;
}

void* StartScanJob()
{
        data = init();
        data.Message[3] = 0x02;
        return &data;
}

void* ReleaseScanResource()
{
        data = init();
        data.Message[3] = 0x01;
        return &data;
}

void* CancelScanJob()
{
        data = init();
        data.Message[3] = 0x03;
        return &data;
}

char* PrintStatus(unsigned char status[4])
{
        unsigned char value[8][4] = {{0,0,0,0},{0,0,0,1},{0,0,0,2},{0,0,0,3},{0,0,0,4},{0,0,0,5},{0,0,0,6},{0,0,0,7}};
        unsigned char *name[9] = {"e_Success","e_Fail","e_Busy","e_CmdNotRecognized","e_InvalidArg","e_AdfEmpty","e_AdfMisPick","e_AdfJam","null"};
        
        for (int i = 0 ; i < 8; i++){
                if(charcmp(status,value[i]) == 0){
                        printf("status: %s\n",name[i]);
                        if (i != 0 && out)
                                LogWrite(ERROR,"%s",name[i]);
                        else if (i == 0 && out)
                                LogWrite(INFO,"%s %s","status: ",name[i]);
                        return name[i];
                }
        }
        return name[8];
}

int Messages(unsigned char message[4],char *who)
{
        unsigned char value[16][4] = {{0,0,0,0},{0,0,0,1},{0,0,0,2},{0,0,0,3},{0,0,0,4},{0,0,0,5},{0,0,0,6},{0,0,0,7},{0,0,0,8},{0,0,0,9},{0,0,0,0x0a},{0,0,0,0x0b},{0,0,0,0x0c},{0,0,0,0x0d},{0,0,0,0x0e},{0,0,0,0x0f}};
        unsigned char *name[16] = {"LockScanResource","ReleaseScanResource","StartScanJob","CancelScanJob","Abortscanjob","ScanImageData","GetScanJobSettings",
        "SetScanJobSettings","SetDefaultScanJobSettings","StartJob","StartSheet","StartPage","EndJob","EndSheet","EndPage","AdfIsPaperPresent"};
        for (int i = 0 ; i < 16; i++){
                if(charcmp(message,value[i]) == 0){
                        out = (i != 5);
                        if (who != NULL)
                                printf("%s: %s ... ",who,name[i]);
                        if (who != NULL && out)
                                LogWrite(INFO,"%s: %s",who,name[i]);
                        return i;
                }
        }
        return -1;
}

int charcmp(unsigned char a[4],unsigned char b[4])
{
        for(int i = 0; i < 4; i++){
                if(a[i] != b[i]){
                        return 1;
                }
        }
        return 0;
}

int IsScanHeader(unsigned char cookie[4])
{
        unsigned char value[4] = {0x41,0x53,0x50,0x01};
        if (charcmp(cookie,value) == 0) {
                return 1;
        }
        return 0;
}

// 判断数据是否接受完毕，( scan_header .Message == eEndJob）
int IsEnd(unsigned char * buffer)
{
        extern int recvLen;
	int offset = recvLen -32;
	scan_header *data = (scan_header*)&(buffer[offset]);
	if (!IsScanHeader(data->Cookie))
		return 0;
	
	return Messages(data->Message,NULL) == eEndJob;
}