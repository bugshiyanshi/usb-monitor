#ifndef __V4L2_OPS_H__
#define __V4L2_OPS_H__

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <assert.h>

/**** v4l2 header ****/
#include <linux/videodev2.h>

#include <format.h>

#include "buf.h"


int v4l2_set_format (int fd, unsigned int width, unsigned int height, unsigned int pix);
int v4l2_get_format (int fd, struct format **fmt);
int v4l2_init_stream (int fd, int *buf_count, struct format *fmt, struct buffer **buf);
int v4l2_get_stream_data (int fd, int buf_count, unsigned char *vaddr, struct buffer *buf);
void v4l2_free_fb (int buf_count, struct format *fmt, struct buffer *buf);

#endif
