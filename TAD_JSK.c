#include <xc.h>
#include "TAD_ADC.h"
#include "TAD_JSK.h"
#include "TAD_SIOInt.h"

#define ACT_HIGH 200
#define ACT_LOW 50
#define LIGHT_MIN_VALUE 5

static unsigned char ligth;
static unsigned char canal_actual = 0; // 0:X, 1:Y, 2:Light
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

    // Lógica de Bloqueo (Flanco) para no saturar la UART
    if (!(joy_lock & mask)) {
        char c = 0;

        if (mostra > ACT_HIGH) {
            // Si i=0 (Eje X) -> 'L', si i=1 (Eje Y) -> 'U'
            c = (i == 0) ? 'L' : 'U';
        } 
        else if (mostra < ACT_LOW) {
            // Si i=0 (Eje X) -> 'R', si i=1 (Eje Y) -> 'D'
            c = (i == 0) ? 'R' : 'D';
        }

        if (c != 0) {
            joy_lock |= mask; 
            SIO_PutChar(c);
            SIO_PutChar('\r');
            SIO_PutChar('\n');
        }
    } 
    // Lógica de Desbloqueo (Vuelta al centro)
    else if (mostra < ACT_HIGH && mostra > ACT_LOW) {
        joy_lock &= ~mask;
    }
}

void JSK_Motor(void) {
    static unsigned char wait_adc = 0;

    if (!wait_adc) {
        ADC_PickChannel(canal_actual);
        ADC_IniciaConversio();
        wait_adc++;
    } else {
        if (ADC_HiHaMostra()) {
            if (canal_actual < 2) {
                // Procesar JoyStick (Ejes 0 y 1)
                CheckMove(ADC_GetMostra(), canal_actual);
            } else {
                // Guardar Luz (Canal 2)
                if(ADC_GetMostra() < LIGHT_MIN_VALUE){
                    // oscuridad detectada -> animal duerme bien
                    //SIO_sendCommand(IDX_CMD_SLEEP_SUCCESSFUL);
                    ligth = 1;
                }else{
                    ligth = 0;
                }
            }
            
            // Ciclar canales: 0 -> 1 -> 2 -> 0
            if (++canal_actual > 2) canal_actual = 0;
            wait_adc--;
        }
    }
}