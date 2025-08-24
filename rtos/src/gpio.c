#include "stm32f303.h"

// 延时函数
void delay_ms(uint32_t ms) {
    // 简单的延时实现，基于8MHz系统时钟
    // 每毫秒大约需要8000个时钟周期
    for (uint32_t i = 0; i < ms; i++) {
        for (volatile uint32_t j = 0; j < 2000; j++) {
            // 空循环延时
        }
    }
}

// LED点亮函数（PC13输出低电平）
void led_on(void) {
    GPIOC->BSRR = (1UL << (13 + 16));  // 复位PC13，输出低电平
}

// LED熄灭函数（PC13输出高电平）
void led_off(void) {
    GPIOC->BSRR = (1UL << 13);  // 设置PC13，输出高电平
}

// GPIO初始化函数
void gpio_init(void) {
    // 使能GPIOC时钟
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    
    // 短暂延时确保时钟稳定
    for (volatile int i = 0; i < 100; i++);
    
    // 配置PC13为输出模式
    // 清除PC13的模式位 (位26-27)
    GPIOC->MODER &= ~(0x3UL << (13 * 2));
    // 设置PC13为输出模式 (01)
    GPIOC->MODER |= (GPIO_MODE_OUTPUT << (13 * 2));
    
    // 配置PC13为推挽输出
    GPIOC->OTYPER &= ~(1UL << 13);
    
    // 配置PC13为中等速度
    GPIOC->OSPEEDR &= ~(0x3UL << (13 * 2));
    GPIOC->OSPEEDR |= (GPIO_SPEED_MEDIUM << (13 * 2));
    
    // 配置PC13无上拉下拉
    GPIOC->PUPDR &= ~(0x3UL << (13 * 2));
    GPIOC->PUPDR |= (GPIO_PUPD_NONE << (13 * 2));
    
    // 初始状态：LED熄灭（PC13输出高电平，假设LED低电平有效）
    led_off();
}

// LED状态切换函数
void led_toggle(void) {
    if (GPIOC->ODR & (1UL << 13)) {
        led_on();   // 如果当前是高电平，则点亮LED
    } else {
        led_off();  // 如果当前是低电平，则熄灭LED
    }
}

// LED闪烁函数
void led_blink(uint32_t delay_time) {
    led_on();
    delay_ms(delay_time);
    led_off();
    delay_ms(delay_time);
}