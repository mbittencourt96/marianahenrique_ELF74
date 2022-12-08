#include <stdint.h>
#include "system_TM4C1294.h" // CMSIS-Core
#include "cmsis_os2.h" // CMSIS-RTOS
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/uart.h"
#include <stdio.h>

/*define useful symbolic names for LCD commands */
#define clear_display     0x01
#define returnHome        0x02
#define moveCursorRight   0x06
#define moveCursorLeft    0x08
#define shiftDisplayRight 0x1C
#define shiftDisplayLeft  0x18
#define cursorBlink       0x0F
#define cursorOff         0x0C
#define cursorOn          0x0E
#define Function_set_4bit 0x28
#define Function_set_8bit 0x38
#define Entry_mode        0x06
#define Function_8_bit    0x32
#define Set5x7FontSize    0x20
#define FirstRow          0x80
#define SecondRow         0xC0

void PortK_Output(uint32_t value)
{
  GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3
                          |GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, value);
}

void PortM_Output_LCD(uint32_t value)
{
  GPIOPinWrite(GPIO_PORTM_BASE,GPIO_PIN_0 | GPIO_PIN_1, value);
}

void initGPIO()
{
  uint32_t g_ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                             SYSCTL_OSC_MAIN |
                                             SYSCTL_USE_PLL |
                                             SYSCTL_CFG_VCO_240), 120000000); 
  
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);  
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);

    //
    // Check if the peripheral access is enabled.
    //
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK & SYSCTL_PERIPH_GPIOM))
    {
    }
    
    GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3
                          |GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
    
    GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_0 | GPIO_PIN_1);  
}

void init_LCD()
{
        initGPIO();
        
	PortK_Output(Function_set_8bit);
	PortM_Output_LCD(0x04);
	osDelayUntil(1);
	PortM_Output_LCD(0x00);
	osDelayUntil(4);

	PortK_Output(moveCursorRight);
	PortM_Output_LCD(0x04);
	osDelayUntil(1);
	PortM_Output_LCD(0x00);
	osDelayUntil(4);

	PortK_Output(cursorOn);
	PortM_Output_LCD(0x04);
	osDelayUntil(1);
	PortM_Output_LCD(0x00);
	osDelayUntil(4);
	
	PortK_Output(clear_display);
	PortM_Output_LCD(0x04);
	osDelayUntil(1);
	PortM_Output_LCD(0x00);
	osDelayUntil(4);
}

void escreve_caracter_LCD(uint32_t c)
{
	PortK_Output(c);
	PortM_Output_LCD(0x05);
	osDelayUntil(1);
	PortM_Output_LCD(0x00);
	osDelayUntil(4);
}


void escreve_texto_LCD (char* texto)
{
	while (*texto != '\0')
	{
		escreve_caracter_LCD(*texto);
		texto++;
	}
}

void seleciona_primeira_linha_LCD(void)
{
	PortK_Output(FirstRow);
	PortM_Output_LCD(0x04);
	osDelayUntil(1);
	PortM_Output_LCD(0x00);
	osDelayUntil(4);
}

void clear_LCD(void)
{
	PortK_Output(clear_display);
	PortM_Output_LCD(0x04);
	osDelayUntil(1);
	PortM_Output_LCD(0x00);
	osDelayUntil(4);
}

void seleciona_segunda_linha_LCD(void)
{
	PortK_Output(SecondRow);
	PortM_Output_LCD(0x04);
	osDelayUntil(1);
	PortM_Output_LCD(0x00);
	osDelayUntil(4);
}

