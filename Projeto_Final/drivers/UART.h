#include <stdint.h>

extern void UART_Rx_Handler();

void UARTSend(const uint8_t *Buffer, uint32_t Len);

void setupUART();

void UART_Tx_Handler();
