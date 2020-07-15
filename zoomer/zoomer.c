#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "zoomer.h"

void zoom_img (unsigned char *src, unsigned char *dst,
		struct format *src_fmt, struct format *dst_fmt)
{
	unsigned long x, y;
	unsigned long dst_width = dst_fmt->width;
	unsigned bytes = dst_fmt->bpp / 8;
	unsigned char *pdst;
	unsigned char *psrc;
	

	unsigned long *table = malloc(dst_width * sizeof(unsigned long));

	for (x = 0; x < dst_width; x++) {

		table[x] = x * src_fmt->width / dst_fmt->width;
	}

	for (y = 0; y < dst_fmt->height; y++) {
	
		unsigned long srcy = y * src_fmt->height / dst_fmt->height;
		pdst = dst + y * dst_fmt->line_size;
		psrc = src + srcy * src_fmt->line_size;

		for (x = 0; x < dst_width; x++)
			memcpy(pdst + x * bytes, psrc + table[x] * bytes, bytes);
	}

	free(table);
	
	printf("zoom finish ....\n");
	
	return;
}



void zoom_close (struct zoomer *dev)
{
	if ((NULL == dev) || (NULL == dev->buf))
		return;
	
	free(dev->buf);
	
	dev->buf = NULL;

	return ;
}

struct zoomer zoomer = {
	.buf = NULL,
	.zoom_close = zoom_close,
	.zoom_img   = zoom_img,
};

int zoom_req_buf (int buf_size, unsigned char **buf)
{
	unsigned char *buffer = NULL;
	
	buffer = calloc(1, buf_size);
	if (NULL == buffer) {
		printf("%s-->zoom buf malloc err %m\n", __func__);
		return -errno;
	}
	
	*buf = buffer;
	
	return 0;
}


int register_zoomer(struct zoomer **dev, int buf_size)
{
	int rval;
	
	rval = zoom_req_buf(buf_size, &zoomer.buf);
	if (rval < 0) {
		*dev = NULL;
		return rval;
	}
	
	*dev = &zoomer;

	return 0;
}
