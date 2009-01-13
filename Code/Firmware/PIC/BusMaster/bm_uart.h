#ifndef BM_UART_H
#define BM_UART_H

void InitializeUART(void);

// Helper Functions
void UARTTXString(char *s);
void UARTTXBin(unsigned char c);
void UARTTXDec(unsigned char c);
void UARTTXHex(unsigned int c);

#endif