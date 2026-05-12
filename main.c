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
#include "TAD_ANIMALS.h"
#include "TAD_EEPROM.h"

//Configuramos el Oscilador a 40MHz, el tiempo de instrucción será de 400ns
#pragma config OSC = HSPLL
#pragma config PBADEN = DIG
#pragma config MCLRE = OFF
#pragma config DEBUG = OFF
#pragma config PWRT = OFF
#pragma config BOR = OFF
#pragma config WDT = OFF
#pragma config LVP = OFF

//Cabeceras de las funciones locales
static void __interrupt (high_priority) LaRSI (void);
void main (void);

//Definición de interrupciones de alta prioridad 
static void __interrupt (high_priority) LaRSI (void){
        //Interrupción de RX (Lectura) de la SIO interruptiva
        if ( PIR1bits.RCIF&&PIE1bits.RCIE )
            SIO_InterrupcioRX();
        //Interrupción de TX (Escritura) de la SIO interruptiva
        if ( PIR1bits.TXIF&&PIE1bits.TXIE )
            SIO_InterrupcioTX();
        //Interrupción del Timer, cada 1ms con FOsc de 40MHz
        if ( TMR0IF&&TMR0IE )
            RSI_Timer0();
    }

//Función principal del programa
void main(void) {
    //Funciones de inicialización
    TI_Init();
    SIO_Init();
    ADC_Init();
    JSK_Init();
    BTN_Init();
    HB_Init();
    ANIMALS_Init();
    TIME_Init();
    IFC_Init();
    LcInit(2, 16); //Inicializamos el LCD con 2 columnas y 16 filas
    
    //Motores de los TADs que deberán ejecutarse continuamente
	while(1){
        HB_Motor();
        JSK_Motor();
        BTN_Motor();
        IFC_Motor();
        TIME_Motor();
        ANIMALS_Motor();
        CLOCK_Motor(); //2do motor de la SIOTime, lleva el contaje de la hora
        LCD_Motor();
        MSG_Motor(); //3er motor de la SIOTime, lleva la escritura de las respuestas
        EE_Motor();
	}
	return;
}