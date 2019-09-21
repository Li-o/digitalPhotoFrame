
/************************************************************************
*************************************************************************

*** $Id: main.c

*** $Desp: 

*** $Author: salmon

*** $Date: 2011-3-12

*************************************************************************
************************************************************************/


#include "myhead.h"




/****************************全局变量定义*************************************/
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

pthread_mutex_t my_lock2 = PTHREAD_MUTEX_INITIALIZER;		//互斥锁，用来保护U盘目录，音乐线程和图片线程都要读取此目录

sem_t bin_sem;		//信号，用在产生键控字符模块和读取字符模块，协调两个线程

char cmd = '0';		//命令字符，设为全局变量，在各线程中都要访问

pthread_t pic_tid, mus_tid, cmd_tid;  	//产生控制命令的线程要在音乐和图片线程之后运行，等待时用到两线程的id,定义为全局
int err;




/****************************函数实现*****************************************/
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
 
int main(int argc, char *argv[])
{
	pid_t pid1;		
	int status;

	int pipefd[2];

	char *dirname = NULL;

	//创建无名管道，用于把子进程获得的U盘路径名传给父进程
	if (-1 == pipe(pipefd))
	{
		perror("err: fail to creat pipe\n");
		exit(EXIT_FAILURE);
	}

	//创建子进程
	pid1 = fork();

	//创建没成功
	if (-1 == pid1)
	{
		perror("err: fork failed\n");
		exit(EXIT_FAILURE);
	}

	//子进程
	else if (0 == pid1)
	{
		close(pipefd[0]);
		printf("\n----->waiting for the USB flash disk....\n");
		dirname = check_disk();	//调用U盘监测函数
		write(pipefd[1], dirname, BUFSZ-1);
		close(pipefd[1]);
		exit(EXIT_SUCCESS);

	}	

	//父进程			  
	else
	{
		//通过管道从子进程那里获取U盘的路径
		wait(&status);		//父进程等待子进程的退出
		close(pipefd[1]);
		char dirname2[BUFSZ];
		read(pipefd[0], dirname2, BUFSZ);		
		close(pipefd[0]);				
		//printf("found the USB disk, and it has been mounted to %s\n", dirname2);


		/****************产生三个子线程，分别播放图片 背景音乐 产生控制命令******************************/
		/******产生控制命令子线程产生一个命令字符传给图片子线程，子线程通过不同按键值做出不同响应*********/


		//初始化互斥锁
		err = pthread_mutex_init(&my_lock2,NULL);
		if(err)
		{
			perror("err: init mutex failed\n");
			exit(EXIT_FAILURE);
		}

		//初始化信号量
		err = sem_init(&bin_sem, 0, 0);
		if(err)
		{
			perror("err: init semphore failed\n");
			exit(EXIT_FAILURE);
		}	  


		//创建获取控制命令的子线程
		err = pthread_create(&cmd_tid, NULL, (void *)cmd_create, NULL);
		if (err != 0)
		{
			perror("err: create cmd_tid failed\n");
			exit(EXIT_FAILURE);
		}


		//创建播放图片的子线程
		err = pthread_create(&pic_tid, NULL, (void *)func_pic, (void *)dirname2); //u盘路径传递给子线程
		if (err != 0)
		{
			perror("err: can't create pic_thread\n");
			exit(EXIT_FAILURE);
		}
		
		//播放背景音乐		
		func_mus(dirname2);
		
		/*******************************************************		
		//创建播放背景音乐的子线程
		err = pthread_create(&mus_tid, NULL, (void *)func_mus3, NULL); //播放背景音乐的线程，播放本地的音频文件
		if (err != 0)
		{
			perror("err: can't create mus_thread\n");
			exit(EXIT_FAILURE);
		}
		****************/

		err = pthread_join(pic_tid, NULL);	//主线程一直阻塞，等待pic_tid的返回
		if (err != 0)
		{
			printf("can't join with pic_thread\n");
		}

		/********************
		err = pthread_join(mus_tid, NULL);	//主线程一直阻塞，等待mus_tid的返回
		if (err != 0)
		{
			printf("can't join with mus_thread\n");
		}
		********************/	


		exit(EXIT_SUCCESS);

	}

	return 0;
}

