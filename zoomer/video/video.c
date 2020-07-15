#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <poll.h>

/**** v4l2 header ****/
#include <linux/videodev2.h>

#include "video.h"


int video_transcode(struct video *dev)
{
	unsigned int pix = dev->v4l2_fmt->pix;

	switch (pix) {

	case V4L2_PIX_FMT_YUYV:
		dev->coder->yuyv422torgb32(dev->v4l2_fmt, dev->coder->buf);
		break;

	default:
		printf("can't find transcode fun ....\n");
		return -1;
	}

	return 0;

}

void video_is_zoom(struct video *dev, unsigned char *src_buf)
{
	if (!dev->zoomer) {	
		dev->video_buf = src_buf;
		return;
	}


	dev->zoomer->zoom_img(src_buf, dev->zoomer->buf, dev->v4l2_fmt, dev->drm_fmt);
	
	dev->video_buf = dev->zoomer->buf;

	return;
}

int video_data_ops(struct video *dev)
{

	int rval;
	

	/* 2. 获取usb摄像头采集的视频数据 */

	rval = dev->v4l2->v4l2_get_stream(dev->v4l2);
	if (rval < 0)
		return rval;


	/* 3. 判断视频数据是否需进行转码 */

	if (!dev->coder) {
		video_is_zoom(dev, dev->v4l2_fmt->vaddr);
		goto out;
	}


	rval = video_transcode(dev);
	if (rval < 0)
		return rval;


	/* 4. 判断是否需要进行缩放 */
	video_is_zoom(dev, dev->coder->buf);



out:
	/* 5. 送显 */

	memcpy(dev->drm_fmt->vaddr, dev->video_buf, dev->drm_fmt->size);

	return 0;

}

int video_display(struct video *dev)
{
	int rval;

	int count = 299;

	/* 1. 检测是否有数据可读 */

	while (1) {

		struct pollfd fds;

		fds.fd = dev->v4l2->fd;
		fds.events = POLLIN;

		if (-1 == (rval = poll(&fds, 1, -1))) {
			printf("%s poll err: %m \n", __func__);
			return -errno;
		}


		if (rval > 0) {
			rval = video_data_ops(dev);
			if (rval < 0)
				return rval;

			count--;
		}

		if (count < 0)
			break;

	}

	return 0;
}


int  video_start(struct video *dev)
{
	int rval;

	/* 1. 启动usb摄像头采集 */

	rval = dev->v4l2->v4l2_start(dev->v4l2->fd);
	if (rval < 0)
		return rval;

	/* 2. 获取usb摄像头数据并送显 */

	return video_display(dev);
}

int  video_stop(struct video *dev)
{

	return dev->v4l2->v4l2_stop(dev->v4l2->fd);
}

void video_close(struct video *dev)
{
	/* 1. 关闭DRM设备 */

	if (dev->drm) {
		dev->drm->drm_close(dev->drm);
		dev->drm_fmt = NULL;
		dev->drm = NULL;
	}


	/* 2. 关闭V4L2设备 */

	if (dev->v4l2) {
		dev->v4l2->v4l2_close(dev->v4l2);
		dev->v4l2_fmt = NULL;
		dev->v4l2 = NULL;
	}


	/* 3. 关闭转码器 */

	if (dev->coder) {
		dev->coder->transcoder_close(dev->coder);
		dev->coder = NULL;
	}

	/* 4. 关闭缩放器 */

	if (dev->zoomer) {
		dev->zoomer->zoom_close(dev->zoomer);
		dev->zoomer = NULL;
	}

	return;
}


int  video_open(const char *card, const char *video, struct video *dev)
{
	int rval;

	/* 1.初始化DRM设备 */

	dev->drm_fmt = dev->drm->drm_init(card, dev->drm);
	if (NULL == dev->drm_fmt)
		goto out;

	/* 2.初始化V4L2设备 */

	dev->v4l2_fmt = dev->v4l2->v4l2_init(video, dev->v4l2);
	if (NULL == dev->v4l2_fmt)
		goto out;

	/* 3.转码器注册 */

	if (dev->drm_fmt->pix != dev->v4l2_fmt->pix) {

		rval = register_transcoder(&dev->coder, dev->v4l2_fmt->size);
		if (rval < 0)
			goto out;
	}	


	/* 4.缩放器注册 */

	if ((dev->drm_fmt->width < dev->v4l2_fmt->width)
		|| (dev->drm_fmt->height < dev->v4l2_fmt->height)) {

		rval = register_zoomer(&dev->zoomer, dev->drm_fmt->size);
		if (rval < 0)
			goto out;
	}

	printf("video device init succeed ..\n");

	return 0;

out:
	video_close(dev);

	return -1;
}



struct video vdev = {
	.video_buf = NULL,
	.drm_fmt   = NULL,
	.v4l2_fmt  = NULL,
	.drm    = NULL,
	.v4l2   = NULL,
	.zoomer = NULL,
	.coder  = NULL,
	.video_open  = video_open,
	.video_close = video_close,
	.video_start = video_start,
	.video_stop  = video_stop,

};


int register_video(struct video **dev)
{
	int rval;

	/* 1.DRM设备注册 */

	rval = register_drm(&vdev.drm);
	if (rval < 0)
		goto out;

	/* 2.V4L2设备注册 */

	rval = register_v4l2(&vdev.v4l2);
	if (rval < 0)
		goto out;

	*dev = &vdev;

	return 0;

out:
	*dev = NULL;

	return -1;
}

