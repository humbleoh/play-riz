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

/* UART驱动程序 */

/* UART初始化 */
void uart_init(uint32_t baudrate) {
    /* 使能GPIOA和USART1时钟 */
    RCC_APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;
    
    /* 配置PA9 (USART1_TX) 为复用推挽输出 */
    GPIOA_CRH &= ~(0xF << 4);   // 清除PA9配置位
    GPIOA_CRH |= (0xB << 4);    // 复用推挽输出，50MHz
    
    /* 配置PA10 (USART1_RX) 为浮空输入 */
    GPIOA_CRH &= ~(0xF << 8);   // 清除PA10配置位
    GPIOA_CRH |= (0x4 << 8);    // 浮空输入
    
    /* 配置USART1波特率 */
    /* 波特率计算: BRR = PCLK2 / baudrate */
    /* PCLK2 = 8MHz (系统时钟) */
    uint32_t brr_value = SYSTEM_CLOCK_HZ / baudrate;
    USART1_BRR = brr_value;
    
    /* 配置USART1控制寄存器 */
    USART1_CR1 = 0;  // 复位所有位
    USART1_CR1 |= USART_CR1_TE | USART_CR1_RE;  // 使能发送和接收
    USART1_CR1 |= USART_CR1_UE;  // 使能USART1
    
    USART1_CR2 = 0;  // 默认配置 (1停止位)
    USART1_CR3 = 0;  // 默认配置 (无硬件流控)
}

/* 发送单个字符 */
void uart_send_char(char c) {
    /* 等待发送数据寄存器空 */
    while (!(USART1_SR & USART_SR_TXE));
    
    /* 发送字符 */
    USART1_DR = c;
    
    /* 等待传输完成 */
    while (!(USART1_SR & USART_SR_TC));
}

/* 发送字符串 */
void uart_send_string(const char *str) {
    while (*str) {
        uart_send_char(*str++);
    }
}

/* 接收单个字符 */
char uart_receive_char(void) {
    /* 等待接收数据寄存器非空 */
    while (!(USART1_SR & USART_SR_RXNE));
    
    /* 读取字符 */
    return (char)(USART1_DR & 0xFF);
}

/* 检查是否有数据可读 */
int uart_data_available(void) {
    return (USART1_SR & USART_SR_RXNE) ? 1 : 0;
}

/* 系统调用重定向 */

/* 重定向printf到UART */
int _write(int file, char *ptr, int len) {
    (void)file;  // 忽略文件描述符
    
    for (int i = 0; i < len; i++) {
        uart_send_char(ptr[i]);
    }
    
    return len;
}

/* 重定向scanf到UART */
int _read(int file, char *ptr, int len) {
    (void)file;  // 忽略文件描述符
    
    for (int i = 0; i < len; i++) {
        ptr[i] = uart_receive_char();
        
        /* 回显字符 */
        uart_send_char(ptr[i]);
        
        /* 遇到回车或换行符时结束 */
        if (ptr[i] == '\r' || ptr[i] == '\n') {
            ptr[i] = '\n';  // 统一为换行符
            return i + 1;
        }
    }
    
    return len;
}

/* 其他系统调用的空实现 */
int _close(int file) {
    (void)file;
    return -1;
}

int _lseek(int file, int ptr, int dir) {
    (void)file;
    (void)ptr;
    (void)dir;
    return 0;
}

int _fstat(int file, void *st) {
    (void)file;
    (void)st;
    return 0;
}

int _isatty(int file) {
    (void)file;
    return 1;
}

/* 简单的堆管理 */
extern char _end;  // 由链接器提供的堆起始地址
static char *heap_ptr = &_end;

void *_sbrk(int incr) {
    char *prev_heap_ptr = heap_ptr;
    heap_ptr += incr;
    return prev_heap_ptr;
}