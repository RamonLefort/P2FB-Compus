#include <xc.h>
#include "TAD_TIMER.h"
#include "TAD_HB.h"
#include "TAD_SIOInt.h"
#include "TAD_JSK.h"
#include "TAD_BTN.h"
#include "TAD_ADC.h"
#include "TAD_IFC.h"
#include "TAD_SIOTime.h"
#include "TAD_LCD.h"

//Configuramos el Oscilador a 40MHz
#pragma config OSC = HSPLL
#pragma config PBADEN = DIG
#pragma config MCLRE = OFF
#pragma config DEBUG = OFF
#pragma config PWRT = OFF
#pragma config BOR = OFF
#pragma config WDT = OFF
#pragma config LVP = OFF

// Capçaleres de les funcions locals
static void __interrupt (high_priority) LaRSI (void);
void main (void);

//Definició de la interrupció d'alta prioritat 
static void __interrupt (high_priority) LaRSI (void){
        if ( PIR1bits.RCIF&&PIE1bits.RCIE )
            SIO_InterrupcioRX();
        if ( PIR1bits.TXIF&&PIE1bits.TXIE )
            SIO_InterrupcioTX();
        if ( TMR0IF&&TMR0IE )
            RSI_Timer0();
    }

void main(void) {
    TI_Init();
    SIO_Init();
    ADC_Init();
    JSK_Init();
    BTN_Init();
    HB_Init();
    TIME_Init();
    LcInit(2, 16);
    LcPutString("Hello");
    LcCursorOff();
	while(1){
        HB_Motor();
        JSK_Motor();
        BTN_Motor();
        IFC_Motor();
        TIME_Motor();
	}
	return;
}