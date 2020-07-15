#ifndef __DRM_H__
#define __DRM_H__


#include <format.h>

#include "kms_config.h"


struct drm_dev {
	int fd;
	struct kms_config *kms;
	struct format* (*drm_init)(const char *card, struct drm_dev *dev);
	void (*drm_close) (struct drm_dev *dev);

};


int register_drm(struct drm_dev **dev);

#endif

