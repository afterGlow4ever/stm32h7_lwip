一	工程项目：
1	项目名称：stm32h7_lwip
2	项目使用的资源与软件：
(1)	正点原子阿波罗STM32H743开发板
(2)	Keil uvision5
(3)	野火调试助手1.0.1.5
(4)	Wireshark2.6.2

二	工程目标：
1	实验现象：
(1)	与上位机进行网络通信，包括TCP Client、TCP Server和UDP三种方式。
2	实验内容：
(1)	使用Lwip协议栈，实现TCP Client、TCP Server和UDP三种功能。
(2)	利用上位机软件，进行网络通信和抓包。
3	具体要求：
(1)	按键控制链接、断开、发送数据，实时准备接收数据。
(2)	在baremental环境下，使用raw接口实现TCP Client、TCP Server和UDP通信。
(3)	在FreeRTOS环境下，使用netconn接口和socket接口实现TCP Client、TCP Server和UDP通信。

三	学习目标（下述内容非先后顺序）：
1	学习STM32H7的以太网外设：
(1)	了解STM32H7的ETH外设，完成初始化和寄存器配置。
(2)	了解并对STM32H7的ETH专用地址空间进行MPU保护。
2	学习网口的使用：
(1)	了解LAN8720A的以太网接口电路。
(2)	了解LAN8720A的以太网初始化操作、寄存器配置（包括自适应操作等）。
3	学习TCP/IP协议：
(1)	掌握MAC地址、IP地址、子网掩码、默认网关的概念。
(2)	了解ARP协议、IGMP协议、ICMP协议、TCP协议、UDP协议及其帧的构成。
4	学习Lwip协议栈：
(1)	了解Lwip协议栈。
(2)	掌握在baremental环境下和FreeRTOS环境下的Lwip移植过程，了解底层的5个基本函数。
(3)	掌握在baremental环境下，使用raw接口实现TCP Client、TCP Server和UDP相关功能的函数。
(4)	掌握在FreeRTOS环境下的，使用netconn接口和socket接口实现TCP Client、TCP Server和UDP相关功能的函数。

四	可以重复利用的资源：
1	工程项目目录下Library下面的内容，包括STM32H7的HAL库、FreeRTOSv9.0.0和Lwip2.1.2的程序代码。
2	工程项目目录下User\fw_lwip_baremental\LwIP_arch目录和User\fw_lwip_freertos\LwIP_arch，这里包含了Lwip协议栈的移植和基本功能的实现。

五	注意事项：
1	该项目包含TCP Client、TCP Server和UDP三种通信方式，具体功能使用由design_define.h里面的宏定义决定。
2	系统上电前，务必检查网口网线链接是否正确。
3	小编时间仓促，因此项目中有不完善的地方敬请大家提出建议。
4	学习交流群1126635164。

六	项目环境目录介绍：
项目目录下，各文件夹的含义是（含空文件夹）：
Doc目录为项目文档，包括参考资料、设计文档、原理图等。
Library目录为项目所使用到的库文件，包括仿真库、Monitor、Model、ThirdParty Firmware等。
Project目录为项目包含的工程的目录，包括fpga工程、firmware工程、pcb工程等，以及工程专用代码、脚本等。
Script目录包含项目专用的脚本、功能等。
Sim目录为项目包含的仿真的目录，包含仿真工程、testbench、激励文件、脚本等、以及仿真专用代码、脚本等。
User目录包含用户主要的项目文件，包含rtl代码、firmware代码等。
目前最适合的使用方法是使用Cygwin，编辑代码、文件操作在Cygwin下面，软件使用在Windows下面，可以大幅节省开发时间。后期会开发快捷键切换目录，届时适合在Cygwin、Linux环境下运行。
