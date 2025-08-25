#include "../include/rtos.h"
#include "../include/stm32f103.h"
#include <stddef.h>
#include <string.h>

/* 外部变量 */
extern task_t task_pool[MAX_TASKS];
extern uint32_t task_count;
extern uint32_t next_task_id;

/* 任务栈初始化函数 */
static void init_task_stack(task_t *task, void (*task_func)(void)) {
    uint32_t *stack_ptr;
    
    /* 栈指针指向栈顶 */
    stack_ptr = &task->stack[STACK_SIZE - 1];
    
    /* 初始化栈帧 (ARM Cortex-M3异常栈帧) */
    *(stack_ptr--) = 0x01000000;    // xPSR (Thumb状态位)
    *(stack_ptr--) = (uint32_t)task_func;  // PC (任务函数地址)
    *(stack_ptr--) = (uint32_t)task_exit;  // LR (任务退出函数)
    *(stack_ptr--) = 0x12121212;    // R12
    *(stack_ptr--) = 0x03030303;    // R3
    *(stack_ptr--) = 0x02020202;    // R2
    *(stack_ptr--) = 0x01010101;    // R1
    *(stack_ptr--) = 0x00000000;    // R0
    
    /* 手动保存的寄存器 */
    *(stack_ptr--) = 0x11111111;    // R11
    *(stack_ptr--) = 0x10101010;    // R10
    *(stack_ptr--) = 0x09090909;    // R9
    *(stack_ptr--) = 0x08080808;    // R8
    *(stack_ptr--) = 0x07070707;    // R7
    *(stack_ptr--) = 0x06060606;    // R6
    *(stack_ptr--) = 0x05050505;    // R5
    *(stack_ptr--) = 0x04040404;    // R4
    
    /* 保存栈指针 */
    task->stack_ptr = stack_ptr + 1;
}

/* 任务退出函数 */
void task_exit(void) {
    /* 任务正常退出时调用 */
    /* 删除当前任务 */
    extern task_t *current_task;
    if (current_task != NULL) {
        rtos_delete_task(current_task->task_id);
    }
    
    /* 触发任务切换 */
    rtos_yield();
    
    /* 不应该到达这里 */
    while (1);
}

/* 创建任务 */
uint32_t rtos_create_task(void (*task_func)(void), task_priority_t priority) {
    /* 检查参数 */
    if (task_func == NULL) {
        return 0;  // 无效的任务函数
    }
    
    /* 查找空闲的任务控制块 */
    for (int i = 0; i < MAX_TASKS; i++) {
        if (task_pool[i].task_id == 0) {  // 找到空闲的TCB
            /* 初始化任务控制块 */
            task_pool[i].task_id = next_task_id++;
            task_pool[i].task_func = task_func;
            task_pool[i].priority = priority;
            task_pool[i].state = TASK_READY;
            task_pool[i].time_slice = TIME_SLICE_MS;
            task_pool[i].wakeup_time = 0;  // 初始化唤醒时间
            
            /* 初始化任务栈 */
            init_task_stack(&task_pool[i], task_func);
            
            /* 增加任务计数 */
            task_count++;
            
            return task_pool[i].task_id;
        }
    }
    
    return 0;  // 任务池已满
}

/* 删除任务 */
void rtos_delete_task(uint32_t task_id) {
    /* 查找要删除的任务 */
    for (int i = 0; i < MAX_TASKS; i++) {
        if (task_pool[i].task_id == task_id) {
            /* 不能删除空闲任务 (ID为1) */
            if (task_id == 1) {
                return;
            }
            
            /* 清除任务控制块 */
            memset(&task_pool[i], 0, sizeof(task_t));
            task_pool[i].state = TASK_SUSPENDED;
            
            /* 减少任务计数 */
            if (task_count > 0) {
                task_count--;
            }
            
            /* 如果删除的是当前任务，触发任务切换 */
            extern task_t *current_task;
            if (current_task == &task_pool[i]) {
                current_task = NULL;
                rtos_yield();
            }
            
            return;
        }
    }
}

/* 挂起任务 */
void rtos_suspend_task(uint32_t task_id) {
    for (int i = 0; i < MAX_TASKS; i++) {
        if (task_pool[i].task_id == task_id) {
            task_pool[i].state = TASK_SUSPENDED;
            
            /* 如果挂起的是当前任务，触发任务切换 */
            extern task_t *current_task;
            if (current_task == &task_pool[i]) {
                rtos_yield();
            }
            return;
        }
    }
}

/* 恢复任务 */
void rtos_resume_task(uint32_t task_id) {
    for (int i = 0; i < MAX_TASKS; i++) {
        if (task_pool[i].task_id == task_id) {
            if (task_pool[i].state == TASK_SUSPENDED) {
                task_pool[i].state = TASK_READY;
            }
            return;
        }
    }
}

/* 获取任务信息 */
task_t* rtos_get_task_info(uint32_t task_id) {
    for (int i = 0; i < MAX_TASKS; i++) {
        if (task_pool[i].task_id == task_id) {
            return &task_pool[i];
        }
    }
    return NULL;
}

/* 获取当前任务数量 */
uint32_t rtos_get_task_count(void) {
    return task_count;
}