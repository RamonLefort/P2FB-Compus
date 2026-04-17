#include <xc.h>

void EE_Init(void){

}

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

//Retorna 1 si segueix escrivint
char EE_IsWriting(void){
    return EECON1bits.WR;
}

void EE_Read(char address){
    EEADR = address;
    EECON1bits.EEPGD = 0;
    EECON1bits.CFGS = 0;
    EECON1bits.RD = 1;
    WREG = EEDATA;
}