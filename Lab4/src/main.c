#include <stdint.h>
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
#include "driverlib/pwm.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/uart.h"


//global variables

uint32_t SysClock;
volatile uint32_t interrupts_counter0 = 0;
volatile uint32_t interrupts_counter1 = 0;
volatile uint32_t interrupts_counter2 = 0;
volatile uint32_t interrupts_counter3 = 0;
volatile uint32_t n_cycles;
volatile char channel;
char mode;
char first_command;
char n_cycles_char[10];
char freq[10];
char duty_cycle[10];
char trigger;

volatile int flagStop = 0;
volatile int maskInput = 0;
volatile int triggerOn = 0;

char receivedCommand[30];
char *receivedCommandStr;
volatile int flagRecebido = 0;


//handlers

//Switches (SW1 and SW2) Handler

void switchesHandler()
{
  int32_t val;
  
  GPIOIntClear(GPIO_PORTJ_BASE,GPIO_INT_PIN_0 | GPIO_INT_PIN_1);
  
  val = GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1);
  
  if (val == 1)    //button sw1 pressed
  {
    maskInput = 1;
    
  }
  else if (val == 2) //button sw2 pressed
  {
    maskInput = 0;
  }
  
}


//UART Handler

extern void UART_Rx_Handler()
{
  uint32_t Status;
  char caract;

  Status = UARTIntStatus(UART0_BASE, true); // Retorna o status da interrupção   
  
  UARTIntClear(UART0_BASE, Status);         // Limpa Flag de interrupção
  
  if ((Status == UART_INT_RX) || (Status == UART_INT_RT))
  {
    int i = 0;
    
    while(caract != '\n')
      {
        // Algoritmo de conversao
        caract = (unsigned char) UARTCharGet(UART0_BASE);
        
        if (caract != '\n')
        {
          receivedCommand[i++] = caract;
        }      
      }
    
      receivedCommand[i] = '\0';
     
      flagRecebido = 1;
  }
  
}

//PWM Handlers (for burst and single modes)

extern void PWM_Int_Handler0()
{
              
  PWMGenIntClear(PWM0_BASE,PWM_GEN_0, PWM_INT_GEN_0);
                 
  if (interrupts_counter0 < n_cycles)
  {  
    interrupts_counter0++;
  }
  else
  {
    interrupts_counter0 = 0;
    flagStop = 1;
  }
}  

void PWM_Int_Handler1()
{
  
  PWMGenIntClear(PWM0_BASE,PWM_GEN_0, PWM_INT_GEN_0);
  
  if (interrupts_counter1 < n_cycles)
  {
    interrupts_counter1++;
  }
  else
  {  
    interrupts_counter1 = 0;
    flagStop = 1;
  }
}  

void PWM_Int_Handler2()
{
  
  PWMGenIntClear(PWM0_BASE,PWM_GEN_2, PWM_INT_GEN_2);
  
  if (interrupts_counter2 < n_cycles)
  {
   
    interrupts_counter2++;
  }
  else
  {
    interrupts_counter2 = 0;
    flagStop = 1;
  }
}  

void PWM_Int_Handler3()
{
  PWMGenIntClear(PWM0_BASE,PWM_GEN_2, PWM_INT_GEN_2);
  
   if (interrupts_counter3 < n_cycles)
  {
    interrupts_counter3++;
  }
  else
  {
    interrupts_counter3 = 0;
    flagStop = 1;
  }
}  


void UARTSend(const uint8_t *Buffer, uint32_t Len)
{
    while(Len--)
    {
        UARTCharPut(UART0_BASE, *Buffer++);
    }
}

void startPWM()
{
   if (channel == '1')
  {
    PWMGenEnable(PWM0_BASE, PWM_GEN_2); // Enable PWM
  }
  else if (channel == '2')
  {
    PWMGenEnable(PWM0_BASE, PWM_GEN_0); // Enable PWM
  }
  else
  {
    PWMGenEnable(PWM0_BASE, PWM_GEN_1);
  }
}

void startPWMByTrigger()
{
  GPIOIntClear(GPIO_PORTC_BASE, GPIO_PIN_6);
  
  if (channel == '1')
  {
    PWMGenEnable(PWM0_BASE, PWM_GEN_2); // Enable PWM
    PWMOutputState(PWM0_BASE, PWM_OUT_4_BIT, true); // Enable PWM output channel 0
  }
  else if (channel == '2')
  {
    PWMGenEnable(PWM0_BASE, PWM_GEN_0); // Enable PWM
    PWMOutputState(PWM0_BASE, PWM_OUT_1_BIT, true); // Enable PWM output channel 1
  }
  else if (channel == '3')
  {
    PWMGenEnable(PWM0_BASE, PWM_GEN_1);
    PWMOutputState(PWM0_BASE, PWM_OUT_2_BIT, true); // Enable PWM output channel 2
  }
  else
  {
    PWMGenEnable(PWM0_BASE, PWM_GEN_1);
    PWMOutputState(PWM0_BASE, PWM_OUT_3_BIT, true); // Enable PWM output channel 2
  }
}

void stopPWM()
{
  if (channel == '1')
  {
    PWMGenDisable(PWM0_BASE, PWM_GEN_2); // Disable PWM
    PWMOutputState(PWM0_BASE, PWM_OUT_4_BIT, false); // Disable PWM output channel 4
  }
  else if (channel == '2')
  {
    PWMGenDisable(PWM0_BASE, PWM_GEN_0); // Disable PWM
    PWMOutputState(PWM0_BASE, PWM_OUT_1_BIT, false); // Disable PWM output channel 1
  }
  else if (channel == '3')
  {
    PWMGenDisable(PWM0_BASE, PWM_GEN_1);
    PWMOutputState(PWM0_BASE, PWM_OUT_2_BIT, false); // Disable PWM output channel 2
  }
  else
  {
    PWMGenDisable(PWM0_BASE, PWM_GEN_1);
    PWMOutputState(PWM0_BASE, PWM_OUT_3_BIT, false); // Disable PWM output channel 3
  }
}

void interpretCommand(char *command)
{
 
  
  if (maskInput == 0)
  {
    
  if (command[1] == 'X')    //pwm single, continuous, or burst
  {
    first_command = 'X';
    
    channel = command[2];
    mode = command[4];
    char n_cycles_internal[10];
    
    int i;
    int m = 0;
    
    for (i = 6; command[i] != '*'; i++)
    {
      freq[m] = command[i];
      m++;
    }
    
    m = 0;
    
    i = i + 1;
    
    int j;
    
    for (j = i; command[j] != '*'; j++)
    {
      duty_cycle[m] = command[j];
      m++;
    }
    
    m = 0;
    
    j = j + 1;
    
    int k;
    
    for (k = j; command[k] != '*'; k++)
    {
      n_cycles_char[m] = command[k];
      m++;
    }
    
    k = k + 1;
    
    if (command[k] == 'Y')
    {
      trigger = true;
    }
    else
    {
      trigger = false;
    }
  }
  else if (command[1] == 'O')   //immediate stop
  {
    first_command = 'O';
    channel = command[2];
    stopPWM();
  }
  else if (command[1] == 'T')  //immediate start
  {
    first_command = 'T';
    channel = command[2];
    stopPWM();
  }
  else
  {
    first_command = 'Y';
    UARTSend("Comando inválido!", strlen("Comando inválido!"));
  }
 }
 else
 {
   first_command = 'Y';
   UARTSend("Entrada mascarada!", strlen("Entrada mascarada!"));
 }
}

//Setup functions

void setupLEDs()
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); // Enable Port F
  
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG); // Enable Port G
  
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF) && !SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOG))
    {
      
    }
 
   GPIOPinConfigure(GPIO_PG0_M0PWM4);
   GPIOPinConfigure(GPIO_PF1_M0PWM1);
   GPIOPinConfigure(GPIO_PF2_M0PWM2);
   GPIOPinConfigure(GPIO_PF3_M0PWM3);
   
   GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);//Configure PWM outputs (PF1,PF2,PF3) 
   
   GPIOPinTypePWM(GPIO_PORTG_BASE, GPIO_PIN_0);
   
   GPIOPinWrite(GPIO_PORTG_BASE, GPIO_PIN_0, 0x0); 
   GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x0);
   GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0x0);  
   GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0x0);  //Leds OFF
   
}
void setupUART()
{
    SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_240), 120000000);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0) && !SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA))
    {
    }                                                 //Wait until UART0 and PORT A is ready
    
    
    IntMasterEnable();
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTConfigSetExpClk(UART0_BASE, SysClock, 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    IntEnable(INT_UART0_TM4C129);
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
}

void setupSwitches()
{
   SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);  //Enable GPIO for switches
  
   while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ))
    {
      
    }  
   
   GPIOIntRegister(GPIO_PORTJ_BASE, switchesHandler);
   GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1);
   IntMasterEnable();
   GPIOIntTypeSet(GPIO_PORTJ_BASE, GPIO_PIN_0, GPIO_FALLING_EDGE);
   GPIOIntTypeSet(GPIO_PORTJ_BASE, GPIO_PIN_1, GPIO_FALLING_EDGE);
   GPIOPadConfigSet(GPIO_PORTJ_BASE, GPIO_PIN_0, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
   GPIOPadConfigSet(GPIO_PORTJ_BASE, GPIO_PIN_1, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
   IntEnable(INT_GPIOJ);
   GPIOIntEnable(GPIO_PORTJ_BASE, GPIO_INT_PIN_0 |GPIO_INT_PIN_1);
  
}

void setupTriggerInput()
{
   SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);  //Enable GPIO for button
  
   while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOC))
    {
      
    }  
   
   GPIOIntRegister(GPIO_PORTC_BASE, startPWMByTrigger);
   GPIOPinTypeGPIOInput(GPIO_PORTC_BASE, GPIO_PIN_6);
   GPIOIntTypeSet(GPIO_PORTC_BASE, GPIO_PIN_6, GPIO_FALLING_EDGE);
   GPIOPadConfigSet(GPIO_PORTC_BASE, GPIO_PIN_6, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
   IntEnable(INT_GPIOC_TM4C129);
   GPIOIntEnable(GPIO_PORTC_BASE, GPIO_INT_PIN_6);
   
   triggerOn = 1;
  
}

void deactivateTriggerInput()
{
  if (triggerOn)
  {
    GPIOIntDisable(GPIO_PORTC_BASE, GPIO_INT_PIN_6);
    triggerOn = 0;
  }
}

void setupPWM(char channel, char mode, int CPU_FREQ, int PWM_FREQ, int duty_cycle_local, int cycles, bool trigger)
{
    float PWM_WORD;
    
    PWM_WORD = (1/PWM_FREQ)*CPU_FREQ;
    
    SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                                     SYSCTL_OSC_MAIN |
                                                     SYSCTL_USE_PLL |
                                                     SYSCTL_CFG_VCO_480), CPU_FREQ);
    SysCtlPWMClockSet(SYSCTL_PWMDIV_1); // Enable clock to PWM module
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0); // Use PWM module 0
    
    switch(channel)
    {
      case '1':  //Config output PWM4
            
            PWMGenConfigure(PWM0_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
            PWMGenPeriodSet(PWM0_BASE, PWM_GEN_2, PWM_WORD); // Set PWM period
            PWMPulseWidthSet(PWM0_BASE, PWM_OUT_4, PWMGenPeriodGet(PWM0_BASE,PWM_GEN_2) * (duty_cycle_local) / 100); // Set Duty cycle 
            PWMOutputState(PWM0_BASE, PWM_OUT_4_BIT, true); // Enable PWM output channel 0
            
            if (mode == 'S' || mode == 'U')   //single or burst - define number of cycles (interrupt)
            {
              n_cycles = cycles;
              IntMasterEnable();
              PWMIntEnable(PWM0_BASE, PWM_INT_GEN_2);
              PWMGenIntTrigEnable(PWM0_BASE, PWM_INT_GEN_2, PWM_INT_CNT_ZERO);
              IntEnable(INT_PWM0_2_TM4C129);
              PWMGenIntRegister(PWM0_BASE, PWM_INT_GEN_2,PWM_Int_Handler0);                 
            }
           
        break;
        
      case '2':   //Config PWM1 output

        PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_DB_NO_SYNC);
        PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, PWM_WORD); // Set PWM frequency
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, PWMGenPeriodGet(PWM0_BASE,PWM_GEN_0) * (duty_cycle_local) / 100); // Set Duty cycle 
        PWMOutputState(PWM0_BASE, PWM_OUT_1_BIT, true); // Enable PWM output channel 1
        
        if (mode == 'S' || mode == 'U')   //single or burst - define number of cycles (interrupt)
        {
          n_cycles = cycles;
          PWMIntEnable(PWM0_BASE, PWM_INT_GEN_0);
          IntEnable(INT_PWM0_0);
          PWMGenIntTrigEnable(PWM0_BASE, PWM_INT_GEN_0, PWM_INT_CNT_AU);
          IntMasterEnable();
          PWMGenIntRegister(PWM0_BASE, PWM_INT_GEN_0,PWM_Int_Handler1);
        }
        
        break;
  
      case '3': //Config PWM2 output
  
        PWMGenConfigure(PWM0_BASE, PWM_GEN_1, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_DB_NO_SYNC);
        PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, PWM_WORD); // Set PWM frequency
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2, PWMGenPeriodGet(PWM0_BASE,PWM_GEN_1) * (duty_cycle_local) / 100); // Set Duty cycle 
        PWMOutputState(PWM0_BASE, PWM_OUT_2_BIT, true); // Enable PWM output channel 2
        
        if (mode == 'S' || mode == 'U')   //single or burst - define number of cycles (interrupt)
        {
          n_cycles = cycles;
          PWMIntEnable(PWM0_BASE, PWM_INT_GEN_1);
          IntEnable(INT_PWM0_1);
          PWMGenIntTrigEnable(PWM0_BASE, PWM_INT_GEN_1, PWM_INT_CNT_AU);
          IntMasterEnable();
          PWMGenIntRegister(PWM0_BASE, PWM_INT_GEN_1,PWM_Int_Handler2);
         }
        break;
        
      case '4': //Config PWM3 output
        
        PWMGenConfigure(PWM0_BASE, PWM_GEN_1, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_DB_NO_SYNC);
        PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, PWM_WORD); // Set PWM frequency
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_3, PWMGenPeriodGet(PWM0_BASE,PWM_GEN_1) * (duty_cycle_local) / 100); // Set Duty cycle 
        PWMOutputState(PWM0_BASE, PWM_OUT_3_BIT, true); // Enable PWM output channel 4
        
        
        if (mode == 'S' || mode == 'U')   //single or burst - define number of cycles (interrupt)
        {
          PWMIntEnable(PWM0_BASE, PWM_INT_GEN_1);
          IntEnable(INT_PWM0_1);
          PWMGenIntTrigEnable(PWM0_BASE, PWM_INT_GEN_1, PWM_INT_CNT_AU);
          IntMasterEnable();
          PWMGenIntRegister(PWM0_BASE, PWM_INT_GEN_1,PWM_Int_Handler3);
        }
        
        break;
    }
    
    if (trigger == true)
     {
       setupTriggerInput();
     }
    else
    {
      deactivateTriggerInput();
    }
        
}

void main()
{
  setupUART(); 
  setupLEDs();
  setupSwitches();
  
  int frequency;
  int num_cycles;
  int dutyCycle;
  
  while(1)
  {
    if (flagRecebido)
    {
      
      int j;
      
      for (j = 0; n_cycles_char[j] != '\0'; j++)
        {
          n_cycles_char[j] = ' ';
        }
      
      for (j = 0; freq[j] != '\0'; j++)
        {
          freq[j] = ' ';
        }
      
      for (j = 0; duty_cycle[j] != '\0'; j++)
        {
          duty_cycle[j] = ' ';
        }
   
      receivedCommandStr = receivedCommand;
      
      interpretCommand(receivedCommandStr);

      for (j = 0; receivedCommand[j] != '\0'; j++)
      {
        receivedCommand[j] = ' ';
      }
     
      if (first_command == 'X')
      {
         frequency = atoi(freq);
      
         num_cycles = atoi(n_cycles_char);
          
         dutyCycle = atoi(duty_cycle);
          
         setupPWM(channel,mode,120000000,frequency,dutyCycle,num_cycles,trigger);
       
         startPWM();
      }
      
      flagRecebido = 0;
    }
  }
  
}
