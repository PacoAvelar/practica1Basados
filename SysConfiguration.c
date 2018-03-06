/*
 * SysConfiguration.c
 *
 *  Created on: Mar 5, 2018
 *      Author:
 */

#include "SysConfiguration.h"
#include "FreeRTOS.h"
#include "task.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#include "fsl_i2c.h"
#include "I2C.h"

//////////////////**user types definitions*////////////////////////////
typedef enum {
    PTA1 = 1, PTA2
} PORTA_buttons_pins_t;

typedef enum {
    PTB9 = 9, PTB23 = 23
} PORTB_buttons_pins_t;

typedef enum {
    PTC2 = 2, PTC3, PTC16 = 16, PTC17
} PORTC_buttons_pins_t;

typedef enum {
    PTB3_RX = 3, PTB4_TX
} PORTC_UART1_pins_t;

typedef enum {
    PTD0_RST, PTD1_CLK, PTD2_DIN, PTD3_DC
} PORTD_SPI_pins_t;

typedef enum {
    PTE24_SCL = 24, PTE25_SDA
} PORTE_I2C0_pins_t;

//////////////////**function prototypes*////////////////////////////
void SYSconfig_ButtonsConfiguration();
void SYSconfig_SPIConfiguration();
void SYSconfig_UARTConfiguration();
void SYSconfig_I2CConfiguration();
void i2c_ReleaseBus();

//////////////////**mechanisms definitions*/////////////////////////
void SystemConfiguration(void* args)
{
    /**ports A,B,C clock will be enabled, as pins from all those will be used*/
    CLOCK_EnableClock(kCLOCK_PortA);
    CLOCK_EnableClock(kCLOCK_PortB);
    CLOCK_EnableClock(kCLOCK_PortC);
    /**modules configuration*/
    SYSconfig_ButtonsConfiguration(); /**buttons configuration*/
    SYSconfig_SPIConfiguration(); /**SPI module configuration (including device initialization)*/
    SYSconfig_UARTConfiguration(); /**UART module configuration*/
    SYSconfig_I2CConfiguration(); /**I2C module configuration*/

    vTaskSuspend(NULL); /**the function auto suspends itself, as it won't be used again*/
}

void SYSconfig_ButtonsConfiguration()
{ /**this function responsibility is to configure everything related to the buttons which will be used*/
    /**button configuration definition*/
    port_pin_config_t button_configuration = { kPORT_PullDisable,
        kPORT_SlowSlewRate, kPORT_PassiveFilterDisable, kPORT_OpenDrainDisable,
        kPORT_LowDriveStrength, kPORT_MuxAsGpio, kPORT_UnlockRegister };
    /**button pins configuration*/
    PORT_SetPinConfig(PORTC, PTC3, &button_configuration); /**B0 configuration*/
    PORT_SetPinConfig(PORTC, PTC2, &button_configuration); /**B1 configuration*/
    PORT_SetPinConfig(PORTA, PTA2, &button_configuration); /**B2 configuration*/
    PORT_SetPinConfig(PORTB, PTB23, &button_configuration);/**B3 configuration*/
    PORT_SetPinConfig(PORTA, PTA1, &button_configuration); /**B4 configuration*/
    PORT_SetPinConfig(PORTB, PTB9, &button_configuration); /**B5 configuration*/
    /**interrupts configuration*/
    PORT_SetPinInterruptConfig(PORTC, PTC3, kPORT_InterruptLogicOne); /**B0 configured to interrupt when a logic 1 is read*/
    PORT_SetPinInterruptConfig(PORTC, PTC2, kPORT_InterruptLogicOne); /**B1 configured to interrupt when a logic 1 is read*/
    PORT_SetPinInterruptConfig(PORTA, PTA2, kPORT_InterruptLogicOne); /**B2 configured to interrupt when a logic 1 is read*/
    PORT_SetPinInterruptConfig(PORTB, PTB23, kPORT_InterruptLogicOne); /**B3 configured to interrupt when a logic 1 is read*/
    PORT_SetPinInterruptConfig(PORTA, PTA1, kPORT_InterruptLogicOne); /**B4 configured to interrupt when a logic 1 is read*/
    PORT_SetPinInterruptConfig(PORTB, PTB9, kPORT_InterruptLogicOne); /**B5 configured to interrupt when a logic 1 is read*/
}

void SYSconfig_SPIConfiguration()
{

}

void SYSconfig_UARTConfiguration()
{

}

void SYSconfig_I2CConfiguration()
{
    i2c_ReleaseBus(); /**bug fixing function provided by NXP*/
    /**I2C_0 and PORTE clock enabling, as those are the modules required for the I2C operation*/
    CLOCK_EnableClock(kCLOCK_I2c0);
    CLOCK_EnableClock(kCLOCK_PortE);
    /**I2C module configuration*/
    port_pin_config_t config_i2c = { kPORT_PullDisable, kPORT_SlowSlewRate,
        kPORT_PassiveFilterDisable, kPORT_OpenDrainDisable,
        kPORT_LowDriveStrength, kPORT_MuxAlt5, kPORT_UnlockRegister };
    PORT_SetPinConfig(PORTE, PTE24_SCL, &config_i2c); /**I2C_0 SCL configured*/
    PORT_SetPinConfig(PORTE, PTE25_SDA, &config_i2c); /**I2C_0 SDA configured*/
    /**I2C_0 master configuration and initialization*/
    i2c_master_config_t masterConfig;
    I2C_MasterGetDefaultConfig(&masterConfig);
    I2C_MasterInit(I2C0, &masterConfig, CLOCK_GetFreq(kCLOCK_BusClk));
    /**I2C_0 master handle creation*/
    i2c_master_handle_t g_m_handle;
    I2C_MasterTransferCreateHandle(I2C0, &g_m_handle, i2c_master_callback,
                                   NULL);
}

/////////////**Bug fixing functions provided by NXP*///////////////
static void i2c_release_bus_delay(void)
{
    uint32_t i = 0;
    for (i = 0; i < 100; i++)
    {
        __NOP();
    }
}
void i2c_ReleaseBus()
{
    uint8_t i = 0;
    gpio_pin_config_t pin_config;
    port_pin_config_t i2c_pin_config = { 0 };

    /* Config pin mux as gpio */
    i2c_pin_config.pullSelect = kPORT_PullUp;
    i2c_pin_config.mux = kPORT_MuxAsGpio;

    pin_config.pinDirection = kGPIO_DigitalOutput;
    pin_config.outputLogic = 1U;
    CLOCK_EnableClock(kCLOCK_PortE);
    PORT_SetPinConfig(PORTE, 24, &i2c_pin_config);
    PORT_SetPinConfig(PORTE, 25, &i2c_pin_config);

    GPIO_PinInit(GPIOE, 24, &pin_config);
    GPIO_PinInit(GPIOE, 25, &pin_config);

    GPIO_PinWrite(GPIOE, 25, 0U);
    i2c_release_bus_delay();

    for (i = 0; i < 9; i++)
    {
        GPIO_PinWrite(GPIOE, 24, 0U);
        i2c_release_bus_delay();

        GPIO_PinWrite(GPIOE, 25, 1U);
        i2c_release_bus_delay();

        GPIO_PinWrite(GPIOE, 24, 1U);
        i2c_release_bus_delay();
        i2c_release_bus_delay();
    }

    GPIO_PinWrite(GPIOE, 24, 0U);
    i2c_release_bus_delay();

    GPIO_PinWrite(GPIOE, 25, 0U);
    i2c_release_bus_delay();

    GPIO_PinWrite(GPIOE, 24, 1U);
    i2c_release_bus_delay();

    GPIO_PinWrite(GPIOE, 25, 1U);
    i2c_release_bus_delay();
}