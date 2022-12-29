// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * GXMicro V4L2 Controls
 *
 * Copyright (C) 2022 GXMicro (ShangHai) Corp.
 *
 * Author:
 * 	Zheng DongXiong <zhengdongxiong@gxmicro.cn>
 */
#include "gxmicro_jpeg.h"

static inline void gxmicro_jpeg_set_quality(struct gxmicro_jpeg_dev *gdev, uint32_t val)
{
	gxmicro_write(gdev, JPEG_ENC_QP, val);
}

static void gxmicro_jpeg_set_subsampling(struct gxmicro_jpeg_dev *gdev, uint32_t val)
{
	uint32_t jconf = 0;

	jconf = gxmicro_read(gdev, JPEG_CONF);
	jconf &= ~JPEG_BS_FORMAT_MASK;

	switch (val) {
	case V4L2_JPEG_CHROMA_SUBSAMPLING_444:
		jconf |= JEPG_BS_YUV444;
		break;
	case V4L2_JPEG_CHROMA_SUBSAMPLING_420:
		jconf |= JEPG_BS_YUV420;
		break;
	}

	gxmicro_write(gdev, JPEG_CONF, jconf);
}

static int gxmicro_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct gxmicro_jpeg_dev *gdev = container_of(ctrl->handler, struct gxmicro_jpeg_dev, hdl);

	switch (ctrl->id) {
	case V4L2_CID_JPEG_COMPRESSION_QUALITY:
		gxmicro_jpeg_set_quality(gdev, ctrl->val);
		break;
	case V4L2_CID_JPEG_CHROMA_SUBSAMPLING:
		gxmicro_jpeg_set_subsampling(gdev, ctrl->val);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static const struct v4l2_ctrl_ops gxmicro_ctrl_ops = {
	.s_ctrl = gxmicro_s_ctrl,
};

int gxmicro_ctrls_init(struct gxmicro_jpeg_dev *gdev)
{
	struct device *dev = gdev->dev;
	struct v4l2_device *v4l2 = &gdev->v4l2;
	struct v4l2_ctrl_handler *hdl = &gdev->hdl;
	int ret;

	ret = v4l2_ctrl_handler_init(hdl, 2);
	if (ret) {
		dev_err(dev, "Failed to init Control Handler\n");
		return ret;
	}

	v4l2_ctrl_new_std(hdl, &gxmicro_ctrl_ops, V4L2_CID_JPEG_COMPRESSION_QUALITY,
			JPEG_QP_MIN, JPEG_QP_MAX, 1, JPEG_QP_DEF);

	v4l2_ctrl_new_std_menu(hdl, &gxmicro_ctrl_ops, V4L2_CID_JPEG_CHROMA_SUBSAMPLING,
			V4L2_JPEG_CHROMA_SUBSAMPLING_420, JPEG_CHROMA_SUBSAMPLING_MASK, V4L2_JPEG_CHROMA_SUBSAMPLING_444);

	ret = hdl->error;
	if (ret) {
		dev_err(dev, "Failed to add Controls\n");
		goto err_hdl_error;
	}

	ret = v4l2_ctrl_handler_setup(hdl);
	if (ret) {
		dev_err(dev, "Failed to init Controls values\n");
		goto err_hdl_error;
	}

	v4l2->ctrl_handler = hdl;

	return 0;

err_hdl_error:
	v4l2_ctrl_handler_free(hdl);
	return ret;
}

void gxmicro_ctrls_fini(struct gxmicro_jpeg_dev *gdev)
{
	struct v4l2_ctrl_handler *hdl = &gdev->hdl;

	v4l2_ctrl_handler_free(hdl);
}
