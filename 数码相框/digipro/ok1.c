#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <jpeglib.h>
#include <jerror.h>

//extern char *p[100]={NULL};
char *p[100]={NULL};
//extern char *pp;
//extern char *p11[100];

char *check_usb()
{
    char *fname2 = NULL;
    DIR *dp;
    struct dirent *entry;
    while(1)
    {
        if((dp=opendir("/dev"))==NULL)
        {
            perror("err: fail to opendir\n");
            return NULL;
        }
        while((entry=readdir(dp)) != NULL)
        {
            if(strcmp("sdc4",entry->d_name)==0)
            {
                system("sudo umount /dev/sdc4");
                system("sudo modprobe usb-storage");
                system("sudo mkdir /mnt/my_usb");
                system("sudo mount /dev/sdc4 /mnt/my_usb");
            }
            else
            {
                continue;
            }
            fname2 = "/mnt/my_usb";             
            break;              
        }
        closedir(dp);           
        if(NULL == entry)    
        {
            continue;
        }
        else         
        {
            break;     
        }
    }
    return fname2;
}


char* choice(char *p)
{
    int i=0;
    char buf[100];
    strcpy(buf,p);
    int num=strlen(buf);
    num-=4;
    if (strcmp(buf+num,".jpg")==0 || strcmp(buf+num,".mp3")==0)
    {
         return(p);
    }
    else
        return(NULL);
}



char *List(char *path)
{
 //   char *p[100]={NULL},*b=NULL;
   char *b=NULL;
     int i=0,j=0;
    struct dirent *ent = NULL;
    DIR *pDir;
    pDir=opendir(path);
    while ( ( ent = readdir(pDir) ) != NULL )
    {
        if (ent->d_reclen==24 || ent->d_reclen==20 || ent->d_reclen==28 )
        {
            if (ent->d_type==8)
            {
                p[i] = choice(ent->d_name);
                if(p[i]!=NULL)
                {
                    i++;
                }
            }
            else if(ent->d_type==4)
            {
               // printf("子目录->%s\n",ent->d_name);
                List(ent->d_name);

               // printf("返回%s\n",ent->d_name);
            }
        }
    }
 //   *pp=*p;
    
    j=0;
    while(p[j]!=NULL)
    {
        printf("%s\n",p[j]);
        j++;
    }
    return(NULL);
}



/*

int main()
{
   // char *pp;
 //   char *pp;
    pp=check_usb();
    List(pp);
  int i, jj=0;
    while(p[jj]!=NULL)
    {
//        printf("%s\n",p[jj]);
        jj++;
    }
for(i=0;i<jj;i++)
{
p11[i]=strcat(pp,p[i]);
}
p11[i]=NULL;
    main1(p11);
    return 0;
}

*/













#define    FB_DEV  "/dev/fb0"

/***************** function declaration ******************/

void            usage(char *msg);
unsigned short  RGB888toRGB565(unsigned char red, unsigned char green, unsigned char blue);
int             fb_open(char *fb_device);
int             fb_close(int fd);
int             fb_stat(int fd, int *width, int *height, int *depth);
void           *fb_mmap(int fd, unsigned int screensize);
int             fb_munmap(void *start, size_t length);
int             fb_pixel(void *fbmem, int width, int height,int x, int y, unsigned short color);

 

/************ function implementation ********************/

int main1(char *p11)

{
    /*  declaration for jpeg decompression  */

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE   *infile;
    unsigned char  *buffer;
  
   /*  declaration for framebuffer device  */

    int fbdev;
    char *fb_device;
    unsigned char  *fbmem;
    unsigned int   screensize;
    unsigned int   fb_width;
    unsigned int   fb_height;
    unsigned int   fb_depth;
    unsigned int   x;
    unsigned int   y;

   /*   check auguments  */
   
   
    
   /*  open framebuffer device   */
//int n=0;
//while(p11[n]!=NULL)
//{
    if ((fb_device = getenv("FRAMEBUFFER")) == NULL)
    {  
        fb_device = FB_DEV;
    }
    fbdev = fb_open(fb_device);

    /*  get status of framebuffer device  */

    fb_stat(fbdev, &fb_width, &fb_height, &fb_depth);

    /*   map framebuffer device to shared memory    */

    screensize = fb_width * fb_height * fb_depth / 8;
    fbmem = fb_mmap(fbdev, screensize);
    /*  open input jpeg file  */
	printf("befor open\n");//break
//	printf("%s\n",p11[n]);//break

    if ((infile = fopen(p11, "rb")) == NULL)
    {
        fprintf(stderr, "open %s failed/n", p11);
        exit(-1);
    }
     
	printf("after open\n");//break
   /*   init jpeg decompress object error handler  */
   
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

   /*    bind jpeg decompress object to infile   */

    jpeg_stdio_src(&cinfo, infile);

   /*    read jpeg header     */

    jpeg_read_header(&cinfo, TRUE);
	
  /*
        * decompress process.
        * note: after jpeg_start_decompress() is called
        * the dimension infomation will be known,
        * so allocate memory buffer for scanline immediately
  */

    jpeg_start_decompress(&cinfo);

    if ((cinfo.output_width > fb_width) || (cinfo.output_height > fb_height)) 
    {
        printf("too large JPEG file,cannot display/n");
        return (-1);
    }
    buffer = (unsigned char *) malloc(cinfo.output_width * cinfo.output_components);
    y = 0;
    while (cinfo.output_scanline < cinfo.output_height) 
    {
        jpeg_read_scanlines(&cinfo, &buffer, 1);
        if (fb_depth == 16) 
        {
            unsigned short  color;
            for (x = 0; x < cinfo.output_width; x++) 
            {
                 color = RGB888toRGB565(buffer[x * 3],
                 buffer[x * 3 + 1], buffer[x * 3 + 2]);
                 fb_pixel(fbmem, fb_width, fb_height, x, y, color);

            }

       } else if (fb_depth == 24)
              {
                  memcpy((unsigned char *) fbmem + y * fb_width * 3,buffer, cinfo.output_width * cinfo.output_components);
              }
              y++;                                   // next scanline
    }

    /*     finish decompress, destroy decompress object    */

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

   /*   release memory buffer    */
    free(buffer);

   /*    close jpeg inputing file   */

    fclose(infile);

   /*    unmap framebuffer's shared memory   */

    fb_munmap(fbmem, screensize);

   /*     close framebuffer device   */

    fb_close(fbdev);
    sleep(2);
   // n++;
//}
    return (0);
}

 

void usage(char *msg)
{
    fprintf(stderr, "%s/n", msg);
    printf("Usage: fv some-jpeg-file.jpg/n");
}

/*  convert 24bit RGB888 to 16bit RGB565 color format   */

unsigned short RGB888toRGB565(unsigned char red, unsigned char green, unsigned char blue)
{
    unsigned short  B = (blue >> 3) & 0x001F;
    unsigned short  G = ((green >> 2) << 5) & 0x07E0;
    unsigned short  R = ((red >> 3) << 11) & 0xF800;
    return (unsigned short) (R | G | B);
}

/*

 * open framebuffer device.

 * return positive file descriptor if success,

 * else return -1.

 */

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

/*

 * get framebuffer's width,height,and depth.

 * return 0 if success, else return -1.

 */

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

/*
 * map shared memory to framebuffer device.
 * return maped memory if success,
 * else return -1, as mmap dose.
*/

void *fb_mmap(int fd, unsigned int screensize)
{
    caddr_t fbmem;
    if ((fbmem = mmap(0, screensize, PROT_READ | PROT_WRITE,MAP_SHARED, fd, 0)) == MAP_FAILED)
    {
        perror(__func__);
        return (void *) (-1);
    }
    return (fbmem);
}

/*
 * unmap map memory for framebuffer device.
*/

int fb_munmap(void *start, size_t length)
{
    return (munmap(start, length));
}

/*
 * close framebuffer device
*/

int fb_close(int fd)
{
    return (close(fd));
}

 

/*
 * display a pixel on the framebuffer device.
 * fbmem is the starting memory of framebuffer,
 * width and height are dimension of framebuffer,
 * x and y are the coordinates to display,
 * color is the pixel's color value.
 * return 0 if success, otherwise return -1.
*/

int fb_pixel(void *fbmem, int width, int height, int x, int y, unsigned short color)
{
    if ((x > width) || (y > height))
        return (-1);
    unsigned short *dst = ((unsigned short *) fbmem + y * width + x);
    *dst = color;
    return (0);

}


int main()
{
	char *p11[100];
		
	char *pp;
	pp=check_usb();
	List(pp);
	int i, jj=0;
    while(p[jj]!=NULL)
    {
        jj++;
    }
for(i=0;i<jj;i++)
{
p11[i]=malloc(strlen("/mnt/my_usb/")+strlen(p[i])+1);
if(p11[i]==NULL)
{
	perror("p11[i] malloc");
	exit(EXIT_FAILURE);
}
strcpy(p11[i],"/mnt/my_usb/");
strcat(p11[i],p[i]);
p11[i][15+sizeof(char)+sizeof(p[i])]='\0';
//p11[i]=strcat(pp,p[i]);
}
	
//p11[i]=NULL;
	for(i=0;i<jj;i++)
	{
		printf("%s\n",p11[i]);
		main1(p11[i]);
	}
	
    return 0;
}

