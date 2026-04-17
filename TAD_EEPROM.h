#ifndef TAD_EEPROM_H
#define	TAD_EEPROM_H

#include <xc.h>

void EE_Init(void);

void EE_Write(char address, char data);

char EE_IsWriting(void);

void EE_Read(char address);

#endif