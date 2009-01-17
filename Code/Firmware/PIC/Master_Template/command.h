#ifndef COMMAND_H
#define COMMAND_H

unsigned char checkCommand(unsigned char c);
void processCommandString(unsigned int nb, unsigned char *commandArr);
void ASCII2BYTE(unsigned char b, unsigned char *numberArr);
unsigned char getMenuReply(unsigned char i);
void echoByteValue(unsigned char c);
void busStartWrite(void);
void busStopWrite(void);
void busWriteByte(unsigned char c);
void busReadByte(void);
void busReadBulk(unsigned char c);
void setBusMode(void);

#endif
