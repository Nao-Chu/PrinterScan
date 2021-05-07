#include "../inc/usb.h"

#include <stdio.h>
#include <stdlib.h>

users* UsersInit(uint16_t idvendor,uint16_t idproduct)
{
        users *init;
        init = malloc(sizeof(users *));
        init->idvendor = idvendor;
        init->idproduct = idproduct;
        init->devs = malloc(sizeof(libusb_device **));
        init->dev_handle = malloc(sizeof(libusb_device_handle *));
        return init;
}

int InitDevice(users *p)
{
        if (libusb_init(NULL) < 0) {
		perror("Init Error\n"); 
		return -1;
	}
}

int OpenDevice(users *p)
{
        libusb_set_debug(NULL, LIBUSB_LOG_LEVEL_INFO); 
 
        ssize_t cnt; //holding number of devices in list
	if ((cnt = libusb_get_device_list(NULL, &p->devs)) < 0) {
		perror("Get Device Error\n"); //there was an error
		return -1;
	}
	for (int i = 0; i < cnt; i++) {
		printdev(p->devs[i]);
	}
	printf("%ld Devices in list.\n", cnt);
 
	p->dev_handle = libusb_open_device_with_vid_pid(NULL, p->idvendor, p->idproduct);
	if(p->dev_handle == NULL)
		perror("Cannot open device");
	
	printf("Device Open success\n");
        
        return (p->dev_handle != NULL) - 1;
}

int ClaimInterface(users *p)
{
        libusb_free_device_list(p->devs, 1); 
	
	if(libusb_kernel_driver_active(p->dev_handle, 0) == 1) { 
		printf("Kernel Driver Active\n");
		if(libusb_detach_kernel_driver(p->dev_handle, 0) == 0) 
			printf("Kernel Driver Detached!\n");
	}

	if (libusb_claim_interface(p->dev_handle, 0) < 0) {
		perror("Cannot Claim Interface\n");
		return -1;
	}

	printf("Claimed Interface\n");
}

void CloseDevice(libusb_device_handle *dev_handle)
{
        libusb_close(dev_handle); 
	libusb_exit(NULL);
}

void printdev(libusb_device *dev)
{
	struct libusb_device_descriptor desc;
	int r = libusb_get_device_descriptor(dev, &desc);
	if(r <0){
		printf("failed to get device descriptor\n");
		return;
	}
	
	if (desc.idVendor != 0x232c && desc.idProduct != 0x5f20) {
		return;
	}
	printf("VendorID: %x\n",desc.idVendor);
	printf("ProductID: %x\n",desc.idProduct);
	struct libusb_config_descriptor *config;
	libusb_get_config_descriptor(dev, 0, &config);
	printf("Interfaces: %d\n",(int)config->bNumInterfaces);
	const struct libusb_interface *inter;
	const struct libusb_interface_descriptor *interdesc;
	const struct libusb_endpoint_descriptor *epdesc;

	for(int i=0; i<(int)config->bNumInterfaces; i++){
		inter =&config->interface[i];
		printf("Number of alternate settings: %d\n",inter->num_altsetting);
		for(int j=0; j<inter->num_altsetting; j++){
			interdesc =&inter->altsetting[j];
			printf("Interface Number: %d\n",(int)interdesc->bInterfaceNumber);
			printf("Number of endpoints: %d\n",(int)interdesc->bNumEndpoints);

			for(int k=0; k<(int)interdesc->bNumEndpoints; k++){
				epdesc =&interdesc->endpoint[k];
				printf("Descriptor Type: %d\n",(int)epdesc->bDescriptorType);
				printf("EP Address: %x\n",(int)epdesc->bEndpointAddress);
			}
		}
	}
	libusb_free_config_descriptor(config);
}