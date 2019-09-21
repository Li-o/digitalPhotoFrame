

/**********************头文件*********************************/
#include "myhead.h"



////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/*************************全局变量定义******************************************/

unsigned char *buffer[600];		//600个指针构成的指针数组，buffer[i]（指针）指向一行像素数据
pthread_mutex_t my_lock = PTHREAD_MUTEX_INITIALIZER;		//定义互斥锁my_lock，用来保护buffer[i]缓冲区，留个进程都要同步访问缓冲区


int img_count = 0;		//统计图片文件数量的变量，应为函数要递归，所以要设为全局的
char *fname_array[BUFSZ];		//指针数组，每个元素指向一个图像文件的全名






////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/*****************监测u盘的函数，并把U盘挂载到制定的路径/mnt/usb,挂载成功，返回这个路径*************************/

char *check_disk(void)
{
	char *fname2 = NULL;

	DIR *dp;
	struct dirent *entry;

	while(1)
	{
		if((dp=opendir("/dev"))==NULL)	//打开/dev目录
		{
			perror("err: fail to opendir\n");
			return NULL;
		}
		
		while((entry=readdir(dp)) != NULL)	//循环读取目录下的文件
		{
			if(strcmp("sdb1",entry->d_name)==0)		//找到/dev/sdb1文件，即u盘设备文件
			{
					//已监测到U盘，挂载U盘到/mnt/usb
					//system("sudo umount /dev/sdb1");		
					system("sudo modprobe usb-storage");
					system("sudo mkdir /mnt/my_usb");
					system("sudo mount /dev/sdb1 /mnt/my_usb");
			}
			else 
			{
				continue;
			}
	
			fname2 = "/mnt/my_usb";		//把挂载点保存
			
			break;		//跳出内层循环
		}
		
		closedir(dp);		//循环内部一定要关闭打开的文件，因为系统能打开的文件夹数量有限
		
 		if(NULL == entry)    //entry与NULL相等说明没监测到u盘
		{
			continue;
		}
		else		//监测到u盘
		{
			break;		//跳出外层循环
		}
		
	}
		
	return fname2;

}





////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/********
**产生字符命令的线程函数，产生的字符，放在全局变量cmd中，在图片线程中被使用
**********************/

void *cmd_create(void *arg)
{
	char buffer[0];
	
	while (1)
	{
		cmd = getchar();	//键盘获取一个字符当作命令
	
		setbuf(stdin, buffer);
		
		sem_post(&bin_sem);	//讲信号量加1
	}
	
	return NULL;
}



////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/************************************************************************
***播放图片的线程函数
***接收目录名作为参数
***从此目录中读取图像文件名，全部放在数组中
***然后循环调用图像显示函数，显示图像
************************************************************************/

void func_pic(char *dirname)
{
		
	int j = 0;

	img_search(dirname);
	
	
	
	/***************************************************************************************/
	/*************************************键盘控制*************************************************/
	/************************变量cmd在main3.c源文件中定义为全局，在头文件中声明*****************************/
	
	//frame_prcs(fname_array[j]);	//先显示第一幅图片
	//j++;
		
	sem_wait(&bin_sem);		//等待信号量变化，此信号量在cmd_create线程中被改变
	
	while (1)
	{

		if (cmd == 'n')
		{	
			frame_prcs(fname_array[j]);
			j++;			
			if (img_count == j)			//i是图片的总数，j用来索引每一幅图片
			{
				j = 0;
			}			
		}
		else if (cmd == 'm')
		{
			if (1 == j)
			{
				j = img_count-1;		
				frame_prcs(fname_array[j]);
				j = 0;				
			}
			
			else if (0 == j)
			{
				j = img_count - 2;
				frame_prcs(fname_array[j]);
				j = img_count-1;
			}
			else 
			{
				frame_prcs(fname_array[j-2]);
				j--;
			}
		}

		
		sem_wait(&bin_sem);
		
	}

}




/////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
/******************遍历挂载U盘的目录，把所有图像文件放在数组中******************************************/


void  img_search(char *dirname)
{
	
	char *fname = NULL;

	char *dirname2 = (char *)malloc(sizeof(char) * 128);

	
	
	DIR *dp;
	struct dirent *entry;	
	struct stat statbuf;

	
	if((dp=opendir(dirname))==NULL)	//打开的目录为当前目录路下的picture目录
	{
		perror("err: failed to opendir\n");
		return;
	}
	
	

	chdir(dirname);		//切换到当前目录	
	while((entry=readdir(dp)) != NULL)//读取指定文件夹下图像文件名（字符串）放到内存 用指针fname指向这个字符串
	{
		lstat(entry->d_name, &statbuf);		//获取读到的文件属性
		
		if(S_ISDIR(statbuf.st_mode))
		{
		
			if(strcmp(".",entry->d_name)==0 || strcmp("..",entry->d_name)==0)
			{
				continue;
			}
			sprintf(dirname2, "%s/%s", dirname, entry->d_name);
			img_search(dirname2);
			
		
		}
		else
		{
			
			//判断是否为图像文件,如果是，则把文件名放到fname_array[i]指向的空间中去，如果不是，继续readdir		
			int len = strlen(entry->d_name);
			len -= 4;
			
			if (strcmp(entry->d_name + len, ".jpg")!=0 && strcmp(entry->d_name + len, ".JPG")!=0)
			{
				continue;
			}
			else
			{
				fname = entry->d_name;
				
				fname_array[img_count] =  (char *)malloc(sizeof(char) * 64);
				
				sprintf(fname_array[img_count], "%s/%s", dirname, fname);	//连接字符串，构造一个完整的文件名
				//printf("%s\n", fname_array[img_count]);
				
				img_count++;		
			}
		}

	}
	
	chdir("..");
	closedir(dp);

	return;
	
}







////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/********************************处理一幅图像的函数，要被循环调用****************************/

void frame_prcs(char *fname)
{
	

       /*

        * declaration for jpeg decompression

        */
	
       struct jpeg_decompress_struct cinfo;

       struct jpeg_error_mgr jerr;

       FILE           *infile;	

       //unsigned char  *buffer;

 

       /*

        * declaration for framebuffer device

        */

       int             fbdev;

       char           *fb_device;

       unsigned char  *fbmem;

       unsigned int    screensize;

       unsigned int    fb_width;

       unsigned int    fb_height;

       unsigned int    fb_depth;

       unsigned int    x;

       unsigned int    y;





       /*

        * open framebuffer device

        */

    if ((fb_device = getenv("FRAMEBUFFER")) == NULL)
	{

              fb_device = FB_DEV;

	}

       fbdev = fb_open(fb_device);

 

       /*

        * get status of framebuffer device

        */

       fb_stat(fbdev, &fb_width, &fb_height, &fb_depth);

 
       /*

        * map framebuffer device to shared memory

        */

       screensize = fb_width * fb_height * fb_depth / 8;

       fbmem = fb_mmap(fbdev, screensize);

 

	/*

        * open input jpeg file 

        */
	
		
		
       if ((infile = fopen(fname, "rb")) == NULL) {		//打开文件
		
		//printf("%s\n", fname);
              
		perror("err: open image failed\n");

              exit(-1);

       }

 


       //init jpeg decompress object error handler


       cinfo.err = jpeg_std_error(&jerr);
       jpeg_create_decompress(&cinfo);


       //bind jpeg decompress object to infile
       jpeg_stdio_src(&cinfo, infile);


        //read jpeg header
       jpeg_read_header(&cinfo, TRUE);

 

       /*

        * decompress process.

        * note: after jpeg_start_decompress() is called

        * the dimension infomation will be known,

        * so allocate memory buffer for scanline immediately

        */

       jpeg_start_decompress(&cinfo);

       if ((cinfo.output_width > fb_width) ||

              (cinfo.output_height > fb_height)) {

              printf("too large JPEG file,cannot display\n");

              //return (-1);
		exit(EXIT_FAILURE);

       }


    
	

	//struct arg_data arg = {cinfo, buffer, fb_width, fb_height, fb_depth, x, y, fbmem};
	
	int i= 0;
	
	//把一幅图像的数据都读进内存
	while (cinfo.output_scanline < cinfo.output_height) 
	{
		
		buffer[i] = (unsigned char *) malloc(cinfo.output_width * cinfo.output_components);	
		
		//读取一行的像素数据，放在缓冲区buffer[i]中
    jpeg_read_scanlines(&cinfo, &buffer[i], 1);	
    
    i++;
		
	}
	
	
	
	
	//创建了六个线程，每个线程显示百叶窗的一部分
	pthread_t thrd1, thrd2, thrd3, thrd4, thrd5, thrd6;
	int ret;
	
	//初始化互斥锁
	ret = pthread_mutex_init(&my_lock,NULL);
	if(ret)
	{
		perror("err: init mutex failed\n");
		exit(EXIT_FAILURE);
	}
	
	
	ret = pthread_create(&thrd1, NULL, (void *)thrd_frame1, (void *)fbmem);
	if (ret)
	{
		perror("err: pthread_create: thrd1\n");
		exit(EXIT_FAILURE);
	}



	ret = pthread_create(&thrd2, NULL, (void *)thrd_frame2, (void *)fbmem);
	if (ret)
	{
		perror("err: pthread_create: thrd2\n");
		exit(EXIT_FAILURE);
	}
	
	
	ret = pthread_create(&thrd3, NULL, (void *)thrd_frame3, (void *)fbmem);
	if (ret)
	{
		perror("err: pthread_create: thrd3\n");
		exit(EXIT_FAILURE);
	}
	
	
	ret = pthread_create(&thrd4, NULL, (void *)thrd_frame4, (void *)fbmem);
	if (ret)
	{
		perror("err: pthread_create: thrd4\n");
		exit(EXIT_FAILURE);
	}
	
	
	ret = pthread_create(&thrd5, NULL, (void *)thrd_frame5, (void *)fbmem);
	if (ret)
	{
		perror("err: pthread_create: thrd5\n");
		exit(EXIT_FAILURE);
	}
	
	
	ret = pthread_create(&thrd6, NULL, (void *)thrd_frame6, (void *)fbmem);
	if (ret)
	{
		perror("err: pthread_create: thrd6\n");
		exit(EXIT_FAILURE);
	}

	pthread_join(thrd1, NULL);
	pthread_join(thrd2, NULL);
	pthread_join(thrd3, NULL);
	pthread_join(thrd4, NULL);
	pthread_join(thrd5, NULL);
	pthread_join(thrd6, NULL);
	
	//销毁互斥锁
	pthread_mutex_destroy(&my_lock);
	
	


	//完成解压，销毁加压对象
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	
	
	fclose(infile);
	
	fb_munmap(fbmem, screensize);
	
	fb_close(fbdev);
	
	return;

}




////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/*****************************6个线程函数，每个函数显示百叶窗的一部分*********************************/
void thrd_frame1(unsigned char *fbmem)
{
	
	//把缓冲区内容写到屏幕
	int i, j;
	for (i = 0; i < 100; i++)		//外围循环控制行
	{
		
		for (j = 0; j < 800; j++)		//内围循环控制每行的每个像素点
		{
			unsigned short  color;
			
			pthread_mutex_lock(&my_lock);
			color = RGB888toRGB565(buffer[i][j * 3], buffer[i][j * 3 + 1], buffer[i][j * 3 + 2]);			
			fb_pixel(fbmem, 800, 600, j, i, color);  
			pthread_mutex_unlock(&my_lock);                                      
		}
		
		usleep(3000);
	}
	
	pthread_exit((void *)0);		
}





void thrd_frame2(unsigned char *fbmem)
{
	
	//把缓冲区内容写到屏幕
	int i, j;
	for (i = 100; i < 200; i++)
	{
		
		for (j = 0; j < 800; j++)
		{
			unsigned short  color;
			pthread_mutex_lock(&my_lock);
			color = RGB888toRGB565(buffer[i][j * 3], buffer[i][j * 3 + 1], buffer[i][j * 3 + 2]);			
			fb_pixel(fbmem, 800, 600, j, i, color); 
			pthread_mutex_unlock(&my_lock);
			                                       
		}
		
		usleep(3000);
	}
	
	pthread_exit((void *)0);
	
}

void thrd_frame3(unsigned char *fbmem)
{
	
	//把缓冲区内容写到屏幕
	int i, j;
	for (i = 200; i < 300; i++)
	{
		
		for (j = 0; j < 800; j++)
		{
			unsigned short  color;
			pthread_mutex_lock(&my_lock);
			color = RGB888toRGB565(buffer[i][j * 3], buffer[i][j * 3 + 1], buffer[i][j * 3 + 2]);			
			fb_pixel(fbmem, 800, 600, j, i, color);  
			pthread_mutex_unlock(&my_lock);                                      
		}
		
		usleep(3000);
	}
	
	pthread_exit((void *)0);		
}

void thrd_frame4(unsigned char *fbmem)
{
	
	//把缓冲区内容写到屏幕
	int i, j;
	for (i = 300; i < 400; i++)
	{
		
		for (j = 0; j < 800; j++)
		{
			unsigned short  color;
			pthread_mutex_lock(&my_lock);
			color = RGB888toRGB565(buffer[i][j * 3], buffer[i][j * 3 + 1], buffer[i][j * 3 + 2]);			
			fb_pixel(fbmem, 800, 600, j, i, color);  
			pthread_mutex_unlock(&my_lock);                                      
		}
		
		usleep(3000);
	}
	
	pthread_exit((void *)0);		
}

void thrd_frame5(unsigned char *fbmem)
{
	
	//把缓冲区内容写到屏幕
	int i, j;
	for (i = 400; i < 500; i++)
	{
		
		for (j = 0; j < 800; j++)
		{
			unsigned short  color;
			pthread_mutex_lock(&my_lock);
			color = RGB888toRGB565(buffer[i][j * 3], buffer[i][j * 3 + 1], buffer[i][j * 3 + 2]);			
			fb_pixel(fbmem, 800, 600, j, i, color);  
			pthread_mutex_unlock(&my_lock);                                      
		}
		
		usleep(3000);
	}
	
	pthread_exit((void *)0);		
}

void thrd_frame6(unsigned char *fbmem)
{
	
	//把缓冲区内容写到屏幕
	int i, j;
	for (i = 500; i < 600; i++)
	{
		
		for (j = 0; j < 800; j++)
		{
			unsigned short  color;
			pthread_mutex_lock(&my_lock);
			color = RGB888toRGB565(buffer[i][j * 3], buffer[i][j * 3 + 1], buffer[i][j * 3 + 2]);			
			fb_pixel(fbmem, 800, 600, j, i, color);  
			pthread_mutex_unlock(&my_lock);                                      
		}
		
		usleep(3000);
	}
	
	pthread_exit((void *)0);		
}
/******************************显示百叶窗的6个线程完毕*******************************************/







////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/*****************播放本地文件指定的音频文件，不需要接收参数****************************/

void func_mus3(char *p)
{
	system("./madplay test.mp3");
	
	return;
}





////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/************** convert 24bit RGB888 to 16bit RGB565 color format*******************************/


unsigned short RGB888toRGB565(unsigned char red, unsigned char green, unsigned char blue)

{

       unsigned short  B = (blue >> 3) & 0x001F;

       unsigned short  G = ((green >> 2) << 5) & 0x07E0;

       unsigned short  R = ((red >> 3) << 11) & 0xF800;

 

       return (unsigned short) (R | G | B);

}

 

////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/*****************************************************
 * 打开 framebuffer device.
 * 如果成功则返回文件描述符
 * 错误则返回-1
 *****************************************************/


int fb_open(char *fb_device)

{
       int  fd;
       if ((fd = open(fb_device, O_RDWR)) < 0) 
       	{

              perror(__func__);

              return (-1);
       }
       
       return (fd);
}

 
////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/*******************************************************************
*** 获取framebuffer's width,height,and depth.
*** 成功则返回0，失败则返回-1
***************************************************************************/


int fb_stat(int fd, int *width, int *height, int *depth)
{
       struct fb_fix_screeninfo fb_finfo;
       struct fb_var_screeninfo fb_vinfo;
 
       if (ioctl(fd, FBIOGET_FSCREENINFO, &fb_finfo)) 
       	{
              perror(__func__);

              return (-1);
       } 

       if (ioctl(fd, FBIOGET_VSCREENINFO, &fb_vinfo)) 
       	{
              perror(__func__);
              return (-1);
       }
 
       *width = fb_vinfo.xres;
       *height = fb_vinfo.yres;
       *depth = fb_vinfo.bits_per_pixel;

       return (0);

}

 
////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/**************************************************
*** 映射shared memory 到 framebuffer device.
***如果成功返回映射区首地址
*** 失败返回-1
**************************************************/

void *fb_mmap(int fd, unsigned int screensize)
{
       caddr_t fbmem;

       if ((fbmem = mmap(0, screensize, PROT_READ | PROT_WRITE,
                                     MAP_SHARED, fd, 0)) == MAP_FAILED) 
       {
              perror(__func__);
              return (void *) (-1);
       }

       return (fbmem);

}

 

////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/**************************** 解除映射******************************/

int fb_munmap(void *start, size_t length)
{
      return (munmap(start, length));
}

 


////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/**********************关闭 framebuffer设备********************************/

int fb_close(int fd)

{
      return (close(fd));
}

 
 
 
////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/***************************************************************************
***把一个像素点的数据写到屏幕，fbmem：framebuffer映射到内存的起始位置  
***(x, y)是写入点的坐标,要在分辨率(800，600)的范围内,color一个像素数据
***********************************************************************************/

int fb_pixel(void *fbmem, int width, int height,
               int x, int y, unsigned short color)
{

       if ((x > width) || (y > height))

              return (-1);

       unsigned short *dst = ((unsigned short *) fbmem + y * width + x);

       *dst = color;

       return (0);

}


////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/*************接收一个文件夹做参数，在此文件夹里搜索.mp3文件，文件名全部放到数组里，然后循环播放这些音频文件***************/

void func_mus(char *dirname)
{
	int j = 0;

	char *fname = NULL;

	DIR *dp;
	struct dirent *entry;


	//操作U盘目录之前上锁
	pthread_mutex_lock(&my_lock2);
	
	
	if((dp=opendir(dirname))==NULL)	//打开的目录为当前目录路下的picture目录
	{
		perror("err: fail to opendir\n");
		return;
	}

	char *fname2 = (char *)malloc(sizeof(char) * 64);	//如果分配太小，很可能放不下文件名（含路径）
	char *fname_array[BUFSZ];
	int i = 0;
		
	while((entry=readdir(dp)) != NULL)//读取指定文件夹下图像文件名（字符串）放到内存 用指针fname指向这个字符串
	{
		if(strcmp(".",entry->d_name)==0 || strcmp("..",entry->d_name)==0)
			continue;
		
		//判断是否为音频文件
		int len = strlen(entry->d_name);
		len -= 4;
		if (strcmp(entry->d_name + len, ".mp3")!=0 && strcmp(entry->d_name + len, ".MP3")!=0)
			continue;

		else
		{
			fname = entry->d_name;
			//printf("%s\n", fname);

			sprintf(fname2, "%s/%s", dirname, fname);	//连接字符串，构造一个完整的文件名
			fname_array[i] =  (char *)malloc(sizeof(char) * 64);
			strcpy(fname_array[i], fname2);	
			//printf("%s\n", fname_array[i]);

			i++;
		}

	}

	//U盘目录操作完毕，解锁
	pthread_mutex_unlock(&my_lock2);
	
	
	free(fname2);
	printf("%d\n", i);

	for(j = 0; j < i;)
	{
		printf("%s\n", fname_array[j]);
		
	
		func_mus2(fname_array[j]);		//执行一次，调用madplay之后可能出错而导致程序退出，无法执行下面的语句
		j++;
		
/*		//for循环一次之后，再次重新循环！！！！！！！！！！！！
		if (j == i)	
		{
			j = 0;
		}
*/
	}
	

	pthread_exit((void *)0);

}

////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/**************接收音频文件的名称作为参数，然后播放此音频文件***********************/
void func_mus2(char *fname)
{
	char *fname2 = (char *)malloc(sizeof(char) * 64);
	sprintf(fname2, "madplay %s", fname);
	system(fname2);
	
	//execlp("madplay", "madplay", fname, NULL);
	
	return;
}



