#ifndef _UART_H_
#define _UART_H_
#include <stdio.h>

#define BAUD 9600             //���ڲ�����

#define NONE_PARITY     0       //��У��
#define ODD_PARITY      1       //��У��
#define EVEN_PARITY     2       //żУ��
#define MARK_PARITY     3       //���У��
#define SPACE_PARITY    4       //�հ�У��

#define PARITYBIT NONE_PARITY   //����У��λ

extern void Uart_SendData(u8 dat);
extern void Uart_SendString(char *s);
extern void Uart_Init();


#endif
