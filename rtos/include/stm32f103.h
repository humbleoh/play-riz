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

/* GPIO寄存器 */
#define GPIOC_BASE            (APB2PERIPH_BASE + 0x1000UL)
#define GPIOC_CRH             (*(volatile uint32_t *)(GPIOC_BASE + 0x04))
#define GPIOC_ODR             (*(volatile uint32_t *)(GPIOC_BASE + 0x0C))
#define GPIOC_BSRR            (*(volatile uint32_t *)(GPIOC_BASE + 0x10))

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
#define RCC_APB2ENR_IOPCEN    (1 << 4)   // GPIOC时钟使能
#define SYSTICK_CTRL_ENABLE   (1 << 0)   // SysTick使能
#define SYSTICK_CTRL_TICKINT  (1 << 1)   // SysTick中断使能
#define SYSTICK_CTRL_CLKSOURCE (1 << 2)  // 时钟源选择
#define SCB_ICSR_PENDSTSET    (1 << 26)  // 设置PendSV中断

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

#endif // STM32F103_H