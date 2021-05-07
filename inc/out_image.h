#ifndef _USBLIB_INC_OUT_IMAGE_H_
#define _USBLIB_INC_OUT_IMAGE_H_
#include <libusb-1.0/libusb.h>
#include "../inc/scan_data_process.h"
#include "../inc/log.h"

void OutImage();
void SendData(libusb_device_handle *dev_handle,void* send_data);
void* RecvData(libusb_device_handle *dev_handle);
void CheckAbnormal(const unsigned char *buffer,libusb_device_handle *dev_handle);
void StatusProcess(unsigned char status[4]);
void ReleaseScanJob(libusb_device_handle *dev_handle);
#endif