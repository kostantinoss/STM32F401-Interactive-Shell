#ifndef UART_DRIVER_H
#define UART_DRIVER_H

#include "main.h"
#include <stdarg.h>

void UART_Init(void);
void UART_Print(const char *format, ...);
UART_HandleTypeDef* UART_GetHandle(void);
extern uint8_t chrx;
extern UART_HandleTypeDef huart2;

#endif