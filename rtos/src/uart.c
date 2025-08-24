#include "../include/stm32f303.h"

// UART1初始化函数
void uart1_init(void) {
    // 使能GPIOA和USART1时钟
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;     // 使能GPIOA时钟
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;  // 使能USART1时钟
    
    // 配置PA9 (TX) 和 PA10 (RX) 为复用功能
    // PA9配置
    GPIOA->MODER &= ~(3UL << (9 * 2));     // 清除PA9模式位
    GPIOA->MODER |= (2UL << (9 * 2));      // 设置PA9为复用功能模式
    GPIOA->OTYPER &= ~(1UL << 9);          // PA9推挽输出
    GPIOA->OSPEEDR |= (3UL << (9 * 2));    // PA9高速
    GPIOA->PUPDR &= ~(3UL << (9 * 2));     // PA9无上拉下拉
    
    // PA10配置
    GPIOA->MODER &= ~(3UL << (10 * 2));    // 清除PA10模式位
    GPIOA->MODER |= (2UL << (10 * 2));     // 设置PA10为复用功能模式
    GPIOA->OTYPER &= ~(1UL << 10);         // PA10推挽输出
    GPIOA->OSPEEDR |= (3UL << (10 * 2));   // PA10高速
    GPIOA->PUPDR |= (1UL << (10 * 2));     // PA10上拉
    
    // 配置复用功能为USART1 (AF7)
    // PA9 -> AFR[1] bit 4-7, PA10 -> AFR[1] bit 8-11
    GPIOA->AFR[1] &= ~(0xFFUL << 4);       // 清除PA9和PA10的复用功能位
    GPIOA->AFR[1] |= (GPIO_AF7_USART1 << 4);  // PA9 -> USART1_TX
    GPIOA->AFR[1] |= (GPIO_AF7_USART1 << 8);  // PA10 -> USART1_RX
    
    // 配置USART1
    // 假设系统时钟为8MHz，波特率115200
    // BRR = 8000000 / 115200 ≈ 69.44 ≈ 69 (0x45)
    USART1->BRR = 69;                      // 设置波特率为115200
    
    // 使能发送和接收
    USART1->CR1 |= USART_CR1_TE;           // 使能发送
    USART1->CR1 |= USART_CR1_RE;           // 使能接收
    
    // 使能USART1
    USART1->CR1 |= USART_CR1_UE;           // 使能USART1
}

// 发送单个字符
void uart1_send_char(char c) {
    // 等待发送数据寄存器空
    while (!(USART1->ISR & USART_ISR_TXE));
    
    // 发送字符
    USART1->TDR = c;
    
    // 等待发送完成
    while (!(USART1->ISR & USART_ISR_TC));
}

// 发送字符串
void uart1_send_string(const char* str) {
    while (*str) {
        uart1_send_char(*str++);
    }
}

// 简单的printf实现（仅支持字符串和整数）
void uart1_printf(const char* format, ...) {
    const char* p = format;
    
    // 简化版本，仅处理字符串
    while (*p) {
        if (*p == '\\' && *(p+1) == 'n') {
            uart1_send_char('\r');
            uart1_send_char('\n');
            p += 2;
        } else {
            uart1_send_char(*p);
            p++;
        }
    }
}

// 发送整数（十进制）
void uart1_send_int(int num) {
    char buffer[12];  // 足够存储32位整数
    int i = 0;
    int is_negative = 0;
    
    // 处理负数
    if (num < 0) {
        is_negative = 1;
        num = -num;
    }
    
    // 处理0的特殊情况
    if (num == 0) {
        uart1_send_char('0');
        return;
    }
    
    // 转换为字符串（逆序）
    while (num > 0) {
        buffer[i++] = '0' + (num % 10);
        num /= 10;
    }
    
    // 添加负号
    if (is_negative) {
        uart1_send_char('-');
    }
    
    // 逆序输出
    while (i > 0) {
        uart1_send_char(buffer[--i]);
    }
}