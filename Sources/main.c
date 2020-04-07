#include "main.h"
#include "Uart.h"
#include "KeyScan.h"

extern xdata circularQueue_t eventQueue;

enum EnumUserKey 				//������źͼ�ֵö�� ��Ŵ�0��ʼ ���ó���(KEY_MAX_NUMBER-1)
{
	EnumKey_A  = 0, EnumKey_Up_TriggerValue = 1<<EnumKey_A,
	EnumKey_B  = 1, EnumKey_Down_TriggerValue = 1<<EnumKey_B,
	EnumKey_C  = 2, EnumKey_Left_TriggerValue = 1<<EnumKey_C,
	EnumKey_D  = 3, EnumKey_Right_TriggerValue = 1<<EnumKey_D
};

#define GPIO_KEY_NUM 4                                  //������Ա����
xdata KeyIOTypedef_t SingleKey[GPIO_KEY_NUM];                   //����������Ա����ָ��
#define FUNC_KEY_NUM 3									//�û��Զ���Ĺ�������
xdata KeyTriggerTypedef_t KeyFuncs[FUNC_KEY_NUM];               //������������ָ��

xdata KeysTypedef_t Keys;                                     //�ܵİ���ģ��ṹ��ָ��

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
	//���̫���� ������! ����������Ҳ���� ���ڱ�����˾ͻῨ��
}


void KeyInit(void)		//����ɨ���ʼ��
{
	u8 i;
	for(i=0; i<FUNC_KEY_NUM; i++){
		KeyFuncs[i].SingleClick = NULL;
		KeyFuncs[i].ComboClick = NULL;
		KeyFuncs[i].LongPress = NULL;
		KeyFuncs[i].MultiPress = NULL;
	}
	
	SingleKey[EnumKey_A].IOPort1 = "P36"; SingleKey[EnumKey_A].IOPort2 = "GND";		//ע�ᰴ�� Port1������IO�� Port2��IO�ڻ�"GND"
	SingleKey[EnumKey_B].IOPort1 = "P52"; SingleKey[EnumKey_B].IOPort2 = "GND";
	SingleKey[EnumKey_C].IOPort1 = "P54"; SingleKey[EnumKey_C].IOPort2 = "GND";
	SingleKey[EnumKey_D].IOPort1 = "P53"; SingleKey[EnumKey_D].IOPort2 = "GND";
	
	KeyFuncs[0].TriggerValue = EnumKey_Up_TriggerValue;			//��Ҫ��Ӧ�ļ�ֵ ע���Ǽ�ֵ! ���Ǽ����! ��ϰ����û�
	KeyFuncs[0].SingleClick = Key7ShortPressEvent;		//ע��ص�����
	KeyFuncs[1].TriggerValue = EnumKey_Down_TriggerValue;		//��Ҫ��Ӧ�ļ�ֵ ע���Ǽ�ֵ! ���Ǽ����! ��ϰ����û�
	KeyFuncs[1].SingleClick = Key12ShortPressEvent;		//ע��ص�����
	KeyFuncs[2].TriggerValue = EnumKey_Left_TriggerValue | EnumKey_Right_TriggerValue;		//��Ҫ��Ӧ�ļ�ֵ ע���Ǽ�ֵ! ���Ǽ����! ��ϰ����û�
	KeyFuncs[2].MultiPress = Key17_22ShortPressEvent;		//ע��ص�����
	
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
	KeyInit();		//����ɨ���ʼ��
	Delay100ms();
	printf("test\r\n");
	while(1)
	{
		//WDT_CONTR = 0x3C;		//ι��
		//Uart_SendString("test\r\n");
		P67 = 0;
		KeyEventProcess();
		P67 = 1;  // off
		//printf("head: %bd, tail: %bd\r\n", eventQueue.head, eventQueue.tail);
	}
}


