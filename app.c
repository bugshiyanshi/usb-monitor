#include "video/video.h"


int main(void)
{
	struct video *video = NULL;
	
	int rval;

	/* 注册视频设备 */

	rval = register_video(&video);
	if (rval < 0)
		return -1;

	/* 打开视频设备 */

	rval = video->video_open("/dev/dri/card0", "/dev/video0", video);
	if (rval < 0)
		goto out;

	/* 启动video设备: 进行摄像头采集 */

	rval = video->video_start(video);
	if (rval < 0)
		goto out;

	/* 停止摄像头采集 */

	rval = video->video_stop(video);
	if (rval < 0)
		goto out;

	
	
	video->video_close(video);
	
	printf("exit app .......\n");

	return 0;

out:
	video->video_close(video);
	return -1;
}
