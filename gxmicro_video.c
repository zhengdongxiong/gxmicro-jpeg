// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * GXMicro V4L2 Video Device
 *
 * Copyright (C) 2022 GXMicro (ShangHai) Corp.
 *
 * Author:
 * 	Zheng DongXiong <zhengdongxiong@gxmicro.cn>
 */
#include <media/videobuf2-v4l2.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-dv-timings.h>

#include "gxmicro_jpeg.h"

/* Reserved, 设置默认 bs len max 寄存器 ? */

static int gxmicro_vidioc_querycap(struct file *file, void *fh, struct v4l2_capability *cap)
{
	struct gxmicro_jpeg_dev *gdev = video_drvdata(file);

	strscpy(cap->driver, DRVNAME, sizeof(cap->driver));
	strscpy(cap->card, "Gxmicro JPEG", sizeof(cap->card));
	snprintf(cap->bus_info, sizeof(cap->bus_info), "platform:%s", dev_name(gdev->dev));

	return 0;
}

static int gxmicro_vidioc_enum_fmt(struct file *file, void *fh, struct v4l2_fmtdesc *f)
{
	if (f->index)
		return -EINVAL;

	f->pixelformat = V4L2_PIX_FMT_JPEG;

	return 0;
}

static int gxmicro_vidioc_g_fmt_vid_cap(struct file *file, void *fh, struct v4l2_format *f)
{
	struct gxmicro_jpeg_dev *gdev = video_drvdata(file);
	uint32_t jconf, width, height;
	uint8_t bpp;
	uint32_t bytesperline, sizeimage;

	width = gxmicro_read(gdev, JPEG_WIDTH);
	height = gxmicro_read(gdev, JPEG_HEIGHT);
	jconf = gxmicro_read(gdev, JPEG_CONF);

	switch (jconf & JPEG_ENC_FORMAT_MASK) {
	case JPEG_ENC_RBG565:
		bpp = JPEG_RGB565_BPP;
		break;
	case JPEG_ENC_XRGB888:
		bpp = JPEG_XRGB888_BPP;
		break;
	}

	bytesperline = width * bpp;
	sizeimage = bytesperline * height;

	f->fmt.pix.width = width;
	f->fmt.pix.height = height;
	f->fmt.pix.pixelformat = V4L2_PIX_FMT_JPEG;
	f->fmt.pix.field = V4L2_FIELD_NONE;
	f->fmt.pix.bytesperline = bytesperline;
	f->fmt.pix.sizeimage = sizeimage;
	f->fmt.pix.colorspace = V4L2_COLORSPACE_JPEG;
	f->fmt.pix.flags = 0;		/* videodev2.h, line: 1667, JPEG: always power on */
	f->fmt.pix.ycbcr_enc = V4L2_YCBCR_ENC_601;
	f->fmt.pix.quantization = V4L2_QUANTIZATION_FULL_RANGE;
	f->fmt.pix.xfer_func = V4L2_XFER_FUNC_SRGB;

	return 0;
}

static int gxmicro_vidioc_s_fmt_vid_cap(struct file *file, void *fh, struct v4l2_format *f)
{
	return 0;
}

static int gxmicro_vidioc_try_fmt_vid_cap(struct file *file, void *fh, struct v4l2_format *f)
{
	return 0;
}

static int gxmicro_vidioc_enum_input(struct file *file, void *fh, struct v4l2_input *inp)
{

	if (inp->index)
		return -EINVAL;

	strscpy(inp->name, "Host VGA Capture", sizeof(inp->name));
	inp->type = V4L2_INPUT_TYPE_CAMERA;
	inp->capabilities = V4L2_IN_CAP_DV_TIMINGS;

	return 0;
}

static int gxmicro_vidioc_g_input(struct file *file, void *fh, unsigned int *input)
{
	*input = 0;

	return 0;
}

static int gxmicro_vidioc_s_input(struct file *file, void *fh, unsigned int input)
{
	return input == 0 ? 0 : -EINVAL;
}

static int gxmicro_vidioc_g_parm(struct file *file, void *fh, struct v4l2_streamparm *sp)
{
	sp->parm.capture.capability = 0;
	sp->parm.capture.readbuffers = JPEG_BUFFERS;
	sp->parm.capture.timeperframe.numerator = 1;
	sp->parm.capture.timeperframe.denominator = JPEG_RATE;

	return 0;
}

static int gxmicro_vidioc_s_parm(struct file *file, void *fh, struct v4l2_streamparm *sp)
{
	return 0;
}

static int gxmicro_vidioc_enum_framesizes(struct file *file, void *fh, struct v4l2_frmsizeenum *fsize)
{
	struct gxmicro_jpeg_dev *gdev = video_drvdata(file);
	uint32_t width, height;

	if (fsize->index || (fsize->pixel_format != V4L2_PIX_FMT_JPEG))
		return -EINVAL;

	width = gxmicro_read(gdev, JPEG_WIDTH);
	height = gxmicro_read(gdev, JPEG_HEIGHT);

	fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
	fsize->discrete.width = width;
	fsize->discrete.height = height;

	return 0;
}

static int gxmicro_vidioc_enum_frameintervals(struct file *file, void *fh, struct v4l2_frmivalenum *fival)
{
	struct gxmicro_jpeg_dev *gdev = video_drvdata(file);
	uint32_t width, height;

	if (fival->index || (fival->pixel_format != V4L2_PIX_FMT_JPEG))
		return -EINVAL;

	width = gxmicro_read(gdev, JPEG_WIDTH);
	height = gxmicro_read(gdev, JPEG_HEIGHT);

	if (width != fival->width || height != fival->height)
		return -EINVAL;

	fival->type = V4L2_FRMIVAL_TYPE_DISCRETE;	/* 仅支持 30 HZ */
	fival->discrete.denominator = JPEG_RATE;
	fival->discrete.numerator = 1;	/* frame_interval [s] = 1 / 30, frame_rate = 1 / frame_interval */

	return 0;
}

static const struct v4l2_dv_timings_cap gxmicro_dv_timings_cap = {
	.type = V4L2_DV_BT_656_1120,
	.bt = {
		.min_width = JPEG_MIN_WIDTH,
		.max_width = JPEG_MAX_WIDTH,
		.min_height = JPEG_MIN_HEIGHT,
		.max_height = JPEG_MAX_HEIGHT,
		.min_pixelclock = JPEG_MIN_PCLK,
		.max_pixelclock = JPEG_MAX_PCLK,
		.standards = V4L2_DV_BT_STD_CEA861 | V4L2_DV_BT_STD_DMT,	/* Unknown */
		.capabilities = V4L2_DV_BT_CAP_PROGRESSIVE | V4L2_DV_BT_CAP_CUSTOM,	/* Unknown */
	},
};

static int gxmicro_vidioc_s_dv_timings(struct file *file, void *fh, struct v4l2_dv_timings *timings)
{
	return 0;
}

static int gxmicro_vidioc_g_dv_timings(struct file *file, void *fh, struct v4l2_dv_timings *timings)
{
	struct gxmicro_jpeg_dev *gdev = video_drvdata(file);
	uint32_t width, height;

	width = gxmicro_read(gdev, JPEG_WIDTH);
	height = gxmicro_read(gdev, JPEG_HEIGHT);

	timings->type = V4L2_DV_BT_656_1120;
	timings->bt.width = width;
	timings->bt.height = height;

	return 0;
}

static int gxmicro_vidioc_query_dv_timings(struct file *file, void *fh, struct v4l2_dv_timings *timings)
{
	struct gxmicro_jpeg_dev *gdev = video_drvdata(file);
	uint32_t width, height;

	width = gxmicro_read(gdev, JPEG_WIDTH);
	height = gxmicro_read(gdev, JPEG_HEIGHT);

	timings->type = V4L2_DV_BT_656_1120;
	timings->bt.width = width;
	timings->bt.height = height;

	return 0;
}

static int gxmicro_vidioc_enum_dv_timings(struct file *file, void *fh, struct v4l2_enum_dv_timings *timings)
{
	return v4l2_enum_dv_timings_cap(timings, &gxmicro_dv_timings_cap, NULL, NULL);
}

static int gxmicro_vidioc_dv_timings_cap(struct file *file, void *fh, struct v4l2_dv_timings_cap *cap)
{
	*cap = gxmicro_dv_timings_cap;

	return 0;
}

static const struct v4l2_file_operations gxmicro_v4l2_fops = {
	.owner = THIS_MODULE,
	.read = vb2_fop_read,
	.poll = vb2_fop_poll,
	.unlocked_ioctl = video_ioctl2,
	.mmap = vb2_fop_mmap,
	.open = v4l2_fh_open,	/* Reserved，需要 open 时初始化某些值, 自定义 open 函数 */
	.release = vb2_fop_release,	/* Reserved, open 时初始化某些值 自定义 release 函数 */
};

static const struct v4l2_ioctl_ops gxmicro_v4l2_ioctl_ops = {
	/* VIDIOC_QUERYCAP */
	.vidioc_querycap = gxmicro_vidioc_querycap,

	/* VIDIOC_ENUM_FMT */
	.vidioc_enum_fmt_vid_cap = gxmicro_vidioc_enum_fmt,

	/* VIDIOC_G_FMT */
	.vidioc_g_fmt_vid_cap = gxmicro_vidioc_g_fmt_vid_cap,

	/* VIDIOC_S_FMT */
	.vidioc_s_fmt_vid_cap = gxmicro_vidioc_s_fmt_vid_cap,

	/* VIDIOC_TRY_FMT */
	.vidioc_try_fmt_vid_cap = gxmicro_vidioc_try_fmt_vid_cap,

	/* Videobuffer */
	.vidioc_reqbufs = vb2_ioctl_reqbufs,
	.vidioc_querybuf = vb2_ioctl_querybuf,
	.vidioc_qbuf = vb2_ioctl_qbuf,
	.vidioc_expbuf = vb2_ioctl_expbuf,
	.vidioc_dqbuf = vb2_ioctl_dqbuf,
	.vidioc_create_bufs = vb2_ioctl_create_bufs,
	.vidioc_prepare_buf = vb2_ioctl_prepare_buf,

	/* Stream */
	.vidioc_streamon = vb2_ioctl_streamon,
	.vidioc_streamoff = vb2_ioctl_streamoff,

	/* Input */
	.vidioc_enum_input = gxmicro_vidioc_enum_input,
	.vidioc_g_input = gxmicro_vidioc_g_input,
	.vidioc_s_input = gxmicro_vidioc_s_input,

	/* Stream type-dependent parameter */
	.vidioc_g_parm = gxmicro_vidioc_g_parm,
	.vidioc_s_parm = gxmicro_vidioc_s_parm,	/* 仅支持 30 HZ */

	/* Log status */
	.vidioc_log_status = v4l2_ctrl_log_status,

	/* Framebuffer */
	.vidioc_enum_framesizes = gxmicro_vidioc_enum_framesizes,
	.vidioc_enum_frameintervals = gxmicro_vidioc_enum_frameintervals,

	/* JPEG: 不支持 dv timgings 相关, 但 ikvm 需要使用 */
	/* DV Timings */
	.vidioc_s_dv_timings = gxmicro_vidioc_s_dv_timings,
	.vidioc_g_dv_timings = gxmicro_vidioc_g_dv_timings,
	.vidioc_query_dv_timings = gxmicro_vidioc_query_dv_timings,
	.vidioc_enum_dv_timings = gxmicro_vidioc_enum_dv_timings,
	.vidioc_dv_timings_cap = gxmicro_vidioc_dv_timings_cap,
};

int gxmicro_video_init(struct gxmicro_jpeg_dev *gdev)
{
	struct video_device *vdev = &gdev->vdev;
	int ret;

	vdev->fops = &gxmicro_v4l2_fops;
	vdev->device_caps = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_READWRITE | V4L2_CAP_STREAMING;
	vdev->v4l2_dev = &gdev->v4l2;
	vdev->queue = &gdev->vbq;
	strscpy(vdev->name, DRVNAME, sizeof(vdev->name));
	vdev->vfl_dir = VFL_DIR_RX;
	vdev->release = video_device_release_empty;
	vdev->ioctl_ops = &gxmicro_v4l2_ioctl_ops;
	vdev->lock = &gdev->vlock;

	video_set_drvdata(vdev, gdev);

	ret = video_register_device(vdev, VFL_TYPE_VIDEO, 0);
	if (ret) {
		dev_err(gdev->dev, "Failed to register Video device\n");
		return ret;
	}

	return 0;
}

void gxmicro_video_fini(struct gxmicro_jpeg_dev *gdev)
{
	struct video_device *vdev = &gdev->vdev;

	vb2_video_unregister_device(vdev);	/* videobuf2-v4l2.h line: 353 */
#if 0
	video_unregister_device(vdev);
#endif
}
