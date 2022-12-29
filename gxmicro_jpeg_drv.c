// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * GXMicro JPEG Controller Platform Driver
 *
 * Copyright (C) 2022 GXMicro (ShangHai) Corp.
 *
 * Author:
 * 	Zheng DongXiong <zhengdongxiong@gxmicro.cn>
 */
#include <linux/platform_device.h>

#include "gxmicro_jpeg.h"

static int gxmicro_jpeg_probe(struct platform_device *pdev)
{
	struct gxmicro_jpeg_dev *gdev;
	struct device *dev = &pdev->dev;
	int ret;

	gdev = devm_kzalloc(dev, sizeof(struct gxmicro_jpeg_dev), GFP_KERNEL);
	if (!gdev)
		return -ENOMEM;

	gdev->dev = dev;
	platform_set_drvdata(pdev, gdev);

	gdev->mem = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(gdev->mem)) {
		dev_err(dev, "Failed to map JPEG mem\n");
		return PTR_ERR(gdev->mem);
	}

	/* clk ? mem reserved ? dma ? irq ? */

	ret = gxmicro_v4l2_init(gdev);
	if (ret)
		goto err_v4l2_init;

	/* Reserved */

	return 0;

err_v4l2_init:
	return ret;
}

static int gxmicro_jpeg_remove(struct platform_device *pdev)
{
	struct gxmicro_jpeg_dev *gdev = platform_get_drvdata(pdev);

	/* Reserved */

	gxmicro_v4l2_fini(gdev);

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

MODULE_DESCRIPTION("GXMicro JPEG Driver");
MODULE_AUTHOR("Zheng DongXiong <zhengdongxiong@gxmicro.cn>");
MODULE_VERSION("v1.0");
MODULE_LICENSE("GPL v2");
