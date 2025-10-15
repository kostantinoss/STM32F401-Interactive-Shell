#include "gpio_driver.h"
#include "main.h"
#include "shell.h"


void GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct_led = {0};
    GPIO_InitTypeDef GPIO_InitStruct_btn = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // /*Configure GPIO pin Output Level */
    // HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
    //
    // /*Configure GPIO pin : B1_Pin */
    // GPIO_InitStruct.Pin = B1_Pin;
    // GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    // GPIO_InitStruct.Pull = GPIO_NOPULL;
    // HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);
    //
    // /*Configure GPIO pin : LD2_Pin */
    // GPIO_InitStruct.Pin = LD2_Pin;
    // GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    // GPIO_InitStruct.Pull = GPIO_NOPULL;
    // GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    // HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct_led.Pin = LD2_Pin;
    GPIO_InitStruct_led.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct_led.Pull = GPIO_NOPULL;
    GPIO_InitStruct_led.Speed = GPIO_SPEED_FREQ_LOW;

    GPIO_InitStruct_btn.Pin = B1_Pin;
    GPIO_InitStruct_btn.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct_btn.Pull = GPIO_NOPULL;

    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct_led);
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct_btn);

    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 1, 1);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);



}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    // Blue button pressed - clear screen and reinitialize shell
    clear_cmd();
    shell_init();
}