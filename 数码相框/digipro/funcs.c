

/**********************ͷ�ļ�*********************************/
#include "myhead.h"



////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/*************************ȫ�ֱ�������******************************************/

unsigned char *buffer[600];		//600��ָ�빹�ɵ�ָ�����飬buffer[i]��ָ�룩ָ��һ����������
pthread_mutex_t my_lock = PTHREAD_MUTEX_INITIALIZER;		//���廥����my_lock����������buffer[i]���������������̶�Ҫͬ�����ʻ�����


int img_count = 0;		//ͳ��ͼƬ�ļ������ı�����ӦΪ����Ҫ�ݹ飬����Ҫ��Ϊȫ�ֵ�
char *fname_array[BUFSZ];		//ָ�����飬ÿ��Ԫ��ָ��һ��ͼ���ļ���ȫ��






////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/*****************���u�̵ĺ���������U�̹��ص��ƶ���·��/mnt/usb,���سɹ����������·��*************************/

char *check_disk(void)
{
	char *fname2 = NULL;

	DIR *dp;
	struct dirent *entry;

	while(1)
	{
		if((dp=opendir("/dev"))==NULL)	//��/devĿ¼
		{
			perror("err: fail to opendir\n");
			return NULL;
		}
		
		while((entry=readdir(dp)) != NULL)	//ѭ����ȡĿ¼�µ��ļ�
		{
			if(strcmp("sdb1",entry->d_name)==0)		//�ҵ�/dev/sdb1�ļ�����u���豸�ļ�
			{
					//�Ѽ�⵽U�̣�����U�̵�/mnt/usb
					//system("sudo umount /dev/sdb1");		
					system("sudo modprobe usb-storage");
					system("sudo mkdir /mnt/my_usb");
					system("sudo mount /dev/sdb1 /mnt/my_usb");
			}
			else 
			{
				continue;
			}
	
			fname2 = "/mnt/my_usb";		//�ѹ��ص㱣��
			
			break;		//�����ڲ�ѭ��
		}
		
		closedir(dp);		//ѭ���ڲ�һ��Ҫ�رմ򿪵��ļ�����Ϊϵͳ�ܴ򿪵��ļ�����������
		
 		if(NULL == entry)    //entry��NULL���˵��û��⵽u��
		{
			continue;
		}
		else		//��⵽u��
		{
			break;		//�������ѭ��
		}
		
	}
		
	return fname2;

}





////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/********
**�����ַ�������̺߳������������ַ�������ȫ�ֱ���cmd�У���ͼƬ�߳��б�ʹ��
**********************/

void *cmd_create(void *arg)
{
	char buffer[0];
	
	while (1)
	{
		cmd = getchar();	//���̻�ȡһ���ַ���������
	
		setbuf(stdin, buffer);
		
		sem_post(&bin_sem);	//���ź�����1
	}
	
	return NULL;
}



////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/************************************************************************
***����ͼƬ���̺߳���
***����Ŀ¼����Ϊ����
***�Ӵ�Ŀ¼�ж�ȡͼ���ļ�����ȫ������������
***Ȼ��ѭ������ͼ����ʾ��������ʾͼ��
************************************************************************/

void func_pic(char *dirname)
{
		
	int j = 0;

	img_search(dirname);
	
	
	
	/***************************************************************************************/
	/*************************************���̿���*************************************************/
	/************************����cmd��main3.cԴ�ļ��ж���Ϊȫ�֣���ͷ�ļ�������*****************************/
	
	//frame_prcs(fname_array[j]);	//����ʾ��һ��ͼƬ
	//j++;
		
	sem_wait(&bin_sem);		//�ȴ��ź����仯�����ź�����cmd_create�߳��б��ı�
	
	while (1)
	{

		if (cmd == 'n')
		{	
			frame_prcs(fname_array[j]);
			j++;			
			if (img_count == j)			//i��ͼƬ��������j��������ÿһ��ͼƬ
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
/******************��������U�̵�Ŀ¼��������ͼ���ļ�����������******************************************/


void  img_search(char *dirname)
{
	
	char *fname = NULL;

	char *dirname2 = (char *)malloc(sizeof(char) * 128);

	
	
	DIR *dp;
	struct dirent *entry;	
	struct stat statbuf;

	
	if((dp=opendir(dirname))==NULL)	//�򿪵�Ŀ¼Ϊ��ǰĿ¼·�µ�pictureĿ¼
	{
		perror("err: failed to opendir\n");
		return;
	}
	
	

	chdir(dirname);		//�л�����ǰĿ¼	
	while((entry=readdir(dp)) != NULL)//��ȡָ���ļ�����ͼ���ļ������ַ������ŵ��ڴ� ��ָ��fnameָ������ַ���
	{
		lstat(entry->d_name, &statbuf);		//��ȡ�������ļ�����
		
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
			
			//�ж��Ƿ�Ϊͼ���ļ�,����ǣ�����ļ����ŵ�fname_array[i]ָ��Ŀռ���ȥ��������ǣ�����readdir		
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
				
				sprintf(fname_array[img_count], "%s/%s", dirname, fname);	//�����ַ���������һ���������ļ���
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
/********************************����һ��ͼ��ĺ�����Ҫ��ѭ������****************************/

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
	
		
		
       if ((infile = fopen(fname, "rb")) == NULL) {		//���ļ�
		
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
	
	//��һ��ͼ������ݶ������ڴ�
	while (cinfo.output_scanline < cinfo.output_height) 
	{
		
		buffer[i] = (unsigned char *) malloc(cinfo.output_width * cinfo.output_components);	
		
		//��ȡһ�е��������ݣ����ڻ�����buffer[i]��
    jpeg_read_scanlines(&cinfo, &buffer[i], 1);	
    
    i++;
		
	}
	
	
	
	
	//�����������̣߳�ÿ���߳���ʾ��Ҷ����һ����
	pthread_t thrd1, thrd2, thrd3, thrd4, thrd5, thrd6;
	int ret;
	
	//��ʼ��������
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
	
	//���ٻ�����
	pthread_mutex_destroy(&my_lock);
	
	


	//��ɽ�ѹ�����ټ�ѹ����
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
/*****************************6���̺߳�����ÿ��������ʾ��Ҷ����һ����*********************************/
void thrd_frame1(unsigned char *fbmem)
{
	
	//�ѻ���������д����Ļ
	int i, j;
	for (i = 0; i < 100; i++)		//��Χѭ��������
	{
		
		for (j = 0; j < 800; j++)		//��Χѭ������ÿ�е�ÿ�����ص�
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
	
	//�ѻ���������д����Ļ
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
	
	//�ѻ���������д����Ļ
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
	
	//�ѻ���������д����Ļ
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
	
	//�ѻ���������д����Ļ
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
	
	//�ѻ���������д����Ļ
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
/******************************��ʾ��Ҷ����6���߳����*******************************************/







////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/*****************���ű����ļ�ָ������Ƶ�ļ�������Ҫ���ղ���****************************/

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
 * �� framebuffer device.
 * ����ɹ��򷵻��ļ�������
 * �����򷵻�-1
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
*** ��ȡframebuffer's width,height,and depth.
*** �ɹ��򷵻�0��ʧ���򷵻�-1
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
*** ӳ��shared memory �� framebuffer device.
***����ɹ�����ӳ�����׵�ַ
*** ʧ�ܷ���-1
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
/**************************** ���ӳ��******************************/

int fb_munmap(void *start, size_t length)
{
      return (munmap(start, length));
}

 


////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/**********************�ر� framebuffer�豸********************************/

int fb_close(int fd)

{
      return (close(fd));
}

 
 
 
////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/***************************************************************************
***��һ�����ص������д����Ļ��fbmem��framebufferӳ�䵽�ڴ����ʼλ��  
***(x, y)��д��������,Ҫ�ڷֱ���(800��600)�ķ�Χ��,colorһ����������
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
/*************����һ���ļ������������ڴ��ļ���������.mp3�ļ����ļ���ȫ���ŵ������Ȼ��ѭ��������Щ��Ƶ�ļ�***************/

void func_mus(char *dirname)
{
	int j = 0;

	char *fname = NULL;

	DIR *dp;
	struct dirent *entry;


	//����U��Ŀ¼֮ǰ����
	pthread_mutex_lock(&my_lock2);
	
	
	if((dp=opendir(dirname))==NULL)	//�򿪵�Ŀ¼Ϊ��ǰĿ¼·�µ�pictureĿ¼
	{
		perror("err: fail to opendir\n");
		return;
	}

	char *fname2 = (char *)malloc(sizeof(char) * 64);	//�������̫С���ܿ��ܷŲ����ļ�������·����
	char *fname_array[BUFSZ];
	int i = 0;
		
	while((entry=readdir(dp)) != NULL)//��ȡָ���ļ�����ͼ���ļ������ַ������ŵ��ڴ� ��ָ��fnameָ������ַ���
	{
		if(strcmp(".",entry->d_name)==0 || strcmp("..",entry->d_name)==0)
			continue;
		
		//�ж��Ƿ�Ϊ��Ƶ�ļ�
		int len = strlen(entry->d_name);
		len -= 4;
		if (strcmp(entry->d_name + len, ".mp3")!=0 && strcmp(entry->d_name + len, ".MP3")!=0)
			continue;

		else
		{
			fname = entry->d_name;
			//printf("%s\n", fname);

			sprintf(fname2, "%s/%s", dirname, fname);	//�����ַ���������һ���������ļ���
			fname_array[i] =  (char *)malloc(sizeof(char) * 64);
			strcpy(fname_array[i], fname2);	
			//printf("%s\n", fname_array[i]);

			i++;
		}

	}

	//U��Ŀ¼������ϣ�����
	pthread_mutex_unlock(&my_lock2);
	
	
	free(fname2);
	printf("%d\n", i);

	for(j = 0; j < i;)
	{
		printf("%s\n", fname_array[j]);
		
	
		func_mus2(fname_array[j]);		//ִ��һ�Σ�����madplay֮����ܳ�������³����˳����޷�ִ����������
		j++;
		
/*		//forѭ��һ��֮���ٴ�����ѭ��������������������������
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
/**************������Ƶ�ļ���������Ϊ������Ȼ�󲥷Ŵ���Ƶ�ļ�***********************/
void func_mus2(char *fname)
{
	char *fname2 = (char *)malloc(sizeof(char) * 64);
	sprintf(fname2, "madplay %s", fname);
	system(fname2);
	
	//execlp("madplay", "madplay", fname, NULL);
	
	return;
}



