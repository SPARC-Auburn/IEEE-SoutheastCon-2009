#ifndef SERIAL_H
#define SERIAL_H

char RXReady(void);
char RXChar(void);
void TXChar(char c);
void TXString(const rom char *s);
void TXBin(unsigned char c);
void TXDec(unsigned char c);
void TXHex(unsigned int c);

#endif
