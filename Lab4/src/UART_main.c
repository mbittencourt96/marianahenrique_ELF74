#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"

uint32_t SysClock;

// Envia string para a uart
void UARTSend(const uint8_t *Buffer, uint32_t Len)
{
    while(Len--)
    {
        UARTCharPutNonBlocking(UART0_BASE, *Buffer++);
    }
}

void UARTIntHandler(void)
{
    uint32_t Status;
    volatile int32_t caract;

    Status = UARTIntStatus(UART0_BASE, true); // Retorna o status da interrupção    
    UARTIntClear(UART0_BASE, Status);         // Limpa Flag de interrupção
    
    //Verifica se tem algum dado na fifo
    while(UARTCharsAvail(UART0_BASE))
    {
        // Algoritmo de conversao
        caract = UARTCharGetNonBlocking(UART0_BASE);       
        UARTCharPutNonBlocking(UART0_BASE, caract); // Escreve na UART
        // Feedback visual de que um dado foi recebido na uart
        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0); 
        SysCtlDelay(SysClock / (1000 * 3));
        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0);
    }
    //zera a variável
    caract = 0;
}

void setup()
{
    SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_240), 120000000);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    IntMasterEnable();
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTConfigSetExpClk(UART0_BASE, SysClock, 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    IntEnable(INT_UART0);
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
}

int main(void)
{
    setup();
    UARTSend((uint8_t *)"\033[H\033[JEnter text: ", 20);
    while(1)
    {
    }
}
