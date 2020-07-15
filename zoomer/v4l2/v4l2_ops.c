#include "v4l2_ops.h"


int v4l2_set_format (int fd, unsigned int width, unsigned int height, unsigned int pix)
{
	struct v4l2_format format   = {};

	
	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	format.fmt.pix.width       = width;
	format.fmt.pix.height      = height;
	format.fmt.pix.pixelformat = pix;

	if ( -1 == ioctl(fd, VIDIOC_S_FMT, &format)) {
		printf("%s-->VIDIOC_S_FMT err %m\n", __func__);
		return -errno;
	}

	return 0;
}


int v4l2_get_format (int fd, struct format **fmt)
{

	struct format *format = NULL;
	struct v4l2_format vfmt;
	
	int rval;
	
	
	format = (struct format*)malloc(sizeof(*format));
	if (NULL == format) {
		printf("%s-->img format malloc err %m\n", __func__);
		return -1;
	}
	
	/* 3.1 获取当前图片尺寸 */
	memset(&vfmt, 0, sizeof(vfmt));
	vfmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (-1 == ioctl(fd, VIDIOC_G_FMT, &vfmt)) {
		printf("%s-->VIDIOC_G_FMT %m", __func__);
		return -errno;
	}
	
	memset(format, 0, sizeof(*format));
	format->width  = vfmt.fmt.pix.width;
	format->height = vfmt.fmt.pix.height;
	format->pix = vfmt.fmt.pix.pixelformat;
	format->bpp = 32;
	format->line_size = format->width * format->bpp /8;
	format->size =  format->line_size * format->height;


		
	*fmt = format;
	
	return 0;

}


int v4l2_req_stream_buf(int fd, int *buf_count)
{
	struct v4l2_requestbuffers reqbuf = {};

	reqbuf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbuf.memory = V4L2_MEMORY_MMAP;
	reqbuf.count  = *buf_count;


	if (-1 == ioctl(fd, VIDIOC_REQBUFS, &reqbuf)) {
		printf("%s-->VIDIOC_REQBUFS", __func__);
		return -errno;
	}

	*buf_count = reqbuf.count;

	printf("request buf count %d\n", reqbuf.count);

	return 0;

}


int v4l2_map_stream_buffer(int fd, int buf_count, struct buffer **buf)
{
	struct buffer *buffers = NULL;
	struct v4l2_buffer vbuf = {};

	int i;

	if (buf_count < 2) {
		printf("%s-->not enought memory\n", __func__);
		return -1;
	}


	buffers = calloc(buf_count, sizeof(*buffers));
	if (NULL == buffers) {
		printf("%s-->calloc v4l stream buffer err %m", __func__);
		return -errno;
	}


	for (i = 0; i < buf_count; i++) {

		memset(&vbuf, 0, sizeof(vbuf));

		vbuf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		vbuf.memory = V4L2_MEMORY_MMAP;
		vbuf.index  = i;

		if (-1 == ioctl(fd, VIDIOC_QUERYBUF, &vbuf)) {
			printf("%s-->VIDIOC_QUERYBUF err %m", __func__);
			return -errno;
		}

		buffers[i].length = vbuf.length;

		buffers[i].start = mmap(NULL, vbuf.length, PROT_READ | PROT_WRITE, 
							MAP_SHARED, fd, vbuf.m.offset);

		if (MAP_FAILED == buffers[i].start) {
			printf("%s-->v4l stream buffer mmap err %m", __func__);
			return -errno;
		}

	}
	
	*buf = buffers;

	return 0;
}

int v4l2_push_stream_buffer (int fd, int buf_count)
{
	int i;

	for (i = 0; i < buf_count; i++) {

		struct v4l2_buffer vbuf = {};

		vbuf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		vbuf.memory = V4L2_MEMORY_MMAP;
		vbuf.index  = i;

		if (-1 == ioctl(fd, VIDIOC_QBUF, &vbuf)) {
			printf("%s-->VIDOC_QBUF err %m", __func__);
			return -errno;
		}
	}

	return 0;

}

int v4l2_req_data_buffer(struct format *fmt)
{
	unsigned char *buf = NULL;

	buf = calloc(1, fmt->size);
	if (NULL == buf) {
		printf("%s-->v4l capture data buffer malloc err %m", __func__);
		return -errno;
	}

	fmt->vaddr = buf;

	return 0;

}

int v4l2_init_stream (int fd, int *buf_count, struct format *fmt, struct buffer **buf)
{
	int rval;


	/* 4.1 申请缓冲区 */

	rval = v4l2_req_stream_buf(fd, buf_count);	
	if (rval < 0)
		return rval;


	/* 4.2 建立内存映射 */

	rval = v4l2_map_stream_buffer(fd, *buf_count, buf);
	if (rval < 0)
		return rval;
	
	

	/* 4.3 将缓冲区放进队列 */

	rval = v4l2_push_stream_buffer(fd, *buf_count);
	if (rval < 0)
		return rval;


	/* 4.4 申请临时缓冲区：用于存放捕获到的视频数据 */

	rval = v4l2_req_data_buffer(fmt);
	if (rval < 0)
		return rval;

	printf("v4l2 device init succeed ...\n");

	return 0;
}



int v4l2_get_stream_data(int fd, int buf_count, unsigned char *vaddr, struct buffer *buf)
{
	
	struct v4l2_buffer vbuf = {};

	assert(NULL != vaddr);

	vbuf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	vbuf.memory = V4L2_MEMORY_MMAP;

	if (-1 == ioctl(fd, VIDIOC_DQBUF, &vbuf)) {
		printf("%s-->VIDIOC_DQBUF err %m", __func__);
		return -errno;
	}

	if (vbuf.index > buf_count) {
		printf("%s-->buffer index invalid, \n", __func__);
		return -1;
	}


	printf("active buffer index : %d  \n", vbuf.index);

	memcpy(vaddr, buf[vbuf.index].start, buf[vbuf.index].length);

	if (-1 == ioctl(fd, VIDIOC_QBUF, &vbuf)) {
		printf("%s-->VIDIOC_QBUF err %m", __func__);
		return -errno;
	}


	return 0;
}



void v4l2_free_fb (int buf_count, struct format *fmt, struct buffer *buf)
{
	int i, rval;


	if (buf) {
		for (i = 0; i < buf_count; i++) {
			rval = munmap(buf[i].start, buf[i].length);
			if (rval < 0)
				printf("%s-->v4l buf munmap err %m", __func__);
		}

		free(buf);		
	}

	if (fmt) {

		if (fmt->vaddr)
			free(fmt->vaddr);

		fmt->vaddr = NULL;
		free(fmt);
	}

	return;
}























