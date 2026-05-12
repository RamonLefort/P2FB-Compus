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
    INTCON2bits.RBPU = 0; //Activamos los pull-ups internos
    TI_NewTimer(&TimRebots);
    ButtonState = 0;
}

void BTN_Motor(void) {
    static unsigned char state = 0;
    
    switch(state) {
        case 0: //Esperamos a que se pulse el botón
            if (BUTTON == PRESSED) {
                //Miramos los rebotes
                if (TI_GetTics(TimRebots) >= TREBOTS) {
                    ButtonState = ON;
                    //Mandamos la letra 'S' por terminal
                    SIO_PutChar('S');
                    SIO_PutChar('\r');
                    SIO_PutChar('\n');
                    TI_ResetTics(TimRebots);
                    state++;
                }
            } else {
                //Reseteamos el timer para ahorrarnos estados
                TI_ResetTics(TimRebots);
            }
            break;

        case 1: //Esperamos a que se suelte el pulsador
            if (BUTTON != PRESSED) {
                if (TI_GetTics(TimRebots) >= TREBOTS) {
                    ButtonState = OFF;
                    TI_ResetTics(TimRebots);
                    state--;
                }
            } else {
               //Reseteamos el timer para ahorrarnos estados
                TI_ResetTics(TimRebots);
            }
            break;
    }
}