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
osMessageQueueId_t mid_RightElevatorCommandQueue;         // right elevator message queue id
osMessageQueueId_t mid_CenterElevatorCommandQueue;        // center elevator message queue id
osMessageQueueId_t mid_LeftElevatorCommandQueue;          // left elevator message queue id
osMessageQueueId_t mid_RightElevatorStatusQueue;         // right elevator message queue id
osMessageQueueId_t mid_CenterElevatorStatusQueue;        // center elevator message queue id
osMessageQueueId_t mid_LeftElevatorStatusQueue;          // left elevator message queue id
osMessageQueueId_t mid_OutputQueue;                // output message queue id
osMessageQueueId_t mid_CurrentActivityQueue;                // output message queue id

osThreadId_t threadED_id;
osThreadId_t threadEE_id;
osThreadId_t threadEC_id;
osThreadId_t threadCD_id;
osThreadId_t threadOutput_id; 
osThreadId_t threadCurrentActivity_id;   //Thread ids

void init_LCD(void);
void escreve_texto_LCD(char* texto);
void escreve_caracter_LCD(uint32_t c);
void clear_LCD(void);
void seleciona_primeira_linha_LCD(void);
void seleciona_segunda_linha_LCD(void);		


int initMessageQueues()
{
  mid_InternalCommandQueue = osMessageQueueNew(MAX_NUM_OF_MESSAGES,sizeof(messageObj),NULL);
  mid_ExternalCommandQueue = osMessageQueueNew(MAX_NUM_OF_MESSAGES,sizeof(messageObj),NULL);
  mid_StatusQueue = osMessageQueueNew(3,sizeof(messageObj),NULL);
  mid_RightElevatorCommandQueue = osMessageQueueNew(MAX_NUM_OF_MESSAGES,sizeof(messageObj),NULL);
  mid_CenterElevatorCommandQueue = osMessageQueueNew(MAX_NUM_OF_MESSAGES,sizeof(messageObj),NULL);
  mid_LeftElevatorCommandQueue = osMessageQueueNew(MAX_NUM_OF_MESSAGES,sizeof(messageObj),NULL);
  mid_RightElevatorStatusQueue = osMessageQueueNew(1,sizeof(messageObj),NULL);
  mid_CenterElevatorStatusQueue = osMessageQueueNew(1,sizeof(messageObj),NULL);
  mid_LeftElevatorStatusQueue = osMessageQueueNew(1,sizeof(messageObj),NULL);
  mid_OutputQueue = osMessageQueueNew(MAX_NUM_OF_MESSAGES,sizeof(messageObj),NULL);
  mid_CurrentActivityQueue = osMessageQueueNew(3,sizeof(messageObj),NULL);
  
  if (mid_InternalCommandQueue == NULL || mid_ExternalCommandQueue == NULL || mid_StatusQueue == NULL ||
      mid_OutputQueue == NULL || mid_RightElevatorCommandQueue == NULL || mid_CenterElevatorCommandQueue == NULL || mid_LeftElevatorCommandQueue == NULL
        || mid_RightElevatorStatusQueue == NULL || mid_CenterElevatorStatusQueue == NULL || mid_LeftElevatorStatusQueue || mid_CurrentActivityQueue == NULL)
  {
    return -1;
  }
  else
  {
    return 0;
  }
}

void sendCommandtoElevator(int chosenElevator, int requestedFloor)
{
  messageObj elevatorCommand;
 
  elevatorCommand.mensagem[0] = requestedFloor + '0';
  
  switch (chosenElevator)
  {
  case 0:        //right elevator
    {
      osMessageQueuePut(mid_RightElevatorCommandQueue,&elevatorCommand,0,0);
      break;
    }
    
   case 1:      //center elevator
    {
      osMessageQueuePut(mid_CenterElevatorCommandQueue,&elevatorCommand,0,0);
      break;
    }
    
   case 2:    //left elevator
    {
      osMessageQueuePut(mid_LeftElevatorCommandQueue,&elevatorCommand,0,0);
      break;
    }
    
  }
  
}

int maxValue (int i, int j, int k)
{
  int arr[3] = {i,j,k};
  int maxValue;
  int maxIndex;
  
   for (int m = 1; m < 3; ++m) {
    if (arr[0] < arr[m]) {
      maxIndex = m;
    }
  }
  
  return maxIndex;
}


void chooseElevatorWithLessComands(int requestedFloor)
{
  int numberOfMsgsRightElevator;
  int numberOfMsgsCenterElevator;
  int numberOfMsgsLeftElevator;
  
  int chosenElevator;
  
  numberOfMsgsRightElevator = osMessageQueueGetCount(mid_RightElevatorCommandQueue);
  numberOfMsgsCenterElevator = osMessageQueueGetCount(mid_CenterElevatorCommandQueue);
  numberOfMsgsLeftElevator = osMessageQueueGetCount(mid_LeftElevatorCommandQueue);
  
  int maxIndex = maxValue(numberOfMsgsRightElevator,numberOfMsgsCenterElevator,numberOfMsgsLeftElevator);
  
  chosenElevator = maxIndex;
  
  sendCommandtoElevator(chosenElevator,requestedFloor);
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

void UART_Rx_Tx_Handler()
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
    UARTIntRegister(UART0_BASE,UART_Rx_Tx_Handler);
    IntMasterEnable();
    IntEnable(INT_UART0_TM4C129);
}


void threadElevadorDireito()
{
  while (1)
  {
    osStatus_t status;
    messageObj receivedMsg;
    messageObj receivedStatus;
    messageObj commandtoSend;
    int requestedFloor;
      
    status = osMessageQueueGet(mid_RightElevatorCommandQueue,&receivedMsg,0,osWaitForever);
    
    if (status == osOK)
    {
      requestedFloor = atoi(receivedMsg.mensagem[1]);
      status = osMessageQueueGet(mid_RightElevatorStatusQueue,&receivedStatus,0,osWaitForever);
      
      
      
      
      
      
      commandtoSend.mensagem[0] = 'd';
      commandtoSend.mensagem[1] = receivedMsg.mensagem[0];
      osMessageQueuePut(mid_OutputQueue,&commandtoSend,0,0);
    }
  }
  
}

void threadElevadorEsquerdo()
{
   while (1)
  {
    osStatus_t status;
    messageObj receivedCommand;
    messageObj commandtoSend;
      
    status = osMessageQueueGet(mid_LeftElevatorQueue,&receivedCommand,0,osWaitForever);
    
    if (status == osOK)
    {
        commandtoSend.mensagem[0] = 'e';
        commandtoSend.mensagem[1] = receivedCommand.mensagem[0];
        osMessageQueuePut(mid_OutputQueue,&commandtoSend,0,0);
    }
  }
  
}

void threadElevadorCentral()
{
   while (1)
  {
    osStatus_t status;
    messageObj receivedCommand;
    messageObj commandtoSend;
      
    status = osMessageQueueGet(mid_CenterElevatorQueue,&receivedCommand,0,osWaitForever);
    
    if (status == osOK)
    {
        commandtoSend.mensagem[0] = 'c';
        commandtoSend.mensagem[1] = receivedCommand.mensagem[0];
        osMessageQueuePut(mid_OutputQueue,&commandtoSend,0,0);
    }
  }
  
}

void threadCommandDecoder(void *arg)
{  
  while(1)
  {
    messageObj receivedStatus;
    messageObj receivedExternalCommand;
    messageObj receivedInternalCommand;
    messageObj receivedFlag;
    messageObj getStatusFromED;
    messageObj getStatusFromEC;
    messageObj getStatusFromEE;
    
    int ECfloor;
    int EEfloor;
    int EDfloor;
    int requestedFloor;
    
    int ECflag;         //0 = Rising ; 1 = Down ; 2 = Stopped
    int EEflag;
    int EDflag;
    
    osStatus_t status;
    
    status = osMessageQueueGet(mid_ExternalCommandQueue,&receivedExternalCommand,0,osWaitForever);
    
    if (status == osOK)   //received external buttons command
    {
      char *extCommand = receivedExternalCommand.mensagem;   //get the command
      
      printf("Received command: %s", receivedExternalCommand.mensagem);   //todo: LCD
      
      for (int j = 0; j < 3; j ++)
      {
        status = osMessageQueueGet(mid_StatusQueue,&receivedStatus,0,osWaitForever);
      
        if (status == osOK)   //received status of one of the elevators
        {
          printf("Received status: %s", receivedStatus.mensagem);   //todo: LCD
          
          if (receivedStatus.mensagem[0] == 'c')    //center elevator
          {
            ECfloor = atoi(receivedStatus.mensagem[1]);
          }
          else if (receivedStatus.mensagem[0] == 'd') //right elevator
          {
            EDfloor = atoi(receivedStatus.mensagem[1]);
          }
          else     //left elevator
          {
            EEfloor = atoi(receivedStatus.mensagem[1]);
          }
      }
     }
 
      char requestedFloorString[2];
      requestedFloorString[0] = extCommand[2];
      requestedFloorString[1] = extCommand[3];
      
      requestedFloor = atoi(requestedFloorString);
      
      for (int j = 0; j < 3; j ++)     //get flags of all elevators
      {
        status = osMessageQueueGet(mid_FlagQueue,&receivedFlag,0,osWaitForever);
      
        if (status == osOK)   //received status of one of the elevators
        {
          printf("Received flag %s", receivedFlag.mensagem);   //todo: LCD
          
          if (receivedStatus.mensagem[0] == 'c')    //center elevator
          {
            ECflag = atoi(receivedStatus.mensagem[1]);
          }
          else if (receivedStatus.mensagem[0] == 'd') //right elevator
          {
            EDflag = atoi(receivedStatus.mensagem[1]);
          }
          else     //left elevator
          {
            EEflag = atoi(receivedStatus.mensagem[1]);
          }
      }
     }
     
     int chosenElevator;
     
      
     int floorArray[3] = {EDfloor,ECfloor,EDfloor};
     int flagArray[3] = {EDflag,ECflag,EDflag};
     
     for (int k = 0; k < 3; k++)  // k = 0 = right elevator ; k = 1 = center elevator; k = 2 = left elevator
     
     {
       if (requestedFloor > floorArray[k] && flagArray[k] == 0)   //elevator is rising and requested floor is above current floor
       {
         chosenElevator = k;
         break;
       }
       else if (requestedFloor < floorArray[k] && flagArray[k] == 1)   //elevator is descending and requested floor is below current floor
       {
         chosenElevator = k;
         break;
       }
       else if (requestedFloor > floorArray[k] && flagArray[k] == 1)  //elevator is descending and requested floor is above current floor
       {
         continue;    //check next elevator
       }
       else if (requestedFloor < floorArray[k] && flagArray[k] == 0) //elevator is rising and requested floor is below current floor
       {
         continue; //check next elevator
       }
       else
       {
         chosenElevator = chooseElevatorWithLessComands(requestedFloor);   //TODO: implement this function
         break;
       }
     }
     
     sendCommandtoElevator(chosenElevator,requestedFloor); //TODO: implement this function
     
     
      }
      
      }
        
        
}

void threadOutput()
{
  while(1)
  {
    osStatus_t status;
    messageObj receivedCommand;
    char* commandtoSend;
      
    status = osMessageQueueGet(mid_OutputQueue,&receivedCommand,0,osWaitForever);
    
    if (status == osOK)
    {
        while(caract != '\n')
      {
        // Algoritmo de conversao
        caract = (unsigned char) UARTCharGetNonBlocking(UART0_BASE);
        
        msg.mensagem[i++] = caract;
      } 
    
        msg.mensagem[i] = '\0';
        osMessageQueuePut(mid_OutputQueue,&commandtoSend,0,0);
    }
  }
 
}

void threadCurrentActivity()
{
  while(1)
  {
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
    init_LCD();
   
    threadED_id = osThreadNew(threadElevadorDireito, NULL, NULL);
    threadEC_id = osThreadNew(threadElevadorCentral, NULL, NULL);
    threadEE_id = osThreadNew(threadElevadorEsquerdo, NULL, NULL);
    threadCD_id = osThreadNew(threadCommandDecoder, NULL, NULL);  
    threadOutput_id = osThreadNew(threadOutput, NULL, NULL);       
    threadCurrentActivity_id = osThreadNew (threadCurrentActivity, NULL, NULL);                   //create and get thread ids
    
    if (threadCD_id != NULL && threadED_id != NULL && threadEC_id != NULL 
        && threadEE_id != NULL && threadOutput_id != NULL)
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