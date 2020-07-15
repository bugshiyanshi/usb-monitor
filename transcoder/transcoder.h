#ifndef __TRANSCODER_H__
#define __TRANSCODER_H__

#include <format.h>

struct transcoder {
	unsigned char *buf;
	void (*yuyv422torgb32)(struct format *v4l2_fmt, unsigned char *output_ptr);
	void (*transcoder_close) (struct transcoder *dev);
};


int register_transcoder(struct transcoder **dev, int buf_size);

#endif

