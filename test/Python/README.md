# Python 模拟设备接入

安装 Python，详见 <https://www.python.org/downloads/>

进入 iot-demo 目录

```shell
cd iot-demo
```

安装阿里云 IoT 套件 SDK 依赖

```shell
pip3 install paho-mqtt
```

启动设备

```shell
python3 device.py
```

上报数据

```json
send data to iot server: 
{
    'id': 1596177647, 
    'params': 
    {
        'Temp': 22, 
        'Humi': 67, 
        'Dust': 12, 
        'eCO2': 447, 
        'TVOC': 107, 
        'HCHO': 43
    }, 
    'method': 'thing.event.property.post'
}
```