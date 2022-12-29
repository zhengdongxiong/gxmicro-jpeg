// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * GXMicro V4L2
 *
 * Copyright (C) 2022 GXMicro (ShangHai) Corp.
 *
 * Author:
 * 	Zheng DongXiong <zhengdongxiong@gxmicro.cn>
 */
#include "gxmicro_jpeg.h"

int gxmicro_v4l2_init(struct gxmicro_jpeg_dev *gdev)
{
	int ret;

	mutex_init(&gdev->vlock);

	ret = v4l2_device_register(gdev->dev, &gdev->v4l2);
	if (ret) {
		dev_err(gdev->dev, "Failed to register V4L2 device\n");
		return ret;
	}

	ret = gxmicro_ctrls_init(gdev);
	if (ret)
		goto err_ctrls_init;

	ret = gxmicro_vb2_init(gdev);
	if (ret)
		goto err_vb2_init;

	ret = gxmicro_video_init(gdev);
	if (ret)
		goto err_video_init;

	return 0;

err_video_init:
	gxmicro_vb2_fini(gdev);
err_vb2_init:
	gxmicro_ctrls_fini(gdev);
err_ctrls_init:
	v4l2_device_unregister(&gdev->v4l2);
	return ret;
}

void gxmicro_v4l2_fini(struct gxmicro_jpeg_dev *gdev)
{
	gxmicro_video_fini(gdev);

	gxmicro_vb2_fini(gdev);

	gxmicro_ctrls_fini(gdev);

	v4l2_device_unregister(&gdev->v4l2);
}
