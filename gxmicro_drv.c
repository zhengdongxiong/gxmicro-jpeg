// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * GXMicro JPEG Controller Driver
 *
 * Copyright (C) 2022 GXMicro (ShangHai) Corp.
 *
 * Author:
 * 	Zheng DongXiong <zhengdongxiong@gxmicro.cn>
 */
#include <linux/platform_device.h>

#include "gxmicro_jpeg.h"

/* ****************************** Platform ****************************** */

static int gxmicro_plat_init(struct gxmicro_jpeg_dev *gdev)
{
	struct platform_device *pdev = to_platform_device(gdev->dev);

	gdev->mem = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(gdev->mem))
		return PTR_ERR(gdev->mem);

	/* Reserved: clk ? mem reserved ? dma ? */

	return 0;
}

static void gxmicro_plat_fini(struct gxmicro_jpeg_dev *gdev)
{

}

/* ****************************** V4L2 ****************************** */

static int gxmicro_v4l2_init(struct gxmicro_jpeg_dev *gdev)
{
	int ret;

	mutex_init(&gdev->vlock);
	spin_lock_init(&gdev->buf_lock);
	INIT_LIST_HEAD(&gdev->buffers);

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

static void gxmicro_v4l2_fini(struct gxmicro_jpeg_dev *gdev)
{
	gxmicro_video_fini(gdev);

	gxmicro_vb2_fini(gdev);

	gxmicro_ctrls_fini(gdev);

	v4l2_device_unregister(&gdev->v4l2);
}

/* ****************************** Platform Probe & Remove ****************************** */

static int gxmicro_jpeg_probe(struct platform_device *pdev)
{
	struct gxmicro_jpeg_dev *gdev;
	int ret;

	gdev = devm_kzalloc(&pdev->dev, sizeof(struct gxmicro_jpeg_dev), GFP_KERNEL);
	if (!gdev)
		return -ENOMEM;

	gdev->dev = &pdev->dev;
	platform_set_drvdata(pdev, gdev);

	ret = gxmicro_plat_init(gdev);
	if (ret)
		goto err_plat_init;

	ret = gxmicro_v4l2_init(gdev);
	if (ret)
		goto err_v4l2_init;

	/* Reserved */

	return 0;

err_v4l2_init:
err_plat_init:
	return ret;
}

static int gxmicro_jpeg_remove(struct platform_device *pdev)
{
	struct gxmicro_jpeg_dev *gdev = platform_get_drvdata(pdev);

	/* Reserved */

	gxmicro_v4l2_fini(gdev);

	gxmicro_plat_fini(gdev);

	return 0;
}

static const struct of_device_id gxmicro_jpeg_of_match[] = {
	{ .compatible = "", },
	{ /* END OF LIST */ }
};
MODULE_DEVICE_TABLE(of, gxmicro_jpeg_of_match);

static struct platform_driver gxmicro_jpeg_drv = {
	.driver = {
		.name = DRVNAME,
		.of_match_table = gxmicro_jpeg_of_match,
	},
	.probe = gxmicro_jpeg_probe,
	.remove = gxmicro_jpeg_remove,
};
module_platform_driver(gxmicro_jpeg_drv);

MODULE_DESCRIPTION("GXMicro JPEG Controller Driver");
MODULE_AUTHOR("Zheng DongXiong <zhengdongxiong@gxmicro.cn>");
MODULE_VERSION("v1.0");
MODULE_LICENSE("GPL v2");
