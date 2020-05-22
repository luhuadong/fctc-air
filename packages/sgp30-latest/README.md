# rtt-sgp30
Support SGP30 Multi-Pixel Gas Sensor which can ouput TVOC and eCO2 data



## 1、介绍

sgp30 软件包是 SGP30 气体传感器的驱动软件包。SGP30 是一款单一芯片上具有多个传感元件的金属氧化物气体传感器，内集成 4 个气体传感器，具有完全校准的空气质量输出信号，可用于检测 TVOC 和 eCO2。

SGP30 模块支持 I2C 接口，地址固定为 0x58。



### 1.1 特性

- 支持多实例。
- 支持静态和动态分配内存。
- 支持 sensor 设备驱动框架。
- 线程安全。



### 1.2 工作模式

| 传感器       | TVOC | eCO2 |
| :----------- | :--- | :--- |
| **通信接口** |      |      |
| I2C          | √    | √    |
| **工作模式** |      |      |
| 轮询         | √    | √    |
| 中断         |      |      |
| FIFO         |      |      |



### 1.3 目录结构

| 名称     | 说明                           |
| -------- | ------------------------------ |
| docs     | 文档目录                       |
| examples | 例子目录（提供两种操作示例）   |
| inc      | 头文件目录                     |
| src      | 源代码目录（提供两种驱动接口） |

驱动源代码提供两种接口，分别是自定义接口，以及 RT-Thread 设备驱动接口（open/read/control/close）。



### 1.4 许可证

sgp30 软件包遵循 Apache license v2.0 许可，详见 `LICENSE` 文件。



### 1.5 依赖

- RT-Thread 4.0+
- 使用动态创建方式需要开启动态内存管理模块
- 使用 sensor 设备接口需要开启 sensor 设备驱动框架模块



## 2、获取 sgp30 软件包

使用 sgp30 package 需要在 RT-Thread 的包管理器中选择它，具体路径如下：

```
RT-Thread online packages --->
    peripheral libraries and drivers --->
        [*] sensors drivers  --->
            [*] SGP30: air sensor by Sensirion for detect TVOC and CO2.
```

然后让 RT-Thread 的包管理器自动更新，或者使用 `pkgs --update` 命令更新包到 BSP 中。



## 3、使用 sgp30 软件包

### 3.1 版本说明

| 版本   | 说明                                           |
| ------ | ---------------------------------------------- |
| v0.8.0 | 支持自定义接口和 sensor 框架，支持气体浓度测量 |
| latest | 进一步优化                                     |

目前处于公测阶段，建议开发者使用 latest 版本。



### 3.2 配置选项

- 是否使用示例程序（`PKG_USING_SGP30_SAMPLE`）
- 是否使用动态内存
- 是否使用 sensor 框架



## 4、API 说明

### 4.1 自定义接口

#### 创建和删除对象

要操作传感器模块，首先需要创建一个传感器对象。

```c
sgp30_device_t sgp30_create(const char *i2c_bus_name);
```

调用这个函数时，会从动态堆内存中分配一个 sgp30_device_t 句柄，并按给定参数初始化。

| 参数           | 描述                         |
| -------------- | ---------------------------- |
| i2c_bus_name   | 设备挂载的 IIC 总线名称      |
| **返回**       | ——                           |
| sgp30_device_t | 创建成功，返回传感器对象句柄 |
| RT_NULL        | 创建失败                     |

对于使用 `sgp30_create()` 创建出来的对象，当不需要使用，或者运行出错时，请使用下面的函数接口将其删除，避免内存泄漏。

```c
void sgp30_delete(sgp30_device_t dev);
```

| **参数**       | **描述**               |
| -------------- | ---------------------- |
| sgp30_device_t | 要删除的传感器对象句柄 |
| **返回**       | ——                     |
| 无             |                        |



#### 初始化对象

如果需要使用静态内存分配，则可调用 `sgp30_init()` 函数。

```c
rt_err_t sgp30_init(struct sgp30_device *dev, const char *i2c_bus_name);
```

使用该函数前需要先创建 sgp30_device 结构体。

| 参数         | 描述                    |
| ------------ | ----------------------- |
| dev          | 传感器对象结构体        |
| i2c_bus_name | 设备挂载的 IIC 总线名称 |
| **返回**     | ——                      |
| RT_EOK       | 初始化成功              |
| -RT_ERROR    | 初始化失败              |



#### 测量数据

测量 TVOC 和 eCO2 浓度值，并将数据保存在传感器对象中。

```c
rt_bool_t sgp30_measure(sgp30_device_t dev);
```

| 参数     | 描述           |
| -------- | -------------- |
| dev      | 传感器对象句柄 |
| **返回** | ——             |
| RT_TRUE  | 读取成功       |
| RT_FALSE | 读取失败       |



#### 测量原生数据

测量 H2 和 Ethanol 浓度值，并将数据保存在传感器对象中。

```c
rt_bool_t sgp30_measure_raw(sgp30_device_t dev);
```

| 参数     | 描述           |
| -------- | -------------- |
| dev      | 传感器对象句柄 |
| **返回** | ——             |
| RT_TRUE  | 读取成功       |
| RT_FALSE | 读取失败       |



#### 读取 baseline

```c
rt_bool_t sgp30_get_baseline(sgp30_device_t dev, 
                             rt_uint16_t *eco2_base, 
                             rt_uint16_t *tvoc_base);
```

| 参数      | 描述                  |
| --------- | --------------------- |
| dev       | 传感器对象句柄        |
| eco2_base | 保存 eCO2 的 baseline |
| tvoc_base | 保存 TVOC 的 baseline |
| **返回**  | ——                    |
| RT_TRUE   | 读取成功              |
| RT_FALSE  | 读取失败              |



#### 设置 baseline

```c
rt_bool_t sgp30_set_baseline(sgp30_device_t dev, 
                             rt_uint16_t eco2_base, 
                             rt_uint16_t tvoc_base);
```

| 参数      | 描述                    |
| --------- | ----------------------- |
| dev       | 传感器对象句柄          |
| eco2_base | eCO2 的 baseline 设置值 |
| tvoc_base | TVOC 的 baseline 设置值 |
| **返回**  | ——                      |
| RT_TRUE   | 设置成功                |
| RT_FALSE  | 设置失败                |



#### 设置湿度值

```c
rt_bool_t sgp30_set_humidity(sgp30_device_t dev, 
                             rt_uint32_t absolute_humidity);
```

| 参数              | 描述                 |
| ----------------- | -------------------- |
| dev               | 传感器对象句柄       |
| absolute_humidity | 当前环境的绝对湿度值 |
| **返回**          | ——                   |
| RT_TRUE           | 设置成功             |
| RT_FALSE          | 设置失败             |



### 4.2 Sensor 接口

sgp30 软件包已对接 sensor 驱动框架，操作传感器模块之前，只需调用下面接口注册传感器设备即可。

```c
rt_err_t rt_hw_sgp30_init(const char *name, struct rt_sensor_config *cfg);
```

| 参数      | 描述            |
| --------- | --------------- |
| name      | 传感器设备名称  |
| cfg       | sensor 配置信息 |
| **返回**  | ——              |
| RT_EOK    | 创建成功        |
| -RT_ERROR | 创建失败        |

示例如下：

```c
static int rt_hw_sgp30_port(void)
{
    struct rt_sensor_config cfg;
    
    cfg.intf.type = RT_SENSOR_INTF_I2C;
    cfg.intf.dev_name = SGP30_I2C_BUS_NAME;
    rt_hw_sgp30_init("sg3", &cfg);
    
    return RT_EOK;
}
INIT_COMPONENT_EXPORT(rt_hw_sgp30_port);
```



## 5、注意事项

1. 为传感器对象提供静态创建和动态创建两种方式，如果使用动态创建，请记得在使用完毕释放对应的内存空间。
2. 由于 SGP30 模块包含一个 TVOC 传感器和一个 eCO2 传感器（实际上还有 H2 和 Ethanol 传感器），因此在 sensor 框架中会注册两个设备，内部提供1位 FIFO 缓存进行同步，缓存空间在调用 `rt_device_open` 函数时创建，因此 read 之前务必确保两个设备都开启成功。
3. 由于使用 I2C 接口进行操作，因此注册时需指定具体的 I2C 总线名称，对应的句柄存放在 user_data 中。
4. SGP30 传感器上电后大约15秒才能正常检测，此前的数据一直是 TVOC 为 0，eCO2 为 400。



## 6、相关文档

见 docs 目录。



## 7、联系方式

- 维护：luhuadong@163.com
- 主页：<https://github.com/luhuadong/rtt-sgp30>