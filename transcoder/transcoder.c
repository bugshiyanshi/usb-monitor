#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "transcoder.h"
#include "color.h"


void transcoder_close (struct transcoder *dev)
{
	if ((NULL == dev) || (NULL == dev->buf))
		return;
	
	free(dev->buf);
	dev->buf = NULL;
	
	freeLut();
	
	return;
}

void yuyv422_to_rgb32(struct format *v4l2_fmt, unsigned char *output_ptr)
{
	unsigned int i, size;
	unsigned char Y, Y1, U, V;
	unsigned char *buff = v4l2_fmt->vaddr;
	unsigned char *output_pt = output_ptr;
	
	size = v4l2_fmt->width * v4l2_fmt->height /2;
	
	for (i = size; i > 0; i--) {
		/* bgr instead rgb ?? */
		Y = buff[0] ;
		U = buff[1] ;
		Y1 = buff[2];
		V = buff[3];
		buff += 4;
		
		*output_pt++ = B_FROMYU(Y,U); //v
		*output_pt++ = G_FROMYUV(Y,U,V); //b
		*output_pt++ = R_FROMYV(Y,V);
		*output_pt++;
			
		*output_pt++ = B_FROMYU(Y,U); //v
		*output_pt++ = G_FROMYUV(Y,U,V); //b
		*output_pt++ = R_FROMYV(Y,V);
		*output_pt++;
	}
	
	return;
}

int transcoder_req_buf (int buf_size, unsigned char **buf)
{
	unsigned char *buffer = NULL;
	
	buffer = calloc(1, buf_size);
	if (NULL == buffer) {
		printf("%s-->encoder calloc buf err %m", __func__);
		return -errno;
	}
	
	*buf = buffer;
	
	initLut();
	
	return 0;
}

struct transcoder coder = {
	.buf = NULL,
	.yuyv422torgb32   = yuyv422_to_rgb32,
	.transcoder_close = transcoder_close,
};


int register_transcoder(struct transcoder **dev, int buf_size)
{
	int rval;
	
	rval = transcoder_req_buf(buf_size, &coder.buf);
	if (rval < 0)
		goto out;
	
	*dev = &coder;

	return 0;
	
out:
	*dev = NULL;
	return -1;
}
