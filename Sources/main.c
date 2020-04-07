#include "main.h"
#include "Uart.h"
#include "KeyScan.h"

extern xdata circularQueue_t eventQueue;

enum EnumUserKey 				//按键编号和键值枚举 编号从0开始 不得超过(KEY_MAX_NUMBER-1)
{
	EnumKey_A  = 0, EnumKey_Up_TriggerValue = 1<<EnumKey_A,
	EnumKey_B  = 1, EnumKey_Down_TriggerValue = 1<<EnumKey_B,
	EnumKey_C  = 2, EnumKey_Left_TriggerValue = 1<<EnumKey_C,
	EnumKey_D  = 3, EnumKey_Right_TriggerValue = 1<<EnumKey_D
};

#define GPIO_KEY_NUM 4                                  //按键成员总数
xdata KeyIOTypedef_t SingleKey[GPIO_KEY_NUM];                   //单个按键成员数组指针
#define FUNC_KEY_NUM 3									//用户自定义的功能总数
xdata KeyTriggerTypedef_t KeyFuncs[FUNC_KEY_NUM];               //按键功能数组指针

xdata KeysTypedef_t Keys;                                     //总的按键模块结构体指针

void Key7ShortPressEvent(void)
{
	P40 = ~P40;
}
void Key12ShortPressEvent(void)
{
	P41 = ~P41;
}
void Key17_22ShortPressEvent(void)
{
	P41 = ~P41;
	Uart_SendString("Func2!lasjdfhaksfdhjaskdnclhfenclwaef \r\n");
	//这个太慢了 会死机! 放主函数里也不行 串口被打断了就会卡死
}


void KeyInit(void)		//按键扫描初始化
{
	u8 i;
	for(i=0; i<FUNC_KEY_NUM; i++){
		KeyFuncs[i].SingleClick = NULL;
		KeyFuncs[i].ComboClick = NULL;
		KeyFuncs[i].LongPress = NULL;
		KeyFuncs[i].MultiPress = NULL;
	}
	
	SingleKey[EnumKey_A].IOPort1 = "P36"; SingleKey[EnumKey_A].IOPort2 = "GND";		//注册按键 Port1必须是IO口 Port2是IO口或"GND"
	SingleKey[EnumKey_B].IOPort1 = "P52"; SingleKey[EnumKey_B].IOPort2 = "GND";
	SingleKey[EnumKey_C].IOPort1 = "P54"; SingleKey[EnumKey_C].IOPort2 = "GND";
	SingleKey[EnumKey_D].IOPort1 = "P53"; SingleKey[EnumKey_D].IOPort2 = "GND";
	
	KeyFuncs[0].TriggerValue = EnumKey_Up_TriggerValue;			//需要响应的键值 注意是键值! 不是键编号! 组合按键用或
	KeyFuncs[0].SingleClick = Key7ShortPressEvent;		//注册回调函数
	KeyFuncs[1].TriggerValue = EnumKey_Down_TriggerValue;		//需要响应的键值 注意是键值! 不是键编号! 组合按键用或
	KeyFuncs[1].SingleClick = Key12ShortPressEvent;		//注册回调函数
	KeyFuncs[2].TriggerValue = EnumKey_Left_TriggerValue | EnumKey_Right_TriggerValue;		//需要响应的键值 注意是键值! 不是键编号! 组合按键用或
	KeyFuncs[2].MultiPress = Key17_22ShortPressEvent;		//注册回调函数
	
	KeyScanInit(SingleKey, GPIO_KEY_NUM, KeyFuncs, FUNC_KEY_NUM);
}

void Delay100ms()		//@11.0592MHz
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

void main()
{
	EA = 1;
	Uart_Init();
	KeyInit();		//按键扫描初始化
	Delay100ms();
	printf("test\r\n");
	while(1)
	{
		//WDT_CONTR = 0x3C;		//喂狗
		//Uart_SendString("test\r\n");
		P67 = 0;
		KeyEventProcess();
		P67 = 1;  // off
		//printf("head: %bd, tail: %bd\r\n", eventQueue.head, eventQueue.tail);
	}
}


