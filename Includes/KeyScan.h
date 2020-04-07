#ifndef _KeyScan_H_
#define _KeyScan_H_

#define			EN_P4						//如果按键连接到P4上需要使能此项 因为老款STC单片机没有P4
#define			EN_P5						//如果按键连接到P5上需要使能此项 因为老款STC单片机没有P5
typedef		unsigned int	keyporttype;	//数据类型是几位就支持几个按键 定义多了占内存

#define SET_BIT(a,b)                            ((a) |= (1 << (b)))
#define CLEAR_BIT(a,b)                          ((a) &= ~(1 << (b)))
#define IS_BIT_SET(a,b)                         ((a) & (1 << (b)))
#define MIN(a,b)								((a)>(b)?(b):(a))
#define MAX(a,b)								((a)>(b)?(a):(b))

#define DEBOUNCE_TIME                           30			//消抖延时ms
#define PRESS_LONG_TIME                         1500		//长按判定时间ms
#define KEY_MAX_NUMBER							(sizeof(keyporttype)<<3)		//最大支持按键数量(sizeof(keyporttype)*8)
#define N_CLICK_NUMBER							2			//连击判定次数
#define N_CLICK_TIMELIMIT						300			//连击间隔超时时间ms
#define	EVENT_QUEUE_LEN							8			// 事件队列长度，为了简化计算，需要为2的整数次幂

enum EnumKeyNum					//扫描按键值枚举
{
	EnumKey_NoKey = 0
};

enum EnumKeyState		//State是8位变量最多8个
{
	EnumKey_SingleClick = 0,
	EnumKey_ComboClick = 1,
	EnumKey_LongPress = 2,
	EnumKey_MultiPress = 3
};

typedef struct 		//按键特征结构体
{
	keyporttype LastValue;		//保存上一次按键值 抬起时刷新
	keyporttype Triggered;		//单次触发时此值等于键值 之后自动置0 用于短按判定
	keyporttype Continuous;		//跟随键值 用于长按判定
	u8 State;					//按照EnumKeyState描述
}KeyStateTypedef_t;

// functional structs
typedef struct
{
	//51单片机sfr寄存器不能间接寻址 所以使用字符串处理方式
//    uint32_t         keyRccPeriph;
//    GPIO_TypeDef     *keyPort;
//    uint32_t         keyGpio;
	u8 *IOPort1;		//字符串
	u8 *IOPort2;
}KeyIOTypedef_t;

typedef struct
{
    keyporttype TriggerValue;			//按键事件触发时的检测值 可以动态赋值为枚举中的按键值或其组合
    FUNCTIONPTR SingleClick;			//各功能回调函数指针 对应EnumKeyState  最多8个 如果添加功能需要在这里添加功能函数指针
    FUNCTIONPTR ComboClick;
    FUNCTIONPTR LongPress;
    FUNCTIONPTR MultiPress;
}KeyTriggerTypedef_t;

typedef struct
{
    u8 KeyTotalNum;						//按键总数量
    KeyIOTypedef_t *SingleKey;			//按键硬件连接描述
	
	u8 FuncTotalNum;					//按键功能数量
	KeyTriggerTypedef_t *KeyTrigger;	//按键功能描述
}KeysTypedef_t; 

// 环形加法
#define		CIRCULAR_INC(x)		(((x)+1)&(EVENT_QUEUE_LEN-1))
// 环形事件队列
typedef struct
{
	u8 head;	// head指向的元素为空
	u8 tail;
	FUNCTIONPTR queue[EVENT_QUEUE_LEN];
}circularQueue_t;

extern void circularQueueInit(circularQueue_t* eventQueue);
extern void circularQueuePush(circularQueue_t* eventQueue, FUNCTIONPTR func);
extern FUNCTIONPTR circularQueuePop(circularQueue_t* eventQueue);
extern void KeyEventProcess(void);

void KeyScanInit(KeyIOTypedef_t* SingleKey, u8 singleKeyNum, KeyTriggerTypedef_t* KeyFuncs, u8 keyFuncNum);
extern void KeyScanEnable();	//按键扫描使能函数
extern void KeyScanDisable();	//按键扫描停止函数
extern void T0_Init();

#endif
