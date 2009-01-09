#ifndef BM_UART_H
#define BM_UART_H

void InitializeUART(void);

// Hardware Functions
unsigned char UARTRXRdy(void);
unsigned char UARTRX(void);
void UARTTX(char c);

// Helper Functions
void UARTTXString(char *s);
void UARTTXBin(unsigned char c);
void UARTTXDec(unsigned char c);
void UARTTXHex(unsigned int c);

#endif