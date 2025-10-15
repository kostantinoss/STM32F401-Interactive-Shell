#ifndef SHELL_H
#define SHELL_H

#include "main.h"
#include <stdint.h>
#include <stddef.h>

// Constants
#define CMD_BUFFER_SIZE 124
#define PRINT_BUFFER_SIZE 256
#define RX_BUFFER_SIZE 256
#define CMD_HISTORY_SIZE 10

// Data structures
typedef struct {
    uint8_t buffer[RX_BUFFER_SIZE];
    int head;
    int tail;
    int count;
} RingBuffer_t;

typedef struct {
    char commands[CMD_HISTORY_SIZE][CMD_BUFFER_SIZE];
    int current_index;
    int display_index;
    int history_count;
} CommandHistory_t;

// Global variables (extern declarations)
extern char cmd_buffer[CMD_BUFFER_SIZE];
extern int cursor_pos;
extern RingBuffer_t rx_buffer;
extern CommandHistory_t cmd_history_buffer;

// Core shell functions
void buffer_putc(RingBuffer_t *rb, uint8_t c);
uint8_t buffer_getc(RingBuffer_t *rb);
void save_cmd_to_history(CommandHistory_t *history, char *cmd);
void show_previous_cmd(CommandHistory_t *history);
void show_next_cmd(CommandHistory_t *history);
void print_shell(const char *format, ...);
void uint32_to_binary_string(uint32_t num, char *buffer, size_t buffer_size);
void shell_prompt(void);
void shell_init(void);

// Input processing functions
void process_char(const uint8_t c);
void process_input(void);
void process_command(char *command);

// Command implementations
void print_help_msg(void);
void print_sys_info_msg(void);
void clear_cmd(void);
void echo_cmd(char *cmd);

// GPIO status functions
void print_gpio_status_cmd(GPIO_TypeDef *GPIOx, const char *port_name);
const char* get_peripheral_name(const char *port_name, int pin, uint32_t af);

// UART status functions
void print_uart_status_cmd(char *args);
void showreg_uart2(void);

// RCC status functions
void print_rcc_status_cmd(void);
void showreg_rcc(void);

// Timer status functions
void print_timer_status_cmd(char *args);
void showreg_timer1(void);

// GPIO register functions
void showreg_gpio(GPIO_TypeDef *GPIOx);

#endif /* SHELL_H */