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
#ifndef _MAIN_H_
#define _MAIN_H_

//#include <reg51.h>
#include <STC15.h>
#include <intrins.h>
#include <string.h>
#include <stdio.h>

typedef        unsigned char    u8;
typedef        unsigned int     u16;
typedef        unsigned long    u32;
typedef        char             s8;
typedef        int              s16;
typedef        long             s32;

typedef  void  (*FUNCTIONPTR)(void);
#define        COUNT_OF_ARRAY(x)        (sizeof(x)/sizeof(x[0]))


#define FOSC 11059200L      //System frequency

#define T1MS (65536-FOSC/12/1000)   //1ms timer calculation method in 12T mode


#endif
