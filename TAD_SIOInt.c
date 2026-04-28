#include <xc.h>
#include "TAD_SIOInt.h"

// Definicions privades del TAD
#define CONFIGURACIO_TXSTA 0x24     // Dades de 8 bits, TX Enable, Asíncron, BRGH=1
#define CONFIGURACIO_RCSTA 0x90     // Enable canal serie, 8 bits, asíncron, RX enable
#define DIVISOR_BAUDRATE 255

// El TAD dimensiona una cua de recepció de valors de 32 chars.
#define MAX_RX 32

// La màscara per al final de la cua. Ull, si la mida no és potència de 2 no funcionarà!
#define MASK_RX 0x1F

// Variables locals, cua de RX
static unsigned char CuaRX [MAX_RX] ;
static unsigned char IniciRX, FiRX, QuantsRX;
static unsigned char CuaTX [MAX_RX] ;
static unsigned char IniciTX, FiTX, QuantsTX;

// Constructor del TAD
void SIO_Init(void){
    IniciRX = FiRX = QuantsRX = 0;
    TRISCbits.TRISC6 = 1;
    TRISCbits.TRISC7 = 1;
    
    BAUDCONbits.BRG16 = 0; 
    
    // 1. Cargar el divisor primero
    SPBRG = DIVISOR_BAUDRATE;
    
    // 2. Configurar modo y habilitar
    TXSTA = CONFIGURACIO_TXSTA; // BRGH=1, TXEN=1
    RCSTA = CONFIGURACIO_RCSTA; // SPEN=1, CREN=1
    
    // 3. Interrupciones
    PIE1bits.RCIE = 1;
    PIE1bits.TXIE = 1; // RECOMENDACIÓN: Mantén TXIE=0 hasta que tengas algo que enviar en SIO_PutChar
}

void SIO_InterrupcioRX (void) {
    // Esborrem el IF de recepció.
    PIR1bits.RCIF=0;
    // Llegim el caràcter rebut i el posem a la cua.
    CuaRX[IniciRX++]=RCREG;
    IniciRX&=MASK_RX;
    QuantsRX++;
}

unsigned char SIO_RXAvail() {
    return QuantsRX;
} 

unsigned char SIO_GetChar() {
    unsigned char ElValor;
    di();
    
    ElValor = CuaRX[FiRX++];
    FiRX &= MASK_RX;
    QuantsRX--;
    
    ei();
    return ElValor;
}


void SIO_InterrupcioTX (void) {
    // Si queda alguna cosa en cua, posem el següent
    if (QuantsTX!=0) {

        TXREG = CuaTX[FiTX++];
        FiTX &= MASK_RX;
        QuantsTX--;

    }
}


unsigned char SIO_TXAvail(void){
    //Post: Retorna CERT si hi ha espai a la cua d'enviament. FALS en cas contrari.
    return (MAX_RX-QuantsTX);
    }

	
void SIO_PutChar (unsigned char ElValor){
    // Pre: SIO_TXAvail() ha retornat CERT.
    // Post: Posa un nou caràcter a enviament.
    if ( TXIF==1 )
        TXREG=ElValor;
    else {
        di();

       CuaTX[IniciTX++] = ElValor;
       IniciTX &= MASK_RX;
       QuantsTX++;
        
        ei();
    }
}


void SIO_PutString (unsigned char *LaFrase) {
// Pre: SIO_TXAvail() ha retornat >= strlen (LaFrase).
// Post: Posa la frase a la cua d'enviament.
    unsigned char Index=0;
    while (LaFrase[Index]!=0x00)
        SIO_PutChar(LaFrase[Index++]);
}

// Destructor del TAD
void SIO_End (){
//  No fa res, realment...
}