#include "main.h"
#include "KeyScan.h"

/* TODO list:
1. config.h
*2. event_queue
3. hash(TriggerValue)
*/


extern xdata KeysTypedef_t Keys;
static xdata circularQueue_t eventQueue;

volatile u8 state = 0;	//��ʼ��Ϊֹͣ״̬
volatile KeyStateTypedef_t Key;		//��������ȫ�ֱ���

void circularQueueInit(circularQueue_t* eventQueue)
{
//	u8 i;
//	for(i=0; i<EVENT_QUEUE_LEN; i++){
//		eventQueue->queue[i] = NULL;
//	}
	eventQueue->head = eventQueue->tail = 0;
}

/** 
 * @brief �������
 * @param 
 *     eventQueue
 *     func
 *
 * @return 
 *     ��ʱ�����ش���ֵ
 * 
 * @note
	//	eventQueue->tail = nextTail;
	//	// ��������ﱻ��ϻ�����⣬��ʱeventQueue->queue[nextTail]��Ч��������ִ��
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
 * @brief ���г���
 * @param 
 *     eventQueue
 *
 * @return 
 *     ���ض���ͷ�ĺ���ָ��
 * 
 * @note
 *     eventQueue->head = CIRCULAR_INC(eventQueue->head);
 *     ���в�����һ�������bug
 *     �ڴ���֮���eventQueue->head��һ�����ʶ����ԣ�������
 *     ʹ��0���Ż����Խ�������⣬���ǿ��˻��Ҳû��������ɶ����...(��_��)?
 *     ����Գ�����һ���취��������������ʾ
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
	while((key_event = circularQueuePop(&eventQueue)) != NULL)	// �����¼�������ѯ
	{
		(*key_event)();
	}
}

void KeyScanInit(KeyIOTypedef_t* SingleKey, u8 singleKeyNum, KeyTriggerTypedef_t* KeyFuncs, u8 keyFuncNum)
{
	circularQueueInit(&eventQueue);		// �����¼����г�ʼ�� ���ﲻ��Ҫ�޸�
	Keys.KeyTotalNum = MIN(singleKeyNum, KEY_MAX_NUMBER);		//������Ա���� ���ﲻ��Ҫ�޸�
    Keys.SingleKey = SingleKey;		//���ӽṹ�� ���ﲻ��Ҫ�޸�
	Keys.FuncTotalNum = keyFuncNum;		//�������� ���ﲻ��Ҫ�޸�
    Keys.KeyTrigger = KeyFuncs;		//���ӽṹ�� ���ﲻ��Ҫ�޸�

	KeyScanEnable();		//ʹ�ܰ���ɨ��
	T0_Init();		//��ʼ����ʱ�� ����ʹ�ö�ʱ��0ɨ�谴��
}

//����1����3λ�ַ��� ��P00 P36 ����2������Ƶ�ƽ
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
//����1����3λ�ַ��� ��P00 P36 ע�ⷵ��ֵ���Ͳ���bit
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
 * @brief ��ֵ��ȡ
 * @param None
 *
 * @return ����˵��
 *     keyprottype ReadData
 *     ��ֵʹ����λ��ʽ��¼��ֵ
 *     ReadData ��0λ ��Ӧ Keys.SingleKey[0] ��״̬
 *     ReadData ��1λ ��Ӧ Keys.SingleKey[1] ��״̬
 *     ReadData ��2λ ��Ӧ Keys.SingleKey[2] ��״̬
 *     ...�Դ�����
 * 
 * @note ���Ҫ��Ϊ����IO�İ�������Ҫ�ȷ�������״̬��Ȼ��ӳ�䵽 ReadData ��Ӧ��λ
 */
static keyporttype singleKeyRead(void)
{
	u8 i;
	keyporttype ReadData = 0;
	
	for(i=0; i<Keys.KeyTotalNum; i++)
	{
		//Port2�Ƕ˿ڲ���GND ���󰴼����ӷ�ʽ
		//������ʽ
		//IO2��1 IO1��0 ��IO2
		//IO1��1 IO2��0 ��IO1
		if(Keys.SingleKey[i].IOPort2[0] == 'P' || Keys.SingleKey[i].IOPort2[0] == 'p')	//Pxx
		{
			GPIO_WriteBit(Keys.SingleKey[i].IOPort2, 1);
			GPIO_WriteBit(Keys.SingleKey[i].IOPort1, 0);
			if(GPIO_ReadBit(Keys.SingleKey[i].IOPort2) == 0x00)			//��һ�ζ���0�����
			{
				GPIO_WriteBit(Keys.SingleKey[i].IOPort1, 1);
				GPIO_WriteBit(Keys.SingleKey[i].IOPort2, 0);
				if(GPIO_ReadBit(Keys.SingleKey[i].IOPort1) == 0x00)		//���ζ�����0
					SET_BIT(ReadData, i);
			}
		}
		//�������ӷ�ʽ ������һ��Ĭ�Ͻӵ�
		//������ʽ
		//IO1��1 ��IO1
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

void State0() {}	//����ֹͣ
void State1()		//���¼��
{
	keyporttype ReadData;
	ReadData = KeyRead();
	if(ReadData != EnumKey_NoKey)		//��⵽����
		state++;
	else								//�ȶ�̧��
	{
		//�ȶ�̧��ʱˢ��
		Key.Triggered = ReadData & (ReadData ^ Key.Continuous);
		Key.Continuous = ReadData;	
	}
}
void State2()		//�������ٴμ��
{
	if(KeyRead() != EnumKey_NoKey)		//ȷ�ϰ���
	{
		state++;
	}
	else								//����
		state--;
}
void State3()		//̧����
{
	static u8 nClick = 0;		//��������
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
	
	if(ReadData == EnumKey_NoKey)		//��⵽̧��
	{
		KeyLongCheck = 0;				//������ʱ����
		state++;
	}
	else								//�ȶ�����
	{
		//�ȶ�����ʱˢ��
		Key.Triggered = ReadData & (ReadData ^ Key.Continuous);
		Key.Continuous = ReadData;	
		
		if(Key.Triggered != EnumKey_NoKey)	//�̰� ������������ϼ�
		{
			SET_BIT(Key.State, ((Key.Triggered == Key.Continuous) ? EnumKey_SingleClick : EnumKey_MultiPress));
		}

		KeyLongCheck++;
		if(KeyLongCheck == (PRESS_LONG_TIME / DEBOUNCE_TIME)-1)			//����
		{
			KeyLongCheck = 0;
			SET_BIT(Key.State, EnumKey_LongPress);
		}
		
		if(Key.LastValue != 0 && Key.Triggered == Key.LastValue)
		{
			nClick++;
			nClickTimeOut = 0;
			if(nClick == N_CLICK_NUMBER)				//����
			{
				nClick = 0;
				SET_BIT(Key.State, EnumKey_ComboClick);
			}
		}
	}
}
void State4()		//�������ٴμ��
{
	if(KeyRead() == EnumKey_NoKey)		//ȷ��̧��
	{
		Key.LastValue = Key.Continuous;		//��¼��ֵ
		state = 1;
	}
	else								//����
		state--;	
}

code FUNCTIONPTR States[] = {State0, State1, State2, State3, State4};		//��״̬����ָ������
void KeyScan() { (*States[state])(); }		//����ɨ�躯�� ��ѯ��״̬
void KeyScanEnable() { Key.State = 0x00; state = 1; }			//����ɨ��ʹ�ܺ���
void KeyScanDisable() {Key.Triggered = Key.Continuous = EnumKey_NoKey; state = 0; }			//����ɨ��ֹͣ����

void KeyHandle(KeysTypedef_t *Keys)
{
	u8 i;
	KeyScan();
	if(Key.Continuous == EnumKey_NoKey) return;
	//���ܱ��� ��ӦEnumKeyState ���8�� �����ӹ�����Ҫ��������ӹ��ܺ���
	if(IS_BIT_SET(Key.State, EnumKey_SingleClick))
	{
		CLEAR_BIT(Key.State, EnumKey_SingleClick);
		for(i=0; i<Keys->FuncTotalNum; i++)			//�������а���
			if(Key.Continuous == Keys->KeyTrigger[i].TriggerValue)		//�ҵ������İ���
				if(Keys->KeyTrigger[i].SingleClick){
					circularQueuePush(&eventQueue, Keys->KeyTrigger[i].SingleClick);
					P1 = ~eventQueue.head; P6 = ~eventQueue.tail;
				}
	}
	
	if(IS_BIT_SET(Key.State, EnumKey_ComboClick))
	{
		CLEAR_BIT(Key.State, EnumKey_ComboClick);
		for(i=0; i<Keys->FuncTotalNum; i++)			//�������а���
			if(Key.Continuous == Keys->KeyTrigger[i].TriggerValue)		//�ҵ������İ���
				if(Keys->KeyTrigger[i].ComboClick)
					circularQueuePush(&eventQueue, Keys->KeyTrigger[i].ComboClick);
	}
	
	if(IS_BIT_SET(Key.State, EnumKey_LongPress))
	{
		CLEAR_BIT(Key.State, EnumKey_LongPress);
		for(i=0; i<Keys->FuncTotalNum; i++)			//�������а���
			if(Key.Continuous == Keys->KeyTrigger[i].TriggerValue)		//�ҵ������İ���
				if(Keys->KeyTrigger[i].LongPress)
					circularQueuePush(&eventQueue, Keys->KeyTrigger[i].LongPress);
	}
	
	if(IS_BIT_SET(Key.State, EnumKey_MultiPress))
	{
		CLEAR_BIT(Key.State, EnumKey_MultiPress);
		for(i=0; i<Keys->FuncTotalNum; i++)			//�������а���
			if(Key.Continuous == Keys->KeyTrigger[i].TriggerValue)		//�ҵ������İ���
				if(Keys->KeyTrigger[i].MultiPress)
					circularQueuePush(&eventQueue, Keys->KeyTrigger[i].MultiPress);
	}
}

void T0_Init(void)
{
//	EA = 1;							//���������д�
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


