#include <stdio.h>
#include <stdlib.h>


#include "v4l2.h"
#include "v4l2_ops.h"


int v4l2_get_stream(struct v4l2_dev *dev)
{

	return v4l2_get_stream_data(dev->fd, dev->buf_count, dev->fmt->vaddr, dev->buf);
}


void v4l2_close (struct v4l2_dev *dev)
{
	if (dev->fd < 0)
		return;


	/* 释放缓冲区 */

	v4l2_free_fb(dev->buf_count, dev->fmt, dev->buf);

	dev->fmt = NULL;
	dev->buf = NULL;


	close(dev->fd);

	dev->fd = -1;

	return ;
}

int v4l2_stop(int fd)
{
	enum v4l2_buf_type type;

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (-1 == ioctl(fd, VIDIOC_STREAMOFF, &type)) {
		printf("%s-->VIDIOC_STREAMOFF err %m", __func__);
		return -errno;
	}

	return 0;

}

int v4l2_start(int fd)
{
	enum v4l2_buf_type type;

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (-1 == ioctl(fd, VIDIOC_STREAMON, &type)) {
		printf("%s-->VIDIOC_STREAMON err %m", __func__);
		return -errno;
	}

	return 0;
}


int v4l2_check_capabilities (int fd)
{
	struct v4l2_capability cap		= {};
	
	int rval;
	
	/* 2.1 检查设备是否支持查询功能 */

	rval = ioctl(fd, VIDIOC_QUERYCAP, &cap);
	if (rval) {
		printf("%s-->this device unable to query device\n", __func__);
		goto out;
	}

	//printf("capabilities : %X \n", cap.capabilities);
	
	/*  2.2 检查设备是否支持:视频捕获 */

	if (!(rval = (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))) {
		printf("%s-->this device does not support capture\n",  __func__);
		goto out;
	}

	/*  2.3 检查设备是否支持:IO流操作 */

	if (!(rval = (cap.capabilities & V4L2_CAP_STREAMING))) {
		printf("%s-->cannot support streaming i/o\n",  __func__);
		goto out;
	}
	
	return 0;
	
out:
	return -1;


}

struct format* v4l2_init(const char *video, struct v4l2_dev *dev)
{
	int rval, fd;	

	/* 1、打开设备 */
	fd = open(video, O_RDWR);
	if (fd < 0) {
		printf("%s-->cannot open '%s' device, %m \n", __func__, video);
		return NULL;
	}

	dev->fd = fd;


	/* 2. 设备支持功能检测 */

	rval = v4l2_check_capabilities(dev->fd);
	if (rval < 0)
		goto out;


	/* 3.1 设置图像数据格式 */

	rval = v4l2_set_format(dev->fd, 640, 480, V4L2_PIX_FMT_YUYV);
	if (rval < 0)
		goto out;

	/* 3.2 获取图像数据格式 */
	rval = v4l2_get_format(dev->fd, &dev->fmt);
	if (rval < 0)
		goto out;

	/* 4. 确定I/O数据流的方法 */

	rval = v4l2_init_stream(fd, &dev->buf_count, dev->fmt, &dev->buf);
	if (rval < 0)
		goto out;

	return dev->fmt;

out:
	v4l2_close(dev);

	return NULL;
}


struct v4l2_dev v4l2 = {
	.fd = -1,
	.buf_count  = 5,
	.buf = NULL,
	.fmt = NULL,
	.v4l2_init  = v4l2_init,
	.v4l2_start = v4l2_start,
	.v4l2_stop  = v4l2_stop,
	.v4l2_close = v4l2_close,
	.v4l2_get_stream = v4l2_get_stream,

};


int register_v4l2(struct v4l2_dev **dev)
{

	*dev = &v4l2;

	return 0;
}
