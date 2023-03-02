// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * GXMicro V4L2 Videobuf2
 *
 * Copyright (C) 2022 GXMicro (ShangHai) Corp.
 *
 * Author:
 * 	Zheng DongXiong <zhengdongxiong@gxmicro.cn>
 */
#include <linux/of_irq.h>
#include <media/videobuf2-dma-contig.h>

#include "gxmicro_jpeg.h"

struct gxmicro_buffer {
	struct vb2_v4l2_buffer vbuf;
	struct list_head list;
};
#define vbuf_to_gxmicro_buffer(vbuf)	container_of(vbuf, struct gxmicro_buffer, vbuf)

/* gdev->buf_lock spinlock must be held by caller */
static void gxmicro_jpeg_start(struct gxmicro_jpeg_dev *gdev)
{
	struct gxmicro_buffer *gbuf;
	dma_addr_t addr;

	gbuf = list_first_entry(&gdev->buffers, struct gxmicro_buffer, list);

	addr = vb2_dma_contig_plane_dma_addr(&gbuf->vbuf.vb2_buf, 0);

	gxmicro_write(gdev, JPEG_BS_BASE, addr);

	gxmicro_write(gdev, JPEG_CTRL, JPEG_ENC_START);
}

/* ****************************** Videobuf2 Queue OPS ****************************** */

static int gxmicro_queue_setup(struct vb2_queue *vbq, unsigned int *nbuffers,
				unsigned int *nplanes, unsigned int sizes[], struct device *alloc_devs[])
{
	struct gxmicro_jpeg_dev *gdev = vb2_get_drv_priv(vbq);
	uint32_t width, height;
	uint8_t bpp;
	uint32_t bpl, sizeimage;

	width = gxmicro_read(gdev, JPEG_WIDTH);
	height = gxmicro_read(gdev, JPEG_HEIGHT);

	switch (gdev->subsampling) {
	case V4L2_JPEG_CHROMA_SUBSAMPLING_444:
		bpp = JPEG_24BPP;
		break;
	case V4L2_JPEG_CHROMA_SUBSAMPLING_420:
		bpp = JPEG_12BPP;
		break;
	default:
		return -EINVAL;
	}

	bpl = JPEG_BPL(width, bpp);
	sizeimage = JPEG_SZ(height, bpl);

	if (*nplanes)
		return sizes[0] < sizeimage ? -EINVAL : 0;

	*nplanes = 1;
	sizes[0] = sizeimage;

	return 0;
}

static int gxmicro_buf_prepare(struct vb2_buffer *vb)
{
	struct gxmicro_jpeg_dev *gdev = vb2_get_drv_priv(vb->vb2_queue);
	uint32_t width, height;
	uint8_t bpp;
	uint32_t bpl, sizeimage;

	width = gxmicro_read(gdev, JPEG_WIDTH);
	height = gxmicro_read(gdev, JPEG_HEIGHT);

	switch (gdev->subsampling) {
	case V4L2_JPEG_CHROMA_SUBSAMPLING_444:
		bpp = JPEG_24BPP;
		break;
	case V4L2_JPEG_CHROMA_SUBSAMPLING_420:
		bpp = JPEG_12BPP;
		break;
	default:
		return -EINVAL;
	}

	bpl = JPEG_BPL(width, bpp);
	sizeimage = JPEG_SZ(height, bpl);

	if (vb2_plane_size(vb, 0) < sizeimage)
		return -EINVAL;

	return 0;
}

static int gxmicro_start_streaming(struct vb2_queue *vbq, unsigned int count)
{
	struct gxmicro_jpeg_dev *gdev = vb2_get_drv_priv(vbq);
	unsigned long flags;

	/* Reserved: JPEG Busy ? */
	/* Reserved: start failed: vb2_buffer_done(, VB2_BUF_STATE_QUEUED) INIT_LIST_HEAD() */

	gdev->sequence = 0;

	spin_lock_irqsave(&gdev->buf_lock, flags);
	gxmicro_jpeg_start(gdev);
	spin_unlock_irqrestore(&gdev->buf_lock, flags);

	return 0;
}

static void gxmicro_stop_streaming(struct vb2_queue *vbq)
{
	struct gxmicro_jpeg_dev *gdev = vb2_get_drv_priv(vbq);
	struct gxmicro_buffer *gbuf;
	unsigned long flags;

	/* Reserved: JPEG Reset ? */

	gxmicro_write(gdev, JPEG_CTRL, JPEG_ENC_STOP);

	spin_lock_irqsave(&gdev->buf_lock, flags);
	list_for_each_entry(gbuf, &gdev->buffers, list)
		vb2_buffer_done(&gbuf->vbuf.vb2_buf, VB2_BUF_STATE_ERROR);
	INIT_LIST_HEAD(&gdev->buffers);
	spin_unlock_irqrestore(&gdev->buf_lock, flags);
}

static void gxmicro_buf_queue(struct vb2_buffer *vb)
{
	struct gxmicro_jpeg_dev *gdev = vb2_get_drv_priv(vb->vb2_queue);
	struct vb2_v4l2_buffer *vbuf = to_vb2_v4l2_buffer(vb);
	struct gxmicro_buffer *gbuf = vbuf_to_gxmicro_buffer(vbuf);
	unsigned long flags;

	spin_lock_irqsave(&gdev->buf_lock, flags);
	list_add_tail(&gbuf->list, &gdev->buffers);
	spin_unlock_irqrestore(&gdev->buf_lock, flags);
}

static const struct vb2_ops gxmicro_vb2_ops = {
	.queue_setup = gxmicro_queue_setup,
	.wait_prepare = vb2_ops_wait_prepare,
	.wait_finish = vb2_ops_wait_finish,
	.buf_prepare = gxmicro_buf_prepare,
	.start_streaming = gxmicro_start_streaming,
	.stop_streaming = gxmicro_stop_streaming,	/* Reserved: JPEG stop, intr clk ... */
	.buf_queue = gxmicro_buf_queue,
};

static int gxmicro_vbq_init(struct gxmicro_jpeg_dev *gdev)
{
	struct vb2_queue *vbq = &gdev->vbq;
	int ret;

	vbq->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	vbq->io_modes = VB2_MMAP | VB2_READ;
	vbq->dev = gdev->dev;
	vbq->lock = &gdev->vlock;
	vbq->ops = &gxmicro_vb2_ops;
	vbq->mem_ops = &vb2_dma_contig_memops;
	vbq->drv_priv = gdev;
	vbq->buf_struct_size = sizeof(struct gxmicro_buffer);	/* 私有buffer, vb2_v4l2_buffer 必须在第一个 */
	vbq->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC;
	vbq->min_buffers_needed = JPEG_BUFFERS;

	ret = vb2_queue_init(vbq);
	if (ret) {
		dev_err(gdev->dev, "Failed to init vb2 queue\n");
		return ret;
	}

	return 0;
}

static void gxmicro_vbq_fini(struct gxmicro_jpeg_dev *gdev)
{
	/* vb2_video_unregister_device() in gxmicro_video_fini() */
}

/* ****************************** IRQ ****************************** */

/*
 * Reserved:
 * 	不设置 JPEG_ENC_START, 不知是否继续编码触发中断
 * 	若不自动编码, 将 irq 合并到 irq thread
 */

static irqreturn_t gxmicro_irq_handler(int irq, void *arg)
{
	struct gxmicro_jpeg_dev *gdev = arg;
	struct gxmicro_buffer *gbuf;
	irqreturn_t ret = IRQ_NONE;
	uint32_t dummy;

	/* Reserved: overflow */
	dummy = gxmicro_read(gdev, JPEG_INTR);
	gxmicro_write(gdev, JPEG_INTR, JPEG_INTR_MASK);
	dev_dbg(gdev->dev, "irq: 0x%04x\n", dummy);

	spin_lock(&gdev->buf_lock);

	gbuf = list_first_entry_or_null(&gdev->buffers, struct gxmicro_buffer, list);
	if (gbuf && !list_is_last(&gbuf->list, &gdev->buffers))
		ret = IRQ_WAKE_THREAD;

	spin_unlock(&gdev->buf_lock);

	return ret;
}

static irqreturn_t gxmicro_irq_thread(int irq, void *arg)
{
	struct gxmicro_jpeg_dev *gdev = arg;
	struct gxmicro_buffer *gbuf;
	irqreturn_t ret = IRQ_NONE;
	uint32_t fsize;

	fsize = gxmicro_read(gdev, JPEG_BS_LENGTH);

	spin_lock(&gdev->buf_lock);

	gbuf = list_first_entry_or_null(&gdev->buffers, struct gxmicro_buffer, list);
	if (!gbuf || list_is_last(&gbuf->list, &gdev->buffers))
		goto irq_thread;

	vb2_set_plane_payload(&gbuf->vbuf.vb2_buf, 0, fsize);
	gbuf->vbuf.vb2_buf.timestamp = ktime_get_ns();
	gbuf->vbuf.sequence = gdev->sequence++;
	gbuf->vbuf.field = V4L2_FIELD_NONE;
	vb2_buffer_done(&gbuf->vbuf.vb2_buf, VB2_BUF_STATE_DONE);
	list_del(&gbuf->list);

	gxmicro_jpeg_start(gdev);

	ret = IRQ_HANDLED;

irq_thread:
	spin_unlock(&gdev->buf_lock);

	return ret;
}

static int gxmicro_irq_init(struct gxmicro_jpeg_dev *gdev)
{
	int irq;
	int ret;

	irq = of_irq_get(gdev->dev->of_node, 0);
	if (irq < 0) {
		dev_err(gdev->dev, "IRQ not found\n");
		return irq;
	}

	ret = devm_request_threaded_irq(gdev->dev, irq, gxmicro_irq_handler,
					gxmicro_irq_thread, IRQF_ONESHOT, DRVNAME, gdev);
	if (ret < 0) {
		dev_err(gdev->dev, "Failed to request irq\n");
		return ret;
	}

	return 0;
}

static void gxmicro_irq_fini(struct gxmicro_jpeg_dev *gdev)
{

}

/* ****************************** Videobuf2 Init & Fini ****************************** */

int gxmicro_vb2_init(struct gxmicro_jpeg_dev *gdev)
{
	int ret;

	ret = gxmicro_vbq_init(gdev);
	if (ret)
		goto err_vbq_init;

	ret = gxmicro_irq_init(gdev);
	if (ret)
		goto err_irq_init;

	return 0;

err_irq_init:
	gxmicro_vbq_fini(gdev);
err_vbq_init:
	return ret;
}

void gxmicro_vb2_fini(struct gxmicro_jpeg_dev *gdev)
{
	gxmicro_irq_fini(gdev);

	gxmicro_vbq_fini(gdev);
}
