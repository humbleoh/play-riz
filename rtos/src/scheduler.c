#include "../include/rtos.h"
#include "../include/stm32f103.h"
#include <stddef.h>
#include <string.h>

/* 外部函数声明 */
extern uint32_t get_tick_count(void);

/* 全局变量 */
task_t task_pool[MAX_TASKS];           // 任务池
task_t *current_task = NULL;           // 当前运行的任务
task_t *next_task = NULL;              // 下一个要运行的任务
bool scheduler_running = false;        // 调度器运行状态
static uint32_t task_count = 0;        // 当前任务数量
static uint32_t next_task_id = 1;      // 下一个任务ID

/* 空闲任务 */
static void idle_task(void) {
    while (1) {
        /* 空闲任务：可以在这里实现低功耗模式 */
        __asm("wfi");  // 等待中断
    }
}

/* 调度器初始化 */
void scheduler_init(void) {
    /* 清空任务池 */
    memset(task_pool, 0, sizeof(task_pool));
    
    /* 初始化所有任务状态为未使用 */
    for (int i = 0; i < MAX_TASKS; i++) {
        task_pool[i].state = TASK_SUSPENDED;
        task_pool[i].task_id = 0;
    }
    
    /* 创建空闲任务 */
    rtos_create_task(idle_task, PRIORITY_LOW);
    
    /* 设置当前任务为空闲任务 */
    current_task = &task_pool[0];
    current_task->state = TASK_RUNNING;
    
    task_count = 1;
    scheduler_running = false;
}

/* 获取下一个就绪任务 */
task_t* scheduler_get_next_task(void) {
    task_t *highest_priority_task = NULL;
    task_priority_t highest_priority = PRIORITY_LOW;
    
    /* 遍历所有任务，找到优先级最高的就绪任务 */
    for (int i = 0; i < MAX_TASKS; i++) {
        if (task_pool[i].state == TASK_READY && task_pool[i].task_id != 0) {
            if (task_pool[i].priority >= highest_priority) {
                highest_priority = task_pool[i].priority;
                highest_priority_task = &task_pool[i];
            }
        }
    }
    
    /* 如果没有找到就绪任务，返回空闲任务 */
    if (highest_priority_task == NULL) {
        /* 确保空闲任务是就绪状态 */
        if (task_pool[0].state != TASK_RUNNING) {
            task_pool[0].state = TASK_READY;
        }
        return &task_pool[0];  // 返回空闲任务
    }
    
    return highest_priority_task;
}

/* 调度器运行 */
void scheduler_run(void) {
    /* 检查所有阻塞任务是否到期 */
    for (int i = 0; i < MAX_TASKS; i++) {
        if (task_pool[i].state == TASK_BLOCKED && 
            task_pool[i].task_id != 0) {
            if (get_tick_count() >= task_pool[i].wakeup_time) {
                task_pool[i].state = TASK_READY;
            }
        }
    }
    
    /* 获取下一个要运行的任务 */
    next_task = scheduler_get_next_task();
    
    /* 如果下一个任务与当前任务不同，进行任务切换 */
    if (next_task != current_task && next_task != NULL) {
        /* 保存当前任务状态 */
        if (current_task != NULL && current_task->state == TASK_RUNNING) {
            current_task->state = TASK_READY;
        }
        
        /* 设置下一个任务为运行状态 */
        next_task->state = TASK_RUNNING;
        next_task->time_slice = TIME_SLICE_MS;
        
        /* 执行上下文切换 */
        context_switch(current_task, next_task);
        
        /* 更新当前任务指针 */
        current_task = next_task;
    }
}

/* 任务让出CPU */
void rtos_yield(void) {
    if (scheduler_running) {
        /* 触发PendSV中断进行任务切换 */
        SCB_ICSR |= SCB_ICSR_PENDSTSET;
    }
}

/* 启动RTOS */
void rtos_start(void) {
    /* 设置PendSV优先级为最低 */
    SCB_SHPR3 |= (PENDSV_PRIORITY << 16);
    
    /* 初始化SysTick */
    systick_init();
    
    /* 标记调度器为运行状态 */
    scheduler_running = true;
    
    /* 启动第一个任务 */
    if (current_task != NULL) {
        /* 设置MSP为任务栈指针 */
        __asm volatile (
            "msr msp, %0\n\t"
            "bx %1\n\t"
            : : "r" (current_task->stack_ptr), "r" (current_task->task_func)
        );
    }
    
    /* 不应该到达这里 */
    while (1);
}