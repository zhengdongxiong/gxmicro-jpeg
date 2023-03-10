/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * GXMicro JPEG Controller
 *
 * Copyright (C) 2022 GXMicro (ShangHai) Corp.
 *
 * Author:
 * 	Zheng DongXiong <zhengdongxiong@gxmicro.cn>
 */
#ifndef __GXMICRO_JPEG_H__
#define __GXMICRO_JPEG_H__

#include <media/v4l2-device.h>
#include <media/v4l2-ctrls.h>
#include <media/videobuf2-core.h>

#define DRVNAME		"GXMicro-jpeg"

/* ****************************** JPEG Controller ****************************** */

#define JPEG_BASE			0x00670000

#define JPEG_OFFSET(offset)		(JPEG_BASE + offset)

#define JPEG_RATE			30
#define JPEG_MIN_WIDTH			640
#define JPEG_MIN_HEIGHT			480
#define JPEG_MIN_PCLK			12587500	/* 640 x 480 x 30Hz, pclk (640 x 480 60Hz) / 2 */
#define JPEG_MAX_WIDTH			1920
#define JPEG_MAX_HEIGHT			1080
#define JPEG_MAX_PCLK			74250000	/* 1920 x 1080 x 30Hz, pclk (1920 x 1080 60Hz) / 2 */
#define JPEG_BUFFERS			3		/* default 3, ikvm 默认申请3个buffer */

/* Registers offset for JPEG  */
#define JPEG_CTRL			JPEG_OFFSET(0x00)
#define JPEG_CONF			JPEG_OFFSET(0x04)
#define JPEG_WIDTH			JPEG_OFFSET(0x08)
#define JPEG_HEIGHT			JPEG_OFFSET(0x0C)
#define JPEG_ENC_QP			JPEG_OFFSET(0x10)
#define JPEG_FB_BASE			JPEG_OFFSET(0x14)
#define JPEG_BS_BASE			JPEG_OFFSET(0x18)
#define JPEG_BS_LENGTH			JPEG_OFFSET(0x1C)
#define JPEG_BS_LEN_MAX			JPEG_OFFSET(0x20)
#define JPEG_INTR			JPEG_OFFSET(0x24)
#define JPEG_VERSION			JPEG_OFFSET(0x2C)

/* JEPG Crtl Resgister */
#define JPEG_ENC_START			BIT(0)
#define JPEG_ENC_STOP			0

/* JEPG Configuration Resgister */
#define JPEG_INTR_ENABLE		BIT(12)
#define JPEG_BS_FORMAT_MASK		GENMASK(7,4)
# define JEPG_BS_YUV444			GENMASK(1,0)
# define JEPG_BS_YUV420			BIT(0)
# define JPEG_CHROMA_SUBSAMPLING_MASK	~(BIT(V4L2_JPEG_CHROMA_SUBSAMPLING_444) | BIT(V4L2_JPEG_CHROMA_SUBSAMPLING_420))
#define JPEG_ENC_FORMAT_MASK		GENMASK(3,0)
# define JPEG_ENC_RBG565		0
# define JPEG_ENC_RBG888		BIT(0)	/* Not Support */
# define JPEG_ENC_YUV422		BIT(1)
# define JPEG_ENC_XRGB888		GENMASK(1,0)
#define JPEG_32BPP			32	/* ARGB8888, XRGB8888 */
#define JPEG_24BPP			24	/* RGB888, YUV444 */
#define JPEG_16BPP			16	/* RGB565, YUV422 */
#define JPEG_12BPP			12	/* YUV420 */
#define JPEG_BPL(w, bpp)		((((w) * (bpp)) / 8))
#define JPEG_SZ(h, bpl)			((h) * (bpl))

/* JEPG BS Len Max Resgister */
#define JPEG_MAX_BS			(((JPEG_MAX_WIDTH) * (JPEG_MAX_HEIGHT) * (JPEG_32BPP)) / 8)
#define JPEG_MIN_BS			0

/* JEPG Quality Resgister */
#define JPEG_QP_MAX			2047
#define JPEG_QP_MIN			1
#define JPEG_QP_DEF			128

/* JEPG Intr Resgister */
#define JPEG_BS_OVERFLOW		BIT(8)
#define JPEG_EOF			BIT(0)
#define JPEG_INTR_MASK			(JPEG_BS_OVERFLOW | JPEG_EOF)

struct gxmicro_jpeg_dev {

	struct device *dev;

	void __iomem *mem;

	struct v4l2_device v4l2;
	struct vb2_queue vbq;
	struct v4l2_ctrl_handler hdl;
	struct video_device vdev;

	/* video, videobuf2 fops lock */
	struct mutex vlock;

	/* videobuf2 */
	spinlock_t buf_lock;	/* buffers list lock */
	struct list_head buffers;

	enum v4l2_jpeg_chroma_subsampling subsampling;
	uint32_t sequence;
};

static inline uint32_t gxmicro_read(struct gxmicro_jpeg_dev *gdev, uint32_t reg)
{
	return ioread32(gdev->mem + reg);
}

static inline void gxmicro_write(struct gxmicro_jpeg_dev *gdev, uint32_t reg, uint32_t value)
{
	iowrite32(value, gdev->mem + reg);
}

int gxmicro_ctrls_init(struct gxmicro_jpeg_dev *gdev);
void gxmicro_ctrls_fini(struct gxmicro_jpeg_dev *gdev);

int gxmicro_vb2_init(struct gxmicro_jpeg_dev *gdev);
void gxmicro_vb2_fini(struct gxmicro_jpeg_dev *gdev);

int gxmicro_video_init(struct gxmicro_jpeg_dev *gdev);
void gxmicro_video_fini(struct gxmicro_jpeg_dev *gdev);

#endif /* __GXMICRO_JPEG_H__ */
