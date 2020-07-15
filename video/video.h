#ifndef __VIDEO_H__
#define __VIDEO_H__

#include <format.h>
#include <drm/drm.h>
#include <v4l2/v4l2.h>
#include <zoomer/zoomer.h>
#include <transcoder/transcoder.h>


struct video {
	unsigned char  *video_buf;
	struct format  *drm_fmt;
	struct format  *v4l2_fmt;
	struct drm_dev *drm;
	struct v4l2_dev *v4l2;
	struct zoomer   *zoomer;
	struct transcoder *coder;
	int  (*video_open)(const char *card, const char *video, struct video *dev);
	int  (*video_start)(struct video *dev);
	int  (*video_stop)(struct video *dev);
	void (*video_close)(struct video *dev);
};


int register_video(struct video **dev);

#endif

