/**
 * Copyright(C),2019-2020
 * FileName: KeyScan.c
 * Author: AdjWang
 * Version: 2.0
 * Date: 2020/04/08
 * Description:  Key scan module source.
 * History: 
 *     2018/10/09    V1.0   first version.
 *     2020/04/08    V2.0   fix fp init bug;
 *                          add eventQueue;
 *                          add KeyScanConfig.h.
 * TODO list:
 *     1. config.h              Done
 *     2. event_queue           Done
 *     3. hash(triggerValue)    If the Keys_t.FuncsNumber is too large, then a hash function is nessessary
*/
#include "main.h"
#include "KeyScan.h"

static xdata Keys_t Keys;       // Keys struct
static xdata CircularQueue_t eventQueue;            // Key press event queue

static volatile u8 state = 0;    // State machine initialized as stop state
static u8 nClickTimeOut = 0;    // Combo click interval counter
static xdata volatile KeyScanStates_t KeyScanStates;      // KeyScanStates struct

/** 
 * @brief Initialize CircularQueue
 *     
 * @param 
 *     CircularQueue_t* CircularQueue
 *
 * @return 
 * 
 * @note
 */
static void CircularQueueInit(CircularQueue_t* CircularQueue){
//    u8 i;
//    for(i=0; i<EVENT_QUEUE_LEN; i++){
//        CircularQueue->queue[i] = NULL;
//    }
    CircularQueue->head = CircularQueue->tail = 0;
}

/** 
 * @brief Push a element to the tail of the queue
 * @param 
 *     CircularQueue_t* CircularQueue
 *     QUEUE_ELEMENT_TYPE e
 *
 * @return 
 * 
 * @note
 *     If the queue is full, the FUNCTIONPTR will be abandened
 * 
    //    CircularQueue->tail = nextTail;
    //    // 如果在这里被打断会出问题，此时CircularQueue->queue[nextTail]无效，不可以执行
    //    CircularQueue->queue[nextTail] = func;
 */
static void CircularQueuePush(CircularQueue_t* CircularQueue, QUEUE_ELEMENT_TYPE e){
    u8 nextTail = CIRCULAR_INC(CircularQueue->tail);
    if(nextTail == CircularQueue->head) return;    // full
    CircularQueue->queue[nextTail] = e;
    CircularQueue->tail = nextTail;
}

/** 
 * @brief Pop a element from the head of the queue
 * @param 
 *     CircularQueue_t* CircularQueue
 *
 * @return 
 *     QUEUE_ELEMENT_TYPE
 * 
 * @note
 *     If the queue is empty, a NONE_ELEMENT will be returned
 * 
 *     CircularQueue->head = CIRCULAR_INC(CircularQueue->head);
 *     这行产生了一个奇妙的bug
 *     如果高速重复此函数，在此行之后读CircularQueue->head有一定概率读不对，出乱码
 *     使用0级优化可以解决该问题，但是看了汇编也没看出来是啥问题...(⊙_⊙)?
 *     最后试出来另一个办法，程序如下面所示
 */
static QUEUE_ELEMENT_TYPE CircularQueuePop(CircularQueue_t* CircularQueue){
    if(CircularQueue->tail == CircularQueue->head){return NONE_ELEMENT;}    // empty
    CircularQueue->head = CIRCULAR_INC(CircularQueue->head);        // Accursed expression, where a strange problem occured!
    if(CircularQueue->head == 256){printf("%d\r\n", *(int*)CircularQueue);} // This line will never be executed, but it does fix the f**king bug!!
    return CircularQueue->queue[CircularQueue->head];
}

/** 
 * @brief Initialize timer 0
 * @param 
 *
 * @return 
 * 
 * @note
 */
static void Timer0Init(void){
//    EA = 1;                            // set in main()
    AUXR &= 0x7F;                    // 12T mode
    TMOD |= 0x00;                   //set timer0 as mode1 (16-bit)
    TL0 = DEBOUNCE_TIME*T1MS;       //initial timer0 low byte
    TH0 = (DEBOUNCE_TIME*T1MS) >> 8;//initial timer0 high byte
    TR0 = 1;                        //timer0 start running
    ET0 = 1;                        //enable timer0 interrupt
}

/** 
 * @brief Pop and execute all event functions in the queue
 * @param 
 *
 * @return 
 * 
 * @note
 *     This function should be put into the while(1) of main()
 */
void KeyEventProcess(void){
    FUNCTIONPTR key_event;
    while((key_event = CircularQueuePop((CircularQueue_t*)&eventQueue)) != NULL){
        (*key_event)();
    }
}

/** 
 * @brief Initialize key scan
 * @param 
 *     KeyIO_t* SingleKey        key IO struct
 *     u8 singleKeyNum            quantity of keys
 * 
 *     KeyFunc_t* KeyFuncs        key function struct
 *     u8 keyFuncNum            quantity of event functions
 *     
 * @return 
 * 
 * @note
 * 
 */
void KeyScanInit(KeyIO_t* SingleKey, u8 singleKeyNum, KeyFunc_t* KeyFuncs, u8 keyFuncNum){
    u8 i;
    
    CircularQueueInit((CircularQueue_t*)&eventQueue);
    
    Keys.KeysNumber = MIN(singleKeyNum, MAX_KEY_NUMBER);        // Quantity of keys
    Keys.KeyIO = SingleKey;                                     // Struct linking
    Keys.FuncsNumber = keyFuncNum;                              // Quantity of event functions
    Keys.KeyFunc = KeyFuncs;                                    // Struct linking

    KeyScanStates.noneState = 0;
    for(i=0; i<Keys.KeysNumber; i++){
        if(Keys.KeyIO[i].type == KEY_TYPE_SWITCH){
            KeyScanStates.noneState |= 1 << i;
        }
    }
    
    KeyScanEnable();
    Timer0Init();
}

/** 
 * @brief Write a bit to GPIO
 * @param 
 *     u8 *GPIO            GPIO to write
 *     bit write_bit    bit to write to the GPIO
 *     
 * @return 
 * 
 * @note
 *     Usage example: GPIO_WriteBit("P49", 1);
 */
static void GPIO_WriteBit(s8 *GPIO, bit write_bit){
    if(GPIO[0] != 'P' && GPIO[0] != 'p') {return;}
    switch(GPIO[1]){
        case '0':
            write_bit ? SET_BIT(P0, GPIO[2]-'0') : CLEAR_BIT(P0, GPIO[2]-'0');
            break;
        case '1':
            write_bit ? SET_BIT(P1, GPIO[2]-'0') : CLEAR_BIT(P1, GPIO[2]-'0');
            break;
        case '2':
            write_bit ? SET_BIT(P2, GPIO[2]-'0') : CLEAR_BIT(P2, GPIO[2]-'0');
            break;
        case '3':
            write_bit ? SET_BIT(P3, GPIO[2]-'0') : CLEAR_BIT(P3, GPIO[2]-'0');
            break;
        #ifdef        EN_P4
        case '4':
            write_bit ? SET_BIT(P4, GPIO[2]-'0') : CLEAR_BIT(P4, GPIO[2]-'0');
            break;
        #endif
        #ifdef        EN_P5
        case '5':
            write_bit ? SET_BIT(P5, GPIO[2]-'0') : CLEAR_BIT(P5, GPIO[2]-'0');
            break;
        #endif
        #ifdef        EN_P6
        case '6':
            write_bit ? SET_BIT(P6, GPIO[2]-'0') : CLEAR_BIT(P6, GPIO[2]-'0');
            break;
        #endif
    }
}
/** 
 * @brief Read a bit from GPIO
 * @param 
 *     u8 *GPIO            GPIO to read
 *     
 * @return 
 *     u8                 bit read from the GPIO(0 or 1)
 * 
 * @note
 *     Usage example: u8 read_bit = GPIO_ReadBit("P49");
 */
static u8 GPIO_ReadBit(s8 *GPIO){
    if(GPIO[0] != 'P' && GPIO[0] != 'p') {return 0;}
    switch(GPIO[1]){
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
        #ifdef        EN_P4
        case '4':
            SET_BIT(P4, GPIO[2]-'0'); return IS_BIT_SET(P4, GPIO[2]-'0');
            break;
        #endif
        #ifdef        EN_P5
        case '5':
            SET_BIT(P5, GPIO[2]-'0'); return IS_BIT_SET(P5, GPIO[2]-'0');
            break;
        #endif
        #ifdef        EN_P6
        case '6':
            SET_BIT(P6, GPIO[2]-'0'); return IS_BIT_SET(P6, GPIO[2]-'0');
            break;
        #endif
    }
    return 0;
}

/** 
 * @brief read GPIO states of keys
 * @param 
 *
 * @return
 *     keyTriggerType_t read_data:    GPIO states read
 *     read_data bit0 related to Keys.KeyIO[0]
 *     read_data bit1 related to Keys.KeyIO[1]
 *     read_data bit2 related to Keys.KeyIO[2]
 *     ...etc
 * 
 * @note
 *     If keys IO changed into a 8bit port, the 8bit data is needed to map 
 *     to the read_data according to its key number in enum EnumUserKey.
 */
static keyTriggerType_t singleKeyRead(void){
    u8 i;
    keyTriggerType_t read_data = 0;
    
    for(i=0; i<Keys.KeysNumber; i++){
        // port 1 and 2 are both connected to MCU
        // read sequence as follow: 
        // set port2, reset port1, read port2
        // set port1, reset port2, read port1
        if(Keys.KeyIO[i].IOPort2[0] == 'P' || Keys.KeyIO[i].IOPort2[0] == 'p'){    //Pxx
            GPIO_WriteBit(Keys.KeyIO[i].IOPort2, 1);
            GPIO_WriteBit(Keys.KeyIO[i].IOPort1, 0);
            if(GPIO_ReadBit(Keys.KeyIO[i].IOPort2) == 0x00){            // read port2
                GPIO_WriteBit(Keys.KeyIO[i].IOPort1, 1);
                GPIO_WriteBit(Keys.KeyIO[i].IOPort2, 0);
                if(GPIO_ReadBit(Keys.KeyIO[i].IOPort1) == 0x00){        // read port1
                    SET_BIT(read_data, i);
                }
            }
        }else{    // read port1 only, port2 is connected to GND
            GPIO_WriteBit(Keys.KeyIO[i].IOPort1, 1);
            if(GPIO_ReadBit(Keys.KeyIO[i].IOPort1) == 0x00){
                SET_BIT(read_data, i);
            }
        }
    }
    return read_data;
}

static keyTriggerType_t KeyRead(void){
    return singleKeyRead();
    // return portKeyRead();
}

/** 
 * @brief State machine functions
 *     5 States
 *     Switch to the next state in Timer0ISR() to debounce
 */    
// stop
static void State0() {}
// Check if pressed down
static void State1(){
    keyTriggerType_t read_data;
    read_data = KeyRead();
    if(read_data != KeyScanStates.curNoKeyState){        // Pressed down
        state++;
    }else{                                 // Released
        KeyScanStates.triggered = read_data & (read_data ^ KeyScanStates.continuous);
        KeyScanStates.continuous = read_data;    
    }
}
// Check again after debounce
static void State2(){
    if(KeyRead() != KeyScanStates.curNoKeyState){
        state++;
    }else{
        state--;
    }
}
// Check if released
static void State3(){
    static u8 nClick = 0;           // Combo count
    static u16 KeyLongCheck = 0;    // Long press time counter
    keyTriggerType_t temp;
    keyTriggerType_t read_data = KeyRead();
    
    //nClickTimeOut++;      // Increase in KeyScan()
    if(nClickTimeOut >= (N_CLICK_TIMELIMIT / DEBOUNCE_TIME)-1){
        nClickTimeOut = 0;
        nClick = 0;
    }
    
    if(read_data == KeyScanStates.curNoKeyState){        // Released
        KeyLongCheck = 0;                // Reset long press time count
        state++;
    }else{                                // Pressed down
        temp = read_data ^ KeyScanStates.continuous;    // Event both button and switch
        KeyScanStates.triggered = read_data & (read_data ^ KeyScanStates.continuous);   // Only button
        KeyScanStates.continuous = read_data;    
        
        // (temp = button and switch) & switch_filter
        if((temp & KeyScanStates.noneState) != 0){      // Switch
            SET_BIT(KeyScanStates.triggerState, EnumKey_SingleClick);
            KeyScanStates.triggerValue = (temp & KeyScanStates.noneState); // Add switch
        }else if(KeyScanStates.triggered != EnumKey_NoKey){// Button
            KeyScanStates.triggerValue = KeyScanStates.continuous & ~KeyScanStates.noneState;
            SET_BIT(KeyScanStates.triggerState, 
                    (KeyScanStates.triggered == (KeyScanStates.continuous & ~KeyScanStates.noneState)) \
                    ? EnumKey_SingleClick : EnumKey_MultiPress
            );
        }

        KeyLongCheck++;
        if(KeyLongCheck >= (LONG_PRESS_TIME / DEBOUNCE_TIME)-1){            // Long press
            KeyLongCheck = 0;
            SET_BIT(KeyScanStates.triggerState, EnumKey_LongPress);
        }
        
        if(KeyScanStates.triggered != EnumKey_NoKey){
            if(KeyScanStates.triggered == KeyScanStates.lastValue){
                nClick++;
                nClickTimeOut = 0;
                if(nClick >= N_CLICK_NUMBER){                // Combo click
                    nClick = 0;
                    SET_BIT(KeyScanStates.triggerState, EnumKey_ComboClick);
                }
            }else{
                nClick = 1; 
                
            }
        }
        KeyScanStates.curNoKeyState = KeyScanStates.continuous & KeyScanStates.noneState;
    }
}
// Check again after debounce
static void State4(){
    if(KeyRead() == KeyScanStates.curNoKeyState){        // Released
        KeyScanStates.lastValue = (KeyScanStates.continuous & ~KeyScanStates.noneState);        // Save key value
        state = 1;
    }else{                                // Jitter
        state--;
    }        
}

static code FUNCTIONPTR fpStates[] = {State0, State1, State2, State3, State4};
static void KeyScan(void){
    (*fpStates[state])(); 
    if(++nClickTimeOut >= 255){nClickTimeOut = 254;}
}
void KeyScanEnable(void){
    KeyScanStates.triggerState = 0;
    KeyScanStates.curNoKeyState = KeyRead() & KeyScanStates.noneState;
    KeyScanStates.triggerValue = KeyScanStates.curNoKeyState;
    state = 1; 
}
void KeyScanDisable(void){
    KeyScanStates.triggered = KeyScanStates.continuous = KeyScanStates.curNoKeyState; 
    state = 0; 
}

/** 
 * @brief key scan function in interrupt service routine
 * @param 
 *
 * @return
 * 
 * @note
 *     This function should be put into the interrupt function
 */
static void KeyHandle(){
    u8 i;
    if(KeyScanStates.triggerState == 0) {return;}
    // Check if a state triggered in EnumKeyTriggerState
    // Max number is 8, add a new if(IS_BIT_SET...) block if a new EnumKeyTriggerState set
    if(IS_BIT_SET(KeyScanStates.triggerState, EnumKey_SingleClick)){
        CLEAR_BIT(KeyScanStates.triggerState, EnumKey_SingleClick);
        // Get through the trigger value of keys to find out the triggered function
        for(i=0; i<Keys.FuncsNumber; i++){
            if(KeyScanStates.triggerValue == Keys.KeyFunc[i].triggerValue){
                if(Keys.KeyFunc[i].fp_singleClick){
                    CircularQueuePush(&eventQueue, Keys.KeyFunc[i].fp_singleClick);
                }
            }
        }
    }
    
    if(IS_BIT_SET(KeyScanStates.triggerState, EnumKey_ComboClick)){
        CLEAR_BIT(KeyScanStates.triggerState, EnumKey_ComboClick);
        for(i=0; i<Keys.FuncsNumber; i++){
            if(KeyScanStates.triggerValue == Keys.KeyFunc[i].triggerValue){
                if(Keys.KeyFunc[i].fp_comboClick){
                    CircularQueuePush(&eventQueue, Keys.KeyFunc[i].fp_comboClick);
                }
            }
        }
    }
    
    if(IS_BIT_SET(KeyScanStates.triggerState, EnumKey_LongPress)){
        CLEAR_BIT(KeyScanStates.triggerState, EnumKey_LongPress);
        for(i=0; i<Keys.FuncsNumber; i++){
            if(KeyScanStates.triggerValue == Keys.KeyFunc[i].triggerValue){
                if(Keys.KeyFunc[i].fp_longPress){
                    CircularQueuePush(&eventQueue, Keys.KeyFunc[i].fp_longPress);
                }
            }
        }
    }
    
    if(IS_BIT_SET(KeyScanStates.triggerState, EnumKey_MultiPress)){
        CLEAR_BIT(KeyScanStates.triggerState, EnumKey_MultiPress);
        for(i=0; i<Keys.FuncsNumber; i++){
            if(KeyScanStates.triggerValue == Keys.KeyFunc[i].triggerValue){
                if(Keys.KeyFunc[i].fp_multiPress){
                    CircularQueuePush(&eventQueue, Keys.KeyFunc[i].fp_multiPress);
                }
            }
        }
    }
}

/* Timer0 interrupt service routine */
void Timer0ISR(void) interrupt 1{
// Auto reload after STC15
//    TL0 = DEBOUNCE_TIME*T1MS;                     //reload timer0 low byte
//    TH0 = (DEBOUNCE_TIME*T1MS) >> 8;              //reload timer0 high byte
    KeyScan();
    KeyHandle();
}


