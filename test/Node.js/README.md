# Node.js 模拟设备接入

安装 Nodejs，详见 <https://nodejs.org>

进入 iot-demo 目录

```shell
cd iot-demo
```

安装阿里云 IoT 套件 SDK 依赖

```shell
npm install
```

启动设备

```shell
node device.js
```

上报数据

```shell
===postData
 topic=/sys/a1p8Pngb3oY/Test01/thing/event/property/post
{
  id: 1596101823441,
  params: { Temp: 21, Humi: 62, Dust: 28, eCO2: 477, TVOC: 68, HCHO: 36 },
  method: 'thing.event.property.post'
}
```