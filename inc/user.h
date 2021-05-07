#ifndef  _USBLIB_INC_USER_H_s
#define _USBLIB_INC_USER_H_s

#include "../inc/usb.h"
#include <pthread.h>


void PrintHelp();
void *ScanJob(void *argv);
void UserOper();
void ReScan(char **argv);
void CancelScan(libusb_device_handle *dev_handle);
void SetDebug();
users *printer;
pthread_t scan_id;

#endif