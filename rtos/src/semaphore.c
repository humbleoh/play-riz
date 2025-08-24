#include "../include/rtos.h"
#include "../include/stm32f103.h"
#include <stddef.h>
#include <string.h>

/* 外部变量 */
extern task_t *current_task;
extern bool scheduler_running;

/* 信号量初始化 */
void semaphore_init(semaphore_t *sem, int32_t initial_count) {
    if (sem == NULL) {
        return;
    }
    
    sem->count = initial_count;
    sem->waiting_count = 0;
    
    /* 清空等待队列 */
    for (int i = 0; i < MAX_TASKS; i++) {
        sem->waiting_tasks[i] = NULL;
    }
}

/* 信号量等待 (P操作) */
void semaphore_wait(semaphore_t *sem) {
    if (sem == NULL || !scheduler_running) {
        return;
    }
    
    /* 禁用中断，保证原子操作 */
    __asm("cpsid i");
    
    /* 如果信号量计数大于0，直接获取 */
    if (sem->count > 0) {
        sem->count--;
        __asm("cpsie i");  // 重新启用中断
        return;
    }
    
    /* 信号量不可用，将当前任务加入等待队列 */
    if (current_task != NULL && sem->waiting_count < MAX_TASKS) {
        /* 查找空闲位置 */
        for (int i = 0; i < MAX_TASKS; i++) {
            if (sem->waiting_tasks[i] == NULL) {
                sem->waiting_tasks[i] = current_task;
                sem->waiting_count++;
                break;
            }
        }
        
        /* 将当前任务设置为阻塞状态 */
        current_task->state = TASK_BLOCKED;
    }
    
    /* 重新启用中断 */
    __asm("cpsie i");
    
    /* 让出CPU，等待信号量可用 */
    rtos_yield();
}

/* 信号量释放 (V操作) */
void semaphore_signal(semaphore_t *sem) {
    if (sem == NULL) {
        return;
    }
    
    /* 禁用中断，保证原子操作 */
    __asm("cpsid i");
    
    /* 如果有任务在等待 */
    if (sem->waiting_count > 0) {
        /* 找到第一个等待的任务并唤醒 */
        for (int i = 0; i < MAX_TASKS; i++) {
            if (sem->waiting_tasks[i] != NULL) {
                /* 唤醒任务 */
                sem->waiting_tasks[i]->state = TASK_READY;
                
                /* 从等待队列中移除 */
                sem->waiting_tasks[i] = NULL;
                sem->waiting_count--;
                
                /* 重新启用中断 */
                __asm("cpsie i");
                
                /* 触发任务切换 */
                if (scheduler_running) {
                    rtos_yield();
                }
                return;
            }
        }
    }
    
    /* 没有等待的任务，增加信号量计数 */
    sem->count++;
    
    /* 重新启用中断 */
    __asm("cpsie i");
}

/* 尝试获取信号量 (非阻塞) */
bool semaphore_try_wait(semaphore_t *sem) {
    if (sem == NULL) {
        return false;
    }
    
    /* 禁用中断，保证原子操作 */
    __asm("cpsid i");
    
    bool result = false;
    if (sem->count > 0) {
        sem->count--;
        result = true;
    }
    
    /* 重新启用中断 */
    __asm("cpsie i");
    
    return result;
}

/* 获取信号量计数 */
int32_t semaphore_get_count(semaphore_t *sem) {
    if (sem == NULL) {
        return -1;
    }
    
    return sem->count;
}

/* 获取等待任务数量 */
uint8_t semaphore_get_waiting_count(semaphore_t *sem) {
    if (sem == NULL) {
        return 0;
    }
    
    return sem->waiting_count;
}

/* 清空信号量等待队列 (紧急情况使用) */
void semaphore_clear_waiting(semaphore_t *sem) {
    if (sem == NULL) {
        return;
    }
    
    /* 禁用中断 */
    __asm("cpsid i");
    
    /* 唤醒所有等待的任务 */
    for (int i = 0; i < MAX_TASKS; i++) {
        if (sem->waiting_tasks[i] != NULL) {
            sem->waiting_tasks[i]->state = TASK_READY;
            sem->waiting_tasks[i] = NULL;
        }
    }
    
    sem->waiting_count = 0;
    
    /* 重新启用中断 */
    __asm("cpsie i");
    
    /* 触发任务切换 */
    if (scheduler_running) {
        rtos_yield();
    }
}