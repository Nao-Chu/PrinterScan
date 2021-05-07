#include "../inc/user.h"
#include "../inc/out_image.h"

#include <stdio.h>
#include <stdlib.h>

void PrintHelp()
{
	printf("==================\n");
	printf("input '1': help\n");
	printf("input '2': ReScan\n");
	printf("input '3': CancelScan\n");
	printf("input '4': setdebug\n");
        printf("input '5': exit\n");
	printf("==================\n");
}

void *ScanJob(void *argv)
{
        scan_id = pthread_self();
        char **pvid = (char **)argv;
        int vid,pid;
        sscanf(pvid[1],"%x",&vid);
        sscanf(pvid[2],"%x",&pid);
        
	printer = UsersInit(vid,pid);
	if (InitDevice(printer) == -1) {
		pthread_cancel(scan_id);
	}

	if (OpenDevice(printer) == -1) {
		pthread_cancel(scan_id);
	}

	if (ClaimInterface(printer) == -1) {
		pthread_cancel(scan_id);
	}

	///OutImage(printer->dev_handle);
	CloseDevice(printer->dev_handle);
}

void UserOper(char **argv)
{
        int ch = 0;
        isdebug = 0;
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
			CancelScan(printer->dev_handle);
			break;
		case 4:
                        SetDebug();
			break;
                case 5:
                        exit(0);
                        break;
		default:
			break;
		}
		ch = 0;
	}
}

void ReScan(char **argv)
{
	LogWrite(INFO,"%s","ReScan...");
	if(pthread_create(&scan_id,NULL,ScanJob,(void *)argv) == -1) {
              fprintf(stderr,"pthread scanJob create error!\n");
        }
}

void CancelScan(libusb_device_handle *dev_handle)
{
	LogWrite(INFO,"%s","Cancel Scaning....");
	SendData(dev_handle,CancelScanJob());
}

void SetDebug()
{
        isdebug = ~isdebug;
        if (isdebug){
                LogWrite(DEBUG,"%s","debuging::::");
                printf("debuging::::\n");
        } else {
		LogWrite(DEBUG,"%s","No Debuging::::");
		printf("No debug::::\n");
	}
                
}