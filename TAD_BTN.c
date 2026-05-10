#include <xc.h>
#include "TAD_TIMER.h"
#include "TAD_SIOInt.h"

#define BUTTON PORTBbits.RB2
#define TREBOTS 15
#define PRESSED 1
#define ON 0
#define OFF 1

unsigned char TimRebots;
char ButtonState;

void BTN_Init(){
    TRISBbits.RB2 = 1; //SW - Botón del JoyStick
    INTCON2bits.RBPU = 0;
    TI_NewTimer(&TimRebots);
    ButtonState = 0;
}

void BTN_Motor(void) {
    static unsigned char state = 0;
    
    switch(state) {
        case 0: // Esperando pulsación
            if (BUTTON == PRESSED) {
                if (TI_GetTics(TimRebots) >= TREBOTS) {
                    ButtonState = ON;
                    SIO_PutChar('S');
                    SIO_PutChar('\r');
                    SIO_PutChar('\n');
                    TI_ResetTics(TimRebots); // Reset para el siguiente cambio
                    state++;
                }
            } else {
                TI_ResetTics(TimRebots); // Si se suelta, reseteamos
            }
            break;

        case 1: // Esperando liberación
            if (BUTTON != PRESSED) {
                if (TI_GetTics(TimRebots) >= TREBOTS) {
                    ButtonState = OFF;
                    TI_ResetTics(TimRebots);
                    state--;
                }
            } else {
                TI_ResetTics(TimRebots); // Si se vuelve a pulsar, reseteamos
            }
            break;
    }
}