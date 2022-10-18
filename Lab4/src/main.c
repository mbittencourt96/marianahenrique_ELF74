#include <stdint.h>
#include <stdbool.h>
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

uint32_t SysClock;
volatile uint32_t interrupts_counter2 = 0;
volatile uint32_t interrupts_counter3 = 0;
volatile uint32_t interrupts_counter4 = 0;
volatile uint32_t interrupts_counter5 = 0;
char receivedCommand[30];
char *receivedCommandStr;
volatile int flagRecebido = 0;


void UARTSend(const uint8_t *Buffer, uint32_t Len)
{
    while(Len--)
    {
        UARTCharPut(UART0_BASE, *Buffer++);
    }
}

void startPWM(int channel)
{
  if (channel == 1 || channel == 2)
  {
    PWMGenEnable(PWM0_BASE, PWM_GEN_1); // Enable PWM
  }
  else
  {
    PWMGenEnable(PWM0_BASE, PWM_GEN_2);
  }
}

void stopPWM(int channel)
{
  if (channel == 1 || channel == 2)
  {
    PWMGenDisable(PWM0_BASE, PWM_GEN_1); // Disable PWM
  }
  else
  {
    PWMGenDisable(PWM0_BASE, PWM_GEN_2);
  }
}

void executeCommand(char *command)
{
  if (command[1] == 'X')
  {
    char channel = command[2];
    char mode = command[4];
    char freq[10];
    char duty_cycle[10];
    char n_cycles[10];
    char trigger;
    
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
      n_cycles[m] = command[k];
      m++;
    }
    
    k = k + 1;
    
    trigger = command[k];
   
  }
  else if (command[1] == 'O')
  {
    
  }
  else if (command[1] == 'T')
  {
    
  }
  else
  {
    UARTSend("Comando inválido!", strlen("Comando inválido!"));
  }
}


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


void PWM_Int_Handler2()
{
  if (interrupts_counter2 < 3)
  {
    GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);  //Led 1 ON
    
    interrupts_counter2++;
  }
  else
  {
    GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0x0);  //Led 1 OFF
    
    interrupts_counter2 = 0;
  }
}  

void PWM_Int_Handler3()
{
  if (interrupts_counter3 < 3)
  {
    GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);  //Led 2 ON
    
    interrupts_counter3++;
  }
  else
  {
    GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0x0);  //Led 2 OFF
    
    interrupts_counter3 = 0;
  }
}  

void PWM_Int_Handler4()
{
  if (interrupts_counter4 < 3)
  {
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);  //Led 3 ON
    
    interrupts_counter4++;
  }
  else
  {
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x0);  //Led 3 OFF
    
    interrupts_counter4 = 0;
  }
}  

void PWM_Int_Handler5()
{
   if (interrupts_counter5 < 3)
  {
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4);  //Led 4 ON
    
    interrupts_counter5++;
  }
  else
  {
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0x0);  //Led 4 OFF
    
    interrupts_counter5 = 0;
  }
}  

void setupLEDs()
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);  //Enable GPIO for embedded LEDs
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); // Enable Port F
  
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION) && !SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
    {
      
    }
   GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);
   GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_1);
   GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);
   GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_4);    //Configure all LEDS as outputs (PN1,PN0,PF0,PF4)
   
   
   GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0x0); 
   GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0x0);  
   GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x0);  
   GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0x0);  //Leds OFF
   
   
   
 
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

void setupButton()
{
   SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);  //Enable GPIO for switches
  
   while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ))
    {
      
    }       
   
   GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0);
   
   GPIOIntTypeSet(GPIO_PORTJ_BASE, GPIO_PIN_0, GPIO_FALLING_EDGE);
   GPIOPadConfigSet(GPIO_PORTJ_BASE, GPIO_PIN_0, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
   
    
}

/*void enableButtonTrigger()
{
  IntMasterEnable();
  IntEnable(INT_GPIOJ);
  GPIOIntRegister(GPIO_PORTJ_BASE, startPWM);
  GPIOIntDisable(GPIO_PORTJ_BASE, GPIO_INT_PIN_1);
  GPIOIntEnable(GPIO_PORTJ_BASE, GPIO_INT_PIN_0);
}*/

void setupPWM(int channel, int mode, float CPU_FREQ, float PWM_FREQ, int cycles, bool trigger)
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
      case 1:  //Config PWM2
            SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); // Enable Port F
            
            while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
                    {
                    }                                                 
                    
            
            GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_2); // Enable alternate function of PF2
            GPIOPinConfigure(GPIO_PF2_M0PWM2);
            PWMGenConfigure(PWM0_BASE, PWM_GEN_1, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_DB_NO_SYNC);
            PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, PWM_WORD); // Set PWM frequency
            PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2, PWMGenPeriodGet(PWM0_BASE, PWM_GEN_1)/1000); // Set Duty cycle 
            PWMOutputState(PWM0_BASE, PWM_OUT_2_BIT, true); // Enable PWM output channel 2
            PWMIntEnable(PWM0_BASE, PWM_INT_GEN_1);
            IntEnable(INT_PWM0_1);
            PWMGenIntTrigEnable(PWM0_BASE, PWM_INT_GEN_1, PWM_INT_CNT_AU);
            IntMasterEnable();
            PWMGenIntRegister(PWM0_BASE, PWM_INT_GEN_1,PWM_Int_Handler2);
            
            
            
            //PWMGenEnable(PWM0_BASE, PWM_GEN_2); // Enable PWM module
        break;
        
      case 2:   //Config PWM3
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); // Enable Port F
            
        while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
                {
                }                                                 
                
        
        GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_3); // Enable alternate function of PF3
        GPIOPinConfigure(GPIO_PF3_M0PWM3);
        PWMGenConfigure(PWM0_BASE, PWM_GEN_1, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_DB_NO_SYNC);
        PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, PWM_WORD); // Set PWM frequency
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_3, PWMGenPeriodGet(PWM0_BASE, PWM_GEN_1)/1000); // Set Duty cycle 
        PWMOutputState(PWM0_BASE, PWM_OUT_3_BIT, true); // Enable PWM output channel 3
        
        PWMIntEnable(PWM0_BASE, PWM_INT_GEN_1);
        IntEnable(INT_PWM0_1);
        PWMGenIntTrigEnable(PWM0_BASE, PWM_INT_GEN_1, PWM_INT_CNT_AU);
        IntMasterEnable();
        PWMGenIntRegister(PWM0_BASE, PWM_INT_GEN_1,PWM_Int_Handler3);
        break;
  
      case 3: //Config PWM4
              SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG); // Enable Port G
            
        while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOG))
                {
                }                                                 
                
        
        GPIOPinTypePWM(GPIO_PORTG_BASE, GPIO_PIN_0); // Enable alternate function of PG0
        GPIOPinConfigure(GPIO_PG0_M0PWM4);
        PWMGenConfigure(PWM0_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_DB_NO_SYNC);
        PWMGenPeriodSet(PWM0_BASE, PWM_GEN_2, PWM_WORD); // Set PWM frequency
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_4, PWMGenPeriodGet(PWM0_BASE, PWM_GEN_2)/1000); // Set Duty cycle 
        PWMOutputState(PWM0_BASE, PWM_OUT_4_BIT, true); // Enable PWM output channel 4
        
        PWMIntEnable(PWM0_BASE, PWM_INT_GEN_2);
        IntEnable(INT_PWM0_2);
        PWMGenIntTrigEnable(PWM0_BASE, PWM_INT_GEN_2, PWM_INT_CNT_AU);
        IntMasterEnable();
        PWMGenIntRegister(PWM0_BASE, PWM_INT_GEN_2,PWM_Int_Handler4);

        break;
        
      case 4: //Config PWM5
        
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG); // Enable Port G
            
        while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOG))
                {
                }                                                 
                
        
        GPIOPinTypePWM(GPIO_PORTG_BASE, GPIO_PIN_1); // Enable alternate function of PG1
        GPIOPinConfigure(GPIO_PG1_M0PWM5);
        PWMGenConfigure(PWM0_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_DB_NO_SYNC);
        PWMGenPeriodSet(PWM0_BASE, PWM_GEN_2, PWM_WORD); // Set PWM frequency
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_5, PWMGenPeriodGet(PWM0_BASE, PWM_GEN_2)/1000); // Set Duty cycle 
        PWMOutputState(PWM0_BASE, PWM_OUT_5_BIT, true); // Enable PWM output channel 4
        
        PWMIntEnable(PWM0_BASE, PWM_INT_GEN_2);
        IntEnable(INT_PWM0_2);
        PWMGenIntTrigEnable(PWM0_BASE, PWM_INT_GEN_2, PWM_INT_CNT_AU);
        IntMasterEnable();
        PWMGenIntRegister(PWM0_BASE, PWM_INT_GEN_2,PWM_Int_Handler5);
        
        break;
        
        
    /* switch(mode)   TODO: Implement other modes
     {
     case 1: // Single mode
       break;
     case 2: //Burst mode
       break;
     case 3: //Continuous mode
       break;
     }
      */
        
    }
     /*if (trigger == true)
     {
       enableButtonTrigger();
     }*/
        
}

void main()
{
  setupUART(); 
  
  while(1)
  {
    if (flagRecebido)
    {
      receivedCommandStr = receivedCommand;
      
      executeCommand(receivedCommandStr);
      
      int j;
      
      for (j = 0; receivedCommand[j] != '\0'; j++)
      {
        receivedCommand[j] = ' ';
      }
      
      flagRecebido = 0;
    }
  }
  
}
