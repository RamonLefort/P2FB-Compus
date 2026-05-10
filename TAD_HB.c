#include <xc.h>
#include "TAD_TIMER.h"

#define PORT LATAbits.LA4
#define PERIODO 20
#define VELOCIDAD 3

static unsigned char TimerHB; 
static unsigned char THigh = 0;
static unsigned char AguantaDC = 0;
static unsigned char pujant = 1, rebellion;

void HB_Init(void) {
    TRISAbits.RA4 = 0;
    PORT = 0;
    TI_NewTimer(&TimerHB);
    rebellion = 0;
}

void HB_setRebellion(unsigned char value){
    rebellion = value;
}

void HB_Motor(void) {
    unsigned char tics = TI_GetTics(TimerHB);
    
    if(rebellion == 1){
        PORT = 0;
        return;
    }

    // Control del PWM: Si el tiempo actual es menor que el Duty Cycle, encendemos.
    if (tics < THigh) {
        PORT = 1;
    } else {
        PORT = 0;
    }

    // Control del Periodo
    if (tics >= PERIODO) {
        TI_ResetTics(TimerHB);
        
        // Control de la velocidad del fading
        if (++AguantaDC >= VELOCIDAD) {
            AguantaDC = 0;
            
            // Lógica de actualización de THigh (Fading)
            if (pujant) {
                if (++THigh >= PERIODO){
                    pujant--;
                }
            } else {
                if (--THigh == 0){
                    pujant++;
                }
            }
        }
    }
}