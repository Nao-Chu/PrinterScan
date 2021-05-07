#ifndef USBLIB_INC_USB_H_
#define USBLIB_INC_USB_H_
#include <libusb-1.0/libusb.h>

typedef struct 
{
	uint16_t idvendor;
	uint16_t idproduct;
	libusb_device **devs; 
	libusb_device_handle *dev_handle; 
}users;

users* UsersInit(uint16_t idvendor,uint16_t idproduct);
int InitDevice(users *p);
int OpenDevice(users *p);
int ClaimInterface(users *p);
void CloseDevice(libusb_device_handle *hd);
int DeviceSatus(libusb_device_handle *hd);
void printdev(libusb_device *dev);

#endif