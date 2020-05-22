# rtt-gp2y10
Support the Particulate Matter sensor for GP2Y1010AU0F and GP2Y1014AU0F



## 1、介绍

gp2y10 软件包适用于 GP2Y10 系列光学灰尘检测传感器模块，包括 GP2Y1010AU0F 和 GP2Y1014AU0F。



### 1.1 特性

- 支持多实例。
- 支持静态和动态分配内存。
- 支持 sensor 设备驱动框架。
- 线程安全。



### 1.2 工作模式

| 设备         | 空气颗粒物浓度 |
| :----------- | :------------- |
| **通信接口** |                |
| ADC          | √              |
| **工作模式** |                |
| 轮询         | √              |
| 中断         |                |
| FIFO         |                |



### 1.3 目录结构

| 名称     | 说明                           |
| -------- | ------------------------------ |
| docs     | 文档目录                       |
| examples | 例子目录（提供两种操作示例）   |
| inc      | 头文件目录                     |
| src      | 源代码目录（提供两种驱动接口） |

驱动源代码提供两种接口，分别是自定义接口，以及 RT-Thread 设备驱动接口（open/read/control/close）。



### 1.4 许可证

gp2y10 软件包遵循 Apache license v2.0 许可，详见 `LICENSE` 文件。



### 1.5 依赖

- RT-Thread 4.0+
- 使用动态创建方式需要开启动态内存管理模块
- 使用 sensor 设备接口需要开启 sensor 设备驱动框架模块



## 2、获取 gp2y10 软件包

使用 gp2y10 package 需要在 RT-Thread 的包管理器中选择它，具体路径如下：

```
RT-Thread online packages --->
    peripheral libraries and drivers --->
        [*] sensors drivers  --->
            [*] GP2Y10: dust sensor by Sharp for detect air quality.
```

然后让 RT-Thread 的包管理器自动更新，或者使用 `pkgs --update` 命令更新包到 BSP 中。



## 3、使用 gp2y10 软件包

### 3.1 版本说明

| 版本   | 说明                         |
| ------ | ---------------------------- |
| v0.8.0 | 支持自定义接口和 sensor 框架 |
| latest | 进一步优化                   |

目前处于公测阶段，建议开发者使用 latest 版本。



### 3.2 配置选项

- ADC 名称（`PKG_USING_GP2Y10_ADC_DEV_NAME`）
- ADC 通道（`PKG_USING_GP2Y10_ADC_DEV_CHANNEL`）
- ADC 分辨率/转换位数（`PKG_USING_GP2Y10_CONVERT_BITS`）
- GP2Y10 模块分压电路电压比（`PKG_USING_GP2Y10_VOLTAGE_RATIO`）
- 是否使用示例程序（`PKG_USING_GP2Y10_SAMPLE`）
- 是否使用动态内存
- 是否使用 sensor 框架



## 4、API 说明

### 4.1 自定义接口

#### 创建和删除对象

要操作传感器模块，首先需要创建一个传感器对象。

```c
gp2y10_device_t gp2y10_create(const rt_base_t iled_pin, const rt_base_t aout_pin);
```

调用这个函数时，会从动态堆内存中分配一个 gp2y10_device_t 句柄，并按给定参数初始化。

| 参数            | 描述                         |
| --------------- | ---------------------------- |
| iled_pin        | ILED 引脚                    |
| aout_pin        | AOUT 引脚                    |
| **返回**        | ——                           |
| gp2y10_device_t | 创建成功，返回传感器对象句柄 |
| RT_NULL         | 创建失败                     |

对于使用 `gp2y10_create()` 创建出来的对象，当不需要使用，或者运行出错时，请使用下面的函数接口将其删除，避免内存泄漏。

```c
void gp2y10_delete(gp2y10_device_t dev);
```

| **参数**        | **描述**               |
| --------------- | ---------------------- |
| gp2y10_device_t | 要删除的传感器对象句柄 |
| **返回**        | ——                     |
| 无              |                        |



#### 初始化对象

如果需要使用静态内存分配，则可调用 `gp2y10_init()` 函数。

```c
rt_err_t gp2y10_init(struct gp2y10_device *dev, 
                     const rt_base_t iled_pin, 
                     const rt_base_t aout_pin);
```

使用该函数前需要先创建 gp2y10_device 结构体。

| 参数      | 描述             |
| --------- | ---------------- |
| dev       | 传感器对象结构体 |
| iled_pin  | ILED 引脚        |
| aout_pin  | AOUT 引脚        |
| **返回**  | ——               |
| RT_EOK    | 初始化成功       |
| -RT_ERROR | 初始化失败       |



#### 读取数据

读取并返回灰尘浓度值，单位 ug/m3。

```c
rt_uint32_t gp2y10_get_dust_density(gp2y10_device_t dev);
```

| 参数     | 描述           |
| -------- | -------------- |
| dev      | 传感器对象句柄 |
| **返回** | ——             |
| 浓度值   | 读取成功       |
| --       | 读取失败       |



### 4.2 Sensor 接口

gp2y10 软件包已对接 sensor 驱动框架，操作传感器模块之前，只需调用下面接口注册传感器设备即可。

```c
rt_err_t rt_hw_gp2y10_init(const char *name, struct rt_sensor_config *cfg);
```

| 参数      | 描述            |
| --------- | --------------- |
| name      | 传感器设备名称  |
| cfg       | sensor 配置信息 |
| **返回**  | ——              |
| RT_EOK    | 创建成功        |
| -RT_ERROR | 创建失败        |

由于 GP2Y10 传感器需要操作两个引脚（iled_pin 和 aout_pin），因此需要构建 gp2y10_device 结构体，并填写引脚信息。同时注意在使用完毕前不能释放该结构体内存，因此建议加上 static 关键字，或动态分配内存空间。如下所示：

```c
static int rt_hw_gp2y10_port(void)
{
    struct gp2y10_device gp2y10_dev;
    struct rt_sensor_config cfg;

    gp2y10_dev.iled_pin = GP2Y10_ILED_PIN;
    gp2y10_dev.aout_pin = GP2Y10_AOUT_PIN;

    cfg.intf.user_data = (void *)&gp2y10_dev;
    rt_hw_gp2y10_init("gp2", &cfg);

    return RT_EOK;
}
INIT_COMPONENT_EXPORT(rt_hw_gp2y10_port);
```



## 5、注意事项

1. 为传感器对象提供静态创建和动态创建两种方式，如果使用动态创建，请记得在使用完毕释放对应的内存空间。
2. 由于 GP2Y10 模块采用光学检测原理，因此出来需要配置 ADC 引脚，还需要指定一个 GPIO 引脚（iled_pin），用于发射光脉冲，因此在调用 `rt_hw_gp2y10_init` 需要构建并传入 `gp2y10_device` 结构体指针。



## 6、相关文档

见 docs 目录。



## 7、联系方式

- 维护：luhuadong@163.com
- 主页：<https://github.com/luhuadong/rtt-gp2y10>