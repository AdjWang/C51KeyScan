#ifndef _UART_H_
#define _UART_H_
#include <stdio.h>

#define BAUD 9600             //串口波特率

#define NONE_PARITY     0       //无校验
#define ODD_PARITY      1       //奇校验
#define EVEN_PARITY     2       //偶校验
#define MARK_PARITY     3       //标记校验
#define SPACE_PARITY    4       //空白校验

#define PARITYBIT NONE_PARITY   //定义校验位

extern void Uart_SendData(u8 dat);
extern void Uart_SendString(char *s);
extern void Uart_Init();


#endif
