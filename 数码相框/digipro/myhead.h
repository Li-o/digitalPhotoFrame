#ifndef myhead_h
#define myhead_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <jpeglib.h>
#include <jerror.h>
#include<dirent.h>

#include<sys/types.h>
#include<sys/stat.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<errno.h>
#include<unistd.h>
#include<time.h>
#include<syslog.h>

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#include <semaphore.h> 		//包含信号量的函数

#define    FB_DEV  "/dev/fb0"
#define BUFSZ 512




/***************** function declaration ******************/

extern void            usage(char *msg);

extern unsigned short  RGB888toRGB565(unsigned char red,

                                                    unsigned char green, unsigned char blue);

extern int             fb_open(char *fb_device);

extern int             fb_close(int fd);

extern int             fb_stat(int fd, int *width, int *height, int *depth);

extern void            *fb_mmap(int fd, unsigned int screensize);

extern int             fb_munmap(void *start, size_t length);

extern int             fb_pixel(void *fbmem, int width, int height,

                                           int x, int y, unsigned short color);



extern int f_count(void);
extern void *cmd_create(void *arg);

extern void func_mus(char *dirname);
extern void func_mus2(char *fname);
extern void func_mus3(char *p);


extern void  img_search(char *dirname);
extern void func_pic(char *dirname);
extern void frame_prcs(char *fname);
extern void thrd_frame1(unsigned char *fbmem);
extern void thrd_frame2(unsigned char *fbmem);
extern void thrd_frame3(unsigned char *fbmem);
extern void thrd_frame4(unsigned char *fbmem);
extern void thrd_frame5(unsigned char *fbmem);
extern void thrd_frame6(unsigned char *fbmem);
extern char *check_disk(void);


extern unsigned char *buffer[600];
extern pthread_mutex_t my_lock;


extern pthread_mutex_t my_lock2;		//在main3.c中定义的，在这里声明即可
		
extern char cmd;	

extern sem_t bin_sem;



#endif
