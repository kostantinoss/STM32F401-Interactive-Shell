#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "main.h"
#include "shell.h"
#include "uart_driver.h"

#include <ctype.h>
#include "stm32f401xe.h"
#include "stm32f4xx_hal_uart.h"

#define CMD_BUFFER_SIZE 124
#define PRINT_BUFFER_SIZE 256
#define RX_BUFFER_SIZE 256

char cmd_buffer[CMD_BUFFER_SIZE];
int cursor_pos;


RingBuffer_t rx_buffer = {0};

void buffer_putc(RingBuffer_t *rb, uint8_t c) {
    if (rb->count < RX_BUFFER_SIZE) {
        rb->buffer[rb->head] = c;
        rb->head = (rb->head + 1) % RX_BUFFER_SIZE;
        rb->count++;
    }
}

uint8_t buffer_getc(RingBuffer_t *rb) {
    uint8_t ch;
    if (rb->count > 0) {
        ch = rb->buffer[rb->tail];
        rb->tail = (rb->tail + 1) % RX_BUFFER_SIZE;
        rb->count--;
        return ch;
    }
    return -1;
}

CommandHistory_t cmd_history_buffer = {0};

void save_cmd_to_history(CommandHistory_t *history, char *cmd) {
    if (strlen(cmd) == 0)
        return;

    strncpy(history->commands[history->current_index], cmd, strlen(cmd));
    history->history_count++;
    cursor_pos = (int) strlen(history->commands[history->display_index]);
    history->current_index = (history->current_index + 1) % CMD_HISTORY_SIZE;
    history->display_index = history->current_index;
}

void show_previous_cmd(CommandHistory_t *history) {
    if  (history->display_index == 0 || history->history_count == 0) {
        return;
    }
    while (cursor_pos > 0) {
        print_shell("\b \b");
        cursor_pos--;
    }
    history->display_index = (history->display_index - 1) % CMD_HISTORY_SIZE;
    print_shell("%s", history->commands[history->display_index]);
    cursor_pos = (int) strlen(history->commands[history->display_index]);
}

void show_next_cmd(CommandHistory_t *history) {
    while (cursor_pos > 0) {
        cursor_pos--;
        print_shell("\b \b");
    }
    history->display_index = (history->display_index + 1) % CMD_HISTORY_SIZE;
    print_shell("%s", history->commands[history->display_index]);
    cursor_pos = (int) strlen(history->commands[history->display_index]);
}

void print_shell(const char *format, ...) {
    char _buffer[PRINT_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(_buffer, PRINT_BUFFER_SIZE, format, args);
    va_end(args);

    HAL_UART_Transmit(UART_GetHandle(), (uint8_t *)_buffer, strlen(_buffer), HAL_MAX_DELAY);
}

void uint32_to_binary_string(uint32_t num, char *buffer, size_t buffer_size) {
    if (buffer_size < 33) return;

    for (int i = 0; i < 32; i++) {
        buffer[i] = '0' + ((num >> (31 - i)) & 0x1);
    }
    buffer[32] = '\0';  // Null terminate
}

void shell_prompt(void) {
    print_shell("STM32> ");
    fflush(stdout);
    cursor_pos = 0;
}

void shell_init() {
    cursor_pos = 0;

    print_shell("\r\n");
    print_shell("===============================================\r\n");
    print_shell("  STM32F401 Interactive Shell\r\n");
    print_shell("===============================================\r\n");
    print_shell("Type 'help' for available commands\r\n");
    print_shell("\r\n");

    shell_prompt();

    HAL_UART_Receive_IT(UART_GetHandle(), &chrx, 1);
}

void process_char(const uint8_t c) {
    uint8_t esc_seq[4];
    static int esc_count;
    static int is_esc;

    // Handle Ctl+C
    if (c == 0x03) {
        print_shell("^C\r\n");
        is_esc = 0;
        esc_count = 0;
        cursor_pos = 0;
        memset(cmd_buffer, 0, CMD_BUFFER_SIZE);
        cmd_history_buffer.display_index = cmd_history_buffer.current_index;
        shell_prompt();

        return;
    }

    if (c == 0x1B) {
        is_esc = 1;
        esc_count = 0;
        esc_seq[esc_count++] = 0x1B;
        return;
    }

    if (is_esc) {
        esc_seq[esc_count++] = c;
        if (esc_count >= 3) {
            esc_seq[esc_count] = '\0';
            switch (esc_seq[2]) {
                case 'A':
                    show_previous_cmd(&cmd_history_buffer);
                    break;
                case 'B':
                    show_next_cmd(&cmd_history_buffer);
                    break;
                case 'C': // Right Arrow
                    if (cursor_pos < strlen(cmd_buffer)) {
                        print_shell("%s", "\033[C");
                        cursor_pos++;
                    }
                    break;
                case 'D': // Left Arrow
                    if (cursor_pos > 0) {
                        print_shell("%s", "\033[D");
                        cursor_pos--;
                    }
                    break;
                default:
                    break;
            }
            esc_count = 0;
            is_esc = 0;
            return;
        }
        return;
    }

    // Backspace
    if (c == '\b' || c == 127) {
        if (cursor_pos > 0) {
            cursor_pos--;
            print_shell("\b \b");
        }
    }

    // Enter
    else if (c == '\n') {
        print_shell("\r\n");
        if (cursor_pos > 0) {
            cmd_buffer[cursor_pos] = '\0';
            save_cmd_to_history(&cmd_history_buffer, cmd_buffer);
            process_command(cmd_buffer);
            cursor_pos = 0;
        }
        shell_prompt();
    }

    // printable characters
    else if (c >= 32 && c <= 126) {
        if (cursor_pos < CMD_BUFFER_SIZE) {
            cmd_buffer[cursor_pos++] = c;
            print_shell("%c", c);
        }
    }

}

void process_input() {
    while (rx_buffer.count > 0) {
        const char c = buffer_getc(&rx_buffer);
        process_char(c);
    }
}

void process_command(char *command) {
    // Supported commands:
    //   help        - Show this help message
    //   sysinfo     - Show system information
    //   status      - Display status for peripherals (GPIOA, GPIOB, GPIOC, GPIOD, UART2, RCC, TIMER1)
    //   echo        - Echo input text
    //   led         - Control onboard LED (on|off|toggle)
    //   showreg     - Show peripheral raw register values (UART2, GPIOA, TIMER1, etc.)

    if (strcmp("help", command) == 0) {
        print_help_msg();
    }
    else if (strcmp("sysinfo", command) == 0) {
        print_sys_info_msg();
    }
    else if (strncmp("status", command, 6) == 0) {
        char *args = command + 6;

        // Trim whitespaces
        while (*args == ' ' || *args == '\t') args++;

        if (strcmp(args, "GPIOA") == 0 || strcmp(args, "gpioa") == 0) {
            print_gpio_status_cmd(GPIOA, "GPIOA");
        }
        else if (strcmp(args, "GPIOB") == 0 || strcmp(args, "gpiob") == 0) {
            print_gpio_status_cmd(GPIOB, "GPIOB");
        }
        else if (strcmp(args, "GPIOC") == 0 || strcmp(args, "gpioc") == 0) {
            print_gpio_status_cmd(GPIOC, "GPIOC");
        }
        else if (strcmp(args, "GPIOD") == 0 || strcmp(args, "gpiod") == 0) {
            print_gpio_status_cmd(GPIOD, "GPIOD");
        }
        else if (strcmp(args, "uart2") == 0 || strcmp(args, "UART2") == 0) {
            print_uart_status_cmd(args);
        }
        else if (strcmp(args, "rcc") == 0 || strcmp(args, "RCC") == 0) {
            print_rcc_status_cmd();
        }
        else if (strcmp(args, "timer1") == 0 || strcmp(args, "TIMER1") == 0) {
            print_timer_status_cmd(args);
        }
        else {
            print_shell("Usage: status <gpioa|gpiob|gpioc|gpiod|uart2|rcc|timer1>\r\n");
        }

    }
    else if (strncmp("echo", command, 4) == 0) {
        echo_cmd(command);
    }
    else if (strncmp("led", command, 3) == 0) {
        // led_control(command);
        // handle led <on|off|toggle>S
    }
    else if (strncmp("showreg", command, 7) == 0) {
        char *args = command + 7;
        while (*args == ' ' || *args == '\t') args++; // Skip spaces

        if (strcmp(args, "uart2") == 0) {
            showreg_uart2();
        }
        else if (strcmp(args, "gpioa") == 0) {
            showreg_gpio(GPIOA);
        }
        else if (strcmp(args, "rcc") == 0) {
            showreg_rcc();
        }
        else if (strcmp(args, "timer1") == 0) {
            showreg_timer1();
        }
        else {
            print_shell("Usage: showreg <uart2|gpioa|rcc|timer1>\r\n");
        }
    }
    else if (strncmp("clear", command, 6) == 0) {
        clear_cmd();
    }
    else {
        print_shell("unknown command: %s\r\n", command);
    }

}

void print_help_msg(void) {
    print_shell("\r\n");
    print_shell("=====================================================\r\n");
    print_shell("  Available Commands\r\n");
    print_shell("=====================================================\r\n");
    print_shell("  help                      - Show this help message\r\n");
    print_shell("  sysinfo                   - Display system information\r\n");
    print_shell("  echo <text>               - Echo text back to console\r\n");
    print_shell("  led <on|off|toggle>       - Control onboard LED\r\n");
    print_shell("  gpio <port> <pin>         - Read GPIO pin state\r\n");
    print_shell("  status                    - Show peripheral status\r\n");
    print_shell("  showreg <periph> <reg>    - Display register value\r\n");
    print_shell("  clear                     - Clear screen\r\n");
    print_shell("=====================================================\r\n");
    print_shell("\r\n");
}

void print_sys_info_msg() {
    uint32_t sysclk = HAL_RCC_GetSysClockFreq();
    uint32_t hclk = HAL_RCC_GetHCLKFreq();
    uint32_t pclk1 = HAL_RCC_GetPCLK1Freq();
    uint32_t pclk2 = HAL_RCC_GetPCLK2Freq();

    print_shell("\r\n");
    print_shell("===============================================\r\n");
    print_shell("  System Information\r\n");
    print_shell("===============================================\r\n");
    print_shell("  MCU:              STM32F401xE\r\n");
    print_shell("  Core:             ARM Cortex-M4\r\n");
    print_shell("  HAL Version:      0x%08lX\r\n", HAL_GetHalVersion());
    print_shell("  Device ID:        0x%08lX\r\n", HAL_GetDEVID());
    print_shell("  Revision ID:      0x%08lX\r\n", HAL_GetREVID());
    print_shell("-----------------------------------------------\r\n");
    print_shell("  SYSCLK:           %lu Hz (%lu MHz)\r\n", sysclk, sysclk / 1000000);
    print_shell("  HCLK (AHB):       %lu Hz (%lu MHz)\r\n", hclk, hclk / 1000000);
    print_shell("  PCLK1 (APB1):     %lu Hz (%lu MHz)\r\n", pclk1, pclk1 / 1000000);
    print_shell("  PCLK2 (APB2):     %lu Hz (%lu MHz)\r\n", pclk2, pclk2 / 1000000);
    print_shell("-----------------------------------------------\r\n");
    print_shell("  Tick Frequency:   %lu Hz\r\n", HAL_GetTickFreq());
    print_shell("  Uptime (ticks):   %lu sec\r\n", HAL_GetTick() / 1000);
    print_shell("===============================================\r\n");
    print_shell("\r\n");

}

void clear_cmd() {
    print_shell("\033[2J");    // Clear screen
    print_shell("\033[H");     // Move cursor to home position
}

void echo_cmd(char *cmd) {
    char *args = cmd + 4;

    while (*args == ' ' || *args == '\t') {
        args++;
    }

    if (*args != '\0') {
        print_shell("%s\r\n", args);
    } else {
        print_shell("\r\n");
    }

}

void print_gpio_status_cmd(GPIO_TypeDef *GPIOx, const char *port_name) {
    /*
    === GPIOA Status ===
    Configured Pins:
      PA2:  AF (USART2), Speed: Low, Pull: None
      PA3:  AF (USART2), Speed: Low, Pull: None
      PA5:  Output, Speed: Low, Pull: None, State: LOW
      PA13: AF (SWDIO), Speed: Very High, Pull: Pull-up
      PA14: AF (SWCLK), Speed: Low, Pull: Pull-down
      PA15: AF (SWO), Speed: Low, Pull: Pull-up

    Input States:
      IDR: 00000000000000001010000000001100 (Pins 2,3,13,15 = HIGH)

    Output States:
      ODR: 00000000000000000000000000000000 (All outputs LOW)
    */

    // TODO: Implementation - High-level summary showing only configured pins
}

const char* get_peripheral_name(const char *port_name, int pin, uint32_t af) {
    if (af == 0) return "GPIO";

    char port = port_name[3]; // Extract port letter (A, B, C, etc.)

    // Handle the most common cases with specific pin mapping
    if (af == 7) { // USART1/2/3
        if (port == 'A') {
            if (pin == 2 || pin == 3) return "USART2";
            if (pin == 9 || pin == 10) return "USART1";
        }
        if (port == 'B') {
            if (pin == 10 || pin == 11) return "USART3";
        }
        if (port == 'C') {
            if (pin == 10 || pin == 11) return "USART3";
        }
        return "USART";
    }

    if (af == 8) { // UART4/5
        if (port == 'A') {
            if (pin == 0 || pin == 1) return "UART4";
        }
        if (port == 'C') {
            if (pin == 10 || pin == 11) return "UART4";
        }
        if (port == 'B') {
            if (pin == 12 || pin == 13) return "UART5";
        }
        if (port == 'C') {
            if (pin == 12 || pin == 13) return "UART5";
        }
        return "UART4/5";
    }

    // Generic fallback for other alternate functions
    switch (af) {
        case 1: return "TIM2/TIM5";
        case 2: return "TIM3/TIM4";
        case 3: return "TIM9/TIM10";
        case 4: return "I2C";
        case 5: return "SPI1";
        case 6: return "SPI2/SPI3";
        case 9: return "CAN1";
        case 10: return "USB";
        case 11: return "SDIO";
        case 12: return "FSMC";
        case 13: return "DCMI";
        case 14: return "EVENTOUT";
        case 15: return "ANALOG";
        default: return "Unknown";
    }
}

void print_uart_status_cmd(char *args) {
    /*
    === USART2 Status ===
    Configuration:
      Baud Rate:    115200
      Data Bits:    8
      Stop Bits:    1
      Parity:       None
      Mode:         TX + RX
      Flow Control: None

    Pins:
      TX: PA2 (AF7)
      RX: PA3 (AF7)

    Status:
      USART:        Enabled
      TX Buffer:    Empty (ready to send)
      RX Buffer:    Empty (no data)
      Errors:       None

    Statistics:
      Overrun Errors:   0
      Framing Errors:   0
      Parity Errors:    0
    */

    // TODO: Implementation - High-level UART status
}

void showreg_uart2() {
    /*
    === USART2 Raw Registers ===
    SR:   0x000000C0  (00000000000000000000000011000000)
    DR:   0x00000000  (00000000000000000000000000000000)
    BRR:  0x00000683  (00000000000000000000011010000011)
    CR1:  0x0000200C  (00000000000000000010000000001100)
    CR2:  0x00000000  (00000000000000000000000000000000)
    CR3:  0x00000000  (00000000000000000000000000000000)
    GTPR: 0x00000000  (00000000000000000000000000000000)
    */

    // TODO: Implementation - Raw register dump
}

void showreg_gpio(GPIO_TypeDef *GPIOx) {
    /*
    === GPIOA Raw Registers ===
    MODER:   0xA80004A0  (10101000000000000000010010100000)
    OTYPER:  0x00000000  (00000000000000000000000000000000)
    OSPEEDR: 0x0C000000  (00001100000000000000000000000000)
    PUPDR:   0x64000000  (01100100000000000000000000000000)
    IDR:     0x0000A00C  (00000000000000001010000000001100)
    ODR:     0x00000000  (00000000000000000000000000000000)
    BSRR:    0x00000000  (00000000000000000000000000000000)
    LCKR:    0x00000000  (00000000000000000000000000000000)
    AFR[0]:  0x00007700  (00000000000000000111011100000000)
    AFR[1]:  0x00000000  (00000000000000000000000000000000)
    */

    // TODO: Implementation - Raw register dump with binary representation
    print_shell("MODER: %H  (%s)\r\n", GPIOx->MODER);
}

void print_rcc_status_cmd() {
    /*
    === Clock Configuration Status ===
    Clock Sources:
      HSI:  Ready
      HSE:  Ready
      PLL:  Ready, Active

    System Clocks:
      SYSCLK:  84 MHz  (Source: PLL)
      HCLK:    84 MHz  (AHB Prescaler: /1)
      PCLK1:   42 MHz  (APB1 Prescaler: /2)
      PCLK2:   84 MHz  (APB2 Prescaler: /1)

    PLL Configuration:
      Source:  HSE (8 MHz)
      M:       8   (VCO Input: 1 MHz)
      N:       336 (VCO Output: 336 MHz)
      P:       2   (Main PLL: 168 MHz)
      Q:       7   (USB/SDIO: 48 MHz)

    Enabled Peripherals:
      AHB1: GPIOA, GPIOC, DMA2
      APB1: USART2, TIM2
      APB2: SYSCFG
    */

    // TODO: Implementation - High-level clock status
}

void showreg_rcc() {
    /*
    === RCC Raw Registers ===
    CR:       0x03010083  (00000011000000010000000010000011)
    PLLCFGR:  0x24403010  (00100100010000000011000000010000)
    CFGR:     0x00001402  (00000000000000000001010000000010)
    CIR:      0x00000000  (00000000000000000000000000000000)
    AHB1RSTR: 0x00000000  (00000000000000000000000000000000)
    AHB2RSTR: 0x00000000  (00000000000000000000000000000000)
    APB1RSTR: 0x00000000  (00000000000000000000000000000000)
    APB2RSTR: 0x00000000  (00000000000000000000000000000000)
    AHB1ENR:  0x00100005  (00000000000100000000000000000101)
    AHB2ENR:  0x00000000  (00000000000000000000000000000000)
    APB1ENR:  0x00001100  (00000000000000000001000100000000)
    APB2ENR:  0x00004000  (00000000000000000100000000000000)
    */

    // TODO: Implementation - Raw register dump
}

void print_timer_status_cmd(char *args) {
    /*
    === TIM1 Status ===
    Configuration:
      Status:       Disabled
      Mode:         PWM Mode 1
      Direction:    Up-counting
      Clock Source: Internal (84 MHz)

    Timing:
      Prescaler:    0 (Clock: 84 MHz)
      Period (ARR): 65535
      Frequency:    1.28 kHz

    Channels:
      CH1: Disabled, CCR=0
      CH2: Disabled, CCR=0
      CH3: Disabled, CCR=0
      CH4: Disabled, CCR=0

    Current State:
      Counter:      0
      Interrupt:    Update flag set
    */

    // TODO: Implementation - High-level timer status
}

void showreg_timer1() {
    /*
    === TIM1 Raw Registers ===
    CR1:   0x00000000  (00000000000000000000000000000000)
    CR2:   0x00000000  (00000000000000000000000000000000)
    SMCR:  0x00000000  (00000000000000000000000000000000)
    DIER:  0x00000000  (00000000000000000000000000000000)
    SR:    0x00000001  (00000000000000000000000000000001)
    EGR:   0x00000000  (00000000000000000000000000000000)
    CCMR1: 0x00000000  (00000000000000000000000000000000)
    CCMR2: 0x00000000  (00000000000000000000000000000000)
    CCER:  0x00000000  (00000000000000000000000000000000)
    CNT:   0x00000000  (00000000000000000000000000000000)
    PSC:   0x00000000  (00000000000000000000000000000000)
    ARR:   0x0000FFFF  (00000000000000001111111111111111)
    CCR1:  0x00000000  (00000000000000000000000000000000)
    CCR2:  0x00000000  (00000000000000000000000000000000)
    CCR3:  0x00000000  (00000000000000000000000000000000)
    CCR4:  0x00000000  (00000000000000000000000000000000)
    */

    // TODO: Implementation - Raw register dump
}