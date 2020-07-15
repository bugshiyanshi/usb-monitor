#include <linux/videodev2.h>

#include "kms_ops.h"


struct format* kms_get_format (struct kms_config *kms)
{
	return kms->fmt;
}

int kms_set (int fd, struct kms_config *kms)
{
	int rval;
	
	/* 保存原来的KMS配置 */
	kms->old_crtc = drmModeGetCrtc(fd, kms->crtc_id);
	
	rval = drmModeSetCrtc(fd, kms->crtc_id, kms->fb_id, 0, 0,
								&kms->conn_id, 1, &kms->mode);
	if (rval < 0){
		printf("cannot set CRTC for connector:%m:%d\n", rval);
		return rval;
	}

	printf("set crtc succeed\n");
		
	return 0;
}

int kms_config_init (struct kms_config **kms)
{
	struct kms_config *pkms;
	
	pkms = (struct kms_config*)malloc(sizeof(*pkms));
	if (NULL == pkms) {
		printf("%s-->kms malloc err: %m\n", __func__);
		return -errno;
	}
	
	*kms = pkms;
	
	return 0;
}

int kms_format_init (struct format **fmt)
{
	struct format *format = NULL;
	
	format = (struct format*)malloc(sizeof(*format));
	if (NULL == format) {
		printf("%s-->kms format malloc err: %m\n", __func__);
		return -errno;
	}
	
	memset(format, 0, sizeof(*format));
	
	*fmt = format;
	
	return 0;
}

void kms_bind_crtc (drmModeConnector *conn, int crtc_id, struct kms_config *kms)
{
	kms->crtc_id = crtc_id;
	kms->conn_id   = conn->connector_id;
	kms->encoder_id = conn->encoder_id;
	
	memcpy(&kms->mode, &conn->modes[0], sizeof(kms->mode));
	
	kms->fmt->width  = conn->modes[0].hdisplay;
	kms->fmt->height = conn->modes[0].vdisplay;
	kms->fmt->pix = V4L2_PIX_FMT_RGB32;
	kms->fmt->bpp = 32;
	kms->fmt->line_size = kms->fmt->width * kms->fmt->bpp/8;
	kms->fmt->size   = kms->fmt->line_size * kms->fmt->height;
	
	return;
}

int kms_find_crtc (int fd, drmModeRes *res, drmModeConnector *conn, struct kms_config *kms)
{
	drmModeEncoder *encoder;
	
	int i, j;
	
	/* 在有效的编码器中，查找适配的'encoder+crtc' */
	for (i = 0; i < conn->count_encoders; i++) {
		encoder = drmModeGetEncoder(fd, conn->encoders[i]);
		if (!encoder)
			continue;

		for (j = 0; j < res->count_crtcs; j++) {

			/* 判断该crtc是否与encoder适配 */
			if (!(encoder->possible_crtcs & (1 << j)))
				continue;

				printf("crtc is %d \n", encoder->crtc_id);
				
				/* 5. 绑定'crtc + encoder + connector' */
				kms_bind_crtc(conn, encoder->crtc_id, kms);

				drmModeFreeEncoder(encoder);
				return 0;
		}
			drmModeFreeEncoder(encoder);
			encoder = NULL;
	}
	
	printf("cannot find suitable CRTC for connector ...\n");
	
	return -1;
}

int kms_find_encoder (int fd, drmModeRes *res, drmModeConnector *conn, struct kms_config *kms)
{
	drmModeEncoder *encoder;
	
	/* 判断当前已有绑定的'encoder+crtc' */
	if (conn->encoder_id)
		encoder = drmModeGetEncoder(fd, conn->encoder_id);
	else
		encoder = NULL;
		
	if (encoder) {
		if (encoder->crtc_id) {
			/* 5. 绑定'crtc + encoder + connector' */
			kms_bind_crtc(conn, encoder->crtc_id, kms);
			drmModeFreeEncoder(encoder);
			return 0;
		}

		drmModeFreeEncoder(encoder);
		encoder = NULL;
	}
	
	/* 4. 查找与'连接器(connector)+编码器(encoder)'适配的CRTC */
	return kms_find_crtc(fd, res, conn, kms);
}

int kms_fb_mmap (int fd, struct kms_config *kms)
{
	struct drm_mode_map_dumb mreq;
	int rval;
	
	mreq.handle = kms->fb_handle;
	
	/* 准备内存映射:获取dumb缓冲区的mmap偏移量 */
	rval = drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq);
	if (rval) {
		printf("%s-->MODE_MAP_DUMBr error:%m\n", __func__);
		return -errno;
	}
	
	/* 内存映射 */
	kms->fmt->vaddr = mmap(0, kms->fmt->size, PROT_READ | PROT_WRITE,
										MAP_SHARED, fd, mreq.offset);
										
	if (MAP_FAILED == kms->fmt->vaddr) {
		printf("%s-->cannot mmap dumb buffer error:%m\n", __func__);
		return -errno;
	}
	
	memset(kms->fmt->vaddr, 0, kms->fmt->size);
	printf("kms framebuffer mmap succeed.....\n");
	
	return 0;
}

int kms_crtc_fb (int fd, struct kms_config *kms)
{
	struct drm_mode_create_dumb creq;
	struct drm_mode_destroy_dumb dreq;
	

	int rval;

	/* 创建dumb buffer */
	memset(&creq, 0, sizeof(creq));
	creq.width  = kms->fmt->width;
	creq.height = kms->fmt->height;
	creq.bpp =  kms->fmt->bpp;
	
	rval = drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq);
	if (rval < 0) {
		printf("%s-->create dumb buffer error:%m\n", __func__);
		return -errno;
	}

	kms->fb_handle = creq.handle;
	
	/* 创建缓冲区framebuffer 对象 */
	rval = drmModeAddFB(fd, kms->fmt->width, kms->fmt->height, 24, kms->fmt->bpp,
						kms->fmt->line_size, kms->fb_handle, &kms->fb_id);
	if (rval < 0) {
		printf("%s-->create framebuffer error: %m\n", __func__);
		return rval;
	}
	
	/* 内存映射 */

	return kms_fb_mmap(fd, kms);
}

int kms_find_connector (int fd, drmModeRes *res, struct kms_config *kms)
{
	drmModeConnector *conn;
	int i, rval;
	
	for (i = 0; i < res->count_connectors; i++) {

		conn = drmModeGetConnector(fd, res->connectors[i]);
		if (!conn)
			continue;

		/* 判断该'该连接器是否连接'，否则忽略 
		 * 判断是否有有效的模式，没有也忽略
		 */
		if ((DRM_MODE_CONNECTED != conn->connection) || (0 == conn->count_modes)) {
			drmModeFreeConnector(conn);
			continue;
		}
		/*
		 * 3.查找编码器
		 */
		rval = kms_find_encoder(fd, res, conn, kms);
		if (rval  < 0) {
			drmModeFreeConnector(conn);
			continue;
		}
		
		/* 6.为crtc申请扫描缓冲区 */
		rval = kms_crtc_fb(fd, kms);
		if (rval < 0) {
			drmModeFreeConnector(conn);
			memset(kms, 0, sizeof(*kms));
			continue;
		}
		
		printf("bind crtc:%d + encoder:%d + connector:%d.. \n",
					kms->crtc_id, kms->encoder_id, kms->conn_id);

		drmModeFreeConnector(conn);
		return 0;
	}
	
	return -1;	
}

int kms_get_res (int fd, struct kms_config *kms)
{
	drmModeRes *res;
	
	int rval;
	
	/* 1. 获取drm设备资源: 连接器、编码器、CRTC */
	res = drmModeGetResources(fd);
	if (!res) {
		printf("%s-->cannot get drm resources error:%m\n", __func__);
		return -errno;
	}
	
	/* 2. 查找连接器 */
	rval = kms_find_connector(fd, res, kms);
	
	drmModeFreeResources(res);
	
	return rval;
}

void kms_free_fb (int fd, struct kms_config *kms)
{
	struct drm_mode_destroy_dumb dreq;
	
	int rval;
	
	if (NULL == kms) {
		printf("kms config is NULL ...\n");
		return ;
	}
	
	if (kms->fmt) {

		/* 取消dumb缓冲区的内存映射 */
		if (kms->fmt->vaddr) {
			rval = munmap(kms->fmt->vaddr, kms->fmt->size);
			if (rval < 0)
				printf("%s-->munmap fb errno error:%m\n", __func__);
		}
		
		/* 释放format内存 */
		free(kms->fmt);
		kms->fmt = NULL;
	}
	
	/* 释放dumb缓冲区 */
	if (kms->fb_handle) {
		
		/* 销毁framebuffer对象 */
		if (kms->fb_id > 0)
			drmModeRmFB(fd, kms->fb_id);
		
		dreq.handle = kms->fb_handle;
		drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
	}
	
	/* 释放KMS内存 */
	free(kms);
	
	return ;
}

int kms_init (int fd, struct kms_config **kms)
{
	struct kms_config *pkms = NULL;
	int rval;
	
	/* 申请KMS的内存空间 */
	rval = kms_config_init(&pkms);
	if (rval < 0)
		return rval;
	
	/* 申请format的内存空间：用于存放图片参数 */
	rval = kms_format_init(&pkms->fmt);
	if (rval < 0)
		return rval;

	/* 1. 获取drm设备资源: 连接器、编码器、CRTC */
	
	rval = kms_get_res(fd, pkms);
	if (rval < 0)
		goto out;
	
	*kms = pkms;
	
	return 0;
out:
	kms_free_fb(fd, pkms);
	*kms = NULL;
	return -1;
}



