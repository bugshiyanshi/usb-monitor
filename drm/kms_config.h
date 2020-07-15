#ifndef __KMS_CONFIG_H__
#define __KMS_CONFIG_H__

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <assert.h>
#include <errno.h>

#include <xf86drm.h>
#include <xf86drmMode.h>

#include <format.h>


struct kms_config {
	struct format *fmt;	// 图片的格式
	unsigned int fb_id;		// 扫描缓冲区的缓冲区对象ID
	unsigned int fb_handle;	// 缓冲区对象的引用句柄
	unsigned int conn_id;	// 与缓冲区绑定的连接器ID
	unsigned int crtc_id;	// 与连接器绑定的CRTC ID
	unsigned int encoder_id;	// 与连接器绑定的编码器 ID
	drmModeCrtc *old_crtc;	// 更改前CRTC配置，退出时恢复原模式
	drmModeModeInfo mode;	// 使用的显示模式配置(时序有关)
};


#endif

