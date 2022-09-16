#include <iostream>
using std::cout;
using std::cin;
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/timer.h"
#include <cmath>

volatile int contadorSysTick = 0;
int FlagVenceuJogo = 0;
int FlagInicioJogo = 0;
volatile int clocksDecorridos;
float tempoSegundos;
uint32_t g_ui32SysClock;

void fimJogo(void)
{
  GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0x0);  //Led OFF
  cout << "Fim de jogo! Passaram-se 3 segundos!\n";
  SysTickDisable();
}

extern "C" void contadorTempo(void)
{
  contadorSysTick = contadorSysTick + 1;
}

void inicioJogo(void)
{

  SysTickDisable();
  contadorSysTick = 0;
  GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);  //Led ON
  
  FlagInicioJogo = 1;
  
  SysTickPeriodSet(16000000);   //Set the period of SysTick timer
  SysTickEnable();
}

extern "C" void venceuJogo(void)
{
  GPIOIntClear(GPIO_PORTJ_BASE, GPIO_PIN_0);
  SysTickDisable();
  GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0x0);  //Led OFF
  FlagVenceuJogo = 1;
}

int main(void)
{
  
  g_ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                             SYSCTL_OSC_MAIN |
                                             SYSCTL_USE_PLL |
                                             SYSCTL_CFG_VCO_240), 40000000); 
  
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);  //Enable GPIO for embedded LEDs
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);  //Enable GPIO for switches

    //
    // Check if the peripheral access is enabled.
    //
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION & SYSCTL_PERIPH_GPIOJ))
    {
    }
   
    //
    // Enable the GPIO pin for the LED (PN0).  Set the direction as output, and
    // enable the GPIO pin for digital function.
    //
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);
    
    // Enable the GPIO pin for the switch (SW1).  Set the direction as input, and
    // enable the GPIO pin for digital function.
    GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0);
   
    GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0x0);  //Led OFF
   
    IntMasterEnable();
    GPIOIntTypeSet(GPIO_PORTJ_BASE, GPIO_PIN_0, GPIO_FALLING_EDGE);
    GPIOPadConfigSet(GPIO_PORTJ_BASE, GPIO_PIN_0, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
    IntEnable(INT_GPIOJ);
    GPIOIntDisable(GPIO_PORTJ_BASE, GPIO_INT_PIN_1);
    GPIOIntEnable(GPIO_PORTJ_BASE, GPIO_INT_PIN_0);
   
    SysTickIntEnable();
    
    SysTickPeriodSet(16000000);   //Sets the period of SysTick timer
    SysTickEnable();
    
    while (contadorSysTick <= 2)
    {
    }
    
    inicioJogo();
    
  while (!FlagInicioJogo)
  {
  }
 
  while(1)
    {
      if (contadorSysTick >= 8)       //3 seconds have passed
      {
        fimJogo();   
        contadorSysTick = 0;
        
        //Restart the game
        
        SysTickPeriodSet(16000000);   //Set the period of SysTick timer
        SysTickEnable();
        
        while (contadorSysTick < 2)
          {
            
          }
    
        inicioJogo();

      }
     
      if (FlagVenceuJogo == 1)
      {
        clocksDecorridos = contadorSysTick * 16;
        FlagVenceuJogo = 0;
        tempoSegundos = (clocksDecorridos/40.0);
        printf("Venceu o jogo! Ciclos de clock decorridos:%d000000\nTempo decorrido:%.2f\n",clocksDecorridos,tempoSegundos);       
        contadorSysTick = 0;
        SysTickPeriodSet(16000000);   //Set the period of SysTick timer
        SysTickEnable();
        
        while (contadorSysTick < 2)
          {
            
          }
    
        inicioJogo();

      }
  }
}
    