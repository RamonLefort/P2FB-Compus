#include <xc.h>
#include "TAD_SIOInt.h"
#include "TAD_TIMER.h"

#define START_BIT 0
#define IN PORTBbits.RB3
#define OUT LATBbits.LATB4

unsigned char Timer;
char bytesRebuts, paritat, byteRebut;
char byteAEnviar, bitsEnviats, paritatTX;
char tx_busy = 0;

void TIME_Init(){
    TRISBbits.TRISB3 = 1; //RX
    TRISBbits.TRISB4 = 0; //TX
    OUT = 1;
    TRISD = 0x00;
    LATD = 0x80;
    TI_NewTimer(&Timer);
    bytesRebuts = paritat = byteRebut = 0;
}

void TIME_SendChar(char c) {
    if (!tx_busy) {
        byteAEnviar = c;
        tx_busy = 1;
    }
}

char TIME_GetMonth(){
    return 0;
}

char TIME_GetDay(){
    return 0;
}

char TIME_GetHour(){
    return 0;
}

char TIME_GetMinute(){
    return 0;
}

char TIME_GetSecond(){
    return 0;
}

void TIME_Motor(void) {
    static char stateRX = 0;
    static char stateTX = 0;

    //Recepción
    switch(stateRX) {
        case 0:
            if (IN == START_BIT) {
                TI_ResetTics(Timer);
                stateRX = 1;
            }
            break;
        case 1:
            if (TI_GetTics(Timer) >= 1) {
                TI_ResetTics(Timer);
                stateRX = 2;
            }
            break;
        case 2:
            if (TI_GetTics(Timer) >= 2) {
                if (bytesRebuts < 8) {
                    byteRebut = ((byteRebut >> 1) & 0x7F) + (IN == 1 ? 0x80 : 0);
                    bytesRebuts++;
                    paritat += IN;
                    TI_ResetTics(Timer);
                } else {
                    paritat += IN;
                    TI_ResetTics(Timer);
                    stateRX = 3;
                }
            }
            break;
        case 3:
            if (TI_GetTics(Timer) >= 2) {
                LATD = byteRebut;
                TIME_SendChar(byteRebut);
                bytesRebuts = paritat = byteRebut = 0;
                stateRX = 0;
            }
            break;
    }

    //Transmisión
    switch(stateTX) {
        case 0:
            if (tx_busy) {
                OUT = 0;
                bitsEnviats = 0;
                paritatTX = 0;
                TI_ResetTics(Timer);
                stateTX = 1;
            }
            break;
        case 1:
            if (TI_GetTics(Timer) >= 2) {
                OUT = byteAEnviar & 0x01;
                paritatTX += OUT;
                byteAEnviar >>= 1;
                bitsEnviats++;
                TI_ResetTics(Timer);
                if (bitsEnviats >= 8){
                    stateTX = 2;
                }
            }
            break;
        case 2: // STOP BIT
            if (TI_GetTics(Timer) >= 2) {
                OUT = 1; 
                TI_ResetTics(Timer);
                stateTX = 3;
            }
            break;
        case 3:
            if (TI_GetTics(Timer) >= 2) {
                tx_busy = 0;
                stateTX = 0;
            }
            break;
    }
}

void SIO_CLOCK_Motor(){
    static unsigned char clock_state = 0;
    static unsigned char segs = 0;
    static unsigned char mins = 0;
    static unsigned char hours = 0;
    static unsigned char days = 1;
    static unsigned char months = 1;
    static unsigned char years = 0;
    
    
    switch(clock_state){
        case 0:
            if(getTics(clockSioTimer) >= 1000){
                clock_state = 1;
            }
            break;
        case 1:
            if(segs >= 59){
                segs = 0;
                clock_state = 2;
            }
            else{
                segs++;
                clock_state = 0;
            }
            break;
        case 2:
            if(mins >= 59){
                mins = 0;
                clock_state = 3;
            }
            else{
                mins++;
                clock_state = 0;
            }
            break;
        case 3:
            if(hours >= 23){
                hours = 0;
                clock_state = 4;
            }
            else{
                hours++;
                clock_state = 0;
            }
            break;
        case 4:
            if((days >= 28 && months == 2) || (days >= 30 && months % 2 == 0) || (days >= 31 && months % 2 == 1)){
                days = 1;
                clock_state = 5;
            }
            else{
                days++;
                clock_state = 0;
            }
            break;
        case 5:
            if(months >= 12){
                months = 1;
                clock_state = 6;
            }
            else{
                months++;
                clock_state = 0;
            }
            break;
        case 6:
            years++;
            clock_state = 0;
            break; 
    }
    
}
