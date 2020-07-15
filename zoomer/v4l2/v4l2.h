#ifndef __V4L2_H__
#define __V4L2_H__

#include <format.h>
#include "buf.h"


struct v4l2_dev {
	int fd;	
	int buf_count;
	struct buffer *buf;
	struct format *fmt;
	int (*v4l2_start)(int fd);
	int (*v4l2_stop)(int fd);
	void (*v4l2_close) (struct v4l2_dev *dev);
	int (*v4l2_get_stream)(struct v4l2_dev *dev);
	struct format* (*v4l2_init)(const char *video, struct v4l2_dev *dev);
};


int register_v4l2(struct v4l2_dev **dev);

#endif

