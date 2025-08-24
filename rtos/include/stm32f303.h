#ifndef STM32F303_H
#define STM32F303_H

#include <stdint.h>

// 基地址定义
#define PERIPH_BASE           0x40000000UL
#define AHB1PERIPH_BASE       (PERIPH_BASE + 0x00020000UL)
#define AHB2PERIPH_BASE       (PERIPH_BASE + 0x08000000UL)
#define APB1PERIPH_BASE       PERIPH_BASE
#define APB2PERIPH_BASE       (PERIPH_BASE + 0x00010000UL)

// RCC寄存器基地址
#define RCC_BASE              (AHB1PERIPH_BASE + 0x1000UL)

// GPIO寄存器基地址
#define GPIOA_BASE            (AHB2PERIPH_BASE + 0x0000UL)
#define GPIOB_BASE            (AHB2PERIPH_BASE + 0x0400UL)
#define GPIOC_BASE            (AHB2PERIPH_BASE + 0x0800UL)

// UART寄存器基地址
#define USART1_BASE           (APB2PERIPH_BASE + 0x3800UL)

// SysTick寄存器基地址
#define SYSTICK_BASE          0xE000E010UL

// RCC寄存器结构体
typedef struct {
    volatile uint32_t CR;         // 0x00: 时钟控制寄存器
    volatile uint32_t CFGR;       // 0x04: 时钟配置寄存器
    volatile uint32_t CIR;        // 0x08: 时钟中断寄存器
    volatile uint32_t APB2RSTR;   // 0x0C: APB2外设复位寄存器
    volatile uint32_t APB1RSTR;   // 0x10: APB1外设复位寄存器
    volatile uint32_t AHBENR;     // 0x14: AHB外设时钟使能寄存器
    volatile uint32_t APB2ENR;    // 0x18: APB2外设时钟使能寄存器
    volatile uint32_t APB1ENR;    // 0x1C: APB1外设时钟使能寄存器
    volatile uint32_t BDCR;       // 0x20: 备份域控制寄存器
    volatile uint32_t CSR;        // 0x24: 控制/状态寄存器
    volatile uint32_t AHBRSTR;    // 0x28: AHB外设复位寄存器
    volatile uint32_t CFGR2;      // 0x2C: 时钟配置寄存器2
    volatile uint32_t CFGR3;      // 0x30: 时钟配置寄存器3
} RCC_TypeDef;

// GPIO寄存器结构体
typedef struct {
    volatile uint32_t MODER;      // 0x00: 模式寄存器
    volatile uint32_t OTYPER;     // 0x04: 输出类型寄存器
    volatile uint32_t OSPEEDR;    // 0x08: 输出速度寄存器
    volatile uint32_t PUPDR;      // 0x0C: 上拉/下拉寄存器
    volatile uint32_t IDR;        // 0x10: 输入数据寄存器
    volatile uint32_t ODR;        // 0x14: 输出数据寄存器
    volatile uint32_t BSRR;       // 0x18: 位设置/复位寄存器
    volatile uint32_t LCKR;       // 0x1C: 配置锁定寄存器
    volatile uint32_t AFR[2];     // 0x20-0x24: 复用功能寄存器
    volatile uint32_t BRR;        // 0x28: 位复位寄存器
} GPIO_TypeDef;

// UART寄存器结构体
typedef struct {
    volatile uint32_t CR1;        // 0x00: 控制寄存器1
    volatile uint32_t CR2;        // 0x04: 控制寄存器2
    volatile uint32_t CR3;        // 0x08: 控制寄存器3
    volatile uint32_t BRR;        // 0x0C: 波特率寄存器
    volatile uint32_t GTPR;       // 0x10: 保护时间和预分频寄存器
    volatile uint32_t RTOR;       // 0x14: 接收超时寄存器
    volatile uint32_t RQR;        // 0x18: 请求寄存器
    volatile uint32_t ISR;        // 0x1C: 中断和状态寄存器
    volatile uint32_t ICR;        // 0x20: 中断标志清除寄存器
    volatile uint32_t RDR;        // 0x24: 接收数据寄存器
    volatile uint32_t TDR;        // 0x28: 发送数据寄存器
} USART_TypeDef;

// SysTick寄存器结构体
typedef struct {
    volatile uint32_t CTRL;       // 0x00: 控制和状态寄存器
    volatile uint32_t LOAD;       // 0x04: 重载值寄存器
    volatile uint32_t VAL;        // 0x08: 当前值寄存器
    volatile uint32_t CALIB;      // 0x0C: 校准值寄存器
} SysTick_TypeDef;

// 外设指针定义
#define RCC                   ((RCC_TypeDef *)RCC_BASE)
#define GPIOA                 ((GPIO_TypeDef *)GPIOA_BASE)
#define GPIOB                 ((GPIO_TypeDef *)GPIOB_BASE)
#define GPIOC                 ((GPIO_TypeDef *)GPIOC_BASE)
#define USART1                ((USART_TypeDef *)USART1_BASE)
#define SysTick               ((SysTick_TypeDef *)SYSTICK_BASE)

// RCC位定义
#define RCC_AHBENR_GPIOAEN    (1UL << 17)  // GPIOA时钟使能
#define RCC_AHBENR_GPIOBEN    (1UL << 18)  // GPIOB时钟使能
#define RCC_AHBENR_GPIOCEN    (1UL << 19)  // GPIOC时钟使能

// APB2外设时钟使能
#define RCC_APB2ENR_USART1EN  (1UL << 14)  // USART1时钟使能

// GPIO模式定义
#define GPIO_MODE_INPUT       0x00  // 输入模式
#define GPIO_MODE_OUTPUT      0x01  // 输出模式
#define GPIO_MODE_AF          0x02  // 复用功能模式
#define GPIO_MODE_ANALOG      0x03  // 模拟模式

// GPIO输出类型定义
#define GPIO_OTYPE_PP         0x00  // 推挽输出
#define GPIO_OTYPE_OD         0x01  // 开漏输出

// GPIO速度定义
#define GPIO_SPEED_LOW        0x00  // 低速
#define GPIO_SPEED_MEDIUM     0x01  // 中速
#define GPIO_SPEED_HIGH       0x03  // 高速

// GPIO上拉/下拉定义
#define GPIO_PUPD_NONE        0x00  // 无上拉下拉
#define GPIO_PUPD_UP          0x01  // 上拉
#define GPIO_PUPD_DOWN        0x02  // 下拉

// GPIO复用功能定义
#define GPIO_AF7_USART1       0x07  // USART1复用功能

// UART位定义
#define USART_CR1_UE          (1UL << 0)   // USART使能
#define USART_CR1_RE          (1UL << 2)   // 接收使能
#define USART_CR1_TE          (1UL << 3)   // 发送使能
#define USART_CR1_RXNEIE      (1UL << 5)   // 接收中断使能
#define USART_CR1_TCIE        (1UL << 6)   // 发送完成中断使能
#define USART_CR1_TXEIE       (1UL << 7)   // 发送缓冲区空中断使能
#define USART_ISR_RXNE        (1UL << 5)   // 接收数据寄存器非空
#define USART_ISR_TC          (1UL << 6)   // 发送完成
#define USART_ISR_TXE         (1UL << 7)   // 发送数据寄存器空

// SysTick位定义
#define SYSTICK_CTRL_ENABLE   (1UL << 0)   // SysTick使能
#define SYSTICK_CTRL_TICKINT  (1UL << 1)   // SysTick中断使能
#define SYSTICK_CTRL_CLKSOURCE (1UL << 2)  // 时钟源选择
#define SYSTICK_CTRL_COUNTFLAG (1UL << 16) // 计数标志

// 系统时钟定义
#define SYSTEM_CLOCK_HZ       8000000UL     // 8MHz HSI时钟
#define SYSTICK_FREQ_HZ       1000UL        // 1kHz SysTick频率

#endif // STM32F303_H