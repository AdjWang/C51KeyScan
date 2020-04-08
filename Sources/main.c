/**
 * Copyright(C),2019-2020
 * FileName: main.c
 * Author: AdjWang
 * Version: 2.0
 * Date: 2020/04/08
 * Description:  main source.
 * History: 
 *     2018/10/09    V1.0   first version.
 *     2020/04/08    V2.0   fix fp init bug;
 *                          add IODataQueue;
 *                          add KeyScanConfig.h.
*/
#include "main.h"
#include "Uart.h"
#include "KeyScan.h"

enum EnumUserKey{                 //按键编号 从0开始 不得超过(MAX_KEY_NUMBER-1)
    EnumKey_A = 0,
    EnumKey_B = 1,
    EnumKey_C = 2,
    EnumKey_D = 3
};
#define GPIO_KEY_NUM 4                                  // 按键总数，即enum EnumUserKey定义的按键数量
xdata KeyIO_t SingleKey[GPIO_KEY_NUM];                  // 按键IO数组
#define FUNC_KEY_NUM 3                                  // 用户自定义的功能总数
xdata KeyFunc_t KeyFuncs[FUNC_KEY_NUM];                 // 按键功能数组

void Delay100ms()        //@11.0592MHz
{
    unsigned char i, j, k;

    _nop_();
    _nop_();
    i = 5;
    j = 52;
    k = 195;
    do
    {
        do
        {
            while (--k);
        } while (--j);
    } while (--i);
}

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
//    Delay100ms();       // 长延时也不会死机了，哈哈
//    UartSendString("testtesttesttesttesttesttesttesttesttesttesttesttest\r\n");
//    Delay100ms();
}

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


