#include "stm32f303.h"

// 函数声明
void gpio_init(void);
void led_on(void);
void led_off(void);
void led_toggle(void);
void led_blink(uint32_t delay_time);
void delay_ms(uint32_t ms);
void system_init(void);

// UART函数声明
void uart1_init(void);
void uart1_send_char(char c);
void uart1_send_string(const char* str);
void uart1_printf(const char* format, ...);
void uart1_send_int(int num);

// 系统初始化函数
void system_init(void) {
    // STM32F303默认使用8MHz HSI时钟
    // 这里可以添加更多的系统初始化代码
    
    // 初始化GPIO
    gpio_init();
    
    // 初始化UART1
    uart1_init();
}

// 主函数
int main(void) {
    // 系统初始化
    system_init();
    
    // 主循环：LED闪烁
    while (1) {
        // LED闪烁，500ms间隔
        led_blink(500);
        
        // 串口输出LED状态
        uart1_send_string("LED Blink\r\n");
        
        // 也可以使用切换方式
        // led_toggle();
        // delay_ms(1000);
    }
    
    return 0;
}