/**
"dependencies": { "mqtt": "2.18.8" }
*/
const crypto = require('crypto');
const mqtt = require('mqtt');
//设备身份三元组+区域
const deviceConfig = {
    productKey: "替换",
    deviceName: "替换",
    deviceSecret: "替换",
    regionId: "cn-shanghai"
};
//根据三元组生成mqtt连接参数
const options = initMqttOptions(deviceConfig);
const url = `tcp://${deviceConfig.productKey}.iot-as-mqtt.${deviceConfig.regionId}.aliyuncs.com:1883`;

//2.建立连接
const client = mqtt.connect(url, options);

//3.属性数据上报
const topic = `/sys/${deviceConfig.productKey}/${deviceConfig.deviceName}/thing/event/property/post`;
setInterval(function() {
    //发布数据到topic
    client.publish(topic, getPostData());
}, 5 * 1000);

//4.订阅主题,接收指令
const subTopic = `/${deviceConfig.productKey}/${deviceConfig.deviceName}/user/control`;
client.subscribe(subTopic)
client.on('message', function(topic, message) {
    console.log("topic " + topic)
    console.log("message " + message)
})

//IoT平台mqtt连接参数初始化
function initMqttOptions(deviceConfig) {

    const params = {
        productKey: deviceConfig.productKey,
        deviceName: deviceConfig.deviceName,
        timestamp: Date.now(),
        clientId: Math.random().toString(36).substr(2),
    }
    //CONNECT参数
    const options = {
        keepalive: 60, //60s
        clean: false, //cleanSession保持持久会话
        protocolVersion: 4 //MQTT v3.1.1
    }
    //1.生成clientId，username，password
    options.password = signHmacSha1(params, deviceConfig.deviceSecret);
    options.clientId = `${params.clientId}|securemode=3,signmethod=hmacsha1,timestamp=${params.timestamp}|`;
    options.username = `${params.deviceName}&${params.productKey}`;

    return options;
}

function getPostData() {
    const payloadJson = {
        id: Date.now(),
        params: {
            Temp: Math.floor((Math.random() * 10) + 20),
            Humi: Math.floor((Math.random() * 20) + 50),
            Dust: Math.floor((Math.random() * 50) + 10),
            eCO2: Math.floor((Math.random() * 200) + 400),
            TVOC: Math.floor((Math.random() * 200) + 0),
            HCHO: Math.floor((Math.random() * 100) + 0)
        },
        method: "thing.event.property.post"
    }

    console.log("===postData\n topic=" + topic)
    console.log(payloadJson)

    return JSON.stringify(payloadJson);
}

/*
  生成基于HmacSha1的password
  参考文档：https://help.aliyun.com/document_detail/73742.html?#h2-url-1
*/
function signHmacSha1(params, deviceSecret) {

    let keys = Object.keys(params).sort();
    // 按字典序排序
    keys = keys.sort();
    const list = [];
    keys.map((key) => {
        list.push(`${key}${params[key]}`);
    });
    const contentStr = list.join('');
    return crypto.createHmac('sha1', deviceSecret).update(contentStr).digest('hex');
}