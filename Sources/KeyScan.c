#include "main.h"
#include "KeyScan.h"

/* TODO list:
1. config.h
*2. event_queue
3. hash(TriggerValue)
*/


extern xdata KeysTypedef_t Keys;
static xdata circularQueue_t eventQueue;

volatile u8 state = 0;	//初始化为停止状态
volatile KeyStateTypedef_t Key;		//按键特征全局变量

void circularQueueInit(circularQueue_t* eventQueue)
{
//	u8 i;
//	for(i=0; i<EVENT_QUEUE_LEN; i++){
//		eventQueue->queue[i] = NULL;
//	}
	eventQueue->head = eventQueue->tail = 0;
}

/** 
 * @brief 队列入队
 * @param 
 *     eventQueue
 *     func
 *
 * @return 
 *     暂时不返回错误值
 * 
 * @note
	//	eventQueue->tail = nextTail;
	//	// 如果在这里被打断会出问题，此时eventQueue->queue[nextTail]无效，不可以执行
	//	eventQueue->queue[nextTail] = func;
 */
void circularQueuePush(circularQueue_t* eventQueue, FUNCTIONPTR func)
{
	u8 nextTail = CIRCULAR_INC(eventQueue->tail);
	if(nextTail == eventQueue->head) return;	// full
	eventQueue->queue[nextTail] = func;
	eventQueue->tail = nextTail;
}

/** 
 * @brief 队列出队
 * @param 
 *     eventQueue
 *
 * @return 
 *     返回队列头的函数指针
 * 
 * @note
 *     eventQueue->head = CIRCULAR_INC(eventQueue->head);
 *     这行产生了一个奇妙的bug
 *     在此行之后读eventQueue->head有一定概率读不对，出乱码
 *     使用0级优化可以解决该问题，但是看了汇编也没看出来是啥问题...(⊙_⊙)?
 *     最后试出来另一个办法，程序如下面所示
 */
FUNCTIONPTR circularQueuePop(circularQueue_t* eventQueue)
{
	if(eventQueue->tail == eventQueue->head){return NULL;}	// empty
	eventQueue->head = CIRCULAR_INC(eventQueue->head);		// Accursed expression, where a strange problem occured!
	if(eventQueue->head == 256){printf("%d\r\n", *(int*)eventQueue);} // This line will never be executed, but it do fix the f**king bug!!
	return eventQueue->queue[eventQueue->head];
}

void KeyEventProcess(void)
{
	FUNCTIONPTR key_event;
	while((key_event = circularQueuePop(&eventQueue)) != NULL)	// 按键事件函数轮询
	{
		(*key_event)();
	}
}

void KeyScanInit(KeyIOTypedef_t* SingleKey, u8 singleKeyNum, KeyTriggerTypedef_t* KeyFuncs, u8 keyFuncNum)
{
	circularQueueInit(&eventQueue);		// 按键事件队列初始化 这里不需要修改
	Keys.KeyTotalNum = MIN(singleKeyNum, KEY_MAX_NUMBER);		//按键成员总数 这里不需要修改
    Keys.SingleKey = SingleKey;		//链接结构体 这里不需要修改
	Keys.FuncTotalNum = keyFuncNum;		//功能总数 这里不需要修改
    Keys.KeyTrigger = KeyFuncs;		//链接结构体 这里不需要修改

	KeyScanEnable();		//使能按键扫描
	T0_Init();		//初始化定时器 这里使用定时器0扫描按键
}

//参数1输入3位字符串 如P00 P36 参数2输入控制电平
void GPIO_WriteBit(u8 *GPIO, bit WriteBit)
{
	if(GPIO[0] != 'P' && GPIO[0] != 'p') return;
	switch(GPIO[1])
	{
		case '0':
			WriteBit ? SET_BIT(P0, GPIO[2]-'0') : CLEAR_BIT(P0, GPIO[2]-'0');
			break;
		case '1':
			WriteBit ? SET_BIT(P1, GPIO[2]-'0') : CLEAR_BIT(P1, GPIO[2]-'0');
			break;
		case '2':
			WriteBit ? SET_BIT(P2, GPIO[2]-'0') : CLEAR_BIT(P2, GPIO[2]-'0');
			break;
		case '3':
			WriteBit ? SET_BIT(P3, GPIO[2]-'0') : CLEAR_BIT(P3, GPIO[2]-'0');
			break;
		#ifdef		EN_P4
		case '4':
			WriteBit ? SET_BIT(P4, GPIO[2]-'0') : CLEAR_BIT(P4, GPIO[2]-'0');
			break;
		#endif
		#ifdef		EN_P5
		case '5':
			WriteBit ? SET_BIT(P5, GPIO[2]-'0') : CLEAR_BIT(P5, GPIO[2]-'0');
			break;
		#endif
	}
}
//参数1输入3位字符串 如P00 P36 注意返回值类型不是bit
u8 GPIO_ReadBit(u8 *GPIO)
{
	if(GPIO[0] != 'P' && GPIO[0] != 'p') return 0;
	switch(GPIO[1])
	{
		case '0':
			SET_BIT(P0, GPIO[2]-'0'); return IS_BIT_SET(P0, GPIO[2]-'0');
			break;
		case '1':
			SET_BIT(P1, GPIO[2]-'0'); return IS_BIT_SET(P1, GPIO[2]-'0');
			break;
		case '2':
			SET_BIT(P2, GPIO[2]-'0'); return IS_BIT_SET(P2, GPIO[2]-'0');
			break;
		case '3':
			SET_BIT(P3, GPIO[2]-'0'); return IS_BIT_SET(P3, GPIO[2]-'0');
			break;
		#ifdef		EN_P4
		case '4':
			SET_BIT(P4, GPIO[2]-'0'); return IS_BIT_SET(P4, GPIO[2]-'0');
			break;
		#endif
		#ifdef		EN_P5
		case '5':
			SET_BIT(P5, GPIO[2]-'0'); return IS_BIT_SET(P5, GPIO[2]-'0');
			break;
		#endif
	}
	return 0;
}

/** 
 * @brief 键值读取
 * @param None
 *
 * @return 返回说明
 *     keyprottype ReadData
 *     该值使用置位方式记录键值
 *     ReadData 第0位 对应 Keys.SingleKey[0] 的状态
 *     ReadData 第1位 对应 Keys.SingleKey[1] 的状态
 *     ReadData 第2位 对应 Keys.SingleKey[2] 的状态
 *     ...以此类推
 * 
 * @note 如果要改为整组IO的按键，需要先分析按键状态，然后映射到 ReadData 对应的位
 */
static keyporttype singleKeyRead(void)
{
	u8 i;
	keyporttype ReadData = 0;
	
	for(i=0; i<Keys.KeyTotalNum; i++)
	{
		//Port2是端口不是GND 矩阵按键连接方式
		//操作方式
		//IO2置1 IO1置0 读IO2
		//IO1置1 IO2置0 读IO1
		if(Keys.SingleKey[i].IOPort2[0] == 'P' || Keys.SingleKey[i].IOPort2[0] == 'p')	//Pxx
		{
			GPIO_WriteBit(Keys.SingleKey[i].IOPort2, 1);
			GPIO_WriteBit(Keys.SingleKey[i].IOPort1, 0);
			if(GPIO_ReadBit(Keys.SingleKey[i].IOPort2) == 0x00)			//第一次读到0则继续
			{
				GPIO_WriteBit(Keys.SingleKey[i].IOPort1, 1);
				GPIO_WriteBit(Keys.SingleKey[i].IOPort2, 0);
				if(GPIO_ReadBit(Keys.SingleKey[i].IOPort1) == 0x00)		//两次都读到0
					SET_BIT(ReadData, i);
			}
		}
		//单线连接方式 按键另一端默认接地
		//操作方式
		//IO1置1 读IO1
		else
		{
			GPIO_WriteBit(Keys.SingleKey[i].IOPort1, 1);
			if(GPIO_ReadBit(Keys.SingleKey[i].IOPort1) == 0x00)
				SET_BIT(ReadData, i);
		}
	}
	return ReadData;
}

keyporttype KeyRead(void)
{
	return singleKeyRead();
	// return portKeyRead();
}

void State0() {}	//任务停止
void State1()		//按下检测
{
	keyporttype ReadData;
	ReadData = KeyRead();
	if(ReadData != EnumKey_NoKey)		//检测到按下
		state++;
	else								//稳定抬起
	{
		//稳定抬起时刷新
		Key.Triggered = ReadData & (ReadData ^ Key.Continuous);
		Key.Continuous = ReadData;	
	}
}
void State2()		//消抖后再次检测
{
	if(KeyRead() != EnumKey_NoKey)		//确认按下
	{
		state++;
	}
	else								//抖动
		state--;
}
void State3()		//抬起检测
{
	static u8 nClick = 0;		//连击计数
	static u8 nClickTimeOut = 0;
	static u16 KeyLongCheck = 0;
	keyporttype ReadData;
	ReadData = KeyRead();
	
	nClickTimeOut++;
	if(nClickTimeOut == (N_CLICK_TIMELIMIT / DEBOUNCE_TIME)-1)
	{
		nClickTimeOut = 0;
		nClick = 0;
	}
	
	if(ReadData == EnumKey_NoKey)		//检测到抬起
	{
		KeyLongCheck = 0;				//长按计时清零
		state++;
	}
	else								//稳定按下
	{
		//稳定按下时刷新
		Key.Triggered = ReadData & (ReadData ^ Key.Continuous);
		Key.Continuous = ReadData;	
		
		if(Key.Triggered != EnumKey_NoKey)	//短按 包含单键和组合键
		{
			SET_BIT(Key.State, ((Key.Triggered == Key.Continuous) ? EnumKey_SingleClick : EnumKey_MultiPress));
		}

		KeyLongCheck++;
		if(KeyLongCheck == (PRESS_LONG_TIME / DEBOUNCE_TIME)-1)			//长按
		{
			KeyLongCheck = 0;
			SET_BIT(Key.State, EnumKey_LongPress);
		}
		
		if(Key.LastValue != 0 && Key.Triggered == Key.LastValue)
		{
			nClick++;
			nClickTimeOut = 0;
			if(nClick == N_CLICK_NUMBER)				//连击
			{
				nClick = 0;
				SET_BIT(Key.State, EnumKey_ComboClick);
			}
		}
	}
}
void State4()		//消抖后再次检测
{
	if(KeyRead() == EnumKey_NoKey)		//确认抬起
	{
		Key.LastValue = Key.Continuous;		//记录键值
		state = 1;
	}
	else								//抖动
		state--;	
}

code FUNCTIONPTR States[] = {State0, State1, State2, State3, State4};		//各状态函数指针数组
void KeyScan() { (*States[state])(); }		//按键扫描函数 轮询各状态
void KeyScanEnable() { Key.State = 0x00; state = 1; }			//按键扫描使能函数
void KeyScanDisable() {Key.Triggered = Key.Continuous = EnumKey_NoKey; state = 0; }			//按键扫描停止函数

void KeyHandle(KeysTypedef_t *Keys)
{
	u8 i;
	KeyScan();
	if(Key.Continuous == EnumKey_NoKey) return;
	//功能遍历 对应EnumKeyState 最多8个 如果添加功能需要在这里添加功能函数
	if(IS_BIT_SET(Key.State, EnumKey_SingleClick))
	{
		CLEAR_BIT(Key.State, EnumKey_SingleClick);
		for(i=0; i<Keys->FuncTotalNum; i++)			//遍历所有按键
			if(Key.Continuous == Keys->KeyTrigger[i].TriggerValue)		//找到触发的按键
				if(Keys->KeyTrigger[i].SingleClick){
					circularQueuePush(&eventQueue, Keys->KeyTrigger[i].SingleClick);
					P1 = ~eventQueue.head; P6 = ~eventQueue.tail;
				}
	}
	
	if(IS_BIT_SET(Key.State, EnumKey_ComboClick))
	{
		CLEAR_BIT(Key.State, EnumKey_ComboClick);
		for(i=0; i<Keys->FuncTotalNum; i++)			//遍历所有按键
			if(Key.Continuous == Keys->KeyTrigger[i].TriggerValue)		//找到触发的按键
				if(Keys->KeyTrigger[i].ComboClick)
					circularQueuePush(&eventQueue, Keys->KeyTrigger[i].ComboClick);
	}
	
	if(IS_BIT_SET(Key.State, EnumKey_LongPress))
	{
		CLEAR_BIT(Key.State, EnumKey_LongPress);
		for(i=0; i<Keys->FuncTotalNum; i++)			//遍历所有按键
			if(Key.Continuous == Keys->KeyTrigger[i].TriggerValue)		//找到触发的按键
				if(Keys->KeyTrigger[i].LongPress)
					circularQueuePush(&eventQueue, Keys->KeyTrigger[i].LongPress);
	}
	
	if(IS_BIT_SET(Key.State, EnumKey_MultiPress))
	{
		CLEAR_BIT(Key.State, EnumKey_MultiPress);
		for(i=0; i<Keys->FuncTotalNum; i++)			//遍历所有按键
			if(Key.Continuous == Keys->KeyTrigger[i].TriggerValue)		//找到触发的按键
				if(Keys->KeyTrigger[i].MultiPress)
					circularQueuePush(&eventQueue, Keys->KeyTrigger[i].MultiPress);
	}
}

void T0_Init(void)
{
//	EA = 1;							//在主函数中打开
	AUXR &= 0x7F;					// 12T mode
    TMOD |= 0x00;                    //set timer0 as mode1 (16-bit)
    TL0 = DEBOUNCE_TIME*T1MS;                     //initial timer0 low byte
    TH0 = (DEBOUNCE_TIME*T1MS) >> 8;                //initial timer0 high byte
    TR0 = 1;                        //timer0 start running
    ET0 = 1;                        //enable timer0 interrupt
}

/* Timer0 interrupt routine */
void Timer0_Int(void) interrupt 1
{
//    TL0 = DEBOUNCE_TIME*T1MS;                     //reload timer0 low byte
//    TH0 = (DEBOUNCE_TIME*T1MS) >> 8;                //reload timer0 high byte
	KeyHandle((KeysTypedef_t *)&Keys);
}


