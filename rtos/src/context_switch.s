.syntax unified
.cpu cortex-m3
.thumb

.global context_switch
.global pendsv_handler

.extern current_task
.extern next_task
.extern scheduler_run

.section .text

/* 上下文切换函数 */
/* void context_switch(task_t *current, task_t *next) */
context_switch:
    /* 如果当前任务为NULL，直接加载新任务 */
    cmp r0, #0
    beq load_next_context
    
    /* 保存当前任务的上下文 */
    /* 获取当前PSP */
    mrs r2, psp
    
    /* 保存r4-r11到栈中 */
    stmdb r2!, {r4-r11}
    
    /* 保存新的栈指针到任务控制块 */
    str r2, [r0]
    
load_next_context:
    /* 加载新任务的上下文 */
    /* 获取新任务的栈指针 */
    ldr r2, [r1]
    
    /* 恢复r4-r11 */
    ldmia r2!, {r4-r11}
    
    /* 设置PSP */
    msr psp, r2
    
    /* 确保使用PSP */
    orr lr, lr, #0x04
    
    /* 返回 */
    bx lr

/* PendSV中断处理函数 */
pendsv_handler:
    /* 禁用中断 */
    cpsid i
    
    /* 保存当前上下文 */
    mrs r0, psp
    
    /* 检查是否是第一次调度 */
    ldr r3, =current_task
    ldr r2, [r3]
    cmp r2, #0
    beq pendsv_nosave
    
    /* 保存r4-r11 */
    stmdb r0!, {r4-r11}
    
    /* 保存栈指针 */
    str r0, [r2]
    
pendsv_nosave:
    /* 调用调度器选择下一个任务 */
    push {lr}
    bl scheduler_run
    pop {lr}
    
    /* 加载新任务 */
    ldr r3, =current_task
    ldr r1, [r3]
    
    /* 获取新任务的栈指针 */
    ldr r0, [r1]
    
    /* 恢复r4-r11 */
    ldmia r0!, {r4-r11}
    
    /* 设置PSP */
    msr psp, r0
    
    /* 启用中断 */
    cpsie i
    
    /* 确保使用PSP并返回线程模式 */
    orr lr, lr, #0x04
    bx lr

.end