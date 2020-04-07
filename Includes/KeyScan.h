/**
 * Copyright(C),2019-2020
 * FileName: KeyScan.h
 * Author: AdjWang
 * Version: 2.0
 * Date: 2020/04/08
 * Description:  Key scan module header.
 * History: 
 *     2018/10/09    V1.0   first version.
 *     2020/04/08    V2.0   fix fp init bug;
 *                          add eventQueue;
 *                          add KeyScanConfig.h.
*/
#ifndef _KeyScan_H_
#define _KeyScan_H_
#include "KeyScanConfig.h"

#define     SET_BIT(a,b)                            ((a) |= (1 << (b)))
#define     CLEAR_BIT(a,b)                          ((a) &= ~(1 << (b)))
#define     IS_BIT_SET(a,b)                         ((a) & (1 << (b)))
#define     MIN(a,b)                                ((a)>(b)?(b):(a))
#define     MAX(a,b)                                ((a)>(b)?(a):(b))
#define     TRIGGER_VALUE(key)                      (1<<(key))

/* Key scan trigger value enum */
enum EnumKeyTriggerValue
{
    EnumKey_NoKey = 0
};

/* Key scan trigger state enum 
 * KeyScanStates_t.triggerState is a 8bit variable,
 * means max_num of EnumKeyTriggerState is 8
*/
enum EnumKeyTriggerState
{
    EnumKey_SingleClick = 0,
    EnumKey_ComboClick = 1,
    EnumKey_LongPress = 2,
    EnumKey_MultiPress = 3
};

/* Key scan variables */
typedef struct
{
    keyTriggerType_t lastValue;        // last state of key, refresh when release
    keyTriggerType_t triggered;        // trigger when a key pressed
    keyTriggerType_t continuous;       // IO state
    u8 triggerState;                   // the EnumKeyTriggerState to match with
}KeyScanStates_t;

/* Key IO struct */
typedef struct
{
    // The indirect addressing mode is not supposed to operate a sfr register
    // Parsing string to operate a GPIO in an iteration statement maybe a good idea
    u8 *IOPort1;        // string such like "P49", "P33"
    u8 *IOPort2;
}KeyIO_t;

/* Key function struct
 * Mapping a key and its event function
*/
typedef struct
{
    keyTriggerType_t triggerValue;         // A value mapping with a key pressed state
    FUNCTIONPTR fp_singleClick;            // A event function pointer, related to enum EnumKeyTriggerState
    FUNCTIONPTR fp_comboClick;
    FUNCTIONPTR fp_longPress;
    FUNCTIONPTR fp_multiPress;
}KeyFunc_t;

/* Keys description struct */
typedef struct
{
    u8 KeysNumber;             // Quantity of keys
    KeyIO_t *KeyIO;            // Key IO struct pointer
    
    u8 FuncsNumber;            // Quantity of event functions
    KeyFunc_t *KeyFunc;        // Key function struct pointer
}Keys_t; 

/* Circular increase operator
 * To replace the operator % with &, EVENT_QUEUE_LEN must be n-th power of 2
*/
#define        CIRCULAR_INC(x)        (((x)+1)&(EVENT_QUEUE_LEN-1))
/* Circular queue struct
 * Empty when head == tail
 * Full when CIRCULAR_INC(tail) == head
*/
typedef struct
{
    u8 head;    // head refers to an empty element
    u8 tail;
    FUNCTIONPTR queue[EVENT_QUEUE_LEN];
}CircularQueue_t;

extern void CircularQueueInit(CircularQueue_t* eventQueue);
extern void CircularQueuePush(CircularQueue_t* eventQueue, FUNCTIONPTR func);
extern FUNCTIONPTR CircularQueuePop(CircularQueue_t* eventQueue);
extern void KeyEventProcess(void);

extern void KeyScanInit(KeyIO_t* SingleKey, u8 singleKeyNum, KeyFunc_t* KeyFuncs, u8 keyFuncNum);
extern void KeyScanEnable();
extern void KeyScanDisable();

#endif
