#include "../include/stm32f103.h"
#include "../include/rtos.h"
#include <stddef.h>

/* 全局变量 */
static volatile uint32_t tick_count = 0;
static volatile uint32_t time_slice_counter = 0;

/* 外部变量声明 */
extern task_t *current_task;
extern bool scheduler_running;

/* SysTick初始化 */
void systick_init(void) {
    /* 计算重载值 (1ms中断) */
    uint32_t reload_value = (SYSTEM_CLOCK_HZ / SYSTICK_FREQ_HZ) - 1;
    
    /* 配置SysTick */
    SYSTICK_LOAD = reload_value;           // 设置重载值
    SYSTICK_VAL = 0;                       // 清除当前值
    
    /* 设置SysTick优先级为最低 */
    SCB_SHPR3 |= (SYSTICK_PRIORITY << 24);
    
    /* 启用SysTick: 使能计数器、中断和时钟源 */
    SYSTICK_CTRL = SYSTICK_CTRL_ENABLE | 
                   SYSTICK_CTRL_TICKINT | 
                   SYSTICK_CTRL_CLKSOURCE;
}

/* SysTick中断处理函数 */
void systick_handler(void) {
    /* 增加系统滴答计数 */
    tick_count++;
    
    /* 如果调度器正在运行 */
    if (scheduler_running) {
        /* 增加时间片计数器 */
        time_slice_counter++;
        
        /* 检查是否需要任务切换 */
        if (time_slice_counter >= TIME_SLICE_MS) {
            time_slice_counter = 0;
            
            /* 触发PendSV中断进行任务切换 */
            SCB_ICSR |= SCB_ICSR_PENDSTSET;
        }
        
        /* 更新当前任务的时间片 */
        if (current_task != NULL) {
            if (current_task->time_slice > 0) {
                current_task->time_slice--;
            }
        }
    }
}

/* 获取系统滴答计数 */
uint32_t get_tick_count(void) {
    return tick_count;
}

/* 延时函数 (非阻塞，基于系统滴答) */
void rtos_delay(uint32_t ms) {
    uint32_t start_tick = get_tick_count();
    
    /* 如果调度器正在运行，让出CPU */
    if (scheduler_running) {
        while ((get_tick_count() - start_tick) < ms) {
            rtos_yield();
        }
    } else {
        /* 调度器未运行，使用阻塞延时 */
        delay_ms(ms);
    }
}

/* 重置时间片计数器 */
void reset_time_slice_counter(void) {
    time_slice_counter = 0;
}