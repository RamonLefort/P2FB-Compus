#ifndef TAD_HB_H
#define	TAD_HB_H

void HB_Init();
//Pre: -
//Post: Inicializa el LED HeartBeat

void HB_Motor();
//Pre: -
//Post: Motor del HeartBeat, se encarga de ir ajustando la luminosidad mediante un PWM

void HB_setRebellion(unsigned char value);
//Pre: 0 <= value <= 1
//Post: Indica el inicio o finalización una rebelión

#endif	/* TAD_HB_H */
