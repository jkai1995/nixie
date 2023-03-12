# nixie
![image](https://user-images.githubusercontent.com/38028595/166111458-cdb046b1-1742-43e4-ba70-ae57c597d19b.png)
![image](https://user-images.githubusercontent.com/38028595/166111470-92a88164-8f54-4ddc-a3d3-26943aa12e6a.png)
![image](https://user-images.githubusercontent.com/38028595/166111516-1ad202c0-a61c-46da-b878-3d569f0bfd22.png)


## 硬件结构
IN-12的管子,串联74HC595驱动

12-170V升压用MAX1771

stm32+DS3231

蓝牙BLE用于控制

## 软件
FreeRTOS

letter-shell调试和控制，非常方便 https://github.com/NevermindZZT/letter-shell

### 软件结构
![image](https://user-images.githubusercontent.com/38028595/224524958-94d6a370-3274-497a-a83e-287177f59f09.png)

realtimeCtrl线程读写时钟芯片ds3231的数据，并通过6片串联的串入并出移位寄存器74HC595驱动辉光管

shell线程通过JDY08蓝牙模块或者直连的串口交互，支持多种命令

## 上位机

## 调试问题

### 升压电路
一开始用的一体成型电感只有空载不会发热，稍微加一点负载就会严重温升

换了更大电流的铁硅铝磁环电感可以不发热

参数选择应该是绰绰有余的，

分析应该是买到了假的芯片，输出并不是手册上说明的PFM，而是固定频率的高频(远低于手册的频率)可调频率的低频 “与” 形成的信号
而买来的max1771模块的输出是和手册一致的高频PFM
