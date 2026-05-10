#include <xc.h>

void ADC_Init (void){
    ADCON0 = 0x01;  //canal AN0
    ADCON1 = 0x09;  //Reads AN0, AN1 and AN2 Analog
    ADCON2 = 0x16; //LEFT
    //Debido a que vamos a 40MHz hemos tenido que bajar la velocidad de 
    //adquisición y de conversión ya que sino nos daban valores aleatorios
}

void ADC_PickChannel(unsigned char i) {
    ADCON0 = (unsigned char)((ADCON0 & 0xC3) | (i << 2));
}

void ADC_IniciaConversio(void){
    ADCON0bits.GO_DONE = 1;
}

char ADC_GetMostra(void){
    return ADRESH;
}

char ADC_HiHaMostra(void){
    return !ADCON0bits.GO_DONE;
}