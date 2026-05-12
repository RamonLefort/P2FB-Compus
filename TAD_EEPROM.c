#include <xc.h>
#include "TAD_EEPROM.h"
#include "TAD_ANIMALS.h"
#include "TAD_SIOInt.h"

static char ee_state = 0;
static unsigned char ee_addr = 0;
static unsigned char *byte_ptr;
static unsigned int bytes_left = 0;

void EE_Reset(void) {
    if (ee_state == 0){
        ee_state = 1;
    }
}

void EE_WriteAll(void) {
    if (ee_state == 0){
        ee_state = 2;
    }
}

void EE_ReadAll(void) {
    if (ee_state == 0){
        ee_state = 4;
    }
}

void EE_Motor(void) {
    static char i = 0;
    //Miramos si la EEPROM esté libre
    if (EE_IsWriting()){
        return;
    }
    switch(ee_state) {
        //Estado de espera del motor de la EEPROM
        case 0:
            break;
        case 1: //Estado de reseteo del sistema
            //Reseteamos la granja poniendo todo el contenido de la EEPROM a 0 
            EE_Write(i, 0);
            //Reiniciamos las variables del sistema una vez acabamos de escribir 0 en la EEPROM
            if(++i == 0) {
                ANIMALS_Init();
                ee_state = 0;
            }
            break;
        case 2: //Estado de escritura del sistema
            //Escribimos el total de animales en la primera dirección de la EEPROM
            EE_Write(0x00, ANIMALS_GetTotalAnimals());
            //Reiniciamos las variables de la EEPROM antes de escribir el array de animales
            byte_ptr = (unsigned char*)ANIMALS_GetAnimals();
            bytes_left = ANIMALS_GetTotalAnimals() * sizeof(Animals);
            ee_addr = 0x01;
            ee_state = 3;
            break;

        case 3:
            //Escribimos el array de animales en el resto de direcciones de la EEPROM
            if (bytes_left > 0) {
                EE_Write(ee_addr++, *byte_ptr++);
                bytes_left--;
            } else {
                ee_state = 0;
            }
            break;
        case 4: //Estado de lectura del sistema
            //Leemos la variable total_animals y la guardamos en el TAD_ANIMALS
            unsigned char num = EE_Read(0x00);
            if (num > 24){
                num = 0;
            }
            ANIMALS_setTotalAnimals(num);
            //Reiniciamos las variables de la EEPROM antes de leer el array de animales
            byte_ptr = (unsigned char*)ANIMALS_GetAnimals();
            bytes_left = num * sizeof(Animals);
            ee_addr = 0x01;
            ee_state = 5;
            break;

        case 5:
            //Leemos el array de animales de la EEPROM y lo guardamos en el TAD_ANIMALS
            if (bytes_left > 0) {
                *byte_ptr++ = EE_Read(ee_addr++);
                bytes_left--;
            } else {
                //Activamos la reconstrucción del sistema en el TAD_ANIMALS
                ANIMALS_Rebuild(); 
                ee_state = 0; 
            }
            break;
    }
}

//Comando de escritura de la EEPROM
void EE_Write(char address, char data){
    EEADR = address;
    EEDATA = data;
    EECON1bits.EEPGD = 0;
    EECON1bits.CFGS = 0;
    EECON1bits.WREN = 1;
    di();
    EECON2 = 0x55;
    EECON2 = 0xAA;
    EECON1bits.WR = 1;
    ei();
}

//Devuelve 1 si está escribiendo
char EE_IsWriting(void){
    return EECON1bits.WR;
}

//Comando de lectura de la EEPROM
unsigned char EE_Read(char address){
    EEADR = address;
    EECON1bits.EEPGD = 0;
    EECON1bits.CFGS = 0;
    EECON1bits.RD = 1;
    return EEDATA;
}