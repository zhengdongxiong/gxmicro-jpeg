# GXMicro JPEG
v4l2 驱动, 用于将捕获图像压缩

# 内核版本
V5.15.50

# 驱动位置
需要文件放入以下目录
```shell
drivers/media/platform/<目录>
```

# 文件目录说明
| 文件目录 | 说明 |
| :---: | :---: |
| gxmicro_drv.c | 驱动 probe 入口, platform 和 V4L2 相关初始化 |
| gxmicro_ctrls.c | v4l2 中控件相关操作 |
| gxmicro_vb2.c | v4l2 中 videobuf2 相关内存管理 |
| gxmicro_video.c | v4l2 中 video 相关 ioctl |
| gxmicro_jpeg.h | 读写函数与设备结构体 |

# TODO
1. 编译通过, 暂未验证
