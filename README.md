# nixie

![image](https://user-images.githubusercontent.com/38028595/166099217-2baee893-2fee-4cbb-b281-c7be31b3ba84.png)

## 硬件结构
IN-12的管子,串联74HC595驱动

12-170V升压用MAX1771

stm32+DS3231

蓝牙BLE用于控制

## 软件
FreeRTOS

letter-shell调试和控制，非常方便 https://github.com/NevermindZZT/letter-shell

## 上位机

## 调试问题

### 升压电路
一开始用的一体成型电感只有空载不会发热，稍微加一点负载就会严重温升

换了更大电流的铁硅铝磁环电感可以不发热

参数选择应该是绰绰有余的，

分析应该是买到了假的芯片，输出并不是手册上说明的PFM，而是固定频率的高频(远低于手册的频率)可调频率的低频 “与” 形成的信号
而买来的max1771模块的输出是和手册一致的高频PFM
