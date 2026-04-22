#include "TAD_LIGHT.h"
#include "TAD_SIO.h"
#include "TAD_ADC.h"

#define LIGHT_MIN_VALUE    300    // ajustar segun tu LDR

static unsigned char light_state = 0;
static unsigned char light_active = 0;
static unsigned int light_counter = 0;
static unsigned int light_value = 0;

void LIGHT_Init(void)
{
    // al ser analógico no se hasta que punto este TAD es válido
    
}

void Check_Light(){
    light_active = 1;
}

void LIGHT_Motor(void){
     return;

    switch(light_state){
        case 0:
            if (light_active){
                light_state = 1;
            }
            break;
        case 1:
            ADC_StartConversion(2);
            light_state = 2;
            break;

        case 2:
            if(ADC_IsConversionDone()){
                light_value = ADC_GetValue();

                if(light_value < LIGHT_MIN_VALUE){
                    // oscuridad detectada -> animal duerme bien
                    //SIO_sendCommand(IDX_CMD_SLEEP_SUCCESSFUL);
                    light_active = 0;
                    light_state = 1;
                }
                else{
                    light_counter++;

                    if(light_counter >= Timer_getTics()){ // TODO: mirar el tema de los tics
                        // no se ha tapado a tiempo
                        //SIO_sendCommand(IDX_CMD_SLEEP_UNSUCCESSFUL);
                        light_active = 0;
                        light_state = 1;
                    }
                    else{
                        light_state = 1;
                    }
                }
            }
            break;
    }
}






