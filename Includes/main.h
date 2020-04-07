#ifndef _MAIN_H_
#define _MAIN_H_

//#include <reg51.h>
#include <STC15.h>
#include "intrins.h"
#include "string.h"
#include <stdio.h>

typedef		unsigned char	u8;
typedef		unsigned int	u16;
typedef		unsigned long	u32;
typedef		char			s8;
typedef		int				s16;
typedef		long			s32;

#define		CountOfArray(x)		(sizeof(x)/sizeof(x[0]))
typedef void (*FUNCTIONPTR)(void);


#define FOSC 11059200L      //System frequency

#define T1MS (65536-FOSC/12/1000)   //1ms timer calculation method in 12T mode


#endif
