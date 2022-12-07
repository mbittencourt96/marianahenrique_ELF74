#include "system_TM4C1294.h" // CMSIS-Core
#include "cmsis_os2.h" // CMSIS-RTOS
#include "stdio.h"
#include <string.h>
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
#include <stdio.h>

#define MAX_NUM_OF_MESSAGES 16

typedef struct {                               
  uint8_t mensagem[32];
} messageObj;       //definition of message object

osMessageQueueId_t mid_CommandQueue;                // command message queue id
osMessageQueueId_t mid_InternalCommandQueue;                // internal buttons command message queue id
osMessageQueueId_t mid_ExternalCommandQueue;                // external buttons command message queue id
osMessageQueueId_t mid_StatusQueue;                // status message queue id
osMessageQueueId_t mid_StatusRequestQueue;                // status request message queue id
osMessageQueueId_t mid_RightElevatorQueue;         // right elevator message queue id
osMessageQueueId_t mid_CenterElevatorQueue;        // center elevator message queue id
osMessageQueueId_t mid_LeftElevatorQueue;          // left elevator message queue id
osMessageQueueId_t mid_OutputQueue;                // output message queue id
osMessageQueueId_t mid_FlagQueue;                // flag message queue id

osThreadId_t threadED_id;
osThreadId_t threadEE_id;
osThreadId_t threadEC_id;
osThreadId_t threadCD_id;    //Thread ids

int initMessageQueues()
{
  mid_InternalCommandQueue = osMessageQueueNew(MAX_NUM_OF_MESSAGES,sizeof(messageObj),NULL);
  mid_ExternalCommandQueue = osMessageQueueNew(MAX_NUM_OF_MESSAGES,sizeof(messageObj),NULL);
  mid_StatusQueue = osMessageQueueNew(1,sizeof(messageObj),NULL);
  mid_StatusRequestQueue = osMessageQueueNew(1,sizeof(messageObj),NULL);
  mid_RightElevatorQueue = osMessageQueueNew(MAX_NUM_OF_MESSAGES,sizeof(messageObj),NULL);
  mid_CenterElevatorQueue = osMessageQueueNew(MAX_NUM_OF_MESSAGES,sizeof(messageObj),NULL);
  mid_LeftElevatorQueue = osMessageQueueNew(MAX_NUM_OF_MESSAGES,sizeof(messageObj),NULL);
  mid_OutputQueue = osMessageQueueNew(MAX_NUM_OF_MESSAGES,sizeof(messageObj),NULL);
  
  if (mid_InternalCommandQueue == NULL || mid_ExternalCommandQueue == NULL || mid_StatusQueue == NULL || mid_StatusRequestQueue ||
      mid_OutputQueue == NULL || mid_RightElevatorQueue == NULL || mid_CenterElevatorQueue == NULL || mid_LeftElevatorQueue == NULL )
  {
    return -1;
  }
  else
  {
    return 0;
  }
}

int checkIfNumber(char c)
{
  int q;

  q=(int)c;
  ++q;

  if((q - c) != 1)
      return -1;
  else
      return 0;
}

//UART functions and variables

void UART_Rx_Handler()
{
  uint32_t Status;
  char caract;
  messageObj msg;

  Status = UARTIntStatus(UART0_BASE, true); // Retorna o status da interrupção   
  
  UARTIntClear(UART0_BASE, Status);         // Limpa Flag de interrupção
  
  if ((Status == UART_INT_RX) || (Status == UART_INT_RT))
  {
    int i = 0;
    
    while(caract != '\n')
      {
        // Algoritmo de conversao
        caract = (unsigned char) UARTCharGetNonBlocking(UART0_BASE);
        
        msg.mensagem[i++] = caract;
      } 
    
    msg.mensagem[i] = '\0';
    
    if (checkIfNumber(msg.mensagem[0]) == -1)
    {
      osMessageQueuePut(mid_CommandQueue,&msg,0,0);   //put command into command queue
    }
    else
    {
      osMessageQueuePut(mid_StatusQueue,&msg,0,0);   //put command into status queue
    }
   
    
    
}
  else if (Status == UART_INT_TX)
  {
    //DO SOMETHING
  }
}

void UARTSend(const unsigned char *Buffer, uint32_t Len)
{
    while(Len--)
    {
        UARTCharPut(UART0_BASE, *Buffer++);
    }
}

void setupUART()
{
    uint32_t SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_240), 120000000);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0) && !SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA))
    {
    }                                                 //Wait until UART0 and PORT A is ready
    
   
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTConfigSetExpClk(UART0_BASE, SysClock, 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT | UART_INT_TX);
    UARTIntRegister(UART0_BASE,UART_Rx_Handler);
    IntMasterEnable();
    IntEnable(INT_UART0_TM4C129);
}


void threadElevadorDireito()
{
  
}

void threadElevadorEsquerdo()
{
  
}

void threadElevadorCentral()
{
  
}

void threadCommandDecoder(void *arg)
{  
  while(1)
  {
    messageObj receivedStatus;
    messageObj receivedExternalCommand;
    messageObj receivedInternalCommand;
    messageObj getStatusFromED;
    messageObj getStatusFromEC;
    messageObj getStatusFromEE;
    
    osStatus_t status;
    
    status = osMessageQueueGet(mid_ExternalCommandQueue,&receivedExternalCommand,0,osWaitForever);
    if (status == osOK)   //received external buttons command
    {
      
      char *request = "cx\r\n";
      int i = 0;
      
      while(request[i] != '\n')
      {
        getStatusFromEC.mensagem[i++] = request[i];
      } 

      osMessageQueuePut(mid_StatusRequestQueue,&getStatusFromEC,0,0);   //put command into status request queue(request status from center elevator)
      
      status = osMessageQueueGet(mid_StatusQueue,&receivedStatus,0,osWaitForever);  //wait for status
      
      if (status == osOK) 
      {
        i = 0;
        *request = "ex\r\n";
        
      while(request[i] != '\n')
        {
          getStatusFromEE.mensagem[i++] = request[i];
        } 

      osMessageQueuePut(mid_StatusRequestQueue,&getStatusFromEE,0,0);   //put command into status request queue(request status from center elevator)
      
      status = osMessageQueueGet(mid_StatusQueue,&receivedStatus,0,osWaitForever);  //wait for status
        
      if (status == osOK) 
      {
        i = 0;
        *request = "dx\r\n";
        
        while(request[i] != '\n')
          {
            getStatusFromED.mensagem[i++] = request[i];
          } 

        osMessageQueuePut(mid_StatusRequestQueue,&getStatusFromED,0,0);   //put command into status request queue(request status from center elevator)
        
        status = osMessageQueueGet(mid_StatusQueue,&receivedStatus,0,osWaitForever);  //wait for status
      }
        
        
      }
      
      
    }
  }
  
}

void main(void)   //main entry function
{
  osKernelInitialize();     //initialize the OS kernel
  
  int statusOfQueue = initMessageQueues();   //Create and init message queues
  
  if (statusOfQueue == -1)
  {
    printf("Uma ou mais filas nao foram criadas, tente novamente!");
  }
  else
  {    
    setupUART();    //Setup the UART;
    threadED_id = osThreadNew(threadElevadorDireito, NULL, NULL);
    threadEC_id = osThreadNew(threadElevadorCentral, NULL, NULL);
    threadEE_id = osThreadNew(threadElevadorEsquerdo, NULL, NULL);
    threadCD_id = osThreadNew(threadCommandDecoder, NULL, NULL);    //create and get thread ids
    
    if (threadCD_id != NULL && threadED_id != NULL && threadEC_id != NULL 
        && threadEE_id != NULL)
    {
      if(osKernelGetState() == osKernelReady)
      {
          osKernelStart();   
      }

      while(1);  
    } 
    else
    {
      printf("Uma ou mais threads nao foram criadas, tente novamente!");
    }
  }

}