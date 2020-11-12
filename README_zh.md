# FCTC-Air

[English](README.md) | 简体中文

[![GitHub](https://img.shields.io/github/license/luhuadong/fctc-air.svg)](https://github.com/luhuadong/fctc-air/blob/master/LICENSE)
[![GitHub release](https://img.shields.io/github/release/luhuadong/fctc-air.svg)](https://github.com/luhuadong/fctc-air/releases)
[![GitHub pull-requests](https://img.shields.io/github/issues-pr/luhuadong/fctc-air.svg)](https://github.com/luhuadong/fctc-air/pulls)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=flat)](https://github.com/luhuadong/fctc-air/issues)



## 概述

FCTC-Air 是一个兼顾公益与教学的开源项目。目的是构建一套人人都可以访问的分布式空气质量监测系统。在构建的过程中，您可以学习到有关物联网技术的许多知识，同时关注空气污染和环境保护的重要性。

“**FCTC**”是“**From Chip To Cloud**”的首字母缩写，意思是“从芯片端到云端”（简称“从端到云”）。从中可以看出，FCTC-Air 是一个涵盖全栈 IoT 设计的项目，包括嵌入式开发、RTOS、网络通信、Web 服务、App 应用等环节。因此，该项目也非常适合于学校教学。



克隆本仓库后，请执行如下命令更新子模块：

```shell
git submodule init
git submodule update
```



## 固件列表

| 硬件平台       | MCU       | Flash                   | RAM               | 链接                                                         |
| -------------- | --------- | ----------------------- | ----------------- | ------------------------------------------------------------ |
| NUCLEO-L4R5ZI  | STM32L4R5 | 2 MB                    | 640 KB            | [details](https://www.st.com/en/evaluation-tools/nucleo-l4r5zi.html) |
| NUCLEO-F767ZI  | STM32F767 | 2 MB                    | 512 KB            | [details](https://www.st.com/en/evaluation-tools/nucleo-f767zi.html) |
| AT-START-F403A | AT32F403A | 1 MB                    | 96 KB             | [details](https://blog.csdn.net/lu_embedded/article/details/105816401) |
| Pandora-IoT    | STM32L475 | 512 KB                  | 128 KB            |                                                              |
| ART-Pi         | STM32H750 | 128 KB + 扩展 SPI Flash | 1 MB + 32MB SDRAM |                                                              |
| ......         |           |                         |                   |                                                              |




## 传感器列表

| 传感器   | 厂商      | 支持模块                                 | 软件包                                                      |
| -------- | --------- | ---------------------------------------- | ----------------------------------------------------------- |
| DHTxx    | Asair     | DHT11, DHT22                             | [dhtxx](https://github.com/luhuadong/rtt-dhtxx)             |
| DS18B20  | Dallas    | DS18B20                                  | [ds18b20](https://github.com/willianchanlovegithub/ds18b20) |
| GP2Y10xx | Sharp     | GP2Y1010, GP2Y1014                       | [gp2y10](https://github.com/luhuadong/rtt-gp2y10)           |
| PMSxx    | Plantower | PMS5003, PMS5003ST, PMS7003, PMS9003 ... | [pmsxx](https://github.com/luhuadong/rtt-pmsxx)             |
| SGP30    | Sensirion | SGP30                                    | [sgp30](https://github.com/luhuadong/rtt-sgp30)             |
| CCS811   | AMS       | CCS811                                   | [ccs811](https://github.com/luhuadong/rtt-ccs811)           |
| BME280   | Bosch     | BME280                                   | [bme280](https://github.com/RT-Thread-packages/bme280)      |
| BME680   | Bosch     | BME680                                   | [bme680](https://github.com/luhuadong/rtt-bme680)           |
| ......   |           |                                          |                                                             |



## 特别鸣谢

<a href="https://www.jetbrains.com/?from=FCTC-Air"><img src="./images/jetbrains.svg" style="zoom:80%;" /></a>

