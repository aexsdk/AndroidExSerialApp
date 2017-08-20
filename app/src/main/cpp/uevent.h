#ifndef __HUEVENT__
#define __HUEVENT__

typedef long                LONG;
typedef unsigned short      WORD;
typedef unsigned long       DWORD;

typedef enum udev_event{
	UE_START 		= 0x10000
	,UE_ERROR 		= 0x10001
	,UE_STATUS		= 0x10201
	,UE_READ_END	= 0x10202
	,UE_EVENT		= 0x0
}PRINT_EVENT;

typedef struct uevent_data{
	int fd;		//句柄
	char devpath[200];
	char subsystem[100];
}UEVENT_DATA,*UEVENT_HANDLE;


typedef void* HKKP;

typedef int (*ON_UEVENT_EVENT)(HKKP env,HKKP obj,int code,char *msg);

int kkuevent_event(HKKP env,HKKP obj,int code,char *pszFormat,...);

void kkuevent_set_event(ON_UEVENT_EVENT oke);

UEVENT_HANDLE kkuevent_open(HKKP env,HKKP obj,char* arg);
int kkuevent_read(UEVENT_HANDLE uevent,HKKP env,HKKP obj,char* arg);
int kkuevent_close(UEVENT_HANDLE uevent,HKKP env,HKKP obj); //程序退出时调用
int kkuevent_gpio_ctrl(UEVENT_HANDLE uevent,HKKP env,HKKP obj,int mask,int value);
int kkuevent_gpio_read(UEVENT_HANDLE uevent,HKKP env,HKKP obj,int mask);

#endif
