# GXMicro JPEG

v4l2 驱动, 用于将捕获图像压缩

# 文件目录说明

| 文件目录 | 说明 |
| :---: | :---: |
| gxmicro_jpeg_drv.c | 驱动 probe 入口, platform 设备相关初始化 |
| gxmicro_ctrls.c | v4l2 中控件相关操作 |
| gxmicro_v4l2.c | v4l2 相关初始化 |
| gxmicro_vb2.c | v4l2 中 videobuf2 相关内存管理 |
| gxmicro_video.c | v4l2 中 video 相关 ioctl |
| gxmicro_jpeg.h | 读写函数与设备结构体 |

# TODO
1. 完善 vb2 部分
2. 验证是否能正常编解码显示图像
