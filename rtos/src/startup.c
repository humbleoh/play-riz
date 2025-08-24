#include "../include/stm32f103.h"
#include "../include/rtos.h"

/* 栈顶地址 (STM32F103C8T6有20KB SRAM) */
#define STACK_TOP 0x20005000

/* 外部函数声明 */
extern int main(void);
extern void systick_handler(void);
extern void pendsv_handler(void);

/* 默认中断处理函数 */
void default_handler(void) {
    while (1) {
        /* 无限循环 */
    }
}

/* 复位处理函数 */
void reset_handler(void) {
    /* 初始化系统 */
    system_init();
    
    /* 跳转到main函数 */
    main();
    
    /* 如果main函数返回，进入无限循环 */
    while (1);
}

/* 中断向量表 */
__attribute__((section(".isr_vector")))
void (* const vector_table[])(void) = {
    (void (*)(void))STACK_TOP,          // 0: 栈顶指针
    reset_handler,                      // 1: 复位处理函数
    default_handler,                    // 2: NMI
    default_handler,                    // 3: Hard Fault
    default_handler,                    // 4: Memory Management Fault
    default_handler,                    // 5: Bus Fault
    default_handler,                    // 6: Usage Fault
    0,                                  // 7: 保留
    0,                                  // 8: 保留
    0,                                  // 9: 保留
    0,                                  // 10: 保留
    default_handler,                    // 11: SVCall
    default_handler,                    // 12: Debug Monitor
    0,                                  // 13: 保留
    pendsv_handler,                     // 14: PendSV (用于任务切换)
    systick_handler,                    // 15: SysTick
    
    /* 外部中断 */
    default_handler,                    // 16: WWDG
    default_handler,                    // 17: PVD
    default_handler,                    // 18: TAMPER
    default_handler,                    // 19: RTC
    default_handler,                    // 20: FLASH
    default_handler,                    // 21: RCC
    default_handler,                    // 22: EXTI0
    default_handler,                    // 23: EXTI1
    default_handler,                    // 24: EXTI2
    default_handler,                    // 25: EXTI3
    default_handler,                    // 26: EXTI4
    default_handler,                    // 27: DMA1_Channel1
    default_handler,                    // 28: DMA1_Channel2
    default_handler,                    // 29: DMA1_Channel3
    default_handler,                    // 30: DMA1_Channel4
    default_handler,                    // 31: DMA1_Channel5
    default_handler,                    // 32: DMA1_Channel6
    default_handler,                    // 33: DMA1_Channel7
    default_handler,                    // 34: ADC1_2
    default_handler,                    // 35: USB_HP_CAN_TX
    default_handler,                    // 36: USB_LP_CAN_RX0
    default_handler,                    // 37: CAN_RX1
    default_handler,                    // 38: CAN_SCE
    default_handler,                    // 39: EXTI9_5
    default_handler,                    // 40: TIM1_BRK
    default_handler,                    // 41: TIM1_UP
    default_handler,                    // 42: TIM1_TRG_COM
    default_handler,                    // 43: TIM1_CC
    default_handler,                    // 44: TIM2
    default_handler,                    // 45: TIM3
    default_handler,                    // 46: TIM4
    default_handler,                    // 47: I2C1_EV
    default_handler,                    // 48: I2C1_ER
    default_handler,                    // 49: I2C2_EV
    default_handler,                    // 50: I2C2_ER
    default_handler,                    // 51: SPI1
    default_handler,                    // 52: SPI2
    default_handler,                    // 53: USART1
    default_handler,                    // 54: USART2
    default_handler,                    // 55: USART3
    default_handler,                    // 56: EXTI15_10
    default_handler,                    // 57: RTCAlarm
    default_handler,                    // 58: USBWakeup
};