#include <stdio.h>
#include <stdlib.h>


#include "drm.h"
#include "kms_ops.h"



void drm_close (struct drm_dev *dev)
{
	if (dev->fd < 0)
		return ;
	
	if (dev->kms)
		kms_free_fb(dev->fd, dev->kms);
	
	dev->kms = NULL;
	
	close(dev->fd);
	
	dev->fd = -1;
	
	return ;
}

int drm_check_capability(int fd)
{
	uint64_t flags;
	
	if (drmGetCap(fd, DRM_CAP_DUMB_BUFFER, &flags) < 0 || !flags) {
		perror("this drm device does not support dumb buffer ");
		return -1;
	}
	
	return 0;
}

int drm_open(int *pfd, const char *card)
{
	int fd, rval;
	 
	assert(NULL != card);
	
	/*
	 * 1.1. 打开设备
	 */
	fd = open(card, O_RDWR);
	if (fd < 0) {
		printf("cannot open drm device '%s' \n", card);
		return fd;
	}
	
	/*
	 * 1.2. 检查DRM设备是否支持特性：DRM_CAP_DUMB_BUFFER
	 */
	 rval = drm_check_capability(fd);
	 if (rval < 0) {
		close(fd);
		return rval;
	 }

	*pfd = fd;
	
	printf("open drm device '%s'....\n", card);
	
	return 0;
}

struct format* drm_init (const char *card, struct drm_dev *dev)
{
	int fd, rval;
	
	/* 1. 打开DRM设备 */
	rval = drm_open(&fd, card);
	if (rval < 0)
			goto out;
	
	dev->fd = fd;
	
	/* 2. 初始化KMS参数 */
	rval = kms_init(dev->fd, &dev->kms);
	if (rval < 0)
		goto out;
		
	/* 3. KMS模式设置 */
	rval = kms_set(dev->fd, dev->kms);
	if (rval < 0)
		goto out;
	
	return kms_get_format(dev->kms);

out:
	drm_close(dev);
	return NULL;
}

struct drm_dev drm = {
	.fd  = -1,
	.kms = NULL,
	.drm_init  = drm_init,
	.drm_close = drm_close,
};


int register_drm(struct drm_dev **dev)
{
	*dev = &drm;


	return 0;
}
