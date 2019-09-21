
/************************************************************************
*************************************************************************

*** $Id: main.c

*** $Desp: 

*** $Author: salmon

*** $Date: 2011-3-12

*************************************************************************
************************************************************************/


#include "myhead.h"




/****************************ȫ�ֱ�������*************************************/
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

pthread_mutex_t my_lock2 = PTHREAD_MUTEX_INITIALIZER;		//����������������U��Ŀ¼�������̺߳�ͼƬ�̶߳�Ҫ��ȡ��Ŀ¼

sem_t bin_sem;		//�źţ����ڲ��������ַ�ģ��Ͷ�ȡ�ַ�ģ�飬Э�������߳�

char cmd = '0';		//�����ַ�����Ϊȫ�ֱ������ڸ��߳��ж�Ҫ����

pthread_t pic_tid, mus_tid, cmd_tid;  	//��������������߳�Ҫ�����ֺ�ͼƬ�߳�֮�����У��ȴ�ʱ�õ����̵߳�id,����Ϊȫ��
int err;




/****************************����ʵ��*****************************************/
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
 
int main(int argc, char *argv[])
{
	pid_t pid1;		
	int status;

	int pipefd[2];

	char *dirname = NULL;

	//���������ܵ������ڰ��ӽ��̻�õ�U��·��������������
	if (-1 == pipe(pipefd))
	{
		perror("err: fail to creat pipe\n");
		exit(EXIT_FAILURE);
	}

	//�����ӽ���
	pid1 = fork();

	//����û�ɹ�
	if (-1 == pid1)
	{
		perror("err: fork failed\n");
		exit(EXIT_FAILURE);
	}

	//�ӽ���
	else if (0 == pid1)
	{
		close(pipefd[0]);
		printf("\n----->waiting for the USB flash disk....\n");
		dirname = check_disk();	//����U�̼�⺯��
		write(pipefd[1], dirname, BUFSZ-1);
		close(pipefd[1]);
		exit(EXIT_SUCCESS);

	}	

	//������			  
	else
	{
		//ͨ���ܵ����ӽ��������ȡU�̵�·��
		wait(&status);		//�����̵ȴ��ӽ��̵��˳�
		close(pipefd[1]);
		char dirname2[BUFSZ];
		read(pipefd[0], dirname2, BUFSZ);		
		close(pipefd[0]);				
		//printf("found the USB disk, and it has been mounted to %s\n", dirname2);


		/****************�����������̣߳��ֱ𲥷�ͼƬ �������� ������������******************************/
		/******���������������̲߳���һ�������ַ�����ͼƬ���̣߳����߳�ͨ����ͬ����ֵ������ͬ��Ӧ*********/


		//��ʼ��������
		err = pthread_mutex_init(&my_lock2,NULL);
		if(err)
		{
			perror("err: init mutex failed\n");
			exit(EXIT_FAILURE);
		}

		//��ʼ���ź���
		err = sem_init(&bin_sem, 0, 0);
		if(err)
		{
			perror("err: init semphore failed\n");
			exit(EXIT_FAILURE);
		}	  


		//������ȡ������������߳�
		err = pthread_create(&cmd_tid, NULL, (void *)cmd_create, NULL);
		if (err != 0)
		{
			perror("err: create cmd_tid failed\n");
			exit(EXIT_FAILURE);
		}


		//��������ͼƬ�����߳�
		err = pthread_create(&pic_tid, NULL, (void *)func_pic, (void *)dirname2); //u��·�����ݸ����߳�
		if (err != 0)
		{
			perror("err: can't create pic_thread\n");
			exit(EXIT_FAILURE);
		}
		
		//���ű�������		
		func_mus(dirname2);
		
		/*******************************************************		
		//�������ű������ֵ����߳�
		err = pthread_create(&mus_tid, NULL, (void *)func_mus3, NULL); //���ű������ֵ��̣߳����ű��ص���Ƶ�ļ�
		if (err != 0)
		{
			perror("err: can't create mus_thread\n");
			exit(EXIT_FAILURE);
		}
		****************/

		err = pthread_join(pic_tid, NULL);	//���߳�һֱ�������ȴ�pic_tid�ķ���
		if (err != 0)
		{
			printf("can't join with pic_thread\n");
		}

		/********************
		err = pthread_join(mus_tid, NULL);	//���߳�һֱ�������ȴ�mus_tid�ķ���
		if (err != 0)
		{
			printf("can't join with mus_thread\n");
		}
		********************/	


		exit(EXIT_SUCCESS);

	}

	return 0;
}

