#include "uart_driver.h"
#include "main.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include "shell.h"

uint8_t chrx;
static UART_HandleTypeDef huart2;

UART_HandleTypeDef* UART_GetHandle(void) {
    return &huart2;
}

void UART_Init(void) {
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;

    if (HAL_UART_Init(&huart2) != HAL_OK) {
        Error_Handler();
    }

    HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
}

void UART_Print(const char *format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 256, format, args);
    va_end(args);
    
    HAL_UART_Transmit(&huart2, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    buffer_putc(&rx_buffer, chrx);
    HAL_UART_Receive_IT(UART_GetHandle(), &chrx, 1);
}