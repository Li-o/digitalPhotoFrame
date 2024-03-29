======================================================================
======================================================================
一、文件结构

源文件包含三个文件：
1.mian.c     项目总体结构，一个进程（检测U盘），三个线程（播放图片，背景音乐，键盘控制）
2.funcs.c    项目中用到的所有函数
3.myhead.h   自定义的头文件，项目用到的所有头文件、宏、函数声明都包含在内



======================================================================



二、实现的功能：

程序运行即等待插入U盘
插入U盘后，自动挂载U盘，并读取U盘目录下的所有图片文件和音乐文件，播放图片和音乐
键盘控制图片的播放（上一幅，下一幅）



======================================================================



三、用法：

1.设置环境：
	解除挂载sdb1设备: $sudo umount /dev/sdb1
	屏幕分辨率设置成800*600  16位色
	安装madplay应用程序: $sudo apt-get install madplay
	libjpeg库
	：头文件libjpeg。h
	Example.c位于：、usr/share/doc/libjpeg/dev/example
	sudo apt-get install libjpeg62-dev
	Ubuntu下需安装libjpeg62-dev包
	
2.编译：
	$gcc main.c funcs.c  -ljpeg -lpthread   生成a.out可执行文件
	
3.运行程序：
	进入字符界面:$sudo ./a.out
	插入u盘
	按任意键，播放U盘里的音乐文件
	按n键，播放下一幅图片，按m键播放上一幅图片
	
	
	
======================================================================



四、不完善的地方

1.没能实现自动播放图片，只能手动翻页
2.背景音乐没实现循环播放，madplay应用的原因
======================================================================
======================================================================
