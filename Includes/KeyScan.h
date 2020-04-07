#ifndef _KeyScan_H_
#define _KeyScan_H_

#define			EN_P4						//����������ӵ�P4����Ҫʹ�ܴ��� ��Ϊ�Ͽ�STC��Ƭ��û��P4
#define			EN_P5						//����������ӵ�P5����Ҫʹ�ܴ��� ��Ϊ�Ͽ�STC��Ƭ��û��P5
typedef		unsigned int	keyporttype;	//���������Ǽ�λ��֧�ּ������� �������ռ�ڴ�

#define SET_BIT(a,b)                            ((a) |= (1 << (b)))
#define CLEAR_BIT(a,b)                          ((a) &= ~(1 << (b)))
#define IS_BIT_SET(a,b)                         ((a) & (1 << (b)))
#define MIN(a,b)								((a)>(b)?(b):(a))
#define MAX(a,b)								((a)>(b)?(a):(b))

#define DEBOUNCE_TIME                           30			//������ʱms
#define PRESS_LONG_TIME                         1500		//�����ж�ʱ��ms
#define KEY_MAX_NUMBER							(sizeof(keyporttype)<<3)		//���֧�ְ�������(sizeof(keyporttype)*8)
#define N_CLICK_NUMBER							2			//�����ж�����
#define N_CLICK_TIMELIMIT						300			//���������ʱʱ��ms
#define	EVENT_QUEUE_LEN							8			// �¼����г��ȣ�Ϊ�˼򻯼��㣬��ҪΪ2����������

enum EnumKeyNum					//ɨ�谴��ֵö��
{
	EnumKey_NoKey = 0
};

enum EnumKeyState		//State��8λ�������8��
{
	EnumKey_SingleClick = 0,
	EnumKey_ComboClick = 1,
	EnumKey_LongPress = 2,
	EnumKey_MultiPress = 3
};

typedef struct 		//���������ṹ��
{
	keyporttype LastValue;		//������һ�ΰ���ֵ ̧��ʱˢ��
	keyporttype Triggered;		//���δ���ʱ��ֵ���ڼ�ֵ ֮���Զ���0 ���ڶ̰��ж�
	keyporttype Continuous;		//�����ֵ ���ڳ����ж�
	u8 State;					//����EnumKeyState����
}KeyStateTypedef_t;

// functional structs
typedef struct
{
	//51��Ƭ��sfr�Ĵ������ܼ��Ѱַ ����ʹ���ַ�������ʽ
//    uint32_t         keyRccPeriph;
//    GPIO_TypeDef     *keyPort;
//    uint32_t         keyGpio;
	u8 *IOPort1;		//�ַ���
	u8 *IOPort2;
}KeyIOTypedef_t;

typedef struct
{
    keyporttype TriggerValue;			//�����¼�����ʱ�ļ��ֵ ���Զ�̬��ֵΪö���еİ���ֵ�������
    FUNCTIONPTR SingleClick;			//�����ܻص�����ָ�� ��ӦEnumKeyState  ���8�� �����ӹ�����Ҫ��������ӹ��ܺ���ָ��
    FUNCTIONPTR ComboClick;
    FUNCTIONPTR LongPress;
    FUNCTIONPTR MultiPress;
}KeyTriggerTypedef_t;

typedef struct
{
    u8 KeyTotalNum;						//����������
    KeyIOTypedef_t *SingleKey;			//����Ӳ����������
	
	u8 FuncTotalNum;					//������������
	KeyTriggerTypedef_t *KeyTrigger;	//������������
}KeysTypedef_t; 

// ���μӷ�
#define		CIRCULAR_INC(x)		(((x)+1)&(EVENT_QUEUE_LEN-1))
// �����¼�����
typedef struct
{
	u8 head;	// headָ���Ԫ��Ϊ��
	u8 tail;
	FUNCTIONPTR queue[EVENT_QUEUE_LEN];
}circularQueue_t;

extern void circularQueueInit(circularQueue_t* eventQueue);
extern void circularQueuePush(circularQueue_t* eventQueue, FUNCTIONPTR func);
extern FUNCTIONPTR circularQueuePop(circularQueue_t* eventQueue);
extern void KeyEventProcess(void);

void KeyScanInit(KeyIOTypedef_t* SingleKey, u8 singleKeyNum, KeyTriggerTypedef_t* KeyFuncs, u8 keyFuncNum);
extern void KeyScanEnable();	//����ɨ��ʹ�ܺ���
extern void KeyScanDisable();	//����ɨ��ֹͣ����
extern void T0_Init();

#endif
