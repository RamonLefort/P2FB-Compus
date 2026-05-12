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

//Aunque le falten los estados y no tenga mucha pinta de motor realmente lo és, pero debido a la memoria flash tiene este aspecto
//Como está representado en la memoria el primer estado es cuando se encuentra en rebelión y el segundo cuando no lo está
void HB_Motor(void) {
    unsigned char tics = TI_GetTics(TimerHB);
    
    //Este sería el 1er estado
    //Si la rebelión está activada ponemos el LED a 0, sin vida... :(
    if(rebellion == 1){
        PORT = 0;
        return;
    }

    //Este sería el 2do estado
    //Si el tiempo actual es menor que el Duty Cycle encendemos el LED
    if (tics < THigh) {
        PORT = 1;
    } else {
        PORT = 0;
    }

    //Miramos si los tics son mayores al periodo
    if (tics >= PERIODO) {
        TI_ResetTics(TimerHB);
        
        //Miramos si la velocidad es la que buscamos
        if (++AguantaDC >= VELOCIDAD) {
            AguantaDC = 0;
            
            //Actualizamos el Tiempo en 1 del periodo
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