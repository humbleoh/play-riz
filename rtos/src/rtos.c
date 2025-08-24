#include "../include/rtos.h"
#include "../include/stm32f103.h"
#include <stddef.h>

/* RTOS初始化 */
void rtos_init(void) {
    /* 初始化硬件 */
    system_init();
    
    /* 初始化调度器 */
    scheduler_init();
    
    /* 系统初始化完成 */
}

/* 获取RTOS版本信息 */
const char* rtos_get_version(void) {
    return "Simple RTOS v1.0 for STM32 BluePill";
}

/* 获取系统运行时间 (毫秒) */
uint32_t rtos_get_uptime_ms(void) {
    return get_tick_count();
}

/* 获取系统运行时间 (秒) */
uint32_t rtos_get_uptime_sec(void) {
    return get_tick_count() / 1000;
}

/* 系统复位 */
void rtos_system_reset(void) {
    /* 触发系统复位 */
    SCB_AIRCR = 0x05FA0004;
    
    /* 等待复位 */
    while (1);
}