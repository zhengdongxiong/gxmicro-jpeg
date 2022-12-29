// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * GXMicro V4L2 Videobuf2 Queue
 *
 * Copyright (C) 2022 GXMicro (ShangHai) Corp.
 *
 * Author:
 * 	Zheng DongXiong <zhengdongxiong@gxmicro.cn>
 */
#include <media/videobuf2-dma-contig.h>

#include "gxmicro_jpeg.h"

static int gxmicro_queue_setup(struct vb2_queue *vbq, unsigned int *nbuffers,
				unsigned int *nplanes, unsigned int sizes[], struct device *alloc_devs[])
{
	struct gxmicro_jpeg_dev *gdev = vb2_get_drv_priv(vbq);
	uint32_t max_compressed_size;

	max_compressed_size = gxmicro_read(gdev, );

	if (*nplanes)
		sizes[0] < max_compressed_size ? -EINVAL : 0;

	*nplanes = 1;
	sizes[0] =
}

static int gxmicro_buf_prepare(struct vb2_buffer *vb)
{
	struct gxmicro_jpeg_dev *gdev = vb2_get_drv_priv(vb->vb2_queue);

	if (vb2_plane_size(vb, 0) <)
		return -EINVAL;

	return 0;
}

static const struct vb2_ops gxmicro_vb2_ops = {
	.queue_setup = gxmicro_queue_setup,	/* Reserved */
	.wait_prepare = vb2_ops_wait_prepare,
	.wait_finish = vb2_ops_wait_finish,
	.buf_prepare = gxmicro_buf_prepare,	/* Reserved */
	.start_streaming = gxmicro_start_streaming,
	.stop_streaming = gxmicro_stop_streaming,
	.buf_queue = gxmicro_buf_queue,
};

int gxmicro_vb2_init(struct gxmicro_jpeg_dev *gdev)
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
	vbq->buf_struct_size = ;	/* 私有buffer, vb2_v4l2_buffer 必须在第一个 */
	vbq->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC;
	vbq->min_buffers_needed = JPEG_BUFFERS;

	ret = vb2_queue_init(vbq);
	if (ret) {
		dev_err(gdev->dev, "Failed to init vb2 queue\n");
		return ret;
	}

	return 0;
}

void gxmicro_vb2_fini(struct gxmicro_jpeg_dev *gdev)
{

}
