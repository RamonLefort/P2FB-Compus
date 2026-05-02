#include <xc.h>
#include "TAD_LIGHT.h"
#include "TAD_SIOInt.h"
#include "TAD_ADC.h"
#include "TAD_TIMER.h"
#include "TAD_JSK.h"

#define LIGHT_MIN_VALUE 300    // ajustar segun tu LDR

static unsigned char light_active = 0;
static unsigned int light_counter = 0;
static unsigned int light_value = 0;
static unsigned char Timer;

void LIGHT_Init(void){
    // al ser analógico no se hasta que punto este TAD es válido
    TRISAbits.RA3 = 1; //AN2 - Light
}

void LIGHT_CheckLight(){
    light_active = 0;
    TI_NewTimer(&Timer);
}

void LIGHT_Motor(void){
    static unsigned char light_state;

    switch(light_state){
        case 0:
            light_state = 1;
            TI_ResetTics(Timer);
            SIO_PutChar('\r');
            SIO_PutChar('\n');
            break;
        case 1:
            light_value = JSK_getLight();
            if(light_value < LIGHT_MIN_VALUE){
                // oscuridad detectada -> animal duerme bien
                //SIO_sendCommand(IDX_CMD_SLEEP_SUCCESSFUL);
                SIO_PutChar('Y');
                light_active = 1;
                light_state = 0;
            }else{
                if(TI_GetTics(Timer) >= 5000){ // 5000 * 1ms = 5s
                    // no se ha tapado a tiempo
                    //SIO_sendCommand(IDX_CMD_SLEEP_UNSUCCESSFUL);
                    SIO_PutChar('N');
                    light_active = 0;
                    light_state = 0;
                }
            }
            break;
    }
}