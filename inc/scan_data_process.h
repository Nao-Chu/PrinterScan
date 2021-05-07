#ifndef  SCANIMAGE_SCAN_DATA_PROCESS_H_
#define SCANIMAGE_SCAN_DATA_PROCESS_H_

#include<stdio.h>

#pragma pack (1)
typedef struct
{
       unsigned char Cookie[4];
       unsigned char  Message[4];
       unsigned char  Param1[4];
       unsigned char  Param2[4];
       unsigned char  status[4];
       unsigned char  DataLength[4];
       unsigned char  Reserved1[4];
       unsigned char  Reserved2[4];
}scan_header;

#pragma pack (1)
typedef struct
{
        unsigned char  Gamma[4];
        unsigned char Brightness[4];
        unsigned char Contrast[4];
        unsigned char Resolution[4];
        unsigned char XYScale[16];
        unsigned char Sharp[4];
        unsigned char  Smooth[4];
        unsigned char BitsPerPixel[4];
        unsigned char reserved1[4];
        unsigned char reserved2[4];
        unsigned char RemoteScan[4];
        unsigned char Flags[4];
        unsigned char DataType[4];
        unsigned char ScanWindow[16];
        unsigned char ScannableArea[16];
        unsigned char ScanType[4];
}scan_job_settings;

enum{
        eLockScanResource,
        eReleaseScanResource,
        eStartScanJob,
        eCancelScanJob,
        eAbortscanjob,
        eScanImageData,
        eGetScanJobSettings,
        eSetScanJobSettings,
        eSetDefaultScanJobSettings,
        eStartJob,
        eStartSheet,
        eStartPage,
        eEndJob,
        eEndSheet,
        eEndPage,
        eAdfIsPaperPresent
};

scan_header data;
scan_header init();
int recvLen;
int sendLen;
int sendimagedone;
int sendimagenumber;
int out;
void* LockScanResource();
void* SetDefaultScanJobSettings();
void* GetScanJobSettings();
void* SetScanJobSettingsHeader(unsigned char* buffer);
void* SetScanJobSettings(unsigned char* buffer);
void* StartScanJob();
void* ReleaseScanResource();
void* CancelScanJob();
char* PrintStatus(unsigned char status[4]);
int Messages(unsigned char message[4],char *who);
int charcmp(unsigned char a[4],unsigned char b[4]);
int IsScanHeader( unsigned char cookie[4]);
int IsEnd(unsigned char * buffer);
void WriteImageData(unsigned char *buffer,FILE *ofp);

#endif