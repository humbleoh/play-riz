#include "../include/stm32f103.h"

/* 系统初始化 */
void system_init(void) {
    /* 配置系统时钟为8MHz HSI */
    RCC_CR |= (1 << 0);  // 使能HSI
    while (!(RCC_CR & (1 << 1)));  // 等待HSI就绪
    
    /* 配置系统时钟源为HSI */
    RCC_CFGR &= ~(3 << 0);  // 清除SW位
    RCC_CFGR |= (0 << 0);   // 选择HSI作为系统时钟
    
    /* 等待系统时钟切换完成 */
    while ((RCC_CFGR & (3 << 2)) != (0 << 2));
    
    /* 初始化GPIO */
    gpio_init();
}

/* GPIO初始化 */
void gpio_init(void) {
    /* 使能GPIOC时钟 */
    RCC_APB2ENR |= RCC_APB2ENR_IOPCEN;
    
    /* 配置PC13为推挽输出，最大速度2MHz */
    GPIOC_CRH &= ~(0xF << 20);  // 清除PC13配置位
    GPIOC_CRH |= (0x2 << 20);   // 配置为推挽输出，2MHz
    
    /* 初始状态LED熄灭 (PC13高电平) */
    led_off();
}

/* LED控制函数 */
void led_on(void) {
    GPIOC_BSRR = (1 << (LED_PIN + 16));  // 复位PC13 (LED点亮)
}

void led_off(void) {
    GPIOC_BSRR = (1 << LED_PIN);  // 置位PC13 (LED熄灭)
}

void led_toggle(void) {
    if (GPIOC_ODR & (1 << LED_PIN)) {
        led_on();
    } else {
        led_off();
    }
}

/* 简单延时函数 (阻塞式) */
void delay_ms(uint32_t ms) {
    volatile uint32_t count;
    for (uint32_t i = 0; i < ms; i++) {
        /* 大约1ms的延时 (基于8MHz时钟) */
        for (count = 0; count < 2000; count++) {
            __asm("nop");
        }
    }
}