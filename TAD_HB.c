#include <xc.h>
#include "TAD_TIMER.h"

#define PORT LATAbits.LA4
#define PERIODO 20
#define VELOCIDAD 3

unsigned char Timer1;
char AguantaDC, THigh, pujant;

void HB_Init(){
    TRISAbits.RA4 = 0;
    PORT = 0;
    TI_NewTimer(&Timer1);
    THigh = AguantaDC = 0;
    pujant = 1;
}

void ActualitzaTHigh(char *THigh){
    if(pujant == 1){
        *THigh = *THigh + 1;
        if(*THigh == PERIODO){
            pujant = 0;
        }
    }else{
        *THigh = *THigh - 1;
        if(*THigh == 0){
            pujant = 1;
        }
    }
}

void HB_Motor(void) {
	static char state = 0;

	switch(state) {
		case 0:
			if(TI_GetTics(Timer1) >= THigh){
				PORT = 0;
				state = 1;
			}
		break;
		case 1:
			if (TI_GetTics(Timer1) >= PERIODO) {
				PORT = 1;
				TI_ResetTics(Timer1);
				AguantaDC++;
				state = 2;
			}
		break;
		case 2:
			if (AguantaDC != VELOCIDAD) {
				state = 0;
			}
			else if (AguantaDC == VELOCIDAD) {
				AguantaDC = 0;
				ActualitzaTHigh(&THigh);
				state = 0;
			}
		break;
	}
}