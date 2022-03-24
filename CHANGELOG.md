# v2.3.2

## 2.3.2 (2021-11-19)

涂鸦IoTOS 2.3.2版本，在之前的版本的基础上，移除了拆分了IoT、网关、IPC部分的代码到各自独立的组件之中，降低了模块之间的耦合，减少了一些冗余代码，降低了代码的体积；增加了接口函数的描述信息，使得接口使用更加友好；增加了一些重要的能力，提高了Powered by Tuya设备的使用体验。
新增功能请查看CHANGELOG.md
开发帮助请查看README.md

## 版本描述:

涂鸦IoTOS 2.3.2版本，在之前的版本的基础上，移除了拆分了IoT、网关、IPC部分的代码到各自独立的组件之中，降低了模块之间的耦合，减少了一些冗余代码，降低了代码的体积；增加了接口函数的描述信息，使得接口使用更加友好；增加了一些重要的能力，提高了`Powered by Tuya`设备的使用体验。

​	

### 新增功能

#### Plug&Play

TuyaOS支持了`Plug&Play`的能力。`Plug&Play`基于涂鸦蓝牙协议，在设备配网出现异常的时候，可以让使用者仍然可以通过蓝牙激活设备，并支持使用者延后设置Wi-Fi网络SSID和密码；使用者也可以直接跳过Wi-Fi配网，直接使用蓝牙激活，实现了设备的`Plug&Play`。`Plug&Play`不需要开发者进行编码，但是需要涂鸦Wi-Fi&BLE双模模组才可以支持。



#### 闪电配网默认支持

TuyaOS在2.3.0版本支持了闪电配网工作，但是是作为可选能力，TuyaOS2.3.2版本将其作为默认支持的能力提供。只需要客户家里有`Powered by Tuya`设备，即可进行伙伴配网，不需要用户进行配网操作即可自行入网。



#### 涂鸦蓝牙协议升级v4.0版本

为了支持`Plug&Play`功能，新蓝牙协议对协议报文格式进行了修改，并对蓝牙包的报文长度、分包机制进行了优化，增加了蓝牙协议传输带宽，提高了蓝牙控制报文的处理效率和安全性。



#### 涂鸦蓝牙遥控器协议升级v2.0版本

涂鸦IoTOS支持了新版本的蓝牙遥控器协议，同时对老版本的蓝牙遥控器进行兼容。涂鸦IoTOS支持对设备通过蓝牙遥控器进行控制，老版本蓝牙遥控器是基于涂鸦蓝牙协议3.x版本实现的，新版本蓝牙遥控器是基于涂鸦蓝牙协议4.0版本实现的。



#### 设备安全证书机制增强

在访问第三方非涂鸦云链接的时候，提供统一的证书获取机制，避免各自模块关心证书机制，让业务、应用组件更专注于业务的开发。同时提供sha128，sha256，sha384签名算法支持，扩大的安全证书的支持范围。



#### 红外码压缩

支持对红外码进行压缩，降低红外码获取时的数据量，提高了红外码的拉取效率。



#### DP统一上报

整理了当前系统的DP上报接口，将原先的`sf_`为前缀的DP上报接口以及蓝牙上报的接口隐藏，统一使用`dev_`为前缀的接口。DP上报的逻辑在DP模块内部处理。



#### attach module支持更新

新增`tuya_iot_dev_update_attachs`接口，帮助开发者更加灵活的上报附属固件，并支持对已经上报的固件进行更新。支持可插拔的配件进行实时的上报和更新。



#### 同名文件改造

整理了同名头文件，修复了因为同名文件导致开发者编译、链接过程出现异常的情况。



#### IoT-DNS优化

优化了`IoTDNS`的存储结构，降低了`flash`和内存的占用，降低了代码的体积。



#### MQTT 离线优化

修改了`MQTT ping`的实现机制，在原先一包超时即离线的基础上，降低了一包的超时时间，并提升了超时离线的间隔为三包超时离线，同时保证正常情况下`MQTT ping`包的发送周期不变。保证在网络抖动的时候设备能够比较稳定的在线。



#### Wi-Fi备份功能优化

涂鸦IoTOS提供了Wi-Fi备份的功能，但是之前的备份功能是在`TLS`模式为`TLS_TUYA_ECC_PSK`或者`TLS_TUYA_ECC_ONLY`才能够支持，TLS模式修改会需要大量的`flash`资源。为了保证大部分平台可以支持此功能，本次对`flash`空间进行了优化，比上一个版本优化了40k的空间，其中20k用于此功能使用，确保每个平台均可以支持Wi-Fi备份。



#### 进入产测删除UF信息

在开发调试的过程中，发现`uf DB`无法正常写入，确认是`uf key`满了。`dump flash`确认出来发现由于调试过程中更换`key`测试等因素，使得`uf DB`中`key`逐渐变多，但是重新烧录授权过程中并不会清除，存在生产隐患。进入产测授权时删除`uf DB`的头部管理区和管理备份区，确保授权之后UF区域干净安全。



#### wf get mac始终使用station

当`station`和`AP`模式共存时，优先获取`station`的`MAC`，避免一个设备有不同的`MAC`引起误解。





#### 新增事件

| 事件名                    | 参数                  | 用途                       | 备注                                                         |
| ------------------------- | --------------------- | -------------------------- | ------------------------------------------------------------ |
| `EVENT_INIT`              | NULL                  | 通知SDK初始化完成          |                                                              |
| `EVENT_RUN`               | NULL                  | 通知设备正常运行           |                                                              |
| `EVENT_RST_FACT`          | NULL                  | 通知CAD协议版本更新完成    |                                                              |
| `EVENT_CAD_VER_UPDATE`    | NULL                  | 通知CAD协议版本更新完成    | CAD协议版本是指云端、APP、设备之间的互联互通协议             |
| `EVENT_FW_VER_UPDATE`     | NULL                  | 通知固件更新完成           | 上报新版本的时候发出通知                                     |
| `EVENT_POST_ACTIVATE`     | NULL                  | 通知设备激活完成           | 在设备激活拉取schema完成的时候通知                           |
| `EVENT_PROD_INFO_CHANGED` | NULL                  | 通知设备产测授权信息更新   | 仅在设备通过接口设置产测信息之后通知                         |
| `EVENT_NETCFG_ERROR`      | AP_CFG_ERR_CODE       | 通知设备配网失败           | 用于获取配网失败信息，并通过蓝牙上报APP，可以触发蓝牙兜底激活 |
| `EVENT_BLE_ACTIVE_START`  | DEVICE_ACTIVE_STAGE_E | 通知设备开始蓝牙兜底激活   |                                                              |
| `EVENT_MQTT_CONNECTED `   | NULL                  | 通知设备已经连接上MQTT     |                                                              |
| `EVENT_MQTT_DISCONNECTED` | NULL                  | 通知设备已经从MQTT服务断开 |                                                              |
| `EVENT_LAN_DISABLE`       | NULL                  | 通知局域网已经被关闭       |                                                              |
| `EVENT_LAN_ENABLE `       | NULL                  | 通知局域网已经被打开       |                                                              |



### 移除功能

#### uf_file.h

移除了头文件uf_file.h，原先的接口移到了tuya_uf_db.h，代码里的uf_file.h可以替换成tuya_uf_db.h。



#### kv_storage.h

移动kv_storage.h到了src文件夹，原先使用kvs_read/kvs_wirte之类的接口，需要替换成wd_common_read/write。



#### tuya_key.h/c

从TuyaOS移除，作为应用组件。



#### tuya_led.h/c

从TuyaOS移除，作为应用组件。



### 新增接口

#### 新增接口`tuya_hal_system_calloc`和`tuya_hal_system_realloc`

各个芯片平台需要对其进行适配。如果使用SDK对接开发则不需要关心。

```
/**
 * @brief tuya_hal_system_calloc 用于分配并清除干净内存
 * @param[in]       nitems      需要分配内存块的数量
 * @param[in]       size        需要分配块的内存大小
 * @return  分配得到的内存指针
 */
VOID_T *tuya_hal_system_calloc(size_t nitems, size_t size);

/**
 * @brief tuya_hal_system_realloc 重新调整内存大小
 * 
 * @param[in]       nitems      源内存指针
 * @param[in]       size        调整后的大小
 * @return  分配得到的内存指针
 */
VOID_T *tuya_hal_system_realloc(VOID_T* ptr, size_t size);

```



#### 新增接口`tuya_hal_get_rcd_flash_desc(void)`

新增record data base，用于存储大规格的数据，相比KV db和uf db，存储效率提高，目前仅用于网关。



#### 新增接口`tuya_qrcode_set_appid`

用于扫描配网功能，默认使用涂鸦智能APP，如果是涂鸦智能APP扫描，则下发token；其他APP扫描则跳转到应用市场。用户通过此接口设置对应的APP ID，使其拥有相同的能力。



### 修改接口

无

### 移除接口

无

## 使用帮助

#### tuya_main.c 相关的修改

由于tuya_main.c 进行了重新的整理, 强制新增如下接口，由于tuya_main.c仅在RTOS系统使用，没有使用tuya_main.c的可以忽略此章节。对比之前的版本应用接口，主要更新的接口有;

```
VOID_T pre_app_init(VOID_T);
VOID_T mf_user_pre_gpio_test_cb(VOID_T);
VOID_T mf_user_enter_callback(VOID_T);
VOID_T mf_user_callback(VOID_T);
OPERATE_RET mf_user_product_test_cb(USHORT_T cmd,UCHAR_T data, UINT_T len, OUT UCHAR_T ret_data,OUT USHORT_T ret_len);
```

详细的函数的作用和用法可以参考上述的函数注释，常见的需求和修改点可以如下:

- ##### 快速点灯业务;

在应用代码里新增pre_app_init 接口，并且在接口中注册 “EVENT_SDK_EARLY_INIT_OK” 事件，在该事件中实现灯类的快速启动;

```
VOID pre_app_init(VOID)
{
    ty_subscribe_event(EVENT_SDK_EARLY_INIT_OK, "early_init", vLightFastBoot, FALSE);
}
```

- ##### 上下电重置的判断处理;

   将原先实现的上下电次数的判断实现修改为如下的方式, 并放置在pre_device_init函数中;

```
if( TRUE == bFastBootInit) {
	/* set pre app init to judge ifnot reset */
    ty_subscribe_event(EVENT_SDK_DB_INIT_OK, "reset_cnt", opLightCtrlResetCntProcess, FALSE);
}
```

- ##### 关闭15min不进入授权;

在mf_init调用之前,应用可以调用 mf_test_ignore_close_flag API接口来关闭15min累计联网不进入授权的流程，比如可以在 pre_app_init 中可以调用。

- ##### gpio检测;

新增了mf_user_pre_gpio_test_cb接口，此接口需要应用实现，并在初始化的时候作为mf_init的参数，原来的gpio_test不需要再提供。

在mf_user_pre_gpio_test_cb中调用 VOID gpio_test_enabe(BOOL_T enable)， 进行开关gpio检测(关闭的时候, 即使上位机打开gpio检测,固件默认TRUE)。

同时原来的gpio_test_all接口会使用IoTOS提供的接口来实现，不再需要应用提供。



#### 和低功耗相关的修改

 在应用上,需要将 

```
tuya_hal_set_lp_mode(TRUE) \ tuya_hal_set_lp_mode(FALSE); 
```

进行更新替换成   

```
tuya_wifi_lp_enable(); \ tuya_wifi_lp_disable();
```

如果支持低功耗的功能，需要在模块上电的时候调用 tuya_cpu_set_lp_mode(TRUE) 和 tuya_wifi_lp_disable() 接口。

