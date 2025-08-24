#ifndef RTOS_H
#define RTOS_H

#include <stdint.h>
#include <stdbool.h>

/* RTOS配置 */
#define MAX_TASKS 8
#define STACK_SIZE 256
#define TIME_SLICE_MS 10

/* 任务状态 */
typedef enum {
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_SUSPENDED
} task_state_t;

/* 任务优先级 */
typedef enum {
    PRIORITY_LOW = 0,
    PRIORITY_NORMAL = 1,
    PRIORITY_HIGH = 2,
    PRIORITY_CRITICAL = 3
} task_priority_t;

/* 任务控制块 (TCB) */
typedef struct {
    uint32_t *stack_ptr;           // 栈指针
    uint32_t stack[STACK_SIZE];    // 任务栈
    task_state_t state;            // 任务状态
    task_priority_t priority;      // 任务优先级
    uint32_t time_slice;           // 时间片
    uint32_t task_id;              // 任务ID
    void (*task_func)(void);       // 任务函数指针
} task_t;

/* 信号量结构 */
typedef struct {
    int32_t count;                 // 信号量计数
    task_t *waiting_tasks[MAX_TASKS]; // 等待队列
    uint8_t waiting_count;         // 等待任务数量
} semaphore_t;

/* RTOS核心函数 */
void rtos_init(void);
void rtos_start(void);
uint32_t rtos_create_task(void (*task_func)(void), task_priority_t priority);
void rtos_delete_task(uint32_t task_id);
void rtos_suspend_task(uint32_t task_id);
void rtos_resume_task(uint32_t task_id);
void rtos_yield(void);
void rtos_delay(uint32_t ms);
void task_exit(void);
task_t* rtos_get_task_info(uint32_t task_id);
uint32_t rtos_get_task_count(void);

/* 调度器函数 */
void scheduler_init(void);
void scheduler_run(void);
task_t* scheduler_get_next_task(void);
void context_switch(task_t *current_task, task_t *next_task);

/* 信号量函数 */
void semaphore_init(semaphore_t *sem, int32_t initial_count);
void semaphore_wait(semaphore_t *sem);
void semaphore_signal(semaphore_t *sem);
bool semaphore_try_wait(semaphore_t *sem);
int32_t semaphore_get_count(semaphore_t *sem);
uint8_t semaphore_get_waiting_count(semaphore_t *sem);
void semaphore_clear_waiting(semaphore_t *sem);

/* 系统时钟函数 */
void systick_init(void);
void systick_handler(void);
uint32_t get_tick_count(void);
void reset_time_slice_counter(void);

/* 系统信息函数 */
const char* rtos_get_version(void);
uint32_t rtos_get_uptime_ms(void);
uint32_t rtos_get_uptime_sec(void);
void rtos_system_reset(void);

#endif // RTOS_H