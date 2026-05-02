#include <xc.h>
#include "TAD_ADC.h"
#include "TAD_JSK.h"
#include "TAD_SIOInt.h"

#define ACT_HIGH 150
#define ACT_LOW  100

static unsigned char ligth;

void JSK_Init(){
    TRISAbits.RA0 = 1; //AN0 - Eje X
    TRISAbits.RA1 = 1; //AN1 - Eje Y
    ligth = 0;
}

char JSK_getLight(){
    return ligth;
}

void PutLight(char mostra){
    ligth = mostra;
}

void CheckMove(char mostra, char i) {
    static char estado_anterior[2] = {0, 0}; 
    
    if (estado_anterior[i] == 0) {
        if (mostra > ACT_HIGH) {
            estado_anterior[i] = 1;
            SIO_PutChar( (i == 0) ? 'L' : 'U' );
            SIO_PutChar('\r');
            SIO_PutChar('\n');
        } 
        else if (mostra < ACT_LOW) {
            estado_anterior[i] = 2;
            SIO_PutChar( (i == 0) ? 'R' : 'D' );
            SIO_PutChar('\r');
            SIO_PutChar('\n');
        }
    } 
    else if (estado_anterior[i] == 1) {
        if (mostra < ACT_HIGH) {
            estado_anterior[i] = 0;
        }
    } 
    else if (estado_anterior[i] == 2) {
        if (mostra > ACT_LOW) {
            estado_anterior[i] = 0;
        }
    }
}

void JSK_Motor(void) {
    static char state = 0;

    switch(state) {
        case 0: //Eje X
            ADC_PickChannel(0);
            ADC_IniciaConversio();
            state = 1;
            break;

        case 1:
            if (ADC_HiHaMostra()) {
                CheckMove(ADC_GetMostra(), 0);
                state = 2;
            }
            break;

        case 2: //Eje Y
            ADC_PickChannel(1);
            ADC_IniciaConversio();
            state = 3;
            break;

        case 3:
            if (ADC_HiHaMostra()) {
                CheckMove(ADC_GetMostra(), 1);
                state = 4;
            }
            break;
            
        case 4: //Light
            ADC_PickChannel(2);
            ADC_IniciaConversio();
            state = 5;
            break;

        case 5:
            if (ADC_HiHaMostra()) {
                PutLight(ADC_GetMostra());
                state = 0;
            }
            break;
    }
}