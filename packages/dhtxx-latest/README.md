# rtt-dhtxx
Digital Humidity and Temperature sensor (communicate via single bus), including DHT11, DHT21 and DHT22



## 1、介绍

dhtxx 是基于 RT-Thread 的 DHTxx 系列传感器驱动软件包，适用于单总线式数字温湿度传感器，包括 DHT11、DHT21 和 DHT22，并提供多种操作接口。



### 1.1 特性

- 支持多实例。
- 支持静态和动态分配内存。
- 支持 sensor 设备驱动框架。
- 线程安全。



### 1.2 工作模式

| 传感器       | 温度 | 湿度 |
| :----------- | :--- | :--- |
| **通信接口** |      |      |
| 单总线       | √    | √    |
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

驱动源代码提供两种接口，分别是自定义接口，以及RT-Thread设备驱动接口（open/read/control/close）。



### 1.4 许可证

dhtxx 软件包遵循 Apache license v2.0 许可，详见 `LICENSE` 文件。



### 1.5 依赖

- RT-Thread 4.0+
- 使用动态创建方式需要开启动态内存管理模块
- 使用 sensor 设备接口需要开启 sensor 设备驱动框架模块



## 2、获取 dhtxx 软件包

使用 dhtxx package 需要在 RT-Thread 的包管理器中选择它，具体路径如下：

```
RT-Thread online packages --->
    peripheral libraries and drivers --->
        [*] sensors drivers  --->
            [*] DHTxx one-wire digital temperature and humidity sensor.
```

然后让 RT-Thread 的包管理器自动更新，或者使用 `pkgs --update` 命令更新包到 BSP 中。



## 3、使用 dhtxx 软件包

### 3.1 版本说明

| 版本   | 说明                                                         |
| ------ | ------------------------------------------------------------ |
| v0.8.0 | 支持自定义接口和 sensor 框架，已在 DHT11 和 DHT22 上完成测试 |
| latest | 进一步优化                                                   |

目前处于公测阶段，建议开发者使用 latest 版本。



### 3.2 配置选项

- 是否使用示例程序（`PKG_USING_DHTXX_SAMPLE`）
- 是否使用动态内存
- 是否使用 sensor 框架



## 4、API 说明

### 4.1 自定义接口

#### 创建和删除对象

要操作传感器模块，首先需要创建一个传感器对象。

```c
dht_device_t dht_create(const rt_uint8_t type, const rt_base_t pin);
```

调用这个函数时，会从动态堆内存中分配一个 dht_device_t 句柄，并按给定参数初始化。

| 参数         | 描述                         |
| ------------ | ---------------------------- |
| type         | 单总线数字温湿度传感器类型   |
| pin          | 数据输出引脚                 |
| **返回**     | ——                           |
| dht_device_t | 创建成功，返回传感器对象句柄 |
| RT_NULL      | 创建失败                     |

传感器类型目前支持：

```c
#define DHT11                0
#define DHT12                1
#define DHT21                2
#define DHT22                3
#define AM2301               DHT21
#define AM2302               DHT22
```

对于使用 `dht_create()` 创建出来的对象，当不需要使用，或者运行出错时，请使用下面的函数接口将其删除，避免内存泄漏。

```c
void dht_delete(dht_device_t dev);
```

| **参数**     | **描述**               |
| ------------ | ---------------------- |
| dht_device_t | 要删除的传感器对象句柄 |
| **返回**     | ——                     |
| 无           |                        |



#### 初始化对象

如果需要使用静态内存分配，则可调用 `dht_init()` 函数。

```c
rt_err_t   dht_init(struct dht_device *dev, const rt_uint8_t type, const rt_base_t pin);
```

使用该函数前需要先创建 dht_device 结构体。

| 参数      | 描述                       |
| --------- | -------------------------- |
| dev       | 传感器对象结构体           |
| type      | 单总线数字温湿度传感器类型 |
| pin       | 数据输出引脚               |
| **返回**  | ——                         |
| RT_EOK    | 初始化成功                 |
| -RT_ERROR | 初始化失败                 |



#### 读传感器

读取温湿度值，并将数据保存在传感器对象中。

```c
rt_bool_t  dht_read(dht_device_t dev);
```

| 参数     | 描述           |
| -------- | -------------- |
| dev      | 传感器对象句柄 |
| **返回** | ——             |
| RT_TRUE  | 读取成功       |
| RT_FALSE | 读取失败       |

由于 DHTxx 系列传感器模块内包含温度和湿度传感器，并且一次访问即可获取温度和湿度值，因此获取具体的温度或湿度值需要调用下面两个函数。



#### 获取湿度值

```c
rt_int32_t dht_get_humidity(dht_device_t dev);
```

| 参数     | 描述                 |
| -------- | -------------------- |
| dev      | 传感器对象句柄       |
| **返回** | ——                   |
| 湿度值   | 数值为真实湿度的十倍 |



#### 获取温度值

```c
rt_int32_t dht_get_temperature(dht_device_t dev);
```

| 参数     | 描述                 |
| -------- | -------------------- |
| dev      | 传感器对象句柄       |
| **返回** | ——                   |
| 温度值   | 数值为真实温度的十倍 |



### 4.2 Sensor 接口

dhtxx 软件包已对接 sensor 驱动框架，操作传感器模块之前，只需调用下面接口注册传感器设备即可。

```c
rt_err_t rt_hw_dht_init(const char *name, struct rt_sensor_config *cfg);
```

| 参数      | 描述            |
| --------- | --------------- |
| name      | 传感器设备名称  |
| cfg       | sensor 配置信息 |
| **返回**  | ——              |
| RT_EOK    | 创建成功        |
| -RT_ERROR | 创建失败        |

为了支持 DHT11、DHT22 等多种传感器类型，在 sensor 接口部分定义了 `dht_info` 结构体，因此在调用 `rt_hw_dht_init` 初始化前需要构建该结构体。并且在使用完毕前不能释放该结构体内存，因此建议加上 static 关键字，或动态分配内存空间。如下所示：

```c
static int rt_hw_dht22_port(void)
{
    static struct dht_info info;
    struct rt_sensor_config cfg;

    info.type = DHT22;
    info.pin  = DHT22_DATA_PIN;
    
    cfg.intf.type = RT_SENSOR_INTF_ONEWIRE;
    cfg.intf.user_data = (void *)&info;
    rt_hw_dht_init("dh2", &cfg);
    
    return RT_EOK;
}
INIT_COMPONENT_EXPORT(rt_hw_dht22_port);
```



### 4.3 其他

另外，在 dhtxx 软件包中还提供了下面一些接口。

#### 温度转换

将摄氏温度转为开氏温度

```c
float convert_c2k(float c);
```

将摄氏温度转为华氏温度

```c
float convert_c2f(float c);
```

将华氏温度转为摄氏温度

```c
float convert_f2c(float f);
```



#### 数值转换

将一个数按倍数关系拆分成整数和小数部分。

```c
rt_int32_t split_int(const rt_int32_t num, rt_int32_t *integer, 
                     rt_int32_t *decimal, const rt_uint32_t times);
```

| 参数     | 描述              |
| -------- | ----------------- |
| num      | 待转换的数值      |
| integer  | 转换后的整数部分  |
| decimal  | 转换后的小数部分  |
| times    | 倍数              |
| **返回** | ——                |
| 0        | 正数（大于等于0） |
| 1        | 负数（小于0）     |

针对 dhtxx 软件包中输出的温湿度都是真实值的十倍，为了保持精度，可以这样输出：

```c
rt_int32_t integer;
rt_int32_t decimal;
rt_int32_t flag = split_int(dht_get_temperature(dev), &integer, &decimal, 10);
rt_kprintf("Temp: %s%d.%d\n", flag > 0 ? "-" : "", integer, decimal);
```



## 5、注意事项

1. 为传感器对象提供静态创建和动态创建两种方式，如果使用动态创建，请记得在使用完毕释放对应的内存空间。
2. 由于 DHTxx 模块包含一个温度传感器和一个湿度传感器，因此在 sensor 框架中会注册两个设备，内部提供 1 位 FIFO 缓存进行同步，缓存空间在调用 `rt_device_open` 函数时创建，因此 read 之前务必确保两个设备都开启成功。
3. dhtxx 软件包为支持 DHT11、DHT22 等多种传感器类型，在 sensor 接口部分定义了 `dht_info` 结构体，因此在调用 `rt_hw_dht_init` 初始化前需要构建并传入该结构体指针。
4. 获取的温度和湿度值均为真实值的十倍，因此应用层需要进行转换，软件包提供了 `split_int` 函数进行处理，可根据您的实际情况选择使用。



## 6、相关文档

见 docs 目录。



## 7、联系方式

- 维护：luhuadong@163.com
- 主页：<https://github.com/luhuadong/rtt-dhtxx>

