Tuya IoTOS Embeded SDK WiFi & BLE for BK7231T

[English] | [中文版](https://github.com/tuya/tuya-iotos-embeded-sdk-wifi-ble-bk7231t/README_zh.md)

## Introduction

Tuya IoTOS is independently developed by TuyaSmart, the world's only IoT operating system covering all levels of IoT sensing, interruption, network, platform, and application. Benefiting from Tuya Smart's accumulation in the IoT industry, TuyaIoTOS provides a full range of products from product design, R&D, access to post-operation solutions.

Tuya IoTOS embedded SDK is an important part of Tuya IoTOS. Through good design, it provides customers with a unified development interface, rich development kits and self-service customization functions, shielding the fragmented characteristics of the IoT industry, which can be applied to IoT applications in industries such as industrial IoT, vehicle networking, security monitoring, outdoor travel, smart home development.

Tuya IoTOS embedded SDK has the following features.

* Flexible tailoring: Tuya IoTOS embedded SDK provides a rich development kit covering IoT operating system kernel, general IoT functions, IoT technology stacks in different fields, developers can choose the corresponding solutions according to the needs of their products, and developers with the ability to do deep customization online and on demand.
* Cross-platform: The Tuya IoTOS embedded SDK design level supports cross-hardware platform and operating system features that can be ported on any hardware chip and system, greatly improving the development efficiency of developers.
* Security and privacy: Tuya IoTOS embedded SDK provides features such as secure data storage, secure network communication, identity verification, secure startup, secure upgrade, and strict compliance with security and privacy policies around the world, effectively guaranteeing the security and privacy features of the product.
* Rich ecology: Tuya Smart has worked closely with major chip originators and imported 100+ models for developers to choose from, while all products developed with Tuya IoTOS are able to interoperate and scene linkage, Powered by Tuya has formed a complete IoT industry ecology.



## Get Started

### Requirements and Dependencies

Some package dependencies need to be installed before compilation:

#### Debian/Ubuntu
``` bash
$ sudo apt-get install wget git python libc6-i386 
```

#### Windows

Supports compilation under Cygwin, MinGW, WSL2 and other environments

### Download SDK

Be sure to clone recursively to download SDK:
``` bash
$ git clone --recursive https://github.com/tuya/tuya-iotos-embeded-sdk-wifi-ble-bk7231t.git
```
or
``` bash
$ git clone https://github.com/tuya/tuya-iotos-embeded-sdk-wifi-ble-bk7231t.git
$ git submodule update --init
```

### Compile

The project is located in the `apps` directory, you can use the `template_demo` project to quickly complete the compile, burn, run process, use the following command in the root directory to compile:
``` bash
$ sh build_app.sh apps/template_demo template_demo 1.0.0
```
- apps/template_demo: compile project path

- template_demo: compile project name

- 1.0.0: compile project version

### Compile clean

To clear the compiled intermediate files, use the following command:

``` bash
$ sh build_app.sh apps/template_demo template_demo 1.0.0 clean
```

### Program and authorization

Firmware program and authorization information please refer to the relevant ：[Wi-Fi + BLE module program and authorization](https://developer.tuya.com/cn/docs/iot/device-development/burn-and-authorization/burn-and-authorize-wifi-ble-modules/burn-and-authorize-wb-series-modules?id=Ka78f4pttsytd)

### Evaluation kits 

Evaluation kits information please refer to the relevant：[Sandwich Evaluation Kits](https://developer.tuya.com/en/docs/iot/device-development/tuya-development-board-kit/tuya-sandwich-evaluation-kits/-tuya-sandwich-evaluation-kits?id=K97o0ixytemvr)



## Support 

You can get support from Tuya with the following methods:
- Tuya Developer Center: https://developer.tuya.com/en/
- Tuya Smart Help Center: https://support.tuya.com/en/help
- Technical Support Council: https://iot.tuya.com/council/