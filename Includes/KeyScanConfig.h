/**
 * Copyright(C),2019-2020
 * FileName: KeyScanConfig.h
 * Author: AdjWang
 * Version: 2.0
 * Date: 2020/04/08
 * Description:  Key scan module  user config header.
 * History: 
 *     2018/10/09    V1.0   first version.
 *     2020/04/08    V2.0   fix fp init bug;
 *                          add eventQueue;
 *                          add KeyScanConfig.h.
*/
#ifndef _KeyScanConfig_H_
#define _KeyScanConfig_H_

#define     EN_P4                        //如果按键连接到P4上需要使能此项 有的封装没有P4
#define     EN_P5                        //如果按键连接到P5上需要使能此项 有的封装没有P5
#define     EN_P6                        //如果按键连接到P6上需要使能此项 有的封装没有P6
typedef     unsigned int        keyTriggerType_t;    //数据类型是几位就支持几个按键 定义多了占内存
#define     MAX_KEY_NUMBER      (sizeof(keyTriggerType_t)<<3)//最大支持按键数量为keyTriggerType_t的位数即(sizeof(keyTriggerType_t)*8)

#define     DEBOUNCE_TIME               20          //消抖延时ms
#define     LONG_PRESS_TIME             1500        //长按判定时间ms
#define     N_CLICK_NUMBER              2           //连击判定次数
#define     N_CLICK_TIMELIMIT           300         //连击间隔超时时间ms (超过此时间判定为单击)
#define     EVENT_QUEUE_LEN             8           // 事件队列长度，为了简化计算，需要为2的整数次幂

#endif
