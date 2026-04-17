#include <xc.h>

void ADC_Init(char quinCanal){
    TRISAbits.RA0 = 1;
    ADCON0 = (quinCanal << 2) & 0xFD;
    ADCON1 = 0x0E;
    ADCON2 = 0x00;
}

void ADC_IniciaConversio(void){
    ADCON0bits.GO_DONE = 1;
}

char ADC_HiHaMostra(void){
    return ADCON0bits.GO_DONE;
}

int ADC_GetMostra(void){
    return (ADRESH * 256) + ADRESL;
}