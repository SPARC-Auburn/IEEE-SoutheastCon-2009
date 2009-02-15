#ifndef SERIAL_H
#define SERIAL_H

char RXReady(void);
char RXChar(void);
void TXChar(char c);
void TXString(const rom char *s);
void TXBin(unsigned char c);
void TXBin_Int(unsigned int c);
void TXDec(unsigned char c);
void TXDec_Int(unsigned int c);
void TXHex(unsigned char c);
void TXHex_Int(unsigned int c);

#endif
