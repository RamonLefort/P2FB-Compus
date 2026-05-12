#include <xc.h>
#include "TAD_ADC.h"
#include "TAD_JSK.h"
#include "TAD_SIOInt.h"

#define ACT_HIGH 200
#define ACT_LOW 50
#define LIGHT_MIN_VALUE 5

static unsigned char ligth;
static unsigned char canal_actual = 0; // 0 = X, 1 = Y, 2 = Light
static unsigned char joy_lock = 0;

void JSK_Init(){
    TRISAbits.RA0 = 1; //AN0 - Eje X
    TRISAbits.RA1 = 1; //AN1 - Eje Y
    TRISAbits.RA2 = 1; //AN2 - Light
    ligth = 0;
}

char JSK_getLight(){
    return ligth;
}

void CheckMove(unsigned char mostra, unsigned char i) {
    unsigned char mask = (1 << i);

    //Bloqueamos la doble escritura de un mismo sentido por la SIO
    if(!(joy_lock & mask)){
        char c = 0;

        if(mostra > ACT_HIGH){
            //Si i = 0: Eje X -> 'L', si i = 1: Eje Y -> 'U'
            c = (i == 0) ? 'L' : 'U';
        }else if(mostra < ACT_LOW){
            // Si i = 0: Eje X -> 'R', si i = 1: Eje Y -> 'D'
            c = (i == 0) ? 'R' : 'D';
        }

        //Mandamos el carácter por la SIO
        if (c != 0) {
            joy_lock |= mask; 
            SIO_PutChar(c);
            SIO_PutChar('\r');
            SIO_PutChar('\n');
        }
    }else if(mostra < ACT_HIGH && mostra > ACT_LOW){
        //Desbloqueamos la SIO
        joy_lock &= ~mask;
    }
}

//Aunque no hagamos uso de un switch realmente es una máquina de estados de 2 estados
//Si la variable wait_adc = 0 estamos en el estado 0, sino estamos en el estado 1
void JSK_Motor(void){
    static unsigned char wait_adc = 0;

    if(!wait_adc){
        //Escogemos el canal actual por el que convertiremos el valor e iniciamos la conversión
        ADC_PickChannel(canal_actual);
        ADC_IniciaConversio();
        wait_adc++;
    }else{
        if (ADC_HiHaMostra()) {
            if (canal_actual < 2) {
                //Miramos si ha habido movimiento
                CheckMove(ADC_GetMostra(), canal_actual);
            } else {
                //Actualizamos la variable de luz del sistema
                if(ADC_GetMostra() < LIGHT_MIN_VALUE){
                    //Está oscuro
                    ligth = 1;
                }else{
                    //No está oscuro
                    ligth = 0;
                }
            }
            
            if(++canal_actual > 2){
                canal_actual = 0;
            }
            wait_adc--;
        }
    }
}