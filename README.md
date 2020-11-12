# FCTC-Air

[中文页](README_zh.md) | English

[![GitHub](https://img.shields.io/github/license/luhuadong/fctc-air.svg)](https://github.com/luhuadong/fctc-air/blob/master/LICENSE)
[![GitHub release](https://img.shields.io/github/release/luhuadong/fctc-air.svg)](https://github.com/luhuadong/fctc-air/releases)
[![GitHub pull-requests](https://img.shields.io/github/issues-pr/luhuadong/fctc-air.svg)](https://github.com/luhuadong/fctc-air/pulls)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=flat)](https://github.com/luhuadong/fctc-air/issues)



## Overview

FCTC-Air is a educational project. The purpose is to build a **distributed air quality monitoring system** that everyone can access. 

"**FCTC**" is the abbreviation of "**From Chip To Cloud**", means that involve all stack of IoT design, including embeded software, web, app, and so on. As well as help people pay attention to air pollution.



After clone repo, you need to upgrade submodule:

```shell
git submodule init
git submodule update
```



## Firmware list

| Board | MCU | Flash | RAM | Link |
| -- | -- | -- | -- | -- |
| NUCLEO-L4R5ZI | STM32L4R5 | 2 MB | 640 KB | [details](https://www.st.com/en/evaluation-tools/nucleo-l4r5zi.html) |
| NUCLEO-F767ZI | STM32F767 | 2 MB | 512 KB | [details](https://www.st.com/en/evaluation-tools/nucleo-f767zi.html) |
| AT-START-F403A | AT32F403A | 1 MB | 96 KB | [details](https://blog.csdn.net/lu_embedded/article/details/105816401) |
| Pandora-IoT | STM32L475 | 512 KB | 128 KB |  |
| ART-Pi | STM32H750 | 128 KB + 扩展 SPI Flash | 1 MB + 32MB SDRAM |  |
| ...... |  |  |  |  |




## Sensor list

| sensor | vendor | model | package |
| -- | -- | -- | -- |
| DHTxx | Asair | DHT11, DHT22 | [dhtxx](https://github.com/luhuadong/rtt-dhtxx) |
| DS18B20 | Dallas | DS18B20 | [ds18b20](https://github.com/willianchanlovegithub/ds18b20) |
| GP2Y10xx | Sharp | GP2Y1010, GP2Y1014 | [gp2y10](https://github.com/luhuadong/rtt-gp2y10) |
| PMSxx | Plantower | PMS5003, PMS5003ST, PMS7003, PMS9003 ... | [pmsxx](https://github.com/luhuadong/rtt-pmsxx) |
| SGP30 | Sensirion | SGP30 | [sgp30](https://github.com/luhuadong/rtt-sgp30) |
| CCS811 | AMS | CCS811 | [ccs811](https://github.com/luhuadong/rtt-ccs811) |
| BME280 | Bosch | BME280 | [bme280](https://github.com/RT-Thread-packages/bme280) |
| BME680 | Bosch | BME680 | [bme680](https://github.com/luhuadong/rtt-bme680) |
| ...... |  |  |  |




## Thanks

<a href="https://www.jetbrains.com/?from=FCTC-Air"><img src="./images/jetbrains.svg" style="zoom:80%;" /></a>

