#ifndef TAD_EEPROM_H
#define	TAD_EEPROM_H

#include <xc.h>

void EE_Init(void);

void EE_Write(char address, char data);

char EE_IsWriting(void);

unsigned char EE_Read(char address);

void EEMOTOR_Task(void);

void EEMOTOR_TriggerReadAll(void);

void EEMOTOR_TriggerWriteAll(void);

void EEMOTOR_TriggerReset(void);

#endif