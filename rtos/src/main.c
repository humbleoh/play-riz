#include "../include/rtos.h"
#include "../include/stm32f103.h"

/* 全局信号量 */
semaphore_t led_semaphore;
semaphore_t print_semaphore;

/* 任务ID */
uint32_t task1_id, task2_id, task3_id;

/* 任务1: LED闪烁任务 (高优先级) */
void led_blink_task(void) {
    while (1) {
        /* 等待信号量 */
        semaphore_wait(&led_semaphore);
        
        /* 点亮LED */
        led_on();
        rtos_delay(100);
        
        /* 熄灭LED */
        led_off();
        rtos_delay(100);
        
        /* 释放信号量 */
        semaphore_signal(&led_semaphore);
        
        /* 延时500ms */
        rtos_delay(500);
    }
}

/* 任务2: 快速闪烁任务 (普通优先级) */
void fast_blink_task(void) {
    uint32_t count = 0;
    
    while (1) {
        /* 每10次循环执行一次快速闪烁 */
        if (count % 10 == 0) {
            /* 等待LED信号量 */
            semaphore_wait(&led_semaphore);
            
            /* 快速闪烁3次 */
            for (int i = 0; i < 3; i++) {
                led_on();
                rtos_delay(50);
                led_off();
                rtos_delay(50);
            }
            
            /* 释放LED信号量 */
            semaphore_signal(&led_semaphore);
        }
        
        count++;
        rtos_delay(200);
    }
}

/* 任务3: 系统监控任务 (低优先级) */
void monitor_task(void) {
    uint32_t last_uptime = 0;
    
    while (1) {
        uint32_t current_uptime = rtos_get_uptime_sec();
        
        /* 每5秒输出一次系统信息 */
        if (current_uptime - last_uptime >= 5) {
            /* 等待打印信号量 */
            semaphore_wait(&print_semaphore);
            
            /* 这里可以通过串口输出系统信息 */
            /* 由于没有实现串口，我们用LED长亮1秒来表示 */
            led_on();
            rtos_delay(1000);
            led_off();
            
            /* 释放打印信号量 */
            semaphore_signal(&print_semaphore);
            
            last_uptime = current_uptime;
        }
        
        rtos_delay(1000);
    }
}

/* 任务4: 测试任务 (用于演示任务创建和删除) */
void test_task(void) {
    uint32_t count = 0;
    
    while (count < 20) {  // 运行20次后自动退出
        /* 短暂闪烁表示测试任务在运行 */
        if (semaphore_try_wait(&led_semaphore)) {
            led_toggle();
            rtos_delay(25);
            led_toggle();
            semaphore_signal(&led_semaphore);
        }
        
        count++;
        rtos_delay(300);
    }
    
    /* 任务完成，自动退出 */
    task_exit();
}

/* 主函数 */
int main(void) {
    /* 初始化RTOS */
    rtos_init();
    
    /* 初始化信号量 */
    semaphore_init(&led_semaphore, 1);    // 互斥信号量
    semaphore_init(&print_semaphore, 1);  // 互斥信号量
    
    /* 创建任务 */
    task1_id = rtos_create_task(led_blink_task, PRIORITY_HIGH);
    task2_id = rtos_create_task(fast_blink_task, PRIORITY_NORMAL);
    task3_id = rtos_create_task(monitor_task, PRIORITY_LOW);
    
    /* 延时2秒后创建测试任务 */
    rtos_delay(2000);
    uint32_t test_task_id = rtos_create_task(test_task, PRIORITY_NORMAL);
    
    /* 启动RTOS调度器 */
    rtos_start();
    
    /* 不应该到达这里 */
    while (1) {
        led_toggle();
        delay_ms(1000);
    }
    
    return 0;
}