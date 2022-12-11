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

osMessageQueueId_t mid_InputCommandQueue;                // input command message queue id
osMessageQueueId_t mid_StatusQueue;                // status message queue id
osMessageQueueId_t mid_OutputQueue;                // output message queue id
osMessageQueueId_t mid_LastCommandQueue;                // last command message queue id
osMessageQueueId_t mid_WaitingQueue;                // waiting commands message queue id
osMessageQueueId_t mid_InfoQueue;                // information from elevator message queue id

osThreadId_t threadStatus_id;
osThreadId_t threadCommand_id;
osThreadId_t threadOutput_id;   //Thread ids

char *initialMessage = " ";

int initMessageQueues()
{
  mid_OutputQueue = osMessageQueueNew(MAX_NUM_OF_MESSAGES,sizeof(messageObj),NULL);
  mid_StatusQueue = osMessageQueueNew(1,sizeof(messageObj),NULL);
  mid_InputCommandQueue = osMessageQueueNew(MAX_NUM_OF_MESSAGES,sizeof(messageObj),NULL);
  mid_LastCommandQueue = osMessageQueueNew(1,sizeof(messageObj),NULL);
  mid_WaitingQueue = osMessageQueueNew(MAX_NUM_OF_MESSAGES,sizeof(messageObj),NULL);
  mid_InfoQueue = osMessageQueueNew(1,sizeof(messageObj),NULL);
   
  if (mid_OutputQueue == NULL || mid_StatusQueue == NULL || mid_InputCommandQueue == NULL 
      || mid_LastCommandQueue == NULL || mid_WaitingQueue == NULL || mid_InfoQueue == NULL)
  {
    return -1;
  }
  else
  {
    return 0;
  }
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
   
    char *mensagem_char = (char *) msg.mensagem;
    
    if (strlen(mensagem_char) == 3 && mensagem_char[1] != 'I')    //status message
    {
      osMessageQueuePut(mid_StatusQueue,&msg,0,0);   //put command into external command queue
                                                              //TODO: implement for internal command
    }
    else if (strlen(mensagem_char) > 8) //initial message
    {
      initialMessage = mensagem_char;
    }
    else
    {
      osMessageQueuePut(mid_InputCommandQueue,&msg,0,0);   //put command into status queue
    }
    
   
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
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
    UARTIntRegister(UART0_BASE,UART_Rx_Handler);
    IntMasterEnable();
    IntEnable(INT_UART0_TM4C129);
}

void threadOutputController(void *arg)
{
  while(1)
  {
    uint32_t n_of_messages;
    osStatus_t status;
    messageObj waitingMsg;
    messageObj outputMsg;
    
    n_of_messages = osMessageQueueGetCount(mid_OutputQueue);
    
    if (n_of_messages == 0)
    {
     status = osMessageQueueGet(mid_WaitingQueue, &waitingMsg, 0, 0);
     
     if (status == osOK)
     {
       UARTSend(waitingMsg.mensagem,strlen(waitingMsg.mensagem));
     }
    }
    else
    {
     status = osMessageQueueGet(mid_OutputQueue, &outputMsg, 0, 0);
     
     if (status == osOK)
     {
       UARTSend(outputMsg.mensagem,strlen(outputMsg.mensagem));
     }
    }
    
  } 
}

void threadCommandDecoder(void *arg)
{  
  while(1)
  {
    messageObj receivedInfo;
    messageObj receivedInputCommand;
    messageObj lastCommand;
    messageObj outputCommand;
    
    int currentFloor;
    int goFlag;
    int movementStatus;   //0 = Rising ; 1 = Going down ; 2 = Still
    int requestedFloor;
  
    osStatus_t statusCommand;
    osStatus_t infoElevator;
    
    statusCommand = osMessageQueueGet(mid_InputCommandQueue,&receivedInputCommand,0,osWaitForever);   //Get command from queue
    
    if (statusCommand == osOK)
    {
      
      char *InputCommand = receivedInputCommand.mensagem;   //get the command
     
      infoElevator = osMessageQueueGet(mid_StatusQueue,&receivedInfo,0,osWaitForever);   //Get information
   
      if (infoElevator == osOK)   //received external buttons command
      {
        
        goFlag = receivedInfo.mensagem[0];   //Get go flag. 0 = don't send the next command; 1 = allowed to send the next command
        
        char currentFloorString[2];
        
        currentFloorString[0] = receivedInfo.mensagem[1];
        currentFloorString[1] = receivedInfo.mensagem[2];
        
        currentFloor = atoi(currentFloorString);
        
        movementStatus = receivedInfo.mensagem[3]; // Get movement status. 0 = Rising; 1 = Going down; 2 = Still
        
        messageObj outputCommand;
        
        char *output = "ca\r";      //send command to open the doors
       
        for (int i = 0 ; i < 3; i++)
        {
          outputCommand.mensagem[i] = output[i];
        }
        
        if (goFlag == 1)
        {
          osMessageQueuePut(mid_OutputQueue, &outputCommand, 0, osWaitForever);
          
          osDelay(3000);   //wait 3 secs
          
          char *output = "cf\r";      //send command to close the doors
       
          for (int i = 0 ; i < 3; i++)
          {
            outputCommand.mensagem[i] = output[i];
          }
          
          if (strlen(InputCommand) == 7)
          {
            char requestedFloorString[2];
            
            requestedFloorString[0] = InputCommand[2];
            requestedFloorString[1] = InputCommand[3];
            
            requestedFloor = atoi(requestedFloorString);
          }
          else
          {
            char requestedFloorChar = atoi((char*)InputCommand[2]);

            if (requestedFloorChar == 'a')
            {
              requestedFloor = 0;
            }
            else if (requestedFloorChar == 'b')
            {
              requestedFloor = 1;
            }
            else if (requestedFloorChar == 'c')
            {
              requestedFloor = 2;
            }
            else if (requestedFloorChar == 'd')
            {
              requestedFloor = 3;
            }
            else if (requestedFloorChar == 'e')
            {
              requestedFloor = 4;
            }
            else if (requestedFloorChar == 'f')
            {
              requestedFloor = 5;
            }
            else if (requestedFloorChar == 'g')
            {
              requestedFloor = 6;
            }
            else if (requestedFloorChar == 'h')
            {
              requestedFloor = 7;
            }
            else if (requestedFloorChar == 'i')
            {
              requestedFloor = 8;
            }
            else if (requestedFloorChar == 'j')
            {
              requestedFloor = 9;
            }
            else if (requestedFloorChar == 'k')
            {
              requestedFloor = 10;
            }
            else if (requestedFloorChar == 'l')
            {
              requestedFloor = 11;
            }
            else if (requestedFloorChar == 'm')
            {
              requestedFloor = 12;
            }
            else if (requestedFloorChar == 'n')
            {
              requestedFloor = 13;
            }
            else if (requestedFloorChar == 'o')
            {
              requestedFloor = 14;
            }
            else
            {
              requestedFloor = 15;
            }
            
          }
          
          if (requestedFloor > currentFloor)
          {
            
            char lastCommandStr[2];
            sprintf(lastCommandStr, "%d", requestedFloor);      // integer to string
            
            int i;
            
            for (i = 0 ; i < 2; i++)
            {
              lastCommand.mensagem[i] = lastCommandStr[i];
            }
            
            lastCommand.mensagem[i] = '\r';
            
            osMessageQueuePut(mid_LastCommandQueue, &lastCommand,0,0);
            
            char *outputCommandStr = "cs\r";    //go up
            
            for (i = 0 ; i < 3; i++)
            {
              outputCommand.mensagem[i] = outputCommandStr[i];
            }
            osMessageQueuePut(mid_OutputQueue,&outputCommand,0,0);          
          }
          else if (requestedFloor < currentFloor)
          {
            char lastCommandStr[2];
            sprintf(lastCommandStr, "%d", requestedFloor);      // integer to string
            
            int i;
            
            for (i = 0 ; i < 2; i++)
            {
              lastCommand.mensagem[i] = lastCommandStr[i];
            }
            
            lastCommand.mensagem[i] = '\r';
            
            osMessageQueuePut(mid_LastCommandQueue, &lastCommand,0,0);
            
            char *outputCommandStr = "cd\r";    //go down
            
            for (i = 0 ; i < 3; i++)
            {
              outputCommand.mensagem[i] = outputCommandStr[i];
            }
            osMessageQueuePut(mid_OutputQueue,&outputCommand,0,0);       
            
          }      
        }  
      }
   }   
}

}

void threadStatusController(void *argument)
{
  while(1)
  {
    osStatus_t status;
    messageObj statusMessage;
    messageObj lastCommandMsg;
    messageObj outputCommandMsg;
    messageObj infoToSend;
    
    int targetFloor;
    int currentFloor;
    
    char infoString [3];
    
    status = osMessageQueueGet(mid_StatusQueue,&statusMessage,0,osWaitForever);
    
    if (status == osOK)
    {
      char currentFloorStr[2];
      
      currentFloorStr[0] = statusMessage.mensagem[1];
      currentFloorStr[1] = statusMessage.mensagem[2];
      
      currentFloor = atoi(currentFloorStr);
    
      status = osMessageQueueGet(mid_LastCommandQueue,&lastCommandMsg,0,osWaitForever);
      
      if (status == osOK)
      {
        char targetFloorStr[2];
        
        targetFloorStr[0] = lastCommandMsg.mensagem[0];
        targetFloorStr[1] = lastCommandMsg.mensagem[1];
        
        targetFloor = atoi(targetFloorStr);
        
        if (targetFloor == currentFloor)
        {
          infoString[0] = '1';
          infoString[1] = currentFloorStr[0];
          infoString[2] = currentFloorStr[1];
          
          char *outputCommandStr = "cp\r";     //command to stop elevator
            
          int i;
          
          for (i = 0 ; i < 3; i++)
          {
            outputCommandMsg.mensagem[i] = outputCommandStr[i];
          }
          
          osMessageQueuePut(mid_OutputQueue,&outputCommandMsg,0,osWaitForever);   //send command
          
          for (i = 0 ; i < 2; i++)
          {
            infoToSend.mensagem[i] = infoString[i];
          }
          
          osMessageQueuePut(mid_InfoQueue,&infoToSend,0,0);   //send info
          
        }
        else
        {
          infoString[0] = '0';   //goFlag = 0
          infoString[1] = currentFloorStr[0];
          infoString[2] = currentFloorStr[1];
          int i;
          
          for (i = 0 ; i < 2; i++)
          {
            infoToSend.mensagem[i] = infoString[i];
          }
          
          osMessageQueuePut(mid_InfoQueue,&infoToSend,0,0);   //send info
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
 
    while (strcmp(initialMessage," ") == 0)
    {
      //wait for initial message
    }
    
    UARTSend("cr\r",strlen("cr\r"));
    
    UARTSend("cf\r",strlen("cf\r"));
    
    threadStatus_id = osThreadNew(threadStatusController, NULL, NULL);
    threadCommand_id = osThreadNew(threadCommandDecoder, NULL, NULL);
    threadOutput_id = osThreadNew(threadOutputController, NULL, NULL);                                  //create and get thread ids
                                                                              
    if (threadStatus_id != NULL && threadCommand_id != NULL && threadOutput_id != NULL)
    {
      if(osKernelGetState() == osKernelReady)
      {
        osKernelStart();   
      }

      while(1);  
    }
  }

}