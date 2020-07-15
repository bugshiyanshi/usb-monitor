#ifndef __FORMAT_H__
#define __FORMAT_H__


struct format {
	unsigned int width;		//缓冲区对象的宽度
	unsigned int height;		//缓冲区对象的高度
	unsigned int line_size;		//缓冲区对象每行像素数，单位字节
	unsigned int size;		//缓冲区对象总的像素数，单位字节
	unsigned int bpp;		//像素占多少位
	unsigned int pix;		//像素格式
	unsigned char *vaddr;		//指向图片缓冲区的内存映射地址
};


#endif

