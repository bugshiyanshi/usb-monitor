#ifndef __ZOOMER_H__
#define __ZOOMER_H__

#include <format.h>

struct zoomer {
	unsigned char *buf;
	void (*zoom_close) (struct zoomer *dev);
	void (*zoom_img) (unsigned char *src, unsigned char *dst,
				struct format *src_fmt, struct format *dst_fmt);
};


int register_zoomer(struct zoomer **dev, int buf_size);

#endif

