﻿# TCP测速软件

# TCP_cli使用参数：
1、编译的tcp_cli可执行文件；
2、服务器终端IP地址；
3、发送TCP测试包包长；
4、发送数据包间隔，以us为单位
事例：  ./tcp_cli 192.168.100.240 4000 5

# TCP_ser使用参数：
1、编译的tcp_ser可执行文件；
2、接收器地址


# udp_cli使用参数：
1、编译的udp_cli可执行文件；
2、服务器终端IP地址；
3、发送udp测试包包长；
4、发送数据包间隔，以us为单位；
5、测试数据发送总报数；

# udp_ser使用参数：
1、编译的udp_ser使用参数可执行文件；

# udp test实现功能：
1、发送速率、发送包长控制；
2、数据包差错检测；
3、包乱序统计，接收数据包速率计算等等
