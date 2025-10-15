# STM32F401 Interactive Shell

A comprehensive interactive shell for STM32F401 microcontrollers, providing debugging, monitoring, and control capabilities through a UART interface.

## Features

### **Interactive Shell Commands**
- **`help`** - Show available commands
- **`sysinfo`** - Display system information (clock frequencies, HAL version, uptime)
- **`echo <text>`** - Echo text back to console
- **`led <on|off|toggle>`** - Control onboard LED
- **`status <peripheral>`** - Show peripheral status
- **`showreg <peripheral>`** - Display raw register values
- **`clear`** - Clear screen

### **Supported Peripherals**
- **UART**: USART1, USART2
- **GPIO**: GPIOA, GPIOB, GPIOC, GPIOD
- **System**: RCC, TIMER1

### ** Features**
- **Command History** - Arrow key navigation through previous commands
- **Ctrl+C Support** - Interrupt current command
- **Real-time UART Interrupts** - Non-blocking character reception

## Project Structure

```
Core/
├── Inc/
│   ├── main.h              # Main project definitions
│   ├── shell.h             # Shell function prototypes
│   ├── uart_driver.h       # UART driver interface
│   └── gpio_driver.h       # GPIO driver interface
└── Src/
    ├── main.c              # Application logic
    ├── shell.c             # Shell implementation
    ├── uart_driver.c       # UART operations
    ├── gpio_driver.c       # GPIO operations
    └── stm32f4xx_it.c      # Interrupt service routines
```

## Hardware Requirements

- **MCU**: STM32F401xE (ARM Cortex-M4)
- **UART**: USART2 (PA2-TX, PA3-RX) at 115200 baud
- **LED**: Onboard LED on PA5
- **Button**: User button on PC13

## 📋 Commands Reference

### **System Information**
```bash
STM32> sysinfo
===============================================
  System Information
===============================================
  MCU:              STM32F401xE
  Core:             ARM Cortex-M4
  HAL Version:      0x00000000
  SYSCLK:           84000000 Hz (84 MHz)
  HCLK (AHB):       84000000 Hz (84 MHz)
  PCLK1 (APB1):     42000000 Hz (42 MHz)
  PCLK2 (APB2):     84000000 Hz (84 MHz)
===============================================
```

### **Peripheral Status**
NOTICE: Currently under development ...
```bash
STM32> showreg gpioa
STM32> status uart2
```

### **Register Dumps**
```bash
STM32> showreg gpioa
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

STM32> showreg uart2
SR:   0x000000D0  (00000000000000000000000011010000)
DR:   0x0000000A  (00000000000000000000000000001010)
BRR:  0x0000016C  (00000000000000000000000101101100)
CR1:  0x0000202C  (00000000000000000010000000101100)
CR2:  0x00000000  (00000000000000000000000000000000)
CR3:  0x00000001  (00000000000000000000000000000001)
GTPR: 0x00000000  (00000000000000000000000000000000)

STM32> showreg rcc
CR:       0x03005783  (00000011000000000101011110000011)
PLLCFGR:  0x07015410  (00000111000000010101010000010000)
CFGR:     0x0000100A  (00000000000000000001000000001010)
CIR:      0x00000000  (00000000000000000000000000000000)
AHB1RSTR: 0x00000000  (00000000000000000000000000000000)
AHB2RSTR: 0x00000000  (00000000000000000000000000000000)
APB1RSTR: 0x00000000  (00000000000000000000000000000000)
APB2RSTR: 0x00000000  (00000000000000000000000000000000)
AHB1ENR:  0x00000087  (00000000000000000000000010000111)
AHB2ENR:  0x00000000  (00000000000000000000000000000000)
APB1ENR:  0x10020000  (00010000000000100000000000000000)
APB2ENR:  0x00004000  (00000000000000000100000000000000)

```

### **LED Control**
```bash
STM32> led on
STM32> led off
STM32> led toggle
```

## Build Instructions

### **Prerequisites**
- STM32CubeIDE or compatible IDE
- STM32CubeMX (for configuration)
- ARM GCC toolchain

### **Build Process**
```bash
git clone <repository-url>
cd Stm32-shell
cmake -B cmake-build-debug -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build-debug --target Stm32-shell -j 10
```

### **Flash to Device**
```bash
# Using ST-Link (if st-link tools installed)
st-flash write cmake-build-debug/Stm32-shell.elf 0x08000000

# Or use STM32CubeIDE to flash
# 1. Open project in STM32CubeIDE
# 2. Right-click project → Run As → STM32 C/C++ Application
```

## UART Configuration

- **Baud Rate**: 115200
- **Data Bits**: 8
- **Stop Bits**: 1
- **Parity**: None
- **Flow Control**: None

### **Pin Configuration**
- **TX**: PA2 (Alternate Function 7)
- **RX**: PA3 (Alternate Function 7)

## Architecture

The project follows a clean modular architecture:

- **`main.c`** - Application entry point and main loop
- **`shell.c`** - Interactive shell implementation
- **`uart_driver.c`** - UART communication driver
- **`gpio_driver.c`** - GPIO control driver
- **`stm32f4xx_it.c`** - Interrupt service routines

### **Key Components**

#### **Shell Engine**
- Command parsing and execution
- Input buffer management with ring buffer
- Command history with arrow key navigation
- Escape sequence handling for terminal control

#### **UART Driver**
- Non-blocking interrupt-driven communication
- Automatic error recovery
- Configurable baud rates and settings

#### **GPIO Driver**
- LED control functions
- Button input handling
- Peripheral status monitoring

## Debugging Features

### **Real-time Monitoring**
- System clock frequencies
- Peripheral register states
- GPIO pin configurations
- UART communication status

### **Interactive Commands**
- Live register inspection
- GPIO state monitoring
- System information display
- LED control and testing

## Memory Usage

- **RAM**: ~2.1 KB (2.12% of 96KB)
- **FLASH**: ~13 KB (2.48% of 512KB)
```bash
arm-none-eabi-size cmake-build-debug/Stm32-shell.elf
   text    data     bss     dec     hex filename
  22948     104    3640   26692    6844 cmake-build-debug/Stm32-shell.elf
```

## Interrupt Handling

### **Critical Interrupts**
- **SysTick_Handler** - Essential for HAL timing functions (CubeMX generated)
- **USART2_IRQHandler** - UART receive/transmit interrupts
- **EXTI15_10_IRQHandler** - Button press interrupts

### **Callback Functions**
- **HAL_UART_RxCpltCallback** - UART receive completion
- **HAL_UART_ErrorCallback** - UART error handling
- **HAL_GPIO_EXTI_Callback** - GPIO interrupt handling

## Troubleshooting

### **Common Issues**

1. **Shell not responding**
   - Check UART baud rate (115200)
   - Verify TX/RX pin connections
   - Ensure UART interrupts are enabled

2. **Commands not working**
   - Check if `shell_init()` is called in main()
   - Verify UART receive interrupt is started
   - Ensure `process_input()` is called in main loop

3. **Build errors**
   - Verify all HAL drivers are included in CMake
   - Check that UART HAL driver is linked
   - Ensure all required headers are included

### **Debug Tips**
- Check `status uart2` for UART configuration
- Monitor `showreg uart2` for register states



## Future Enhancements

- [ ] Add RTOS tasks for receiving and transmiting to the terminal
- [ ] Add more peripheral support (SPI, I2C, ADC)
- [ ] Implement file system commands
- [ ] Add network connectivity support
- [ ] Add logging and data logging capabilities

## Contributing

This is a learning project demonstrating embedded systems development with STM32 microcontrollers. Contributions and improvements are welcome!

