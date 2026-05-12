#include <xc.h>

void ADC_Init (void){
    ADCON0 = 0x01;  //Canal AN0
    ADCON1 = 0x09;  //AN0, AN1 y AN2 Analogos
    ADCON2 = 0x16; //Justificiación a la izquierda, le bajamos la velocidad de adquisición de valores
    //Debido a que vamos a 40MHz hemos tenido que bajar la velocidad de 
    //adquisición y de conversión ya que sino nos daban valores aleatorios
}

void ADC_PickChannel(unsigned char i) {
    //Aplica una máscara para sincronizar los valores, la seleccion del canal es del ADCON0[2..5]
    //También mueve 2 bits a la izquierda el valor de 'i' para que coja el rango del [2..5]
    ADCON0 = (unsigned char)((ADCON0 & 0xC3) | (i << 2));
}

void ADC_IniciaConversio(void){
    //Activa el bit de GO_DONE para inciar la conversión del ADC
    ADCON0bits.GO_DONE = 1;
}

char ADC_GetMostra(void){
    //Devuelve el valor del resultado del ADC, faltan los 2 bits de menos peso
    //Debido a que no nos pedian mucha exactitud nos hemos decantado la justificación a la izquierda
    return ADRESH;
}

char ADC_HiHaMostra(void){
    //Revisa si la conversión ha acabado mediante el valor del bir GO_DONE
    return !ADCON0bits.GO_DONE;
}