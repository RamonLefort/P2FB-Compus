#ifndef TAD_EEPROM_H
#define	TAD_EEPROM_H

#include <xc.h>

void EE_Init(void);
//Pre: -
//Post: Inicializa la EEPROM

void EE_Write(char address, char data);
//Pre: 0 < address < 255
//Post: Escribe el valor de la dirección de 'address' en la EEPROM

char EE_IsWriting(void);
//Pre: -
//Post: Devuelve si la EEPROM esta realizando una lectura actualmente

unsigned char EE_Read(char address);
//Pre: 0 < address < 255
//Post: Lee el valor de la dirección de 'address' de la EEPROM

void EE_Motor(void);
//Pre: -
//Post: Motor de la EEPROM, se encarga de leer, escribir y resetear el contenido de la EEPROM

void EE_ReadAll(void);
//Pre: -
//Post: Activa la lectura del contenido de la EEPROM

void EE_WriteAll(void);
//Pre: -
//Post: Activa la escritura del contenido en la EEPROM

void EE_Reset(void);
//Pre: -
//Post: Activa el reinicio del contenido de la EEPROM

#endif
