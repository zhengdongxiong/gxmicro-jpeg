# SPDX-License-Identifier: GPL-2.0-only

gxmicro_jpeg-y += gxmicro_drv.o gxmicro_ctrls.o gxmicro_vb2.o gxmicro_video.o
obj-$(CONFIG_VIDEO_GXMICRO) += gxmicro_jpeg.o

ccflags-y += -Werror
