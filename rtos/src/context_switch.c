#include "../include/rtos.h"
#include "../include/stm32f103.h"
#include <stddef.h>

/* 外部变量声明 */
extern task_t *current_task;
extern task_t *next_task;
extern void scheduler_run(void);

/* 内联汇编辅助函数 */

/* 获取进程栈指针 PSP */
static inline uint32_t get_psp(void) {
    uint32_t psp;
    __asm volatile ("mrs %0, psp" : "=r" (psp));
    return psp;
}

/* 设置进程栈指针 PSP */
static inline void set_psp(uint32_t psp) {
    __asm volatile ("msr psp, %0" : : "r" (psp) : "memory");
}

/* 保存寄存器 r4-r11 到栈中 */
static inline uint32_t* save_context_registers(uint32_t *stack_ptr) {
    __asm volatile (
        "stmdb %0!, {r4-r11}\n"
        : "+r" (stack_ptr)
        :
        : "memory"
    );
    return stack_ptr;
}

/* 从栈中恢复寄存器 r4-r11 */
static inline uint32_t* restore_context_registers(uint32_t *stack_ptr) {
    __asm volatile (
        "ldmia %0!, {r4-r11}\n"
        : "+r" (stack_ptr)
        :
        : "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "memory"
    );
    return stack_ptr;
}

/* 设置返回时使用 PSP */
static inline void set_psp_return(void) {
    __asm volatile ("orr lr, lr, #0x04" : : : "lr");
}

/* 禁用中断 */
static inline void disable_interrupts(void) {
    __asm volatile ("cpsid i" : : : "memory");
}

/* 启用中断 */
static inline void enable_interrupts(void) {
    __asm volatile ("cpsie i" : : : "memory");
}

/* 上下文切换函数 */
/* void context_switch(task_t *current, task_t *next) */
void context_switch(task_t *current, task_t *next) {
    /* 如果当前任务为NULL，直接加载新任务 */
    if (current == NULL) {
        goto load_next_context;
    }
    
    /* 保存当前任务的上下文 */
    /* 获取当前PSP */
    uint32_t current_psp = get_psp();
    
    /* 保存r4-r11到栈中 */
    uint32_t *saved_stack = save_context_registers((uint32_t*)current_psp);
    
    /* 保存新的栈指针到任务控制块 */
    current->stack_ptr = saved_stack;
    
load_next_context:
    /* 加载新任务的上下文 */
    /* 获取新任务的栈指针 */
    uint32_t *new_stack_ptr = (uint32_t*)next->stack_ptr;
    
    /* 恢复r4-r11 */
    uint32_t *restored_stack = restore_context_registers(new_stack_ptr);
    
    /* 设置PSP */
    set_psp((uint32_t)restored_stack);
    
    /* 确保使用PSP */
    set_psp_return();
    
    /* 返回 */
    return;
}

/* PendSV中断处理函数 */
void pendsv_handler(void) {
    /* 禁用中断 */
    disable_interrupts();
    
    /* 保存当前上下文 */
    uint32_t current_psp = get_psp();
    
    /* 检查是否是第一次调度 */
    if (current_task != NULL) {
        /* 保存r4-r11 */
        uint32_t *saved_stack = save_context_registers((uint32_t*)current_psp);
        
        /* 保存栈指针 */
        current_task->stack_ptr = saved_stack;
    }
    
    /* 调用调度器选择下一个任务 */
    scheduler_run();
    
    /* 加载新任务 */
    if (current_task != NULL) {
        /* 获取新任务的栈指针 */
        uint32_t *new_stack_ptr = (uint32_t*)current_task->stack_ptr;
        
        /* 恢复r4-r11 */
        uint32_t *restored_stack = restore_context_registers(new_stack_ptr);
        
        /* 设置PSP */
        set_psp((uint32_t)restored_stack);
    }
    
    /* 启用中断 */
    enable_interrupts();
    
    /* 确保使用PSP并返回线程模式 */
    set_psp_return();
}