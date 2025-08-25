#ifndef STM32F103_H
#define STM32F103_H

#include <stdint.h>

/* STM32F103C8T6 (BluePill) 寄存器定义 */

/* 基地址定义 */
#define PERIPH_BASE           0x40000000UL
#define APB2PERIPH_BASE       (PERIPH_BASE + 0x10000UL)
#define AHBPERIPH_BASE        (PERIPH_BASE + 0x20000UL)

/* RCC寄存器 */
#define RCC_BASE              (AHBPERIPH_BASE + 0x1000UL)
#define RCC_CR                (*(volatile uint32_t *)(RCC_BASE + 0x00))
#define RCC_CFGR              (*(volatile uint32_t *)(RCC_BASE + 0x04))
#define RCC_APB2ENR           (*(volatile uint32_t *)(RCC_BASE + 0x18))
#define RCC_APB1ENR           (*(volatile uint32_t *)(RCC_BASE + 0x1C))

/* GPIO寄存器 */
#define GPIOA_BASE            (APB2PERIPH_BASE + 0x0800UL)
#define GPIOA_CRL             (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_CRH             (*(volatile uint32_t *)(GPIOA_BASE + 0x04))
#define GPIOA_ODR             (*(volatile uint32_t *)(GPIOA_BASE + 0x0C))
#define GPIOA_BSRR            (*(volatile uint32_t *)(GPIOA_BASE + 0x10))

#define GPIOC_BASE            (APB2PERIPH_BASE + 0x1000UL)
#define GPIOC_CRH             (*(volatile uint32_t *)(GPIOC_BASE + 0x04))
#define GPIOC_ODR             (*(volatile uint32_t *)(GPIOC_BASE + 0x0C))
#define GPIOC_BSRR            (*(volatile uint32_t *)(GPIOC_BASE + 0x10))

/* UART1寄存器 */
#define USART1_BASE           (APB2PERIPH_BASE + 0x3800UL)
#define USART1_SR             (*(volatile uint32_t *)(USART1_BASE + 0x00))
#define USART1_DR             (*(volatile uint32_t *)(USART1_BASE + 0x04))
#define USART1_BRR            (*(volatile uint32_t *)(USART1_BASE + 0x08))
#define USART1_CR1            (*(volatile uint32_t *)(USART1_BASE + 0x0C))
#define USART1_CR2            (*(volatile uint32_t *)(USART1_BASE + 0x10))
#define USART1_CR3            (*(volatile uint32_t *)(USART1_BASE + 0x14))

/* SysTick寄存器 */
#define SYSTICK_BASE          0xE000E010UL
#define SYSTICK_CTRL          (*(volatile uint32_t *)(SYSTICK_BASE + 0x00))
#define SYSTICK_LOAD          (*(volatile uint32_t *)(SYSTICK_BASE + 0x04))
#define SYSTICK_VAL           (*(volatile uint32_t *)(SYSTICK_BASE + 0x08))
#define SYSTICK_CALIB         (*(volatile uint32_t *)(SYSTICK_BASE + 0x0C))

/* NVIC寄存器 */
#define NVIC_BASE             0xE000E100UL
#define NVIC_ISER             (*(volatile uint32_t *)(NVIC_BASE + 0x00))
#define NVIC_ICER             (*(volatile uint32_t *)(NVIC_BASE + 0x80))
#define NVIC_ISPR             (*(volatile uint32_t *)(NVIC_BASE + 0x100))
#define NVIC_ICPR             (*(volatile uint32_t *)(NVIC_BASE + 0x180))

/* SCB寄存器 */
#define SCB_BASE              0xE000ED00UL
#define SCB_ICSR              (*(volatile uint32_t *)(SCB_BASE + 0x04))
#define SCB_VTOR              (*(volatile uint32_t *)(SCB_BASE + 0x08))
#define SCB_AIRCR             (*(volatile uint32_t *)(SCB_BASE + 0x0C))
#define SCB_SCR               (*(volatile uint32_t *)(SCB_BASE + 0x10))
#define SCB_SHPR3             (*(volatile uint32_t *)(SCB_BASE + 0x20))

/* 位定义 */
/* RCC位定义 */
#define RCC_APB2ENR_IOPAEN    (1 << 2)   // GPIOA时钟使能
#define RCC_APB2ENR_IOPCEN    (1 << 4)   // GPIOC时钟使能
#define RCC_APB2ENR_USART1EN  (1 << 14)  // USART1时钟使能

/* SysTick位定义 */
#define SYSTICK_CTRL_ENABLE   (1 << 0)   // SysTick使能
#define SYSTICK_CTRL_TICKINT  (1 << 1)   // SysTick中断使能
#define SYSTICK_CTRL_CLKSOURCE (1 << 2)  // 时钟源选择

/* SCB位定义 */
#define SCB_ICSR_PENDSTSET    (1 << 26)  // 设置PendSV中断

/* UART位定义 */
#define USART_SR_TXE          (1 << 7)   // 发送数据寄存器空
#define USART_SR_RXNE         (1 << 5)   // 接收数据寄存器非空
#define USART_SR_TC           (1 << 6)   // 传输完成
#define USART_CR1_UE          (1 << 13)  // USART使能
#define USART_CR1_TE          (1 << 3)   // 发送使能
#define USART_CR1_RE          (1 << 2)   // 接收使能

/* 系统时钟频率 */
#define SYSTEM_CLOCK_HZ       8000000UL  // 8MHz HSI
#define SYSTICK_FREQ_HZ       1000UL     // 1kHz (1ms)

/* GPIO引脚定义 */
#define LED_PIN               13         // PC13 (BluePill板载LED)

/* 中断优先级 */
#define SYSTICK_PRIORITY      15         // 最低优先级
#define PENDSV_PRIORITY       15         // 最低优先级

/* 函数声明 */
void system_init(void);
void gpio_init(void);
void led_on(void);
void led_off(void);
void led_toggle(void);
void delay_ms(uint32_t ms);

/* UART函数声明 */
void uart_init(uint32_t baudrate);
void uart_send_char(char c);
void uart_send_string(const char *str);
char uart_receive_char(void);
int uart_data_available(void);

/* 系统调用重定向 */
int _write(int file, char *ptr, int len);
int _read(int file, char *ptr, int len);
int _close(int file);
int _lseek(int file, int ptr, int dir);
int _fstat(int file, void *st);
int _isatty(int file);
void *_sbrk(int incr);

#endif // STM32F103_H