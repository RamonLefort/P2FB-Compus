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

void BTN_Motor(void){
	static char state = 0;

	switch(state) {
		case 0:
			if (BUTTON == PRESSED) {
				TI_ResetTics(TimRebots);
				state = 1;
			}
		break;
		case 1:
			if (TI_GetTics(TimRebots) >= TREBOTS && BUTTON == PRESSED) {
				ButtonState = ON;
                SIO_PutString("OK");
				state = 2;
			}
			else if (TI_GetTics(TimRebots) >= TREBOTS && BUTTON != PRESSED) {
				state = 0;
			}
		break;
		case 2:
			if (BUTTON != PRESSED) {
				TI_ResetTics(TimRebots);
				state = 3;
			}
		break;
		case 3:
			if (TI_GetTics(TimRebots) >= TREBOTS && BUTTON == PRESSED) {
				state = 2;
			}
			else if (TI_GetTics(TimRebots) >= TREBOTS && BUTTON != PRESSED) {
				ButtonState = OFF;
				state = 0;
			}
		break;
	}
}