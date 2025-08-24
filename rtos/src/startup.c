#include "stm32f303.h"

// 外部符号声明（来自链接脚本）
extern uint32_t _estack;     // 栈顶地址
extern uint32_t _sdata;      // 数据段起始地址
extern uint32_t _edata;      // 数据段结束地址
extern uint32_t _sidata;     // 数据段在Flash中的起始地址
extern uint32_t _sbss;       // BSS段起始地址
extern uint32_t _ebss;       // BSS段结束地址

// 主函数声明
int main(void);

// 默认中断处理函数
void default_handler(void) {
    while (1) {
        // 无限循环
    }
}

// 复位处理函数
void reset_handler(void) {
    uint32_t *src, *dst;
    
    // 复制数据段从Flash到SRAM
    src = &_sidata;
    dst = &_sdata;
    while (dst < &_edata) {
        *dst++ = *src++;
    }
    
    // 清零BSS段
    dst = &_sbss;
    while (dst < &_ebss) {
        *dst++ = 0;
    }
    
    // 调用主函数
    main();
    
    // 如果main函数返回，进入无限循环
    while (1);
}

// 中断处理函数声明（弱符号，可被重写）
void nmi_handler(void) __attribute__((weak, alias("default_handler")));
void hardfault_handler(void) __attribute__((weak, alias("default_handler")));
void memmanage_handler(void) __attribute__((weak, alias("default_handler")));
void busfault_handler(void) __attribute__((weak, alias("default_handler")));
void usagefault_handler(void) __attribute__((weak, alias("default_handler")));
void svc_handler(void) __attribute__((weak, alias("default_handler")));
void debugmon_handler(void) __attribute__((weak, alias("default_handler")));
void pendsv_handler(void) __attribute__((weak, alias("default_handler")));
void systick_handler(void) __attribute__((weak, alias("default_handler")));

// 外设中断处理函数声明（部分常用的）
void wwdg_irq_handler(void) __attribute__((weak, alias("default_handler")));
void pvd_irq_handler(void) __attribute__((weak, alias("default_handler")));
void tamper_stamp_irq_handler(void) __attribute__((weak, alias("default_handler")));
void rtc_wkup_irq_handler(void) __attribute__((weak, alias("default_handler")));
void flash_irq_handler(void) __attribute__((weak, alias("default_handler")));
void rcc_irq_handler(void) __attribute__((weak, alias("default_handler")));
void exti0_irq_handler(void) __attribute__((weak, alias("default_handler")));
void exti1_irq_handler(void) __attribute__((weak, alias("default_handler")));
void exti2_tsc_irq_handler(void) __attribute__((weak, alias("default_handler")));
void exti3_irq_handler(void) __attribute__((weak, alias("default_handler")));
void exti4_irq_handler(void) __attribute__((weak, alias("default_handler")));
void dma1_ch1_irq_handler(void) __attribute__((weak, alias("default_handler")));
void dma1_ch2_irq_handler(void) __attribute__((weak, alias("default_handler")));
void dma1_ch3_irq_handler(void) __attribute__((weak, alias("default_handler")));
void dma1_ch4_irq_handler(void) __attribute__((weak, alias("default_handler")));
void dma1_ch5_irq_handler(void) __attribute__((weak, alias("default_handler")));
void dma1_ch6_irq_handler(void) __attribute__((weak, alias("default_handler")));
void dma1_ch7_irq_handler(void) __attribute__((weak, alias("default_handler")));
void adc1_2_irq_handler(void) __attribute__((weak, alias("default_handler")));
void usb_hp_can_tx_irq_handler(void) __attribute__((weak, alias("default_handler")));
void usb_lp_can_rx0_irq_handler(void) __attribute__((weak, alias("default_handler")));
void can_rx1_irq_handler(void) __attribute__((weak, alias("default_handler")));
void can_sce_irq_handler(void) __attribute__((weak, alias("default_handler")));
void exti9_5_irq_handler(void) __attribute__((weak, alias("default_handler")));
void tim1_brk_tim15_irq_handler(void) __attribute__((weak, alias("default_handler")));
void tim1_up_tim16_irq_handler(void) __attribute__((weak, alias("default_handler")));
void tim1_trg_com_tim17_irq_handler(void) __attribute__((weak, alias("default_handler")));
void tim1_cc_irq_handler(void) __attribute__((weak, alias("default_handler")));
void tim2_irq_handler(void) __attribute__((weak, alias("default_handler")));
void tim3_irq_handler(void) __attribute__((weak, alias("default_handler")));
void tim4_irq_handler(void) __attribute__((weak, alias("default_handler")));
void i2c1_ev_irq_handler(void) __attribute__((weak, alias("default_handler")));
void i2c1_er_irq_handler(void) __attribute__((weak, alias("default_handler")));
void i2c2_ev_irq_handler(void) __attribute__((weak, alias("default_handler")));
void i2c2_er_irq_handler(void) __attribute__((weak, alias("default_handler")));
void spi1_irq_handler(void) __attribute__((weak, alias("default_handler")));
void spi2_irq_handler(void) __attribute__((weak, alias("default_handler")));
void usart1_irq_handler(void) __attribute__((weak, alias("default_handler")));
void usart2_irq_handler(void) __attribute__((weak, alias("default_handler")));
void usart3_irq_handler(void) __attribute__((weak, alias("default_handler")));
void exti15_10_irq_handler(void) __attribute__((weak, alias("default_handler")));
void rtc_alarm_irq_handler(void) __attribute__((weak, alias("default_handler")));
void usb_wkup_irq_handler(void) __attribute__((weak, alias("default_handler")));

// 中断向量表
__attribute__((section(".isr_vector")))
const void* vector_table[] = {
    &_estack,                              // 0: 栈顶指针
    reset_handler,                         // 1: 复位处理函数
    nmi_handler,                           // 2: NMI处理函数
    hardfault_handler,                     // 3: 硬件错误处理函数
    memmanage_handler,                     // 4: 内存管理错误处理函数
    busfault_handler,                      // 5: 总线错误处理函数
    usagefault_handler,                    // 6: 使用错误处理函数
    0,                                     // 7: 保留
    0,                                     // 8: 保留
    0,                                     // 9: 保留
    0,                                     // 10: 保留
    svc_handler,                           // 11: SVC处理函数
    debugmon_handler,                      // 12: 调试监视器处理函数
    0,                                     // 13: 保留
    pendsv_handler,                        // 14: PendSV处理函数
    systick_handler,                       // 15: SysTick处理函数
    
    // 外设中断向量
    wwdg_irq_handler,                      // 16: WWDG
    pvd_irq_handler,                       // 17: PVD
    tamper_stamp_irq_handler,              // 18: Tamper and TimeStamp
    rtc_wkup_irq_handler,                  // 19: RTC Wakeup
    flash_irq_handler,                     // 20: Flash
    rcc_irq_handler,                       // 21: RCC
    exti0_irq_handler,                     // 22: EXTI Line 0
    exti1_irq_handler,                     // 23: EXTI Line 1
    exti2_tsc_irq_handler,                 // 24: EXTI Line 2 and TSC
    exti3_irq_handler,                     // 25: EXTI Line 3
    exti4_irq_handler,                     // 26: EXTI Line 4
    dma1_ch1_irq_handler,                  // 27: DMA1 Channel 1
    dma1_ch2_irq_handler,                  // 28: DMA1 Channel 2
    dma1_ch3_irq_handler,                  // 29: DMA1 Channel 3
    dma1_ch4_irq_handler,                  // 30: DMA1 Channel 4
    dma1_ch5_irq_handler,                  // 31: DMA1 Channel 5
    dma1_ch6_irq_handler,                  // 32: DMA1 Channel 6
    dma1_ch7_irq_handler,                  // 33: DMA1 Channel 7
    adc1_2_irq_handler,                    // 34: ADC1 and ADC2
    usb_hp_can_tx_irq_handler,             // 35: USB High Priority or CAN TX
    usb_lp_can_rx0_irq_handler,            // 36: USB Low Priority or CAN RX0
    can_rx1_irq_handler,                   // 37: CAN RX1
    can_sce_irq_handler,                   // 38: CAN SCE
    exti9_5_irq_handler,                   // 39: EXTI Line 9..5
    tim1_brk_tim15_irq_handler,            // 40: TIM1 Break and TIM15
    tim1_up_tim16_irq_handler,             // 41: TIM1 Update and TIM16
    tim1_trg_com_tim17_irq_handler,        // 42: TIM1 Trigger and Commutation and TIM17
    tim1_cc_irq_handler,                   // 43: TIM1 Capture Compare
    tim2_irq_handler,                      // 44: TIM2
    tim3_irq_handler,                      // 45: TIM3
    tim4_irq_handler,                      // 46: TIM4
    i2c1_ev_irq_handler,                   // 47: I2C1 Event
    i2c1_er_irq_handler,                   // 48: I2C1 Error
    i2c2_ev_irq_handler,                   // 49: I2C2 Event
    i2c2_er_irq_handler,                   // 50: I2C2 Error
    spi1_irq_handler,                      // 51: SPI1
    spi2_irq_handler,                      // 52: SPI2
    usart1_irq_handler,                    // 53: USART1
    usart2_irq_handler,                    // 54: USART2
    usart3_irq_handler,                    // 55: USART3
    exti15_10_irq_handler,                 // 56: EXTI Line 15..10
    rtc_alarm_irq_handler,                 // 57: RTC Alarm through EXTI Line
    usb_wkup_irq_handler,                  // 58: USB Wakeup from suspend
};