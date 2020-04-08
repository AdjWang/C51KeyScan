# C51KeyScan

状态机、事件队列

状态机还是挺常见，不过为什么要用事件队列呢？因为中断函数不能拖得太长，如果定时器中断执行的事件函数还没完，下一次中断又来了可怎么办……所以加入事件队列机制，中断只负责把需要执行的事件入队，由主函数执行并清空队列里面的事件函数，也算是某种“异步”了吧。

假如主函数清空队列的速度还没有定时器入队快，那么队列满了以后，之后的按键事件直接抛弃，不会响应。

## 参考资料

[机智云的STM32按键扫描](http://docs.gizwits.com/zh-cn/GCP/STM32.html#STM32代码自动生成)

## 资源占用

- 定时器`0` - 按键扫描
- 定时器`1` - 串口`1`
- `Program Size: data=59.3 xdata=100 code=4214`(MDK5 C51编译，8级优化，还是占了挺多空间的，不过功能强大)

## 文件说明

- `KeyScanConfig.h`

  按键扫描相关的一些常量，根据需要修改。

  ```c
  #define     EN_P4                        //如果按键连接到P4上需要使能此项 有的封装没有P4
  #define     EN_P5                        //如果按键连接到P5上需要使能此项 有的封装没有P5
  #define     EN_P6                        //如果按键连接到P6上需要使能此项 有的封装没有P6
  typedef     unsigned int        keyTriggerType_t;    //数据类型是几位就支持几个按键 定义多了占内存
  #define     MAX_KEY_NUMBER      (sizeof(keyTriggerType_t)<<3)//最大支持按键数量为keyTriggerType_t的位数即(sizeof(keyTriggerType_t)*8)
  
  #define     DEBOUNCE_TIME               20          //消抖延时ms
  #define     LONG_PRESS_TIME             1500        //长按判定时间ms
  #define     N_CLICK_NUMBER              2           //连击判定次数
  #define     N_CLICK_TIMELIMIT           300         //连击间隔超时时间ms (超过此时间判定为单击)
  #define     EVENT_QUEUE_LEN             8           // 事件队列长度，为了简化计算，需要为2的整数次幂
  ```

- `KeyScan.h`

  各种枚举、结构体、函数定义。不需要用户修改。

- `KeyScan.c`

  按键扫描的实现。不需要用户修改。


## 使用方法

以下步骤在主函数`main.c`中操作。

1. 把`KeyScan.h, KeyScan.c, KeyScanConfig.h`放到工程目录下。

2. 在主函数中引入头文件

   ```c
   #include "main.h"
   #include "Uart.h"
   #include "KeyScan.h"
   ```

   `main.h`包含基本数据类型的`typedef`定义，一些C库的头文件引入和系统时钟设置；

   `Uart.h`是单片机串口模块头文件；

   `KeyScan.h`是按键扫描模块头文件。

3. 假如现在有`4`个按键`A,B,C,D`，给按键编号，放到枚举类型里。

   ```c
   enum EnumUserKey{                 //按键编号 从0开始 不得超过(MAX_KEY_NUMBER-1)
       EnumKey_A = 0,
       EnumKey_B = 1,
       EnumKey_C = 2,
       EnumKey_D = 3
   };
   ```
   
4. 定义按键相关的两个结构体，作为全局变量。`GPIO_KEY_NUM`是第`3`步中按键的数量，这里是`4`；`FUNC_KEY_NUM`是第`5`步中事件函数的数量，这里是`3`，功能与按键是独立的，数量可以不相等。

   ```c
   #define GPIO_KEY_NUM 4                                  // 按键总数，即enum EnumUserKey定义的按键数量
   xdata KeyIO_t SingleKey[GPIO_KEY_NUM];                  // 按键IO数组
   #define FUNC_KEY_NUM 3                                  // 用户自定义的功能总数
   xdata KeyFunc_t KeyFuncs[FUNC_KEY_NUM];                 // 按键功能数组
   ```

5. 定义按键功能函数。比如，需要按键`A,B`单击分别触发，按键`C,D`同时按下触发，功能函数可以定义成下面这样，函数名字随意。

   ```c
   void KeyAPressEvent(void){
       P40 = ~P40;
   }
   void KeyBPressEvent(void){
       Delay100ms();
   }
   void KeyCDPressEvent(void){
       P41 = ~P41;
       // printf发送长串被中断打断会死机，使用UartSendString
       // 如果很短可以使用printf
       UartSendString("testtesttesttesttesttesttesttesttesttesttesttesttest\r\n");
       Delay100ms();       // 长延时也不会死机了，哈哈
       UartSendString("testtesttesttesttesttesttesttesttesttesttesttesttest\r\n");
       Delay100ms();
   }
   ```

6. 好的，现在按键有了，功能也有了，但是还没联系到一起。下面是按键扫描初始化函数，把它们联系起来。

   ```c
   //按键扫描初始化
   void KeyInit(void){
       u8 i;
       // 函数指针必须全部初始化为NULL
       for(i=0; i<FUNC_KEY_NUM; i++){
           KeyFuncs[i].fp_singleClick = NULL;
           KeyFuncs[i].fp_comboClick = NULL;
           KeyFuncs[i].fp_longPress = NULL;
           KeyFuncs[i].fp_multiPress = NULL;
       }
       
       // 注册按键 Port1必须是IO口 Port2是IO口或"GND"
       SingleKey[EnumKey_A].IOPort1 = "P36"; SingleKey[EnumKey_A].IOPort2 = "GND";
       SingleKey[EnumKey_B].IOPort1 = "P52"; SingleKey[EnumKey_B].IOPort2 = "GND";
       SingleKey[EnumKey_C].IOPort1 = "P54"; SingleKey[EnumKey_C].IOPort2 = "GND";
       SingleKey[EnumKey_D].IOPort1 = "P53"; SingleKey[EnumKey_D].IOPort2 = "GND";
       
       // 需要响应的键值 注意是键值! 不是键编号! 组合按键用或
       KeyFuncs[0].triggerValue = TRIGGER_VALUE(EnumKey_A);
       // 注册回调函数为单击功能
       KeyFuncs[0].fp_singleClick = KeyAPressEvent;
       
       // 需要响应的键值 注意是键值! 不是键编号! 组合按键用或
       KeyFuncs[1].triggerValue = TRIGGER_VALUE(EnumKey_B);
       // 注册回调函数为单击功能
       KeyFuncs[1].fp_singleClick = KeyBPressEvent;
       
       // 需要响应的键值 注意是键值! 不是键编号! 组合按键用或
       KeyFuncs[2].triggerValue = TRIGGER_VALUE(EnumKey_C) | TRIGGER_VALUE(EnumKey_D);
       // 注册回调函数为组合键功能
       KeyFuncs[2].fp_multiPress = KeyCDPressEvent;
       
       KeyScanInit((KeyIO_t*)&SingleKey, GPIO_KEY_NUM, (KeyFunc_t*)&KeyFuncs, FUNC_KEY_NUM);
   }
   ```

   初始化过程可以分为`4`个步骤：

   1. 初始化函数指针为`NULL`，这里的函数指针变量来自第`4`步定义的`xdata KeyFunc_t KeyFuncs[FUNC_KEY_NUM]`

   2. 告诉单片机按键的硬件连线位置，假如按键`A,B,C,D`的一端分别连到单片机的`P36,P52,P54,P53`上，另一端接地，就按照上面的程序设置。如果按键是矩阵的，没有接地，就把按键两端的`IO`都对应写成字符串。
   > 这么做的好处就是可以把按键随便乱接，毕竟有的封装，比如`SOP-16`是没有完整的一组`8bit IO`引出的，假如在这个单片机上用传统的方式应用`4×4`矩阵键盘，位处理是不是特别难受？
   
   3. 这一步把按键和一个事件函数联系起来。
   
      需要用到`TRIGGER_VALUE`宏，把按键编号转换成触发值，如果用到组合键，把各个按键的触发值用`|`连接即可。
   
      还需要注意的就是按键的功能是靠结构体成员的名字来区分的，有`fp_singleClick, fp_comboClick, fp_longPress, fp_multiPress`共`4`种，给哪个赋值，对应的事件函数就是什么功能。
   
   4. 把刚才设置好的结构体给到按键扫描程序，开始按键扫描。
   
7. 编写主函数，把`KeyInit()`放到初始化里，把`KeyEventProcess()`放到`while(1)`里。

   ```c
   void main(){
       EA = 1;
       UartInit();
       KeyInit();        //按键扫描初始化
       // printf发送长串被中断打断会死机，使用UartSendString
       // printf("testtesttesttesttesttesttesttesttesttesttesttesttest\r\n");
       UartSendString("testtesttesttesttesttesttesttesttesttesttesttesttest\r\n");
       while(1){
           KeyEventProcess();
       }
   }
   ```

   `KeyEventProcess()`检查事件队列，执行队列里所有函数。如果队列满了，那么再有按键事件的话，就会被忽略。所以，慢点按，哈哈。(其实单片机速度够，快点按也没事，而且队列长度能改，在`KeyScanConfig.h`里)

## 注意事项

- `printf`发送字符串有`bug`。

  具体发生在`printf`发送过程中被中断打断的时候，现象是死机，串口不断发送乱码。调节串口中断优先级为最高没有改善。

  如果没有中断打断，不会有`bug`。
  
- 系统主频率在`main.h`里对应实际主频率修改。

- 串口波特率在`Uart.h`里对应实际波特率修改。

- 目前仅在`IAP15W4K61S4`上实验，`15`系列的单片机可以直接用，如果更换其他系列单片机需要修改`Timer0`的初始化和中断函数。(在`KeyScan.c`里定义)

- 编译报了`3`个警告，不影响。

## 其他

设计思路和函数说明参考`wiki`(还在写……)

