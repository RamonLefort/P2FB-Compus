#ifndef TAD_SIOINT_H
#define	TAD_SIOINT_H

void SIO_Init(void);

void SIO_InterrupcioRX(void);

unsigned char SIO_RXAvail();

unsigned char SIO_GetChar();

void SIO_End();

void SIO_InterrupcioTX (void) ;
// Pre: Es crida des de la RSI PIR1bits.TXIF==1 i PIE1bits.TXIE==1.
// Post: Processa la interrupció a la TX d'un char.

unsigned char SIO_TXAvail(void);
//Post: Retorna el nombre de caràcters disponibles a la cua de TX. 0 si no hi ha espai, i fins a MAX_TX si està buida.

void SIO_PutChar (unsigned char Valor);
// Pre: SIO_TXAvail() ha retornat CERT.
// Post: Posa un nou caràcter a enviament.

void SIO_PutString (unsigned char *LaFrase);
// Pre: SIO_TXAvail() ha retornat >= strlen (LaFrase).
// Post: Posa la frase a la cua d'enviament.

#endif	/* TAD_SIOINT_H */